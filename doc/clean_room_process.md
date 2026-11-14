# Clean-room process

OpenDrakan contains no code or data from the original game. We follow
[clean-room design](https://en.wikipedia.org/wiki/Clean-room_design) and the
[Wine clean-room guidelines](https://gitlab.winehq.org/wine/wine/-/wikis/Clean-Room-Guidelines).

When you need to know how the original program behaves, split the work into three roles. Use a separate session for each role.
Prefer public specifications and analysis of the game's data files first. Inspect the original program only when they can't
answer the question.

## Roles and rule files

Each role has its own rule file. Give each session only the file for its role.
Don't show the examiner's rules to the reviewer, or the reviewer's rules to the examiner.

| Role | Task | Rules |
|---|---|---|
| Examiner | Inspects the original program and writes a description of its behavior. | `clean_room_examiner.md` |
| Reviewer | Checks the description for leaks and writes a verdict. | `clean_room_reviewer.md` |
| Implementer | Writes the brief, starts the other roles and implements from the description. | This file |

## Workflow

1. The implementer writes a brief with the questions to answer.
2. The examiner inspects the original program and writes a description file.
3. The reviewer checks the description file and writes a verdict.
4. If the verdict is `CLEAN`, the implementer reads the description.
   If it's `DIRTY`, the examiner rewrites the file and the reviewer checks it again.
5. The implementer saves the description in `doc/` and implements from it.

## Implementer rules

Read a description only after its verdict is `CLEAN`. Never look at the original program, the examiner's working files or the
transcript.

When you start the other roles:

- Give the examiner the brief and `clean_room_examiner.md`.
- Give the reviewer the path of the description and `clean_room_reviewer.md`.
- Send the output of both sessions to nowhere. Wait for the description and the verdict files to appear.
- For a `DIRTY` verdict, ask the examiner to rewrite the file. Pass on only the line numbers and categories.

## Provenance and leaks

- Save each finished description in `doc/` with a provenance header. The header names the examiner and reviewer sessions and
  says the description contains no code.
- The history then shows the specification before the implementation.
- If something leaks, tell the others, delete the material, don't rely on it and note it in the provenance header.
- Never commit game assets or other copyrighted material.

## Existing descriptions

- `animation_behaviour.md`: skeletal animation.
- `collision_behaviour.md`: character collision.
