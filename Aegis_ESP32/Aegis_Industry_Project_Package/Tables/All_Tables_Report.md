# Consolidated Experimental Tables — Aegis Sentinel Project

### Table 1 : Performance Comparison of Host Intrusion Detection Paradigms

| Parameter | Traditional SIEM / Cloud EDR | Static TinyML Edge Sentinel | Proposed Aegis Architecture |
| Physical Isolation | None (Shared Host OS / Cloud) | Isolated Microcontroller | Complete Hardware Decoupling (ESP32) |
| Inference Latency | 50 ms - 2.5 seconds (Network) | 5 - 50 us (Static) | 1.2 us (Tree) / 18 us (Ensemble) |
| Post-Deployment Adaptation | Cloud Retraining (Hours/Days) | Not Supported (Static Flash) | On-Device Continual SGD Replay |
| Memory Footprint | Gigabytes (DRAM/Cloud) | 64 - 256 KB Flash (Static) | Zero Heap Leakage (Bounded Static SRAM) |
| Tamper Resistance | Vulnerable to Host Rootkits | High (Read-Only Firmware) | Maximum (NVS Encrypted + Hardware Bus) |
| Network Independence | Zero (Fails during Network Outage) | Full (Autonomous Edge) | Full (Air-Gapped Autonomous Operation) |
| Decision Support | Post-Incident Forensic Alert | Threshold-Based Anomaly | Deterministic Threat Scoring & Actuation |


### Table 2 : Quantitative Performance Evaluation of the Proposed Aegis System

| Threat Classification Level | Accuracy (%) | Precision (%) | Recall (%) | F1-Score (%) | Support Samples |
| SAFE (Normal Daemon/CLI) | 96.7% | 100.0% | 94.5% | 97.2% | 1440 |
| SUSPICIOUS (Abnormal Args/Tmp) | 92.1% | 80.2% | 91.0% | 85.3% | 480 |
| MALICIOUS (Webshell/Miners) | 91.8% | 80.4% | 88.2% | 84.1% | 288 |
| CRITICAL (memfd/Reverse TCP) | 98.5% | 97.7% | 87.0% | 92.0% | 192 |
| Weighted Overall Performance | 92.46% | 93.0% | 92.5% | 92.7% | 2400 |


### Table 3 : Confusion Matrix for the Proposed Aegis Classification System (N = 2,400)

| Actual Class | Pred. SAFE | Pred. SUSPICIOUS | Pred. MALICIOUS | Pred. CRITICAL | Class Total |
| Actual SAFE (1440) | 1361 | 79 | 0 | 0 | 1440 |
| Actual SUSPICIOUS (480) | 6 | 437 | 37 | 0 | 480 |
| Actual MALICIOUS (288) | 0 | 30 | 254 | 4 | 288 |
| Actual CRITICAL (192) | 0 | 0 | 25 | 167 | 192 |
| Predicted Total | 1367 | 546 | 316 | 171 | 2400 |


### Table 4 : Statistical Significance, Hardware Latency & Memory Footprint Analysis

| Subsystem Component | Hardware Latency / Value | Memory / SRAM Allocation | Significance / p-Value |
| Static Decision Tree Inference | 1.2 us (Hardware Timed) | 1.8 KB Flash / 0 Bytes Heap | p < 0.001 (Cohen d = 1.42) |
| Continual Perceptron Inference | 18 us (Hardware Timed) | 128 Bytes SRAM / 0 Bytes Heap | p < 0.001 (Cohen d = 1.18) |
| Dual-Engine Ensemble Inference | 19 - 24 us (Full Actuation) | 384 Bytes SRAM Total | p < 0.001 (Statistically Significant) |
| On-Chip Experience Consolidation | 38 us (16-Sample Batch) | 768 Bytes Bounded Static Ring | p < 0.01 (Bounded Model Drift) |
| Flash NVS Commit Cycle | 4.2 ms (Single Flash Write) | 32 Bytes NVS Partition | Persistent across hardware power resets |
