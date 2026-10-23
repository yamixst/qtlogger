#!/usr/bin/env python3

# Copyright (C) 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

import argparse
import os
import re
import subprocess
import sys

VERSION_FILE = os.path.join("src", "qtlogger", "version.h")
HEADER_FILE = "qtlogger.h"
CHANGELOG_FILE = os.path.join("docs", "CHANGELOG.md")

VERSION_RE = r"#define QTLOGGER_VERSION (\d+)\.(\d+)\.(\d+)$"
UNRELEASED_RE = r"^## \[Unreleased\][ \t]*$"


class ReleaseError(Exception):
    pass


def get_script_dir():
    return os.path.dirname(os.path.abspath(__file__))


def get_root_dir():
    return os.path.abspath(os.path.join(get_script_dir(), ".."))


def root_path(name):
    return os.path.join(get_root_dir(), name)


def read_file(name):
    with open(root_path(name), "r") as f:
        return f.read()


def write_file(name, content):
    with open(root_path(name), "w") as f:
        f.write(content)


def run_command(cmd, check=True):
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=get_root_dir(), capture_output=True, text=True)
    if check and result.returncode != 0:
        raise ReleaseError(f"Command failed: {' '.join(cmd)}\n{result.stdout}{result.stderr}")
    return result


def read_current_version():
    match = re.search(VERSION_RE, read_file(VERSION_FILE), re.MULTILINE)
    if not match:
        raise ReleaseError(f"Could not find version in {VERSION_FILE}")

    return int(match.group(1)), int(match.group(2)), int(match.group(3))


def write_version(version):
    content, count = re.subn(
        VERSION_RE, f"#define QTLOGGER_VERSION {version}", read_file(VERSION_FILE), flags=re.MULTILINE
    )
    if count != 1:
        raise ReleaseError(f"Expected exactly one version define in {VERSION_FILE}, found {count}")

    write_file(VERSION_FILE, content)


def release_changelog(content, version):
    match = re.search(UNRELEASED_RE, content, re.MULTILINE)
    if not match:
        raise ReleaseError(f"Could not find '## [Unreleased]' section in {CHANGELOG_FILE}")

    next_section = re.search(r"^## ", content[match.end():], re.MULTILINE)
    end = match.end() + next_section.start() if next_section else len(content)
    if not content[match.end():end].strip():
        raise ReleaseError(f"The '## [Unreleased]' section in {CHANGELOG_FILE} is empty")

    heading = f"## [Unreleased]\n\n## [{version}]"
    return content[: match.start()] + heading + content[match.end():]


def check_preconditions(tag_name):
    branch = run_command(["git", "rev-parse", "--abbrev-ref", "HEAD"]).stdout.strip()
    if branch == "HEAD":
        raise ReleaseError("HEAD is detached, switch to a branch first")

    # Only the changelog may have uncommitted changes, it goes into the release commit
    status = run_command(["git", "status", "--porcelain", "--untracked-files=all"]).stdout
    dirty = [line[3:] for line in status.splitlines() if line[3:] != CHANGELOG_FILE]
    if dirty:
        raise ReleaseError("Working tree has uncommitted changes:\n  " + "\n  ".join(dirty))

    if run_command(["git", "rev-parse", "-q", "--verify", f"refs/tags/{tag_name}"], check=False).returncode == 0:
        raise ReleaseError(f"Tag {tag_name} already exists")

    return branch


def main():
    parser = argparse.ArgumentParser(
        description="Release qtlogger: move the Unreleased section of docs/CHANGELOG.md under the new "
        "version, bump version.h, regenerate qtlogger.h, commit and tag"
    )
    parser.add_argument("-x", "--major", action="store_true", help="Bump major version")
    parser.add_argument("-y", "--minor", action="store_true", help="Bump minor version")
    parser.add_argument("-z", "--patch", action="store_true", help="Bump patch version")
    args = parser.parse_args()

    # Check that exactly one option is specified
    options_count = sum([args.major, args.minor, args.patch])
    if options_count != 1:
        parser.error("Please specify exactly one of --major, --minor, or --patch")

    try:
        # Read current version
        major, minor, patch = read_current_version()
        print(f"Current version: {major}.{minor}.{patch}")

        # Calculate new version
        if args.major:
            major, minor, patch = major + 1, 0, 0
        elif args.minor:
            minor, patch = minor + 1, 0
        else:
            patch += 1

        new_version = f"{major}.{minor}.{patch}"
        tag_name = f"v{new_version}"
        print(f"New version: {new_version}")

        branch = check_preconditions(tag_name)
        print(f"Current branch: {branch}")

        changelog = read_file(CHANGELOG_FILE)
        new_changelog = release_changelog(changelog, new_version)
    except ReleaseError as e:
        print(f"Error: {e}")
        sys.exit(1)

    files = [CHANGELOG_FILE, VERSION_FILE, HEADER_FILE]
    commit_message = f"Release {tag_name}"

    try:
        write_file(CHANGELOG_FILE, new_changelog)
        print(f"Updated {CHANGELOG_FILE}")

        write_version(new_version)
        print(f"Updated {VERSION_FILE} to {new_version}")

        run_command([sys.executable, os.path.join(get_script_dir(), "gen_qtlogger.h.py")])
        print(f"Generated {HEADER_FILE}")

        run_command(["git", "add", "--"] + files)
        run_command(["git", "commit", "-m", commit_message, "--"] + files)
        print(f"Committed with message: {commit_message}")
    except ReleaseError as e:
        print(f"Error: {e}")
        print("Rolling back changes")
        # Keep the user's changelog edits, only drop the release heading
        write_file(CHANGELOG_FILE, changelog)
        run_command(["git", "reset", "-q", "--"] + files, check=False)
        run_command(["git", "checkout", "--", VERSION_FILE, HEADER_FILE], check=False)
        sys.exit(1)

    try:
        run_command(["git", "tag", "-a", tag_name, "-m", commit_message])
        print(f"Created tag: {tag_name}")
    except ReleaseError as e:
        print(f"Error: {e}")
        print(f"The release commit is on '{branch}', create the tag manually")
        sys.exit(1)

    print(f"\nRelease {tag_name} is ready on branch '{branch}'.")
    print("Review it with 'git show' and publish with:")
    print(f"  git push origin {branch} {tag_name}")


if __name__ == "__main__":
    main()
