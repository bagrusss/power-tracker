#!/usr/bin/env python3
"""
Script to run PlatformIO build for each environment and report flash/ram sizes.
Outputs a table with columns: Environment, Status, Duration, Flash, RAM.
Colors match PlatformIO's output style.
"""

import subprocess
import re
import sys
import os
from pathlib import Path

# ANSI color codes
COLOR_RESET = "\033[0m"
COLOR_GREEN = "\033[0;32m"
COLOR_RED = "\033[0;31m"
COLOR_YELLOW = "\033[0;33m"
COLOR_CYAN = "\033[0;36m"
COLOR_WHITE = "\033[1;37m"
COLOR_GRAY = "\033[0;90m"

ANSI_ESCAPE = re.compile(r'\x1b\[[0-9;]*m')

def strip_ansi(s):
    return ANSI_ESCAPE.sub('', s)

def len_without_ansi(s):
    return len(strip_ansi(s))

def colorize(text, color):
    return color + text + COLOR_RESET

def fmt_colored(text, color, width):
    """Return colored text left-justified to visible width."""
    padding = width - len(text)
    if padding > 0:
        return color + text + COLOR_RESET + ' ' * padding
    else:
        return color + text + COLOR_RESET

def run_pio(env):
    """Run pio run for given environment and capture output."""
    cmd = ["pio", "run", "-e", env]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=os.getcwd())
        return result.stdout, result.stderr, result.returncode
    except FileNotFoundError:
        print("Error: PlatformIO not found. Make sure 'pio' is in PATH.")
        sys.exit(1)

def extract_sizes(output):
    """Extract flash and RAM sizes from PlatformIO output."""
    flash = None
    ram = None
    # Pattern for lines like "Flash: [=====     ]  47.0% (used 490469 bytes from 1044464 bytes)"
    flash_pattern = r"Flash:\s*\[.*?\]\s*[\d.]+%\s*\(used\s*(\d+)\s*bytes"
    ram_pattern = r"RAM:\s*\[.*?\]\s*[\d.]+%\s*\(used\s*(\d+)\s*bytes"
    for line in output.split('\n'):
        if flash is None:
            match = re.search(flash_pattern, line)
            if match:
                flash = int(match.group(1))
        if ram is None:
            match = re.search(ram_pattern, line)
            if match:
                ram = int(match.group(1))
    return flash, ram

def extract_table_row(output, env):
    """Extract status and duration from the final table for a specific environment."""
    # Find the line that starts with the environment name
    lines = output.split('\n')
    for line in lines:
        stripped = line.strip()
        # The table row format: environment, status, duration (separated by spaces)
        # We'll split by whitespace and check if first token matches env
        parts = stripped.split()
        if len(parts) >= 3 and parts[0] == env:
            status = parts[1]
            duration = parts[2]
            return status, duration
    return None, None

def main():
    # Determine environments from platformio.ini (simplified)
    # For now, hardcode known environments
    envs = ["nodemcu", "esp32_env", "nodemcu_app"]
    results = []
    
    for env in envs:
        print(f"{colorize('Building', COLOR_CYAN)} {env}...", file=sys.stderr)
        stdout, stderr, rc = run_pio(env)
        if rc != 0:
            print(f"Build failed for {env}:", file=sys.stderr)
            print(stderr, file=sys.stderr)
            continue
        flash, ram = extract_sizes(stdout)
        status, duration = extract_table_row(stdout, env)
        if status is None:
            status = "SUCCESS" if rc == 0 else "FAILED"
            duration = "N/A"
        results.append({
            'env': env,
            'status': status,
            'duration': duration,
            'flash': flash,
            'ram': ram
        })
    
    # Print table header with colors similar to PlatformIO
    print()
    print(colorize("=" * 80, COLOR_GRAY))
    # Header with visible widths
    header = (
        f"{'Environment':<15} "
        f"{'Status':<10} "
        f"{'Duration':<15} "
        f"{'Flash (bytes)':<15} "
        f"{'RAM (bytes)':<5}"
    )
    print(colorize(header, COLOR_WHITE))
    print(colorize("-" * 80, COLOR_GRAY))
    
    for r in results:
        flash_str = str(r['flash']) if r['flash'] is not None else "N/A"
        ram_str = str(r['ram']) if r['ram'] is not None else "N/A"
        # Colorize status
        if r['status'] == "SUCCESS":
            status_colored = fmt_colored(r['status'], COLOR_GREEN, 10)
        elif r['status'] == "FAILED":
            status_colored = fmt_colored(r['status'], COLOR_RED, 10)
        else:
            status_colored = fmt_colored(r['status'], COLOR_YELLOW, 10)
        # Environment in cyan
        env_colored = fmt_colored(r['env'], COLOR_CYAN, 15)
        # Duration in default color, left-justified
        duration_colored = r['duration'].ljust(15)
        # Flash and RAM in default color
        flash_colored = flash_str.ljust(15)
        ram_colored = ram_str.ljust(5)
        # Combine with single spaces between columns (matching header)
        print(f"{env_colored} {status_colored} {duration_colored} {flash_colored} {ram_colored}")
    
    print(colorize("=" * 80, COLOR_GRAY))

if __name__ == "__main__":
    main()