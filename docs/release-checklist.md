# Release & Milestone Checklist

Use this checklist whenever milestone status or release metadata changes.

Branch policy note:
- Use `main` for release integration and `minidocx` only for isolated DOCX stream work.
- Commit directly to `main` for approved changes unless an explicit `minidocx` stream task is active.
- Do not create or use `work` branch names.

## Milestone traceability checklist

- [ ] `milestones.md` reflects the new status and milestone description.
- [ ] A candidate commit SHA is identified for the milestone.
- [ ] A signed annotated tag is created on `main`.
- [ ] Tag signature is verified with both `git tag -v` and `git verify-tag`.
- [ ] `RELEASES.md` is updated with milestone → SHA → tag mapping.
- [ ] Pushes completed: `git push origin main` and `git push origin <tag>`.

## PR review checklist additions

- [ ] If a milestone status changed, release traceability updates are included in this PR.
- [ ] If release information changed, `README.md` and `milestones.md` are consistent.
- [ ] User-facing milestone messaging (dialogs/status text) matches roadmap terms.
