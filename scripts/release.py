#!/usr/bin/env python3
"""
Release script: builds firmware, creates a GitHub Release with the binary
attached as an asset. The binary is NOT copied to the project root — it only
exists inside .pio/build/ and as a GitHub Release asset.

Requires: gh CLI (https://cli.github.com) authenticated with `gh auth login`

Usage:
    python scripts/release.py                          # use version from platformio.ini
    python scripts/release.py -v 1.0.5                 # override version
    python scripts/release.py --version 2.0.0          # override version
    python scripts/release.py --env nodemcu            # PlatformIO env (default: nodemcu)
    python scripts/release.py --dry-run                # show what would happen
    python scripts/release.py --skip-push              # don't push to remote
    python scripts/release.py --skip-gh-release        # tag + binary only, no GitHub Release

Before running:
    - Pull request must be merged to master
    - Working directory must be clean (no uncommitted changes)
    - gh CLI must be authenticated (gh auth login)

Output:
    - Git tag:   v{version}-b{build_number}
    - GitHub Release with binary as downloadable asset
    - Binary is NOT stored in the project root
"""

import argparse
import configparser
import os
import subprocess
import sys
from pathlib import Path


# Project paths
PROJECT_DIR = Path(__file__).parent.parent.resolve()
PLATFORMIO_INI = PROJECT_DIR / "platformio.ini"
SCRIPTS_DIR = PROJECT_DIR / "scripts"


def run(cmd, cwd=None, check=True, capture=False, allow_failure=False):
    """Run a shell command and print output."""
    cwd = cwd or PROJECT_DIR
    print(f"\033[1;36m>>> {' '.join(cmd)}\033[0m")
    result = subprocess.run(
        cmd,
        cwd=cwd,
        text=True,
        capture_output=capture,
    )
    if result.returncode != 0:
        if check and not allow_failure:
            print(f"\033[1;31mCommand failed with exit code {result.returncode}\033[0m")
            if capture:
                print(result.stdout)
                print(result.stderr, file=sys.stderr)
            sys.exit(result.returncode)
        if allow_failure:
            print(f"\033[1;33mCommand returned exit code {result.returncode} (non-fatal)\033[0m")
    if capture and result.stdout:
        print(result.stdout.strip())
    return result


def read_config():
    """Read platformio.ini and return (version, build_type, build_number, config)."""
    config = configparser.ConfigParser()
    config.read(PLATFORMIO_INI)
    version = config.get("env", "version", fallback="1.0.0")
    build_type = config.get("env", "build_type", fallback="debug")
    build_number = config.getint("env", "build_number", fallback="1")
    return version, build_type, build_number, config


def get_platform_string_from_config(config, env_name):
    """Map platformio env name to lowercase platform string."""
    section = f"env:{env_name}"
    platform_spec = config.get(section, "platform", fallback="")
    if "espressif8266" in platform_spec:
        return "esp8266"
    elif "espressif32" in platform_spec:
        return "esp32"
    if "nodemcu" in env_name or "8266" in env_name:
        return "esp8266"
    if "esp32" in env_name:
        return "esp32"
    return "unknown"


def get_build_number():
    """Read current build_number from platformio.ini (without incrementing)."""
    _, _, build_number, _ = read_config()
    return build_number


def check_git_clean():
    """Ensure working directory is clean."""
    result = run(
        ["git", "status", "--porcelain"],
        check=False,
        capture=True,
    )
    if result.stdout.strip():
        print("\033[1;31mError: Working directory is not clean. Commit or stash changes first.\033[0m")
        print(result.stdout)
        sys.exit(1)

    result = run(
        ["git", "rev-parse", "--abbrev-ref", "HEAD"],
        check=False,
        capture=True,
    )
    branch = result.stdout.strip()
    print(f"Current branch: {branch}")


def check_gh_cli():
    """Verify gh CLI is installed and authenticated."""
    result = run(
        ["gh", "auth", "status"],
        check=False,
        capture=True,
        allow_failure=True,
    )
    if result.returncode != 0:
        print("\033[1;31mError: gh CLI not authenticated.\033[0m")
        print("Please run: gh auth login")
        print("Or use --skip-gh-release to skip GitHub Release creation.")
        return False
    print("gh CLI: authenticated")
    return True


def get_prev_tag():
    """Get the most recent tag for release notes generation."""
    result = run(
        ["git", "tag", "--sort=-creatordate"],
        check=False,
        capture=True,
    )
    tags = [t.strip() for t in result.stdout.splitlines() if t.strip()]
    return tags[0] if tags else None


