# Commit messages

Follow the [Google developer documentation style guide](https://developers.google.com/style).
Use short sentences and active voice. Keep paragraphs to five or six sentences.

## Subject line

- Use the imperative mood: "Fix the data race", not "Fixed the data race".
- Keep it to 50 characters or fewer.
- Don't end it with a period.

## Body

- Separate the body from the subject with a blank line.
- Explain what was wrong and what the change does. Don't repeat the diff.
- Use a bulleted list for several independent changes.

## Trailers

Don't add `Co-Authored-By` trailers.

## Organizing commits

- Make each commit one meaningful change.
- Squash fixups into the commit they fix.
- Put a specification before the change that implements it.
- Rebase onto `upstream/master` instead of merging.
