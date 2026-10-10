# Aegis Process Telemetry Dataset (v2.0)

This dataset contains 12,000 multi-class Linux process telemetry records compiled for training and benchmarking TinyML intrusion detection models on edge hardware.

## Feature Schema
| Feature | Data Type | Description | Distribution Baselines |
|---|---|---|---|
| `is_temp` | int (0/1) | Executing from volatile/temp paths (`/tmp`, `/dev/shm`) | 2% in SAFE, 70% in SUSP, 60% in MAL |
| `arg_len` | int | Length of command string in characters | Mean 45 in SAFE, 130 in SUSP, 230 in CRIT |
| `entropy` | float | Shannon Information Entropy $H(s)$ | Mean 3.15 in SAFE, 4.12 in SUSP, 4.88 in CRIT |
| `is_root` | int (0/1) | Real Process UID == 0 (Superuser) | 20% in SAFE, 40% in MAL, 100% in CRIT |
| `web_parent`| int (0/1) | Web server lineage (`nginx`, `apache2`) | 0% in SAFE, 100% in webshell attacks |
| `is_parent_dead` | int (0/1) | Orphaned process status (PPID dead) | 4% in SAFE, 50% in MAL/CRIT |
| `memfd` | int (0/1) | Anonymous fileless RAM execution | 0% in SAFE/SUSP, 100% in memfd droppers |
| `ld_preload` | int (0/1) | Linker preloading or `/dev/tcp` shell socket | 0% in SAFE, 100% in reverse shells |
| `LABEL` | int | 0: SAFE, 1: SUSPICIOUS, 2: MALICIOUS, 3: CRITICAL | Stratified 60/20/12/8% |
