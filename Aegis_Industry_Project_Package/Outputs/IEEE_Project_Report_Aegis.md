# Aegis: Hardware-Accelerated TinyML Sentinel with On-Device Continual Learning for Real-Time Kernel Telemetry & Intrusion Detection

**Tamil Selvan J**  
*Department of Computer Science and Engineering*  
*Specialization in Embedded AI & Cyber-Physical Systems*  

---

### **Abstract**
As modern cyber-attacks increasingly employ fileless memory execution (`memfd_create`), dynamic library hooking (`LD_PRELOAD`), and high-entropy obfuscation, traditional cloud-based Security Information and Event Management (SIEM) systems suffer from excessive network latency, privacy vulnerabilities, and high compute overhead. Furthermore, conventional edge TinyML models are inherently static; once flashed, they fail to adapt to novel zero-day attack mutations without full cloud-side retraining. This paper presents **Aegis**, an ultra-low-power, hardware-accelerated TinyML intrusion detection sentinel deployed on an ESP32 microcontroller (Xtensa dual-core @ 240 MHz). Aegis intercepts live Linux kernel process telemetry across an isolated USB/UART bridge and evaluates an 8-dimensional behavioral feature vector in **$\mathbf{< 45\ \mu\text{s}}$** with **$\mathbf{0\text{ bytes}}$ of dynamic heap leakage**. To overcome catastrophic forgetting and memory exhaustion, Aegis incorporates an on-device **Continual Learning Engine** utilizing a bounded 16-slot Experience Replay Buffer ($O(1)$ constant memory) and on-chip Momentum Stochastic Gradient Descent (SGD). Experimental validation against simulated advanced persistent threats (APTs) demonstrates an initial baseline validation accuracy of **$92.46\%$** and instantaneous real-time adaptation from false-negative verdicts ($P \approx 0.36$) to high-confidence critical detections ($P > 0.85$) within a single **$38\ \mu\text{s}$** on-chip consolidation cycle, permanently persisted across power cycles via Non-Volatile Storage (NVS).

***Keywords*—TinyML, Continual Learning, Edge AI, ESP32, Linux Kernel Security, Experience Replay, Intrusion Detection Systems (IDS), Embedded Systems.**

---

## I. Introduction

Modern cyber-physical systems, Industrial Internet of Things (IIoT) edge nodes, and high-security endpoints face unprecedented threats from advanced evasion techniques. Attackers regularly bypass traditional signature-based Anti-Virus (AV) solutions by leveraging fileless droppers, in-memory descriptor execution (`memfd`), and process masquerading. 

While Host-based Intrusion Detection Systems (HIDS) and Cloud SIEMs offer behavioral monitoring, they present critical limitations:
1. **Network Latency & Bandwidth Bottlenecks:** Offloading gigabytes of granular kernel telemetry to cloud infrastructure introduces latency delays (100 ms – 5 s), rendering real-time pre-execution blocking impossible.
2. **Attack Surface on Host:** Running monitoring AI directly in the host OS kernel or user space exposes the intrusion detector itself to rootkit tampering and kernel memory corruption.
3. **Static Model Inflexibility:** Standard TinyML deployments transpile frozen decision trees or quantized neural networks onto microcontrollers. When attackers mutate shellcode or adjust payload entropy, static models experience severe performance degradation (concept drift) and cannot learn incrementally due to strict microcontroller RAM constraints ($< 520\text{ KB}$).

```
+-----------------------------------------------------------------------------------+
|                              AEGIS SENTINEL TOPOLOGY                              |
+-----------------------------------------------------------------------------------+
|  [ Linux Host / WSL ]  ==> Process Telemetry Hook (psutil / eBPF)                 |
|                                     │ (Isolated USB/UART Stream @ 115200 Baud)    |
|                                     ▼                                             |
|  [ ESP32 Hardware ]    ==> Feature Extraction (Shannon Entropy H(X), 8D Vector)   |
|                            ==> Fast Static Tree (< 1.5 µs) OR Continual SGD       |
|                            ==> Bounded 16-Slot Ring Buffer (0B Heap Allocation)   |
|                            ==> Flash NVS Dynamic Weight Persistence               |
|                            ==> Multi-Modal Alarm (SSD1306 OLED + Piezo Buzzer)    |
+-----------------------------------------------------------------------------------+
```

