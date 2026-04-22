# CLAIDE Release Traceability

This document maps milestone progress to concrete commits and release tags.

## Versioning policy (current)

- Versioning now follows commit-count semantics: `v0.0.<commit_count>-dev`.
- Current baseline version: `v0.0.143-dev`.

## Pre-repo rename checkpoint

- Target checkpoint tag: `v0.0.143-dev`
- Purpose: project-name consistency release after repository rename to `CLAIDE`.
- Branch policy moving forward: `main` only for long-lived development.

## Current repository tag status

- Milestone tags are present in this clone.
- Finalized milestones must use annotated GPG-signed tags that verify locally and are pushed to `origin`.

## Milestone commit map (historical anchors)

| Milestone | Final Tag | Final Commit SHA | Status | Evidence |
|---|---|---:|---|---|
| M1: Core IDE Shell | `v0.0.1-dev` | `ae06874` | ✅ Finalized (signed tag) | Initial baseline commit message explicitly references `v0.0.1-dev`. |
| M2: Vulkan Canvas / image-processing foundation | `v0.0.2-dev` | `fc25ac1` | ⏳ In progress (candidate anchor) | Commit message marks Milestone 2 work-in-progress for image processing + explorer integration. |
| M2.1: Vulkan foundation follow-up | `v0.0.3-dev` | `b6bce56` | ✅ Finalized (signed tag) | Commit message references Vulkan foundation completion against `0.0.3-dev`. |

> Notes
> - Finalized milestones map one signed tag to one immutable commit SHA.
> - Candidate anchors remain provisional until a signed tag is created, verified, and pushed.

## Signed tag procedure (required)

From repository root:

```bash
git checkout main
git pull --ff-only origin main
git tag -s <tag> -m "<message>" <commit_sha>
git tag -v <tag>
git verify-tag <tag>
git push origin main
git push origin <tag>
```

## Release completion criteria

A milestone status can be marked complete only when all are true:

1. `milestones.md` status updated to complete.
2. Signed annotated tag created and signature verified locally.
3. Tag pushed to `origin`.
4. `RELEASES.md` updated with final SHA + tag.
5. PR checklist confirms roadmap + release traceability updates.
