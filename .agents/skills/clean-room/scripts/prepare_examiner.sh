#!/bin/sh
# usage: GAME_DIR=<game data directory> prepare_examiner.sh <work dir> <brief file>
# Prepares a work directory for the examiner session: the examiner rules, the brief and a prompt.
# The work directory must be outside the repository. It receives no other material.
set -e
. "$(dirname "$0")/common.sh"

WORK=$1; BRIEF=$2
[ -n "$WORK" ] && [ -n "$BRIEF" ] || { echo "usage: GAME_DIR=<dir> $0 <work dir> <brief file>" >&2; exit 1; }
[ -n "$GAME_DIR" ] || { echo "error: set GAME_DIR to the directory with the game data" >&2; exit 1; }
[ -f "$BRIEF" ] || { echo "error: $BRIEF not found" >&2; exit 1; }
mkdir -p "$WORK"
require_outside_repo "$WORK"

cp "$REPO_ROOT/doc/clean_room_examiner.md" "$WORK/rules.md"
cp "$BRIEF" "$WORK/brief.md"
sed "s|@GAME_DIR@|$GAME_DIR|" "$(dirname "$0")/../templates/examiner_prompt.md" > "$WORK/prompt.md"
rm -f "$WORK/description.md"
echo "examiner work directory ready: $WORK"
echo "start a separate session there with the prompt in prompt.md. the description will appear in description.md"
