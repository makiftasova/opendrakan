# Rules for the examiner

You inspect the original program and write a description of its behavior.
Another person will implement the behavior from your description. They must not learn anything about how the original
program's code looks.

## Setup

- Work in a scratch directory outside the repository.
- Keep working files, notes and transcripts out of the repository.
- Inspect the original program statically only. Don't run it, or any part of it (executables, installers, tools).
  This applies to emulators, compatibility layers and headless displays too.
- Answer only the questions in the brief. Don't describe unrelated parts of the program.
- Write the result to the file named in the brief.

## Writing the description

Write the description in your own words. It must contain only:

- plain English prose,
- plain math: formulas, numeric constants and the order of operations,
- descriptions of public data formats,
- confidence labels.

Label each statement with one of these levels:

| Label | Meaning |
|---|---|
| CERTAIN | You verified it directly. |
| LIKELY | You inferred it from strong evidence. |
| GUESS | It's plausible but unverified. |
| not determined | You couldn't find out. |

Don't guess silently. If you couldn't answer a question, say so.

## Prohibited content

Never put any of these in the description:

- source code, pseudo-code, assembly or decompiler output, even short fragments,
- function, symbol or class names from the original program,
- addresses, offsets and hexadecimal values that look like either,
- structure layouts, field tables and bit-position tables of internal data,
- long verbatim strings from the program (short messages shown to users are fine),
- text copied from analysis tools.

Describe what the program does, not how its code is organized. For example, write "the position is the sum of X and Y, scaled by
the time step", not the expression from the code.

## Check your work

Before you finish, read the whole file once more. Remove anything from the prohibited list.

## Rewrites

You may be asked to rewrite your description because of a problem in it. You get only line numbers and categories.
Remove the flagged content, then re-read the whole file for similar problems.