def generate_release_notes(version, build_number, env_name, platform_lower):
    """Generate release notes markdown. Uses release_notes.md if present, otherwise git log."""
    notes = f"""## v{version}-b{build_number}

**Platform:** {platform_lower} ({env_name})
**Version:** {version}
**Build:** {build_number}

### Changes
"""

    # Check for user-provided release notes file
    user_notes_path = PROJECT_DIR / "release_notes.md"
    if user_notes_path.exists():
        print(f"  Using release_notes.md for changes")
        user_text = user_notes_path.read_text(encoding="utf-8").strip()
        notes += user_text + "\n"
    else:
        # Auto-generate from git log
        prev_tag = get_prev_tag()
        if prev_tag:
            result = run(
                ["git", "log", f"{prev_tag}..HEAD", "--oneline", "--no-merges"],
                check=False,
                capture=True,
            )
            commits = result.stdout.strip()
        else:
            result = run(
                ["git", "log", "--oneline", "--no-merges", "-20"],
                check=False,
                capture=True,
            )
            commits = result.stdout.strip()

        if commits:
            for line in commits.splitlines():
                notes += f"- {line}\n"
        else:
            notes += "- No changes recorded\n"

    notes += f"\n### Download\n- [`{platform_lower}_{version}_b{build_number}.bin`](https://github.com/bagrusss/power-tracker/releases/download/v{version}-b{build_number}/{platform_lower}_{version}_b{build_number}.bin)\n"
    notes += f"- [`ota.json`](https://github.com/bagrusss/power-tracker/releases/download/v{version}-b{build_number}/ota.json)\n"
    return notes


def get_platform_display_from_config(config, env_name):
    """Map platformio env name to lowercase platform string (BuildInfo format)."""
    section = f"env:{env_name}"
    platform_spec = config.get(section, "platform", fallback="")
    if "espressif8266" in platform_spec:
        return "esp8266"
    elif "espressif32" in platform_spec:
        return "esp32"
    if "nodemcu" in env_name or "8266" in env_name:
        return "esp8266"
    if "esp32" in env_name:
        return "esp32"
    return "Unknown"


def generate_ota_json(version, build_number, env_name):
    """Generate ota.json manifest for OTA updates."""
    import json

    _, _, _, config = read_config()
    platform_lower = get_platform_string_from_config(config, env_name)
    platform_display = get_platform_display_from_config(config, env_name)

    tag = f"v{version}-b{build_number}"
    bin_name = f"{platform_lower}_{version}_b{build_number}.bin"
    firmware_url = f"https://github.com/bagrusss/power-tracker/releases/download/{tag}/{bin_name}"

    ota_data = {
        "version": version,
        "buildNumber": build_number,
        platform_display: firmware_url,
    }

    ota_path = PROJECT_DIR / "ota.json"
    ota_path.write_text(json.dumps(ota_data, indent=4) + "\n", encoding="utf-8")
    print(f"  ota.json generated: {ota_path}")
    return ota_path


def commit_and_push_ota_json(ota_path, version, build_number, skip_push=False):
    """Commit ota.json to master and push."""
    tag = f"v{version}-b{build_number}"

    run(["git", "add", str(ota_path)])

    result = run(
        ["git", "diff", "--cached", "--quiet"],
        check=False,
        capture=True,
        allow_failure=True,
    )
    if result.returncode == 0:
        print("  ota.json unchanged, skipping commit")
        return

    run(["git", "commit", "-m", f"OTA manifest for {tag}"])

    if not skip_push:
        run(["git", "push", "origin", "master"])
        print(f"  ota.json pushed to master")
    else:
        print(f"  ota.json committed locally (--skip-push)")


def build_firmware(env_name, version):
    """Run generate_buildinfo.py (no increment, no persist) and PlatformIO build."""
    run([
        sys.executable,
        str(SCRIPTS_DIR / "generate_buildinfo.py"),
        "-v", version,
        "--no-increment",
        "--no-persist",
    ])
    run(["platformio", "run", "--environment", env_name])


