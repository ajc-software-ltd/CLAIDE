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