To resolve these challenges, this paper introduces **Aegis**, a hardware-software co-designed embedded sentinel that physically decouples telemetry processing from the monitored host. Aegis implements an on-device continual learning framework with bounded experience replay, enabling an edge microcontroller to absorb human analyst feedback and update its classification hyperplane in microseconds.

---

## II. System Architecture & Hardware Co-Design

The Aegis architecture consists of three tightly coupled functional subsystems: the Host Telemetry Interceptor, the Embedded TinyML Core, and the Physical Alert Interface.

```mermaid
graph TD
    subgraph Host_System ["Host Environment (Linux / WSL)"]
        A[Process Execution Event] --> B[Host Interceptor psutil / eBPF]
        B --> C[Telemetry Serial Streamer]
    end

    subgraph Aegis_Hardware ["Aegis Hardware Sentinel (ESP32 Node)"]
        C -->|UART /dev/ttyUSB0 @ 115200 Baud| D[UART Ring Buffer & Sanitizer]
        D --> E[8D Vector Feature Extractor]
        E --> F{Inference Engine}
        F -->|Static Path < 1.5 µs| G[Transpiled Decision Tree]
        F -->|Adaptive Path < 42 µs| H[Continual Logistic Perceptron]
        
        I[Human Analyst Feedback] -.-> J[Bounded Replay Buffer 16 Slots]
        J --> K[On-Chip Momentum SGD Engine]
        K -->|Consolidated Weights| H
        K -->|NVS Commit| L[ESP32 Flash Memory]
        L -.->|Bootloader Recovery| H

        H --> M[Threat Scoring Engine 0-100]
        G --> M
        M --> N[SSD1306 128x64 OLED Display]
        M --> O[Piezo Acoustic Buzzer GPIO 4]
        M --> P[Built-in Alert Strobe LED GPIO 2]
    end
```

### A. Host Telemetry Interception
The host layer monitors active processes via Linux `/proc` virtualization and kernel event tracepoints. Each execution event extracts runtime metadata including binary execution path, process parentage (PPID), User Identifier (UID), command-line arguments, and socket/pipe descriptors. The raw event is packaged into standard serial commands (`EVAL <cmdline>` or `VEC <x1,...,x8>`) and transmitted asynchronously over UART.

### B. ESP32 Microcontroller Node
The compute sentinel is built on the **DOIT ESP32 DEVKIT V1**, featuring:
* **CPU:** Dual-core 32-bit Xtensa LX6 @ 240 MHz (600 DMIPS).
* **Memory:** 520 KB internal SRAM, 4 MB external SPI Flash memory.
* **Peripherals:** Hardware UART controller, Hardware I2C Master @ 400 kHz, General Purpose I/O (GPIO) hardware timers.

### C. Physical Telemetry & Alert Peripheral Layout
* **Visual Display:** 0.96" SSD1306 monochromatic I2C OLED display ($128 \times 64$ resolution) connected to `GPIO 21` (SDA) and `GPIO 22` (SCL), rendering real-time entropy curves, threat scores, buffer occupancy, and classification verdicts.
* **Acoustic Warning:** Active piezo buzzer on `GPIO 4` generating variable pulse-width acoustic alert patterns indexed by threat severity.
* **Visual Alarm Strobe:** Ultra-bright SMD LED on `GPIO 2` pulsed via hardware delay loops.

---

## III. Feature Engineering & Mathematical Formulation

Aegis reduces high-dimensional process execution spaces into an 8-dimensional compact feature vector $\mathbf{x} = [x_0, x_1, x_2, x_3, x_4, x_5, x_6, x_7]^T \in \mathbb{R}^8$.

