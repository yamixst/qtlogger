#!/usr/bin/env python3

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

import argparse
import os
import re
import subprocess
import sys


def get_script_dir():
    return os.path.dirname(os.path.abspath(__file__))


def get_version_file_path():
    return os.path.join(get_script_dir(), "..", "src", "qtlogger", "version.h")


def read_current_version():
    version_file = get_version_file_path()
    with open(version_file, "r") as f:
        content = f.read()

    match = re.search(r"#define QTLOGGER_VERSION (\d+)\.(\d+)\.(\d+)", content)
    if not match:
        raise ValueError("Could not find version in version.h")

    return int(match.group(1)), int(match.group(2)), int(match.group(3))


def write_version(major, minor, patch):
    version_file = get_version_file_path()
    with open(version_file, "r") as f:
        content = f.read()

    new_content = re.sub(
        r"#define QTLOGGER_VERSION \d+\.\d+\.\d+",
        f"#define QTLOGGER_VERSION {major}.{minor}.{patch}",
        content,
    )

    with open(version_file, "w") as f:
        f.write(new_content)


def run_command(cmd, cwd=None):
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error: {result.stderr}")
        sys.exit(1)
    if result.stdout:
        print(result.stdout)
    return result


def main():
    parser = argparse.ArgumentParser(description="Bump version for qtlogger")
    parser.add_argument("-x", "--major", action="store_true", help="Bump major version")
    parser.add_argument("-y", "--minor", action="store_true", help="Bump minor version")
    parser.add_argument("-z", "--patch", action="store_true", help="Bump patch version")
    args = parser.parse_args()

    # Check that exactly one option is specified
    options_count = sum([args.major, args.minor, args.patch])
    if options_count != 1:
        parser.error("Please specify exactly one of --major, --minor, or --patch")

    # Read current version
    major, minor, patch = read_current_version()
    print(f"Current version: {major}.{minor}.{patch}")

    # Calculate new version
    if args.major:
        major += 1
        minor = 0
        patch = 0
    elif args.minor:
        minor += 1
        patch = 0
    elif args.patch:
        patch += 1

    new_version = f"{major}.{minor}.{patch}"
    print(f"New version: {new_version}")

    # Get project root directory
    root_dir = os.path.abspath(os.path.join(get_script_dir(), ".."))

    # Get current branch name
    result = run_command(["git", "rev-parse", "--abbrev-ref", "HEAD"], cwd=root_dir)
    current_branch = result.stdout.strip()
    print(f"Current branch: {current_branch}")

    # Update version.h
    write_version(major, minor, patch)
    print(f"Updated version.h to {new_version}")

    # Run gen_qtlogger.h.py to update qtlogger.h
    gen_script = os.path.join(get_script_dir(), "gen_qtlogger.h.py")
    run_command([sys.executable, gen_script], cwd=root_dir)
    print("Generated qtlogger.h")

    # Create new branch with version number
    branch_name = f"v{new_version}"
    run_command(["git", "checkout", "-b", branch_name], cwd=root_dir)
    print(f"Created and switched to branch: {branch_name}")

    # Git add the changed files
    run_command(["git", "add", "src/qtlogger/version.h", "qtlogger.h"], cwd=root_dir)
    print("Added files to git staging area")

    # Git commit
    commit_message = f"Bump version to {new_version}"
    run_command(["git", "commit", "-m", commit_message], cwd=root_dir)
    print(f"Committed with message: {commit_message}")

    # Create tag
    tag_name = f"v{new_version}"
    run_command(["git", "tag", tag_name], cwd=root_dir)
    print(f"Created tag: {tag_name}")

    # Switch back to original branch
    run_command(["git", "checkout", current_branch], cwd=root_dir)
    print(f"Switched back to branch: {current_branch}")

    print(f"\nVersion bump complete! New version: {new_version}")
    print(f"Branch '{branch_name}' and tag '{tag_name}' have been created.")
    print(f"You are now back on branch '{current_branch}'.")


if __name__ == "__main__":
    main()
