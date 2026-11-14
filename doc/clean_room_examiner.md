# Rules for the examiner

You inspect the original program and write a description of its behavior.
Another person will implement the behavior from your description. They must not learn anything about how the original
program's code looks.

## Setup

- Work in a scratch directory outside the repository.
- Keep working files, notes and transcripts out of the repository.
- Use only the original program and the game files. Never read the re-implementation: its source code, documentation,
  git history, notes or build output. Values from there are not evidence about the original program.
- Inspect the original program statically only. Don't run it, or any part of it (executables, installers, tools).
  This applies to emulators, compatibility layers and headless displays too.
- Answer only the questions in the brief. Don't describe unrelated parts of the program.
- Write the result to the file named in the brief.

## Writing the description

Write the description in your own words. It must contain only:

- plain English prose,
- plain math: formulas, numeric constants and the order of operations,
- mathematical-style pseudo-code (see below),
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

### Mathematical-style pseudo-code

A short formula or a few numbered formulas often say more than a paragraph of prose. You may use
[mathematical-style pseudo-code](https://en.wikipedia.org/wiki/Pseudocode#Mathematical_style_pseudocode) under these rules:

- Use ASCII only. Write `>=`, `<=`, `!=`, `floor(x)`, `ceil(x)`, `abs(x)`, `min(a, b)`, `max(a, b)`, `sqrt(x)`, `sin(x)`, `cos(x)`
  and `sum(...)`.
- Define every symbol in plain English before you use it. Choose short, descriptive symbols of your own, such as `speed`,
  `pitch` or `t_next`. Never use names that you saw in the program.
- Describe results as functions of inputs: `v_new = v_old + accel * dt`. Use `=` to define a quantity and `:=` to update one.
- Use `if ... then ... else` and `case` only to express a mathematical case distinction on values.
- Number the steps if the order matters.
- Subscripts and indexed quantities are fine for mathematical sequences, vectors and matrices that you define yourself.

The pseudo-code must describe behavior, not the structure of the code. Don't write:

- statements that mirror the flow of the program: loops over the program's data structures, early returns, jumps, flags that
  are set and tested, or one statement per instruction,
- member access, pointers or scope syntax (`a.b`, `a->b`, `x::y`) or the program's own data structures,
- calls to anything other than the standard mathematical functions above,
- anything that is a transcription of code, even if you translated it to another notation.

If a formula looks like what the code does, step by step, describe the result instead.

## Prohibited content

Never put any of these in the description:

- source code, assembly or decompiler output, even short fragments, and pseudo-code that doesn't follow the rules above,
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