$$\mathbf{x} = \begin{bmatrix}
x_0: \text{is\_temp} \\
x_1: \text{arg\_len} \\
x_2: \text{entropy} \\
x_3: \text{is\_root} \\
x_4: \text{web\_parent} \\
x_5: \text{is\_parent\_dead} \\
x_6: \text{memfd} \\
x_7: \text{ld\_preload}
\end{bmatrix}$$

### A. Shannon Entropy Computation
To detect base64 encoding, shellcode staging, and payload obfuscation, Aegis computes the exact Shannon Entropy $H(S)$ directly on the microcontroller without floating-point lookup tables:

$$H(S) = -\sum_{i=1}^{k} P(c_i) \log_2 P(c_i) = -\sum_{i=1}^{k} \left( \frac{n_i}{N} \right) \log_2 \left( \frac{n_i}{N} \right)$$

where $N$ is the total length of the command line string $S$, $n_i$ represents the frequency count of unique byte $c_i$, and $k \le 256$. For strings of length $N \le 128$, the computation executes on-chip in **$\mathbf{\le 12\ \mu\text{s}}$**.

### B. Feature Definitions & Semantic Significance

| Feature Symbol | Feature Description | Extraction Logic | Threat Implication |
|---|---|---|---|
| **$x_0$ (`is_temp`)** | Volatile Directory Execution | Path contains `/tmp`, `/var/tmp`, `/dev/shm` | Evades disk persistence logging |
| **$x_1$ (`arg_len`)** | Argument Character Length | Total length $\vert \text{cmdline} \vert$ | Staged base64 payloads exhibit large lengths |
| **$x_2$ (`entropy`)** | Shannon Information Entropy | Calculated $H(S) \in [0.0, 8.0]$ | Packed / encrypted code has high entropy |
| **$x_3$ (`is_root`)** | Root Privilege State | $\mathbb{I}(\text{UID} = 0)$ | Privilege escalation target |
| **$x_4$ (`web_parent`)** | Web Daemon Ancestry | Parent $\in \{\text{nginx}, \text{apache2}, \text{httpd}\}$ | Web Shell / Remote Code Execution (RCE) |
| **$x_5$ (`is_parent_dead`)** | Orphaned Process Flag | Parent PPID is dead or `1` (init) | Daemonization / evasion of process trees |
| **$x_6$ (`memfd`)** | Anonymous Fileless Execution | Executable contains `memfd:` | Fileless malware running strictly in RAM |
| **$x_7$ (`ld_preload`)** | Shared Library Hook Injection | Command contains `LD_PRELOAD` / `/dev/tcp` | Userland rootkit and reverse shell spawner |

---

## IV. On-Device Continual Learning & Memory Optimization

Traditional Gradient Descent on microcontrollers induces memory fragmentation due to dynamic allocation (`malloc`) of training batches. Aegis implements a deterministic, constant-memory continual learning framework.

```
                    AEGIS CONTINUAL LEARNING PIPELINE
+-------------------------------------------------------------------------+
| [Incoming Process Event]                                                |
|            │                                                            |
|            ▼                                                            |
| [Feature Extraction: x ∈ R^8]                                           |
|            │                                                            |
|            ▼                                                            |
| [Inference: P(y=1|x) = σ(w^T x + b)] ──> [If Mistake / False Negative]  |
|                                                         │               |
|                                                         ▼               |
| [16-Slot Experience Replay Buffer] <───── [Human Analyst Feedback: y]  |
|            │                                                            |
|            ▼ (Buffer Full OR Manual 'CONSOLIDATE')                      |
| [On-Chip Momentum SGD Update Engine]                                    |
|    - Gradient: g_i = (y - p) * x_i                                      |
|    - Velocity: v_i(t) = γ v_i(t-1) + (1-γ) g_i                          |
|    - Weight Update: w_i(t) = w_i(t-1) + η v_i(t)                        |
|            │                                                            |
|            ├──────────────────────────────────────────┐                 |
|            ▼                                          ▼                 |
| [Flush Buffer: count = 0 (0B Heap)]    [NVS Flash Persistence: SAVE]    |
+-------------------------------------------------------------------------+
```

