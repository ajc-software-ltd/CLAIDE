# Branch and Merge Policy

## Core branches
- `main`: stable/release-only
- `work`: integration branch

## Runtime branches
- `vulkan-runtime`
- `ffmpeg-runtime`
- `miniaudio-runtime`
- `wxwidgets-runtime` (optional)

## Merge flow
1. runtime branch -> `work`
2. integration validation on `work`
3. `work` -> `main` only on explicit approval

## Promotion checks
- configure/build succeeds
- test suite passes
- runtime diagnostics report healthy module state

## Required validation gate commands

Before merging into `main`, run:

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
- Lint gate thresholds are enforced by `scripts/run_lint.sh` through `scripts/lint_unique_report.py`:
  - `LINT_MAX_BUCKET_A` (default `0`)
  - `LINT_MAX_SRC_BUCKET_B` (default `24`, current baseline lock to prevent regressions)
- Local override is allowed only for troubleshooting (e.g. `LINT_MAX_SRC_BUCKET_B=<value>`), but CI/merge gates must use default thresholds.
