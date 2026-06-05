#!/usr/bin/env python3
"""
Generate BuildInfo.h and BuildInfo.cpp from platformio.ini and environment variables.
This script is intended to be run as a pre-build script in PlatformIO.

Automatically increments build_number on each run (unless --no-increment is passed).

Usage:
    python generate_buildinfo.py                  # uses version from platformio.ini
    python generate_buildinfo.py -v 1.0.5         # overrides version via CLI
    python generate_buildinfo.py --version 2.0.0  # overrides version via CLI
    python generate_buildinfo.py --no-increment   # skips auto-increment of build_number

When --version is provided, it overrides platformio.ini [env] version AND
updates platformio.ini with the new value for persistence.
"""

import argparse
import configparser
import os
import re
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
BUILDINFO_CPP = PROJECT_DIR / "src/util/BuildInfo.cpp"


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


def increment_build_number(config):
    """
    Increment build_number in platformio.ini.
    Uses regex to preserve formatting (spaces, comments).
    """
    build_number = int(config.get("env", "build_number", fallback="1"))

    # Read raw file content
    raw = PLATFORMIO_INI.read_text(encoding="utf-8")

    # Match build_number = <value> in the [env] section
    pattern = re.compile(
        r'^(\s*build_number\s*=\s*)(\d+)(\s*;.*)?$',
        re.MULTILINE
    )

    new_number = build_number + 1

    def replacer(m):
        return m.group(1) + str(new_number) + (m.group(3) or "")

    new_raw, count = pattern.subn(replacer, raw, count=1)

    if count == 0:
        print("Warning: build_number not found in platformio.ini, appending")
        new_raw += f"\nbuild_number = {new_number}\n"

    PLATFORMIO_INI.write_text(new_raw, encoding="utf-8")
    return new_number


def update_version_in_ini(version):
    """
    Update version in platformio.ini.
    Uses regex to preserve formatting (spaces, comments).
    """
    raw = PLATFORMIO_INI.read_text(encoding="utf-8")
    pattern = re.compile(
        r'^(\s*version\s*=\s*)(\S+)(\s*;.*)?$',
        re.MULTILINE
    )
    new_raw, count = pattern.subn(
        lambda m: m.group(1) + version + (m.group(3) or ""),
        raw,
        count=1
    )
    if count == 0:
        print("Warning: version not found in platformio.ini, appending")
        new_raw += f"\nversion = {version}\n"
    PLATFORMIO_INI.write_text(new_raw, encoding="utf-8")


def main():
    # Parse command-line arguments
    parser = argparse.ArgumentParser(
        description="Generate BuildInfo.h and BuildInfo.cpp for PlatformIO"
    )
    parser.add_argument(
        "-v", "--version",
        type=str,
        default=None,
        help="Override version string (e.g. 1.0.5). If provided, also updates platformio.ini"
    )
    parser.add_argument(
        "--no-increment",
        action="store_true",
        help="Skip auto-increment of build_number (use current value as-is)"
    )
    parser.add_argument(
        "--no-persist",
        action="store_true",
        help="Do not update platformio.ini with the CLI version (use for release builds)"
    )
    args, unknown = parser.parse_known_args()

    # Read platformio.ini
    config = configparser.ConfigParser()
    config.read(PLATFORMIO_INI)

    # Get version: CLI argument takes priority over platformio.ini [env] section
    if args.version is not None:
        version = args.version
        if not args.no_persist:
            update_version_in_ini(version)
        print(f"Version overridden via CLI: {version}" + (" (not persisted)" if args.no_persist else ""))
    else:
        version = config.get("env", "version", fallback="1.0.0")

    build_type = config.get("env", "build_type", fallback="debug")

    # Auto-increment build_number (or use current value if --no-increment)
    if args.no_increment:
        build_number = int(config.get("env", "build_number", fallback="1"))
        print(f"Build number (no increment): {build_number}")
    else:
        build_number = increment_build_number(config)

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

    # Generate BuildInfo.h content (declarations only)
    header_content = f"""#pragma once

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
    const char* getVersion();
    const char* getPlatform();
    const char* getBuildType();
    int getBuildNumber();
    void getFullVersion(char *buf, const size_t &size);
}}
"""

    # Generate BuildInfo.cpp content (implementations)
    source_content = f"""#include "BuildInfo.h"
#include <stdio.h>
#include <string.h>

namespace BuildInfo
{{
    const char *getVersion()
    {{
        return BUILD_VERSION;
    }}

    const char *getPlatform()
    {{
        return BUILD_PLATFORM;
    }}

    const char *getBuildType()
    {{
        return BUILD_TYPE;
    }}

    int getBuildNumber()
    {{
        return BUILD_NUMBER;
    }}

    void getFullVersion(char *buf, const size_t &size)
    {{
        snprintf(buf, size, "%s_%s_%s_%d",
                 BUILD_VERSION, BUILD_PLATFORM, BUILD_TYPE, BUILD_NUMBER);
    }}
}}
"""

    # Write to files
    BUILDINFO_H.parent.mkdir(parents=True, exist_ok=True)
    BUILDINFO_H.write_text(header_content, encoding="utf-8")
    BUILDINFO_CPP.write_text(source_content, encoding="utf-8")
    print(f"Generated {BUILDINFO_H} with version={version}, platform={platform}, build_type={build_type}, build_number={build_number}")
    print(f"Generated {BUILDINFO_CPP}")


if __name__ == "__main__":
    main()
