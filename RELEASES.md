# CLIADE Release Traceability

This document maps milestone progress to concrete commits and release tags.

## Current repository tag status

- No milestone tags are currently present in this clone (`git tag --list` returned no tags).
- Milestone completion should not be considered release-traceable until a signed tag is created and pushed.

## Milestone commit map (historical anchors)

| Milestone | Intended Tag | Candidate Commit | Evidence |
|---|---|---:|---|
| M1: Core IDE Shell | `v0.0.1-dev` | `ae06874` | Initial baseline commit message explicitly references `v0.0.1-dev`. |
| M2: Vulkan Canvas / image-processing foundation (in progress) | `v0.0.2-dev` | `fc25ac1` | Commit message marks Milestone 2 work-in-progress for image processing + explorer integration. |
| M2.1: Vulkan foundation follow-up | `v0.0.3-dev` | `b6bce56` | Commit message references Vulkan foundation completion against `0.0.3-dev`. |

> Notes
> - Candidate commits are traceability anchors only until signed tags are created.
> - If release managers choose different SHAs, update this table and the matching tag notes together.

## Signed tag procedure (required)

From repository root:

```bash
git checkout main
git pull --ff-only origin main
git tag -s <tag> -m "<message>" main
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