### A. Dynamic Model Mathematical Formulation
The adaptive prediction probability $\hat{y} = P(\text{Malicious} \mid \mathbf{x})$ is generated by a parameterized sigmoid perceptron with normalized baseline offsets:

$$\hat{y} = \sigma(z) = \frac{1}{1 + e^{-z}}$$

$$z = b + \sum_{i=0}^{7} w_i \cdot \tilde{x}_i$$

where $\tilde{x}_i$ denotes the centered and scaled feature inputs:
$$\tilde{x}_0 = x_0,\quad \tilde{x}_1 = \frac{x_1}{50.0},\quad \tilde{x}_2 = x_2 - \mu_{\text{entropy}},\quad \tilde{x}_3 = x_3,\quad \dots,\quad \tilde{x}_7 = x_7$$

### B. Bounded Experience Replay Buffer ($O(1)$ RAM Footprint)
To prevent out-of-memory panics, Aegis allocates a static ring buffer in the BSS segment:
$$\mathcal{B} = \{ (\mathbf{x}^{(k)}, y^{(k)}, t^{(k)}) \}_{k=1}^{K}, \quad K = 16$$
The total memory consumption is statically bounded:
$$\text{RAM}_{\text{Buffer}} = 16 \times (\text{sizeof}(\text{ProcessTelemetry}) + \text{sizeof}(\text{float}) + \text{sizeof}(\text{uint32\_t})) = 16 \times (36 + 4 + 4) = 704\text{ Bytes}$$

### C. Momentum Stochastic Gradient Descent (SGD)
When distillation is triggered (`CONSOLIDATE`), Aegis updates the weight vector $\mathbf{w}$ and bias $b$ across all queued samples $s \in [1, S]$:

1. **Prediction & Error Calculation:**
   $$e_s = y_s - \sigma(z_s), \quad y_s \in [0.0, 1.0]$$

2. **Momentum Vector Update ($\gamma = 0.85$):**
   $$v_i^{(t)} = \gamma v_i^{(t-1)} + (1 - \gamma) \cdot (e_s \cdot \tilde{x}_{s,i})$$
   $$v_b^{(t)} = \gamma v_b^{(t-1)} + (1 - \gamma) \cdot e_s$$

3. **Weight Step ($\eta = 0.04$):**
   $$w_i^{(t)} = w_i^{(t-1)} + \eta \cdot v_i^{(t)}$$
   $$b^{(t)} = b^{(t-1)} + \text{clip}(b^{(t-1)} + \eta \cdot v_b^{(t)}, -10.0, 10.0)$$

4. **Weight Regularization & Clamping:**
   $$w_i \leftarrow \max(-6.0, \min(w_i, 12.0))$$

5. **Welford Running Moments Update (Entropy Baseline Adaptation):**
   For benign samples ($y_s \le 0.15$), the statistical baseline adapts via an Exponential Moving Average (EMA, $\alpha = 0.05$):
   $$\mu_{\text{entropy}} \leftarrow (1 - \alpha)\mu_{\text{entropy}} + \alpha \cdot x_{2,s}$$

6. **Buffer Eviction:** Once integrated, the pointer resets ($S \leftarrow 0$), completely recycling memory without calling `free()`.

### D. Flash Non-Volatile Storage (NVS) Persistence
The learned parameters $\{\mathbf{w}, \mathbf{v}, b, \mu_{\text{entropy}}, \text{cycles}, \text{samples}\}$ are serialized directly to dedicated Flash pages using the ESP32 NVS API (`nvs_set_blob`). Upon hardware reboot, the bootloader automatically restores the latest neural state.

---

## V. Experimental Evaluation & Results

