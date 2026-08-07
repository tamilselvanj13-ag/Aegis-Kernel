#!/usr/bin/env python3
from bcc import BPF
import serial
import time

# --- 1. Try ESP32 Connection (Graceful Fallback) ---
ser = None
try:
    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    print("[+] ESP32 connected successfully!")
except Exception:
    print("[!] No physical ESP32 detected. Running in STANDALONE TERMINAL MODE.")

# --- 2. List of Malicious / Suspicious Commands to Detect ---
SUSPICIOUS_KEYWORDS = [
    "nc", "netcat", "ncat",          # Network shell tools
    "/etc/shadow", "/etc/passwd",   # Sensitive file access
    "chmod 777", "chmod +x",        # Unrestricted permission changes
    "curl", "wget",                  # File downloads
    "whoami", "id",                  # Reconnaissance commands
    "reverse", "shell", "bash -i"   # Common payload terms
]

# --- 3. eBPF C Kernel Code ---
ebpf_code = """
#include <uapi/linux/ptrace.h>

struct data_t {
    u32 pid;
    char comm[16];
};

BPF_PERF_OUTPUT(events);

int trace_exec(struct pt_regs *ctx) {
    struct data_t data = {};
    data.pid = bpf_get_current_pid_tgid() >> 32;
    bpf_get_current_comm(&data.comm, sizeof(data.comm));
    events.perf_submit(ctx, &data, sizeof(data));
    return 0;
}
"""

# --- 4. Load eBPF Program ---
b = BPF(text=ebpf_code)
b.attach_kprobe(event=b.get_syscall_fnname("execve"), fn_name="trace_exec")

print("\n==================================================")
print("  PROJECT AEGIS: eBPF Threat Monitor Active")
print("  Monitoring process executions in real time...")
print("==================================================\n")

# --- 5. Event Callback ---
def process_event(cpu, data, size):
    event = b["events"].event(data)
    command = event.comm.decode('utf-8', 'replace').strip()

    # Check if command matches our threat list
    is_threat = any(keyword in command for keyword in SUSPICIOUS_KEYWORDS)

    if is_threat:
        print(f"🚨 [THREAT DETECTED] PID: {event.pid} | Command: '{command}'")
        if ser and ser.is_open:
            ser.write(b"THREAT\n")
    else:
        print(f"🟢 [SAFE EVENT]     PID: {event.pid} | Command: '{command}'")
        if ser and ser.is_open:
            ser.write(b"SAFE\n")

# Open ring buffer listener
b["events"].open_perf_buffer(process_event)

while True:
    try:
        b.perf_buffer_poll()
    except KeyboardInterrupt:
        print("\nStopping Aegis Monitor...")
        exit()