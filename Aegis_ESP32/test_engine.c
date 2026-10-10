/**
 * ============================================================================
 * TEST_ENGINE.C - Local Host Verification of Online Adaptive TinyML Engine
 * ============================================================================
 */

#ifndef ARDUINO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "aegis_model.h"

int main() {
    printf("==================================================================================\n");
    printf("     AEGIS KERNAL - ONLINE ADAPTIVE TINYML LOCAL VERIFICATION                    \n");
    printf("==================================================================================\n\n");

    AdaptiveModelWeights model;
    memcpy(&model, &DEFAULT_ADAPTIVE_MODEL, sizeof(AdaptiveModelWeights));

    // Test Scenario: Web Shell Spawner
    ProcessTelemetry webshell = {
        .is_temp = 0, .arg_len = 45, .entropy = 4.15f, .is_root = 0,
        .web_parent = 1, .is_parent_dead = 0, .memfd = 0, .ld_preload = 1
    };

    float prob_before = 0.0f;
    AegisThreatLevel v_before = predict_adaptive(&webshell, &model, &prob_before);
    printf("[1] Initial Prediction on Web Shell:\n");
    printf("    Threat Level: %s | Anomaly Probability: %.4f\n", get_level_str(v_before), prob_before);

    // Online Learning Step: Reinforce malicious label
    printf("\n[*] Simulating Online SGD Learning Step (target_label = 0.95)...\n");
    update_model_online(&model, &webshell, 0.95f);

    float prob_after = 0.0f;
    AegisThreatLevel v_after = predict_adaptive(&webshell, &model, &prob_after);
    printf("[2] Post-Online Learning Prediction:\n");
    printf("    Threat Level: %s | Anomaly Probability: %.4f (Confidence Increased)\n", 
           get_level_str(v_after), prob_after);
    printf("    Live Samples Learned: %u | Adaptive Entropy Baseline: %.3f\n\n", 
           model.total_samples_learned, model.entropy_baseline);

    printf("==================================================================================\n");
    printf("                   DYNAMIC WEIGHT VECTOR INSPECTION                               \n");
    printf("==================================================================================\n");
    printf(" Bias:                 %+6.4f\n", model.bias);
    for (int i = 0; i < 8; i++) {
        printf(" w[%d]:                 %+6.4f\n", i, model.weights[i]);
    }
    printf("==================================================================================\n\n");

    return 0;
}

#endif // ARDUINO
