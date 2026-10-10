import serial
import time
import sys
import os
from pathlib import Path

COM_PORT  = sys.argv[1] if len(sys.argv) > 1 else "COM6"
BAUD      = 115200
PIPE_FILE = Path(__file__).resolve().parent / "wsl_pipe.txt"

def main():
    print("="*70)
    print(" 🌉 AEGIS WINDOWS -> ESP32 SERIAL BRIDGE (Bidirectional) ")
    print("="*70)
    
    # Create file if it doesn't exist
    open(PIPE_FILE, 'a').close()
    
    try:
        print(f"[*] Connecting to ESP32 on {COM_PORT}...")
        ser = serial.Serial(COM_PORT, BAUD, timeout=0.1)
        time.sleep(2)
        print(f"[+] USB Link established! Tailing {PIPE_FILE} for WSL commands...\n")
        
        with open(PIPE_FILE, 'r') as f:
            f.seek(0, 2) # Go to end of file
            
            while True:
                # 1. Forward new commands from WSL to ESP32
                line = f.readline()
                if line:
                    line = line.strip()
                    if line:
                        # Fix: do NOT prepend EVAL if line already starts with VEC or EVAL
                        if line.startswith("VEC ") or line.startswith("EVAL ") or line.startswith("vec "):
                            cmd = f"{line}\n"
                        else:
                            cmd = f"EVAL {line}\n"
                        print(f"\n[Bridging to ESP32] {line}")
                        ser.write(cmd.encode('utf-8'))
                        ser.flush()

                # 2. Read live responses from ESP32 and print to console
                while ser.in_waiting > 0:
                    try:
                        raw = ser.readline().decode('utf-8', errors='ignore').strip()
                        if raw:
                            print(f"  [ESP32] {raw}")
                    except Exception:
                        break

                time.sleep(0.05)
                
    except serial.SerialException:
        print(f"[!] Could not connect to {COM_PORT}. Is the Arduino Serial Monitor closed?")
    except KeyboardInterrupt:
        print("\n[*] Shutting down Bridge.")

if __name__ == "__main__":
    main()
