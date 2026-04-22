# Branch and Merge Policy

## Core branch
- `main`: the only branch used for project commits.

## Commit flow
1. apply approved changes on `main`
2. validate locally
3. push `main`

## Promotion checks
- configure/build succeeds
- test suite passes
- runtime diagnostics report healthy module state

## Required validation gate commands

Before pushing `main`, run:

```bash
./scripts/build_with_prereqs.sh Debug on
cmake --build build --target format-check
cmake --build build --target lint
ctest --test-dir build --output-on-failure
```

If any gate cannot run due to environment constraints, include the exact failure and remediation plan in the PR.

Lint note:
- `modernize-use-std-print` is currently disabled in `.clang-tidy` due to a reproducible clang-tidy crash on project sources.

Lint triage policy:
- Prioritize unique diagnostics, not raw totals.
- Group by `file:line:check` (see `build/reports/clang-tidy.unique.txt` and `clang-tidy.summary.md`).
- Fix in order: tool/parser errors first, then `clang-analyzer-*` and `bugprone-*`, then other checks.
- Track readability warnings, but do not prioritize them ahead of correctness and analyzer findings.
- Test lint excludes `bugprone-chained-comparison` to avoid Catch2 macro decomposition noise; this exclusion applies to tests only.
