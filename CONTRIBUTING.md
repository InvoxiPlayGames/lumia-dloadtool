# Contributor Guide

## Testing

This is a tool that messes with low-level firmware on a device. Unexpected data
sent to the device can cause unexpected crashes or, worse, can **brick** a
connected device. Depending on what is being changed, you're expected to do
**extensive** tests on the relevant changed functionality to ensure it does not
do this.

## Reverse Engineering

Much of the information in this codebase was obtained by reverse engineering the
Nokia DLOAD firmware dumped from a device. If you'd like a Ghidra project for
the bootloader, please reach out and I can provide one with relevantly labelled
functions and structures.

## Code Quality

Please try to keep the code style consistent with the surrounding code. I
haven't done this myself admittedly but it would be nice if you tried.

## LLM Policy

Responsible usage of LLMs to contribute code into lumia-dloadtool is highly
discouraged, but allowed. When working on and submitting pull requests to
lumia-dloadtool using Large Language Models (or "Generative AI"), please keep
the following in mind:

* **Purely "agentic" PRs with no human involvement are strictly forbidden and
    will be rejected.**
* **All human-facing output must be written by a human.** No command line
  output, GUI text or README/markdown file changes can be done with an LLM.
* When making the PR, you must at minimum disclose that LLMs were used in their
  creation, as well as the following:
    * Which model/service was used
    * How much the LLM was relied on (whether only to provide a few lines, or
      if it was entirely prompt driven / "vibe coded", for example)
    * Ideally, the commit message would also note the LLM usage.
* You must ensure the code builds across all supported platforms and does the
  intended behaviour.
* You're expected to be able to point to any given line of code in your own
  change and explain why a certain decision was made - the answer should not be
  "IDK, the AI did it."
* LLMs are known to emit some amounts of code from copyrighted projects. If we
  suspect the output has enough copyrightable code from an incompatibly licensed
  project, we won't accept it.
* LLMs can *not* be used to make large changes (e.g. "cleanups") to the codebase
  unprompted. The only changed code must be directly relevant to the PR.
* New files designed to assist with specific proprietary and closed models (e.g.
  `CLAUDE.md`) will be rejected.

If mistakes are made in earnest, and there is clear human intent behind the
change, you'll have more than enough chance to make it right.
