#!/usr/bin/env bash
# Computes version metadata from git, mirroring the logic previously inlined
# in the Makefile. Used by mise tasks (see mise.toml [vars]).
#
# Usage: version.sh [version|tag|commit|date_pretty]
set -euo pipefail

TAG_COMMIT=$(git rev-list --abbrev-commit --tags --max-count=1 2>/dev/null || true)
TAG=$(git describe --abbrev=0 --tags "${TAG_COMMIT}" 2>/dev/null || true)
COMMIT=$(git rev-parse --short HEAD)
DATE_PRETTY=$(env TZ=UTC0 git log -1 --format=%cd --date=format:"%Y-%m-%d")
DATE=$(env TZ=UTC0 git log -1 --format=%cd --date=format:"%Y%m%d")

VERSION="${TAG#v}"

if [ "$COMMIT" != "$TAG_COMMIT" ]; then
    VERSION="${VERSION}-next-${COMMIT}-${DATE}"
fi

if [ -z "$VERSION" ]; then
    VERSION="${COMMIT}-${DATE}"
fi

if [ -n "$(git status --porcelain)" ]; then
    VERSION="${VERSION}-dirty"
fi

case "${1:-version}" in
    version) echo "$VERSION" ;;
    tag) echo "$TAG" ;;
    commit) echo "$COMMIT" ;;
    date_pretty) echo "$DATE_PRETTY" ;;
    *) echo "Unknown field: ${1}" >&2; exit 1 ;;
esac
