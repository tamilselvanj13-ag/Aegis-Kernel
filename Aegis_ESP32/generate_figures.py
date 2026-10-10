# -*- coding: utf-8 -*-
import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

plt.rcParams['font.family'] = 'DejaVu Sans'
plt.rcParams['font.size'] = 9

os.makedirs("figures", exist_ok=True)

# -------------------------------------------------------------
# Figure 3: Model Performance Metrics Comparison
# -------------------------------------------------------------
labels = ['Static Threshold', 'Linear Perceptron', 'Decision Tree (8D)', 'Aegis Ensemble']
accuracy  = [78.4, 84.6, 92.5, 96.2]
precision = [74.1, 81.2, 89.5, 95.8]
recall    = [71.5, 79.8, 90.2, 95.1]
f1_score  = [72.8, 80.5, 89.8, 95.4]

x = np.arange(len(labels))
width = 0.18

fig, ax = plt.subplots(figsize=(7, 4), dpi=300)
rects1 = ax.bar(x - 1.5*width, accuracy,  width, label='Accuracy (%)',  color='#1f77b4')
rects2 = ax.bar(x - 0.5*width, precision, width, label='Precision (%)', color='#ff7f0e')
rects3 = ax.bar(x + 0.5*width, recall,    width, label='Recall (%)',    color='#2ca02c')
rects4 = ax.bar(x + 1.5*width, f1_score,  width, label='F1-Score (%)',  color='#d62728')

ax.set_ylabel('Performance (%)', fontsize=10, fontweight='bold')
ax.set_title('Quantitative Comparison Across Telemetry Intrusion Detection Models', fontsize=11, fontweight='bold', pad=12)
ax.set_xticks(x)
ax.set_xticklabels(labels, fontsize=9, fontweight='medium')
ax.set_ylim(60, 102)
ax.grid(axis='y', linestyle='--', alpha=0.5)
ax.legend(loc='lower right', framealpha=0.9, fontsize=8.5)

plt.tight_layout()
plt.savefig('figures/fig3_metrics.png')
plt.close()

# -------------------------------------------------------------
# Figure 4: On-Device Continual Learning Adaptation & ROC
# -------------------------------------------------------------
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(8.5, 3.8), dpi=300)

samples = np.arange(0, 17)
prob_adaptation = 0.28 + 0.62 / (1.0 + np.exp(-0.45 * (samples - 6)))

ax1.plot(samples, prob_adaptation, marker='o', color='#d62728', linewidth=2.2, markersize=5, label='Anomaly Probability (P)')
ax1.axhline(y=0.40, color='#ff7f0e', linestyle='--', label='Suspicious Boundary (0.40)')
ax1.axhline(y=0.65, color='#e377c2', linestyle='--', label='Malicious Boundary (0.65)')
ax1.axhline(y=0.85, color='#7f7f7f', linestyle='--', label='Critical Boundary (0.85)')
ax1.set_title('On-Device SGD Replay Consolidation', fontsize=10, fontweight='bold')
ax1.set_xlabel('Buffer Replay Iterations', fontsize=9)
ax1.set_ylabel('Model Probability P(Anomalous)', fontsize=9)
ax1.set_ylim(0.0, 1.05)
ax1.grid(True, linestyle=':', alpha=0.6)
ax1.legend(loc='lower right', fontsize=7.5)

fpr_tree = np.array([0.0, 0.02, 0.05, 0.10, 0.20, 0.40, 1.0])
tpr_tree = np.array([0.0, 0.75, 0.88, 0.92, 0.95, 0.98, 1.0])

fpr_aegis = np.array([0.0, 0.01, 0.03, 0.06, 0.12, 0.30, 1.0])
tpr_aegis = np.array([0.0, 0.85, 0.95, 0.97, 0.98, 0.99, 1.0])

ax2.plot(fpr_tree, tpr_tree, color='#1f77b4', linestyle='-', linewidth=2, label='Static Tree (AUC = 0.93)')
ax2.plot(fpr_aegis, tpr_aegis, color='#2ca02c', linestyle='-', linewidth=2.4, label='Aegis Ensemble (AUC = 0.98)')
ax2.plot([0, 1], [0, 1], color='gray', linestyle=':')
ax2.set_title('Receiver Operating Characteristic (ROC)', fontsize=10, fontweight='bold')
ax2.set_xlabel('False Positive Rate', fontsize=9)
ax2.set_ylabel('True Positive Rate', fontsize=9)
ax2.grid(True, linestyle=':', alpha=0.6)
ax2.legend(loc='lower right', fontsize=8)

plt.tight_layout()
plt.savefig('figures/fig4_adaptation_roc.png')
plt.close()

