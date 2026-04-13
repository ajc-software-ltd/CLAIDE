# codex.md — CLIADE Codex Coding Practices

Behavioral guidelines for OpenAI Codex agents to reduce common coding mistakes.
Apply these practices first, then follow project-specific instructions in `AGENTS.md`.

**Tradeoff:** these practices bias toward caution over speed. For trivial tasks, use judgment.

## 1) Think Before Coding

**Do not assume. Do not hide confusion. Surface tradeoffs.**

Before implementing:
- State assumptions explicitly; if uncertain, ask.
- If multiple interpretations exist, present options instead of silently picking one.
- If a simpler approach exists, call it out.
- If requirements are unclear, stop and ask clarifying questions.

## 2) Simplicity First

**Write the minimum code that solves the asked problem.**

- Do not add features beyond scope.
- Do not introduce abstractions for single-use code.
- Do not add configurability/flexibility unless requested.
- Do not add defensive handling for impossible scenarios.
- If a large solution can be much smaller, simplify it.

Sanity check: would a senior engineer call this overcomplicated? If yes, simplify.

## 3) Surgical Changes

**Touch only what is required for the request.**

When editing existing code:
- Avoid unrelated cleanup and refactors.
- Match local style and conventions.
- If unrelated issues are found, note them separately instead of changing them.

When your change creates orphans:
- Remove only imports/variables/functions made unused by your change.
- Do not remove unrelated legacy code unless asked.

Rule of thumb: every changed line should trace directly to user intent.

## 4) Goal-Driven Execution

**Define success criteria and verify them.**

Turn tasks into verifiable goals:
- Bug fix: reproduce -> fix -> verify.
- Validation request: add checks/tests -> make them pass.
- Refactor: preserve behavior -> verify before/after.

For multi-step work, use concise step/check planning:

1. [Step] -> verify: [check]
2. [Step] -> verify: [check]
3. [Step] -> verify: [check]

Strong criteria reduce back-and-forth and prevent speculative coding.

---

These practices are working when diffs are smaller, rewrites are fewer, and clarification happens before implementation.
