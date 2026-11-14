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
- descriptions of public data formats,
- confidence labels, such as CERTAIN, LIKELY, GUESS and "not determined",
- short messages that the program shows to users.

## Leaks

Flag any of these:

- source code, pseudo-code, assembly or decompiler output, even short fragments,
- function, symbol or class names,
- addresses, offsets and hexadecimal values that look like either,
- structure layouts, field tables and bit-position tables of internal data,
- long verbatim strings from the program,
- text that looks copied from analysis tools.

Check these cases carefully:

- Inline fragments that look like code, such as `a->b`, `x::y`, array indexing or assignments.
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
