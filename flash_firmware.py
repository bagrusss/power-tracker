#!/usr/bin/env python3
"""
Cross-platform firmware flashing script for ESP8266/ESP32 using esptool.
Automatically installs esptool if missing.
"""

import sys
import subprocess
import shutil
import os
import platform
from pathlib import Path
import glob
import argparse


def run_command(cmd, check=True):
    """Run shell command and print output."""
    print(f"Executing: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    if check and result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)
    return result

def check_esptool():
    """Check if esptool is available, install if not."""
    if shutil.which("esptool.py") is None:
        print("esptool.py not found, installing via pip...")
        try:
            run_command([sys.executable, "-m", "pip", "install", "esptool"])
        except Exception as e:
            print(f"Failed to install esptool: {e}")
            print("Please install manually: pip install esptool")
            sys.exit(1)
    # Verify installation
    result = run_command(["esptool.py", "version"], check=False)
    if result.returncode != 0:
        print("esptool.py still not working with exit code {result.returncode}. Please check installation.")
        sys.exit(1)

def detect_port():
    """Try to detect ESP device serial port."""
    system = platform.system()
    if system == "Darwin":  # macOS
        # Typical ESP devices on macOS
        candidates = ["/dev/cu.SLAB_USBtoUART", "/dev/cu.usbserial-*", "/dev/cu.wchusbserial*"]
    elif system == "Linux":
        candidates = ["/dev/ttyUSB0", "/dev/ttyACM0", "/dev/ttyAMA0"]
    elif system == "Windows":
        candidates = ["COM3", "COM4", "COM5", "COM6"]
    else:
        candidates = []
    for pattern in candidates:
        if "*" in pattern:
            matches = glob.glob(pattern)
            if matches:
                return matches[0]
        elif os.path.exists(pattern):
            return pattern
    return None

def flash_firmware(bin_path, port=None, baud=460800):
    """Flash binary to ESP using esptool."""
    if not os.path.isfile(bin_path):
        print(f"Error - binary file not found: {bin_path}")
        sys.exit(1)

    if port is None:
        port = detect_port()
        if port is None:
            print("Could not auto-detect serial port. Please specify with --port.")
            sys.exit(1)
        print(f"Auto-detected port: {port}")
    else:
        if not os.path.exists(port):
            print(f"Warning: Port {port} does not appear to exist.")

    # Determine chip type (ESP8266 or ESP32) based on binary? Hardcode for now.
    # User can specify via --chip option. Default to ESP8266.
    chip = "esp8266"
    # For simplicity, we assume ESP8266. You can extend with argument.
    cmd = [
        "esptool.py",
        "--chip", chip,
        "--port", port,
        "--baud", str(baud),
        "write_flash",
        "0x0",
        bin_path
    ]
    print(f"Flashing {bin_path} to {port} at baud {baud}...")
    run_command(cmd)

def main():
    parser = argparse.ArgumentParser(description="Flash firmware to ESP8266/ESP32")
    parser.add_argument("--firmware", "-f", help="Path to firmware binary (.bin)")
    parser.add_argument("--port", "-p", help="Serial port (auto-detect if not provided)")
    parser.add_argument("--baud", "-b", type=int, default=460800, help="Baud rate (default: 460800)")
    parser.add_argument("--chip", choices=["esp8266", "esp32"], default="esp8266", help="ESP chip type")
    args = parser.parse_args()

    check_esptool()
    flash_firmware(args.binary, args.port, args.baud)
    print("Flash completed successfully.")

if __name__ == "__main__":
    main()