---
name: clean-room
description: Inspect the original Drakan program under clean-room rules. Use when a question about the original engine's behavior can't be answered from public specs or the game's data files. Runs a separate examiner session and a separate reviewer session.
---

# Clean-room inspection

Follow `doc/clean_room_process.md`. This skill automates the steps. Read that file first.

You are the implementer. You never look at the original program, the examiner's working files or the transcript.
You read a description only after its verdict is `CLEAN`.

## Local configuration

The harness, the models, the commands and the game data directory are specific to the user's machine.
They live in `.agents/skills/clean-room/local/config.md`. Git ignores that directory. Never commit it.

- If the file exists, read it and use it. Use `GAME_DIR` from it below.
- If it doesn't exist, ask the user:
  - Which agent harness (command line tool) to use for the examiner and reviewer sessions.
    It must be able to run a prompt non-interactively, with write access to its work directory.
  - Which model to use for each role. The user knows which models are available and suitable.
  - How to invoke the harness, and where the game data directory is.

  Then write the answers to `local/config.md`. Use `config.example.md` as the template.
- If something doesn't work (rate limit, model unavailable), update the notes in the file.

Don't assume a default for any of these.

## Starting a separate session

The examiner and reviewer must each run in a fresh session that shares nothing with yours.
Use the non-interactive mode of the chosen harness. Run it in the role's work directory and give it the text of `prompt.md`.

- Send all output of the session to nowhere (for example `/dev/null`). Never read or poll it.
- Allow the session to write files in its work directory without asking.
- Wait for the output file to appear. Don't poll the process list with a pattern that matches your own command.

## Steps

1. Pick a scratch directory outside the repository, here `$WORK`. Write a brief to `$WORK/brief_in.md`.
   - State the questions. Be specific. Send only what the question needs.
2. Prepare the examiner:
   ```
   GAME_DIR=<game data> .agents/skills/clean-room/scripts/prepare_examiner.sh $WORK/examiner $WORK/brief_in.md
   ```
3. Start the examiner session in `$WORK/examiner`. Wait for `description.md` to appear. Don't read it.
4. Prepare the reviewer:
   ```
   .agents/skills/clean-room/scripts/prepare_reviewer.sh $WORK/examiner/description.md $WORK/reviewer
   ```
5. Start the reviewer session in `$WORK/reviewer`. Wait for `verdict.txt` to appear.
6. Check the verdict. Don't read the description yet:
   ```
   .agents/skills/clean-room/scripts/check_verdict.sh $WORK/reviewer/verdict.txt
   ```
   - Exit code 0 (`CLEAN`): read `$WORK/examiner/description.md` and implement from it.
   - Exit code 1 (`DIRTY`): don't read the description. Write a new brief that asks for a rewrite and lists only the
     line numbers and categories the script printed. Go back to step 2.
   - Exit code 2: the verdict is missing or malformed. Run the reviewer again.
7. Save the description in `doc/` with a provenance header (see the process file). Commit it before the implementation.

## Rules

- Never run the original program or any part of it, and don't let a session do it. Inspection is static only.
- The scripts refuse to use a directory inside the repository.
- Each work directory gets only the rules of its own role. Never copy the other role's rules into it.
- Don't put the description or its contents into the examiner's brief.