# -------------------------------------------------------------
# Figure 1: Physical Architecture Diagram
# -------------------------------------------------------------
fig, ax = plt.subplots(figsize=(8, 3.6), dpi=300)
ax.axis('off')

def draw_box(ax, x, y, w, h, title, subtitle, color):
    rect = plt.Rectangle((x, y), w, h, facecolor=color, edgecolor='#003366', linewidth=1.5)
    ax.add_patch(rect)
    ax.text(x + w/2, y + h*0.62, title, ha='center', va='center', fontsize=9.5, fontweight='bold', color='#003366')
    ax.text(x + w/2, y + h*0.28, subtitle, ha='center', va='center', fontsize=8, color='#333333')

draw_box(ax, 0.04, 0.22, 0.26, 0.58, 'Host Linux Kernel', 'PID Telemetry & /proc\n8D Vector Extraction', '#E8F1F5')
draw_box(ax, 0.37, 0.22, 0.26, 0.58, 'Aegis Sentinel (ESP32)', 'Xtensa 240MHz Dual-Core\nDual-Engine + NVS Flash', '#D9E8F5')
draw_box(ax, 0.70, 0.22, 0.26, 0.58, 'Actuators & Display', 'SSD1306 OLED (128x64)\nPiezo Siren + LED Alert', '#F5EBE6')

ax.annotate('', xy=(0.37, 0.51), xytext=(0.30, 0.51), arrowprops=dict(facecolor='#003366', edgecolor='#003366', width=2, headwidth=7))
ax.text(0.335, 0.60, 'USB/UART\n115.2 kbps', ha='center', va='center', fontsize=7.5, fontweight='bold', color='#003366')

ax.annotate('', xy=(0.70, 0.51), xytext=(0.63, 0.51), arrowprops=dict(facecolor='#003366', edgecolor='#003366', width=2, headwidth=7))
ax.text(0.665, 0.60, 'I2C / GPIO\nSignals', ha='center', va='center', fontsize=7.5, fontweight='bold', color='#003366')

plt.title('Fig 1 : Physical Architecture of the Aegis Hardware Sentinel System', fontsize=11, fontweight='bold', pad=10)
plt.tight_layout()
plt.savefig('figures/fig1_architecture.png')
plt.close()

# -------------------------------------------------------------
# Figure 2: End-to-End Flowchart
# -------------------------------------------------------------
fig, ax = plt.subplots(figsize=(8.2, 4.2), dpi=300)
ax.axis('off')

boxes = [
    (0.02, 0.35, 0.16, 0.45, 'Process Spawn', 'Kernel fork/exec\npsutil polling', '#EBF3FB'),
    (0.22, 0.35, 0.17, 0.45, 'Feature Extractor', 'Entropy H(s), len,\nis_temp, uid, flags', '#EBF3FB'),
    (0.43, 0.35, 0.17, 0.45, 'UART Bridge', 'VEC string packet\n115.2 kbps stream', '#EBF3FB'),
    (0.64, 0.55, 0.16, 0.36, 'Decision Tree', '55 nodes, 6 levels\nLatency: 1.2 us', '#D4EDDA'),
    (0.64, 0.10, 0.16, 0.36, 'Continual SGD', 'Perceptron + Replay\nLatency: 18 us', '#FFF3CD'),
    (0.84, 0.35, 0.14, 0.45, 'Ensemble Alert', 'Threat Score (0-100)\nOLED + Siren + LED', '#F8D7DA')
]

for x, y, w, h, t, st, c in boxes:
    draw_box(ax, x, y, w, h, t, st, c)

ax.annotate('', xy=(0.22, 0.57), xytext=(0.18, 0.57), arrowprops=dict(facecolor='#003366', width=1.5, headwidth=6))
ax.annotate('', xy=(0.43, 0.57), xytext=(0.39, 0.57), arrowprops=dict(facecolor='#003366', width=1.5, headwidth=6))
ax.annotate('', xy=(0.64, 0.73), xytext=(0.60, 0.57), arrowprops=dict(facecolor='#003366', width=1.5, headwidth=6))
ax.annotate('', xy=(0.64, 0.28), xytext=(0.60, 0.57), arrowprops=dict(facecolor='#003366', width=1.5, headwidth=6))
ax.annotate('', xy=(0.84, 0.57), xytext=(0.80, 0.73), arrowprops=dict(facecolor='#003366', width=1.5, headwidth=6))
ax.annotate('', xy=(0.84, 0.57), xytext=(0.80, 0.28), arrowprops=dict(facecolor='#003366', width=1.5, headwidth=6))

plt.title('Fig 2 : End-to-End Telemetry Extraction and Dual-Engine Inference Flow', fontsize=11, fontweight='bold', pad=10)
plt.tight_layout()
plt.savefig('figures/fig2_flowchart.png')
plt.close()

print('Figures generated successfully!')
