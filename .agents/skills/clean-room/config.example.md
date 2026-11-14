# Clean-room skill: local configuration (template)

Copy this file to `local/config.md` and fill it in. Everything under `local/` is ignored by git.
Never commit it. It describes your machine, not the project.

## Game data

- `GAME_DIR`: `<absolute path of the directory with the game data>`

## Examiner

- Harness: `<name of the agent harness>`
- Model: `<model name as the harness expects it>`
- Command, run in the examiner work directory. `<prompt>` stands for the text of `prompt.md`:

  ```
  <command that runs <prompt> non-interactively, writes files without asking and discards its output>
  ```

## Reviewer

- Harness: `<name of the agent harness>`
- Model: `<model name>`
- Command, run in the reviewer work directory:

  ```
  <command>
  ```

## Notes

- `<anything else worth remembering, such as rate limits, models that didn't work, timeouts>`
