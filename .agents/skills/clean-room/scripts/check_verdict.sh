#!/bin/sh
# usage: check_verdict.sh <verdict file>
# Exit code 0: the verdict is CLEAN. 1: DIRTY. 2: missing or malformed verdict.
# For DIRTY, prints only the problem lines (line numbers and categories).
V=$1
[ -s "$V" ] || { echo "no verdict" >&2; exit 2; }
case "$(head -n 1 "$V" | tr -d '[:space:]')" in
    CLEAN) echo CLEAN; exit 0 ;;
    DIRTY) echo DIRTY; tail -n +2 "$V" | grep -E '^line [0-9]+: [a-z-]+$'; exit 1 ;;
    *)     echo "malformed verdict" >&2; exit 2 ;;
esac
