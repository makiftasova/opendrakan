#!/bin/sh
# usage: prepare_reviewer.sh <description file> <review dir>
# Prepares a work directory for the reviewer session: the reviewer rules, a copy of the description and a prompt.
# The review directory must be outside the repository. It receives no other material.
set -e
. "$(dirname "$0")/common.sh"

DESC=$1; DIR=$2
[ -n "$DESC" ] && [ -n "$DIR" ] || { echo "usage: $0 <description file> <review dir>" >&2; exit 1; }
[ -f "$DESC" ] || { echo "error: $DESC not found" >&2; exit 1; }
mkdir -p "$DIR"
require_outside_repo "$DIR"

cp "$REPO_ROOT/doc/clean_room_reviewer.md" "$DIR/rules.md"
cp "$DESC" "$DIR/description.md"
cp "$(dirname "$0")/../templates/reviewer_prompt.md" "$DIR/prompt.md"
rm -f "$DIR/verdict.txt"
echo "reviewer work directory ready: $DIR"
echo "start a separate session there with the prompt in prompt.md. the verdict will appear in verdict.txt"
