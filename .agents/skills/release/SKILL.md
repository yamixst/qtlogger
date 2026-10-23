---
name: release
description: Prepare a qtlogger release - fill the Unreleased section of docs/CHANGELOG.md from commits since the last tag, then run tools/bump_version.py to bump the version, regenerate qtlogger.h, commit and tag. Use when the user asks to make a release, cut a version, or bump the version.
---

# Release qtlogger

A release is one commit `Release vX.Y.Z` with three files and an annotated tag `vX.Y.Z`:

- `docs/CHANGELOG.md` - the `## [Unreleased]` section becomes `## [X.Y.Z]`
- `src/qtlogger/version.h` - `QTLOGGER_VERSION`
- `qtlogger.h` - the generated single header

`tools/bump_version.py` makes the commit and the tag. Your job is the changelog and the version level. Never edit `version.h` or `qtlogger.h` by hand and never run `tools/gen_qtlogger.h.py` yourself: the script does both.

Pushing the tag starts `.github/workflows/release.yml`, which publishes the GitHub release with the `## [X.Y.Z]` section as its text. Push only after the user confirms.

## Steps

### 1. Check the state

```bash
git status --short
git rev-parse --abbrev-ref HEAD
git describe --tags --abbrev=0
```

- The working tree must be clean, except for `docs/CHANGELOG.md`. Otherwise stop and ask the user what to do with the changes.
- Releases are made from `main`. If on another branch, ask before continuing.
- Pull the latest `main` if the user hasn't said otherwise: `git pull --ff-only`.

### 2. Collect the changes

```bash
git log --no-merges --format='%h %s%n%b' $(git describe --tags --abbrev=0)..HEAD
```

For commits that are unclear from the message, look at `git show --stat <sha>` or the diff. Commits that close issues or PRs (`Fixes #N`, `(#N)`) can be checked with `gh issue view N` / `gh pr view N`.

### 3. Fill `## [Unreleased]` in `docs/CHANGELOG.md`

Read the file first and follow the style of the existing entries:

- Sections in this order, only the non-empty ones: `### Added`, `### Changed`, `### Fixed`, `### Removed`.
- One line per change, starting with `- `, written for library users: what changed in behaviour or API, not how the code was changed.
- Code identifiers in backticks: `` `LogMessage` ``, `` `QTLOGGER_STATIC` ``.
- Add the issue number at the end when there is one: `(#5)`.
- Skip changes users don't see: CI, workflow, refactoring without behaviour change, tests, typo fixes in comments, the release commits themselves. Documentation changes go in only if they are significant.
- Keep entries the user already wrote in `Unreleased`; add only what is missing and don't duplicate.
- Don't add a date or a version heading: the script renames `## [Unreleased]` itself.

### 4. Choose the version level

The project is pre-1.0 (`0.y.z`):

- `-z` (patch) - only fixes and minor internal changes
- `-y` (minor) - new features, or any incompatible API or behaviour change
- `-x` (major) - only when the user explicitly asks (e.g. 1.0.0)

### 5. Confirm with the user

Show the final `## [Unreleased]` section, the current version and the proposed new one, and wait for the user's confirmation. Apply their corrections to the changelog before going on.

### 6. Run the script

```bash
python3 tools/bump_version.py -z   # or -y / -x
```

Do not commit `docs/CHANGELOG.md` beforehand: the script includes it in the release commit.

If the script fails, it rolls back `version.h` and `qtlogger.h` and keeps the changelog text. Report the error, fix the cause and run it again. Don't redo its steps by hand.

### 7. Review and publish

```bash
git show --stat HEAD
git show v<version> --no-patch
```

Show the result to the user. Push only after they confirm:

```bash
git push origin main v<version>
```

After the push, give the user the link to the release workflow run: `gh run list --workflow=release.yml --limit 1`.
