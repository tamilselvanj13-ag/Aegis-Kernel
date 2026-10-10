# Aegis: Industry-Grade Project Package & Telemetry Benchmark Suite

This comprehensive industry package contains the complete research deliverables, raw datasets, multi-sheet Excel workbooks, experimental tables, publication-quality figures, and hardware validation logs for the **Aegis TinyML Hardware Sentinel Project**.

## Folder Structure Overview

```
Aegis_Industry_Project_Package/
│
├── 📊 Aegis_Telemetry_Dataset_and_Benchmarks.xlsx   <-- Master Multi-Sheet Excel Workbook
│   ├── Sheet 1: Dataset_Samples (Full 12,000 process telemetry rows with severity tags)
│   ├── Sheet 2: Paradigm_Comparison (Table 1 architecture comparison)
│   ├── Sheet 3: Model_Metrics (Table 2 quantitative performance percentages)
│   ├── Sheet 4: Confusion_Matrix (Table 3 4x4 matrix with true/false detection counts)
│   ├── Sheet 5: Hardware_Benchmarks (Table 4 microsecond execution latencies & SRAM specs)
│   └── Sheet 6: Live_WSL_Test_Scenarios (Empirical WSL attack tests & hardware responses)
│
├── 📁 Dataset/                                      <-- Raw Process Telemetry & Schema
│   ├── dataset.csv                                 <-- 12,000 process vectors (CSV format)
│   ├── dataset_metadata.json                       <-- Schema specifications & class distributions
│   └── README_Dataset.md                           <-- Feature extraction definitions
│
├── 📁 Tables/                                       <-- Clean CSV & Markdown Tables
│   ├── Table1_Paradigm_Comparison.csv
│   ├── Table2_Quantitative_Evaluation.csv
│   ├── Table3_Confusion_Matrix.csv
│   ├── Table4_Hardware_Resource_Analysis.csv
│   └── All_Tables_Report.md                        <-- Consolidated markdown tables
│
├── 📁 Figures/                                      <-- 300 DPI Publication-Grade Figures
│   ├── Fig1_Physical_Architecture.png              <-- Hardware sentinel diagram
│   ├── Fig2_End_to_End_Flowchart.png               <-- Dual-engine inference pipeline
│   ├── Fig3_Performance_Metrics_Comparison.png    <-- Model accuracy/F1 comparison
│   ├── Fig4_On_Device_Adaptation_and_ROC.png       <-- SGD convergence & ROC (AUC=0.98)
│   └── Fig5_Confusion_Matrix_Heatmap.png           <-- High-res colorized confusion matrix
│
├── 📁 Outputs/                                      <-- Manuscripts & Verification Logs
│   ├── Aegis_Conference_Paper_Single_Column.docx   <-- Primary Single-Column Conference Paper
│   ├── Aegis_Conference_Paper_Single_Column.md     <-- Markdown companion
│   ├── IEEE_Project_Report_Aegis.docx              <-- IEEE Two-Column Project Report
│   └── Hardware_Inference_Execution_Log.txt        <-- Empirical ESP32 UART execution logs
│
└── 📄 README.md                                    <-- This document
```

## Authors & Research Affiliations
- **Tamil Selvan J** (Lead Researcher & Developer, CSE Dept., Chennai Institute of Technology)
- **Prem Sangeeth V** (Assistant Professor, ECE Dept., Chennai Institute of Technology)
- **Suresh Kumar P** (Professor, ECE Dept., Chennai Institute of Technology)
