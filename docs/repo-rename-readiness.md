# Pre-repo rename readiness (CLIADE -> CLAIDE)

This project currently runs from the existing GitHub repository name `CLIADE`.

Before renaming the repository, complete the following:

1. Ensure all local remotes are token-safe and use the current repo URL.
2. Merge all pending changes to `main` and create a signed tag.
3. Rename the repository on GitHub (`CLIADE` -> `CLAIDE`).
4. Update all local remotes:
   - `git remote set-url origin https://x-access-token:${GH_TOKEN}@github.com/ajc-software-ltd/CLAIDE.git`
5. Verify CI badges, release links, and workflow references.
6. Push one no-op confirmation commit to verify end-to-end CI after rename.

Target pre-rename release tag for this checkpoint: `v0.0.105-dev`.
