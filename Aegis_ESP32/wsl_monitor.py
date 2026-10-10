import psutil
import time
import math
from pathlib import Path

# WSL maps the E: drive to /mnt/e/
PIPE_FILE = Path(__file__).resolve().parent / "wsl_pipe.txt"

# Web server process names
WEB_SERVERS = {"nginx", "apache", "apache2", "httpd", "gunicorn", "uwsgi", "node", "lighttpd"}

# Noisy system processes to skip
SKIP_NAMES  = {"wsl_monitor", "kworker", "kthreadd", "ksoftirqd", "migration",
               "rcu_sched", "watchdog", "systemd-journal", "dbus-daemon", "udevd"}

def shannon_entropy(s):
    if not s:
        return 0.0
    freq = {}
    for c in s:
        freq[c] = freq.get(c, 0) + 1
    n = len(s)
    return -sum((cnt / n) * math.log2(cnt / n) for cnt in freq.values())

def extract_vec(p, cmdline_parts):
    """
    Extract full 8D feature vector with REAL uid/ppid from psutil.
    Returns (vec_str, display_str) or (None, None) if process is uninteresting.
    """
    cmd_str = " ".join(cmdline_parts)[:200]

    # Real UID - works natively in WSL/Linux
    try:
        uid = p.uids().real
    except Exception:
        uid = 1000  # assume non-root on failure

    # Real PPID
    try:
        ppid = p.ppid()
        parent_alive = psutil.pid_exists(ppid) and ppid > 1
        is_parent_dead = 0 if parent_alive else 1
    except Exception:
        is_parent_dead = 0

    # Features
    is_temp        = 1 if any(t in cmd_str for t in ["/tmp", "/var/tmp", "/dev/shm"]) else 0
    arg_len        = min(len(cmd_str), 500)
    entropy        = round(shannon_entropy(cmd_str), 4)
    is_root        = 1 if uid == 0 else 0
    web_parent     = 0
    memfd          = 1 if "memfd:" in cmd_str else 0
    ld_preload     = 1 if any(s in cmd_str for s in [
                        "LD_PRELOAD", "/bin/sh", "/bin/bash", "/bin/zsh",
                        "/dev/tcp", ">&", "0>&1"]) else 0

    # Check parent for web server
    try:
        parent = psutil.Process(p.ppid())
        parent_name = parent.name().lower()
        web_parent = 1 if any(w in parent_name for w in WEB_SERVERS) else 0
    except Exception:
        web_parent = 0

    vec_str     = f"VEC {is_temp},{arg_len},{entropy},{is_root},{web_parent},{is_parent_dead},{memfd},{ld_preload}"
    display_str = (f"[WSL pid={p.pid} uid={uid} ppid={p.ppid()}] {cmd_str[:80]}\n"
                   f"  -> {vec_str}")
    return vec_str, display_str


def main():
    print("="*70)
    print(" 🐧 AEGIS NATIVE WSL PROCESS MONITOR (VEC mode — real UID/PPID) ")
    print("="*70)

    try:
        open(PIPE_FILE, 'a').close()
    except PermissionError:
        print(f"[!] Cannot write to {PIPE_FILE}")
        return

    seen_pids = set(psutil.pids())
    print(f"[+] Hooked into WSL kernel. Sending VEC to Windows bridge via {PIPE_FILE.name}\n")

    while True:
        current_pids = set(psutil.pids())
        new_pids = current_pids - seen_pids

        for pid in new_pids:
            try:
                p = psutil.Process(pid)
                cmdline = p.cmdline()
                name    = p.name()

                if not cmdline:
                    continue
                if name in SKIP_NAMES or any(s in name for s in SKIP_NAMES):
                    continue

                vec_str, display_str = extract_vec(p, cmdline)
                if vec_str:
                    print(display_str)
                    with open(PIPE_FILE, 'a') as f:
                        f.write(f"{vec_str}\n")

            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                pass

        seen_pids = current_pids
        time.sleep(0.05)


if __name__ == "__main__":
    main()
