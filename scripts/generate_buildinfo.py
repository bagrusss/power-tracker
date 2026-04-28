#!/usr/bin/env python3
"""
Generate BuildInfo.h from platformio.ini and environment variables.
This script is intended to be run as a pre-build script in PlatformIO.
"""

import configparser
import os
import sys
from pathlib import Path

# Determine project directory
try:
    PROJECT_DIR = Path(__file__).parent.parent
except NameError:
    # __file__ is not defined (e.g., running under SCons)
    PROJECT_DIR = Path(os.getcwd())

PLATFORMIO_INI = PROJECT_DIR / "platformio.ini"
BUILDINFO_H = PROJECT_DIR / "src/util/BuildInfo.h"

def get_platform_from_env(env_name, config):
    """Map PlatformIO environment name to platform string."""
    if not env_name:
        return "Unknown"
    # Check if env_name is a section in config
    section = f"env:{env_name}"
    if section in config:
        platform_spec = config.get(section, "platform", fallback="")
        if "espressif8266" in platform_spec:
            return "ESP8266"
        elif "espressif32" in platform_spec:
            return "ESP32"
    # Fallback to name matching
    if "nodemcu" in env_name:
        return "ESP8266"
    elif "esp32" in env_name:
        return "ESP32"
    return "Unknown"

def main():
    # Read platformio.ini
    config = configparser.ConfigParser()
    config.read(PLATFORMIO_INI)

    # Get version, build_type, build_number from [env] section
    version = config.get("env", "version", fallback="1.0.0")
    build_type = config.get("env", "build_type", fallback="debug")
    build_number = config.get("env", "build_number", fallback="1")

    # Determine platform from PLATFORMIO_ENV environment variable
    env_name = os.getenv("PLATFORMIO_ENV", "")
    platform = get_platform_from_env(env_name, config)

    # If still unknown, try to guess from the first env section
    if platform == "Unknown":
        for section in config.sections():
            if section.startswith("env:"):
                platform_spec = config.get(section, "platform", fallback="")
                if "espressif8266" in platform_spec:
                    platform = "ESP8266"
                    break
                elif "espressif32" in platform_spec:
                    platform = "ESP32"
                    break

    # Generate BuildInfo.h content
    content = f"""#pragma once

#include <Arduino.h>

#ifndef BUILD_VERSION
#define BUILD_VERSION "{version}"
#endif

#ifndef BUILD_PLATFORM
#define BUILD_PLATFORM "{platform}"
#endif

#ifndef BUILD_TYPE
#define BUILD_TYPE "{build_type}"
#endif

#ifndef BUILD_NUMBER
#define BUILD_NUMBER {build_number}
#endif

namespace BuildInfo {{
    const char* getVersion() {{
        return BUILD_VERSION;
    }}

    const char* getPlatform() {{
        return BUILD_PLATFORM;
    }}

    const char* getBuildType() {{
        return BUILD_TYPE;
    }}

    int getBuildNumber() {{
        return BUILD_NUMBER;
    }}

    String getFullVersion() {{
        return String(BUILD_VERSION) + '_' + BUILD_PLATFORM + '_' + BUILD_TYPE + '_' + String(BUILD_NUMBER);
    }}
}}
"""

    # Write to file
    BUILDINFO_H.parent.mkdir(parents=True, exist_ok=True)
    BUILDINFO_H.write_text(content, encoding="utf-8")
    print(f"Generated {BUILDINFO_H} with version={version}, platform={platform}, build_type={build_type}, build_number={build_number}")

if __name__ == "__main__":
    main()