def create_release(env_name, version, dry_run=False, skip_push=False, skip_gh_release=False):
    """Create git tag and GitHub Release with binary asset (binary stays in .pio/build/)."""
    build_dir = PROJECT_DIR / ".pio" / "build" / env_name
    firmware_bin = build_dir / "firmware.bin"

    if not firmware_bin.exists():
        print(f"\033[1;31mError: firmware.bin not found at {firmware_bin}\033[0m")
        sys.exit(1)

    build_number = get_build_number()
    _, _, _, config = read_config()
    platform_lower = get_platform_string_from_config(config, env_name)

    tag = f"v{version}-b{build_number}"
    bin_name = f"{platform_lower}_{version}_b{build_number}.bin"

    print(f"\n\033[1;33m=== Release Summary ===\033[0m")
    print(f"  Platform:     {platform_lower} ({env_name})")
    print(f"  Version:      {version}")
    print(f"  Build #:      {build_number}")
    print(f"  Tag:          {tag}")
    print(f"  Binary:       {bin_name} (in .pio/build/{env_name}/)")
    print(f"  OTA manifest: ota.json")
    print(f"  GitHub Rel:   {'SKIP' if skip_gh_release else 'YES'}")
    print(f"  Push:         {'SKIP' if skip_push else 'YES'}")

    if dry_run:
        print("\n\033[1;33m[DRY RUN] No actions performed.\033[0m")
        return

    # Step 1: Generate ota.json and commit to master
    print(f"\n\033[1;33m[1/4] Generating ota.json\033[0m")
    ota_path = generate_ota_json(version, build_number, env_name)
    commit_and_push_ota_json(ota_path, version, build_number, skip_push=skip_push)

    # Step 2: Create annotated tag (pointing to current HEAD with ota.json commit)
    print(f"\n\033[1;33m[2/4] Creating tag {tag}\033[0m")
    run(["git", "tag", "-a", tag, "-m", f"Release {tag}"])

    # Step 3: Push tag to remote
    if not skip_push:
        print(f"\n\033[1;33m[3/4] Pushing tag to origin\033[0m")
        run(["git", "push", "origin", tag])
    else:
        print(f"\n\033[1;33m[3/4] Skipping push (--skip-push)\033[0m")

    # Step 4: Create GitHub Release (binary + ota.json uploaded as assets)
    if not skip_gh_release:
        print(f"\n\033[1;33m[4/4] Creating GitHub Release\033[0m")

        gh_ok = check_gh_cli()
        if not gh_ok:
            print("\033[1;31mCannot create GitHub Release without authenticated gh CLI.\033[0m")
            print(f"After authenticating, run manually:")
            print(f"  gh release create {tag} '{firmware_bin}#{bin_name}' '{ota_path}' --title 'v{version}-b{build_number}' --notes-from-tag")
            return

        # Ensure the tag is pushed before creating release
        if skip_push:
            print("\033[1;33mWarning: tag not pushed to remote. GitHub Release requires the tag on remote.\033[0m")
            print("Push the tag first, then run:")
            print(f"  gh release create {tag} '{firmware_bin}#{bin_name}' '{ota_path}' --title 'v{version}-b{build_number}' --notes-from-tag")
            return

        release_notes = generate_release_notes(version, build_number, env_name, platform_lower)
        notes_file = PROJECT_DIR / ".release_notes.md"
        notes_file.write_text(release_notes, encoding="utf-8")

        run([
            "gh", "release", "create", tag,
            str(firmware_bin) + "#" + bin_name,
            str(ota_path),
            "--title", f"v{version}-b{build_number}",
            "--notes-file", str(notes_file),
        ])

        notes_file.unlink(missing_ok=True)
        print(f"\n\033[1;32mGitHub Release created: https://github.com/bagrusss/power-tracker/releases/tag/{tag}\033[0m")

    print(f"\n\033[1;32m=== Release {tag} complete! ===\033[0m")
    if not skip_gh_release:
        print(f"  Release: https://github.com/bagrusss/power-tracker/releases/tag/{tag}")


def main():
    parser = argparse.ArgumentParser(
        description="Build firmware and create a GitHub Release"
    )
    parser.add_argument(
        "-v", "--version",
        type=str,
        default=None,
        help="Version string (e.g. 1.0.5). Uses platformio.ini if not provided",
    )
    parser.add_argument(
        "--env",
        type=str,
        default="nodemcu",
        help="PlatformIO environment name (default: nodemcu)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show what would happen without executing",
    )
    parser.add_argument(
        "--skip-push",
        action="store_true",
        help="Don't push tag to remote",
    )
    parser.add_argument(
        "--skip-gh-release",
        action="store_true",
        help="Only create tag, skip GitHub Release creation",
    )
    args = parser.parse_args()

    version, build_type, build_number, config = read_config()

    if args.version:
        version = args.version
        print(f"Version override via CLI: {version}")
    else:
        print(f"Version from platformio.ini: {version}")

    print(f"Build type: {build_type}")
    print(f"Current build number: {build_number} (will NOT be auto-incremented)")

    if args.dry_run:
        platform_lower = get_platform_string_from_config(config, args.env)
        tag = f"v{version}-b{build_number}"
        bin_name = f"{platform_lower}_{version}_b{build_number}.bin"
        print(f"\n\033[1;33m[DRY RUN]\033[0m")
        print(f"  Would build env:    {args.env}")
        print(f"  Would create tag:   {tag}")
        print(f"  Would create GitHub Release with {bin_name} as asset")
        return

    if not args.skip_gh_release and not args.skip_push:
        print("\n--- Checking gh CLI ---")
        check_gh_cli()

    # Build
    print("\n--- Building firmware ---")
    build_firmware(args.env, version)

    # Release
    print("\n--- Creating release ---")
    create_release(
        args.env, version,
        dry_run=args.dry_run,
        skip_push=args.skip_push,
        skip_gh_release=args.skip_gh_release,
    )


if __name__ == "__main__":
    main()
