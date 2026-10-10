import serial
import psutil
import time
import sys

# Default to COM6, but allow override via command line arguments
COM_PORT = sys.argv[1] if len(sys.argv) > 1 else "COM6"
BAUD = 115200

def main():
    print("="*70)
    print(" 🛡️ AEGIS LIVE LAPTOP MONITOR (Windows & WSL) ")
    print("="*70)
    
    try:
        print(f"[*] Connecting to ESP32 on {COM_PORT}...")
        ser = serial.Serial(COM_PORT, BAUD, timeout=1)
        time.sleep(2) # Wait for ESP32 to reboot from serial connection
        print("[+] Link established! Watching for new processes...\n")
        
        # Take a snapshot of currently running processes so we only capture NEW ones
        seen_pids = set(psutil.pids())
        
        while True:
            current_pids = set(psutil.pids())
            new_pids = current_pids - seen_pids
            
            for pid in new_pids:
                try:
                    p = psutil.Process(pid)
                    cmdline = p.cmdline()
                    
                    if cmdline:
                        # Join the command into a single string
                        cmd_str = " ".join(cmdline)[:120] 
                        
                        # Filter out basic noisy Windows background tasks to keep the OLED clean
                        if "svchost" not in cmd_str and "conhost" not in cmd_str:
                            print(f"[>] Captured: {cmd_str}")
                            
                            # Stream it to the ESP32 for AI Evaluation!
                            ser.write(f"EVAL {cmd_str}\n".encode('utf-8'))
                            
                except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                    pass
            
            seen_pids = current_pids
            time.sleep(0.5) # Poll twice a second
            
    except serial.SerialException:
        print(f"[!] Could not connect to {COM_PORT}. Is the Serial Monitor closed?")
    except KeyboardInterrupt:
        print("\n[*] Shutting down Aegis Monitor.")

if __name__ == "__main__":
    main()
