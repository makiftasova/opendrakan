#!/bin/sh
# Shared helpers for the clean-room scripts. Source this file.

REPO_ROOT=$(cd "$(dirname "$0")/../../../.." && pwd)

# Exit if the directory is inside the repository.
require_outside_repo()
{
    case "$(cd "$1" && pwd)/" in
        "$REPO_ROOT"/*)
            echo "error: $1 is inside the repository. Use a scratch directory elsewhere." >&2
            exit 2 ;;
    esac
}