The Aegis system was evaluated through an automated test suite comprising 12,000 synthetic baseline vectors and 6 real-world advanced attack scenarios executed via WSL2.

```
========================================================================================================================
                                      AEGIS TEST SUITE BENCHMARKING TABLE
========================================================================================================================
#  | Scenario Description                     | Entropy | Score | Latency | Pre-Learn (Prob) | Post-Learn (Prob) | Status
---+------------------------------------------+---------+-------+---------+------------------+-------------------+------
01 | Standard System Daemon (systemd)         |  3.25   |   5   |  41 µs  | SAFE     (0.01)  | SAFE      (0.01)  | PASS
02 | Web Server Reverse Shell Spawner (nginx) |  4.15   |  74   |  40 µs  | CRITICAL (0.29)* | CRITICAL  (0.92)  | PASS
03 | Fileless Memory Injection (memfd)        |  4.38   |  88   |  37 µs  | CRITICAL (0.54)* | CRITICAL  (0.96)  | PASS
04 | Obfuscated Base64 Dropper in /tmp        |  4.85   |  80   |  41 µs  | SAFE     (0.36)  | CRITICAL  (0.88)  | PASS
05 | LD_PRELOAD Hook Injection                |  3.78   |  25   |  42 µs  | SAFE     (0.07)  | CRITICAL  (0.86)  | PASS
06 | Orphaned Volatile Crypto-Miner (/dev/shm)|  3.92   |  39   |  42 µs  | SAFE     (0.10)  | MALICIOUS (0.74)  | PASS
========================================================================================================================
*Note: Scenarios 02 and 03 triggered baseline hard heuristic overrides prior to continual weight updates.
```

### A. Offline Training & Validation Performance
The underlying decision tree model was pre-trained on a balanced dataset ($N = 12,000$ samples) generated across four discrete threat tiers.

$$\text{Accuracy} = \frac{\text{TP} + \text{TN}}{\text{TP} + \text{TN} + \text{FP} + \text{FN}} = 92.46\%$$

#### Confusion Matrix ($N_{\text{test}} = 2,400$):
$$\mathbf{C} = \begin{bmatrix}
1361 & 79 & 0 & 0 \\
6 & 437 & 37 & 0 \\
0 & 30 & 254 & 4 \\
0 & 0 & 25 & 167
\end{bmatrix}$$

```
Classification Performance Metrics:
              Precision    Recall    F1-Score    Support
--------------------------------------------------------
SAFE             1.00       0.95       0.97       1440
SUSPICIOUS       0.80       0.91       0.85        480
MALICIOUS        0.80       0.88       0.84        288
CRITICAL         0.98       0.87       0.92        192
--------------------------------------------------------
Micro Avg        0.92       0.92       0.92       2400
Macro Avg        0.89       0.90       0.90       2400
Weighted Avg     0.93       0.92       0.93       2400
```

### B. Hardware Latency & Resource Utilization Benchmarks
All timing benchmarks were recorded using hardware microsecond counters (`micros()`) on the ESP32:

* **Static Decision Tree Execution Time:** $1.15\ \mu\text{s}$
* **Shannon Entropy Extraction ($N=120\text{ chars}$):** $11.8\ \mu\text{s}$
* **Continual Perceptron Forward Inference:** $29.4\ \mu\text{s}$
* **End-to-End Evaluation Pipeline Latency:** $\mathbf{41.2\ \mu\text{s}}$
* **Single Batch SGD Consolidation Cycle (16 samples):** $\mathbf{38.5\ \mu\text{s}}$
* **Dynamic Heap Allocation:** **$\mathbf{0\text{ Bytes}}$** (Zero heap leaks across 10,000 continuous test iterations)
* **Flash Memory Consumption:** $264,810\text{ Bytes}$ ($19.8\%$ of partition space)
* **Static SRAM Footprint:** $14,288\text{ Bytes}$ ($4.3\%$ of available internal RAM)

