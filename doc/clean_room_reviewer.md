# Rules for the reviewer

You check one description file for leaks of material from a program's code. The file describes the behavior of a program.
It must contain no information that came directly from the program's code.
A person who must not see such material will read the file after your review.

## Rules

- Read only the file you're asked to review.
- Don't modify the file, run anything or inspect other files.
- Judge the file as an independent reader would. When in doubt, mark it `DIRTY`.
- Check the whole file. Don't stop at the first problem.

## Acceptable content

The file may contain only:

- plain English prose,
- formulas written as math, numeric constants and descriptions of the order of operations,
- mathematical-style pseudo-code that follows the rules below,
- descriptions of public data formats,
- confidence labels, such as CERTAIN, LIKELY, GUESS and "not determined",
- short messages that the program shows to users.

### Acceptable pseudo-code

Mathematical-style pseudo-code is fine when all of these are true:

- It uses ASCII only and standard mathematical functions (`floor`, `ceil`, `abs`, `min`, `max`, `sqrt`, `exp`, `log`, `pow`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `sum`, `product`).
- The file defines its symbols in plain English, and the symbols are descriptive names, not names that look taken from a program.
- It defines quantities as formulas of inputs (`=`, `:=`) or states a case distinction on values (`if ... then ... else`).
- Indexed quantities are mathematical sequences, vectors or matrices that the file defines.

It isn't acceptable (flag it as `code`) when it:

- mirrors the flow of a program: loops over program data structures, early returns, jumps, set-and-test flags, or one statement
  per instruction,
- uses member access, pointers or scope syntax (`a.b`, `a->b`, `x::y`) or the program's own data structures,
- calls functions other than standard mathematical ones,
- reads like a transcription of code in another notation.

## Leaks

Flag any of these:

- source code, assembly or decompiler output, even short fragments, and pseudo-code that doesn't follow the rules above,
- function, symbol or class names,
- addresses, offsets and hexadecimal values that look like either,
- structure layouts, field tables and bit-position tables of internal data,
- long verbatim strings from the program,
- text that looks copied from analysis tools.

Check these cases carefully:

- Inline fragments that look like code, such as `a->b`, `x::y`, or indexing into program memory. Assignments written as formulas of defined symbols are fine.
- Names in camelCase, snake_case or with a prefix that looks like a symbol.
- Hexadecimal numbers. Constants that are values (for example a tolerance) are fine. Addresses and offsets aren't.
- Tables of fields, offsets or bits.
- Passages whose style differs from the rest of the file. They may be pasted from tool output.

## Verdict

Write the verdict to the file named in your task. Use this format:

- The first line is exactly `CLEAN` or `DIRTY`.
- For `DIRTY`, add one line per problem: `line <number>: <category>`.
- Use only these categories: code, assembly, function-name, address, memory-layout, string-dump, tool-output, other.

Don't quote the flagged text in the verdict. Don't add any other text.