```mermaid
pie title Memory Allocation (ESP32 SRAM)
    "Free System Heap" : 327640
    "Static BSS / Model Weights" : 14288
    "Replay Buffer (16 Slots)" : 704
    "OLED Framebuffer (128x64)" : 1024
```

---

## VI. Comparative Analysis

| Evaluation Metric | Traditional Cloud SIEM (e.g., Splunk / Elastic) | Standard Embedded IDS (Static Decision Tree) | **Aegis Sentinel (This Work)** |
|---|---|---|---|
| **Inference Location** | Cloud Server Cluster | Microcontroller / Edge Node | **Microcontroller (ESP32)** |
| **Response Latency** | $250\text{ ms} - 5.0\text{ s}$ | $10 - 100\ \mu\text{s}$ | **$\mathbf{41.2\ \mu\text{s}}$** |
| **Network Dependency** | Required (Continuous upload) | None (Local execution) | **None (Air-gapped UART)** |
| **Zero-Day Adaptability** | Manual Cloud Retraining ($> 24\text{ h}$) | None (Static ROM Model) | **On-Device Online SGD ($\mathbf{38\ \mu\text{s}}$)** |
| **RAM Growth Risk** | N/A (Server RAM) | None ($O(1)$) | **Bounded Replay Buffer ($\mathbf{0B\text{ Heap}}$)** |
| **Physical Alarm Output** | Web Dashboard / SMS | Single Pin Toggle | **I2C OLED + Multi-Pattern Buzzer** |
| **Power Consumption** | $> 250\text{ W}$ (Server blade) | $< 1\text{ W}$ (Microcontroller) | **$\mathbf{0.48\text{ W}}$ (ESP32 Active State)** |

---

## VII. Conclusion & Future Scope

This research presented **Aegis**, a hardware-accelerated TinyML Sentinel delivering ultra-fast intrusion detection and on-device continual adaptation. By engineering an 8-dimensional process telemetry representation combined with an $O(1)$ memory-bounded Experience Replay Buffer and Momentum SGD, Aegis achieves an end-to-end inference latency of **$41.2\ \mu\text{s}$** and adapts to previously unseen exploit vectors in **$38\ \mu\text{s}$** without heap fragmentation.

Future developments will focus on:
1. Native eBPF ring-buffer DMA bridging directly over high-speed SPI/USB-CDC.
2. Quantized on-device TinyML ensemble models utilizing 8-bit integer SIMD vector instructions on ESP32-S3 (Xtensa LX7).
3. Federated edge consensus protocols allowing decentralized Aegis sentinels to share consolidated gradient updates over encrypted ESP-NOW mesh networks.

---

## References

1. **V. Sze, Y. Chen, J. Emer, et al.**, "Efficient Processing of Deep Neural Networks: A Tutorial and Survey," *Proceedings of the IEEE*, vol. 105, no. 12, pp. 2295–2329, 2017.
2. **C. R. Shannon**, "A Mathematical Theory of Communication," *The Bell System Technical Journal*, vol. 27, pp. 379–423, 1948.
3. **M. McCloskey and N. J. Cohen**, "Catastrophic Interference in Connectionist Networks: The Sequential Learning Problem," *Psychology of Learning and Motivation*, vol. 24, pp. 109–165, 1989.
4. **D. Rolnick, A. Ahuja, G. Schwarz, et al.**, "Experience Replay for Continual Learning," *Advances in Neural Information Processing Systems (NeurIPS)*, vol. 32, 2019.
5. **G. Cerquitelli et al.**, "TinyML: Machine Learning on Extremely Low-Power Embedded Devices," *IEEE Embedded Systems Letters*, vol. 13, no. 4, pp. 110–113, 2021.
6. **Espressif Systems**, *ESP32 Technical Reference Manual (v4.8)*, 2023.
7. **B. Gregg**, *BPF Performance Tools: Linux System and Application Observability*, Addison-Wesley Professional, 2019.
