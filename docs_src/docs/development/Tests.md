---
version: "2.13"
---

# Test Plans

![Status](https://img.shields.io/badge/Status-Index-lightgrey) ![Pages](https://img.shields.io/badge/Pages-1-blue)

Test plans live beside the specifications in `docs_src/docs/development/`. This
page is the index: **new test plans are added here, not to the sidebar.**

A test plan is named after the specification it verifies, with `Spec` replaced by
`Test` — `SpecStorageIdentity.md` is tested by `TestStorageIdentity.md`.

| Page | Subject | Specification | Cases |
|------|---------|---------------|-------|
| [TestStorageIdentity](TestStorageIdentity.md) | TEST Storage Identity | [SpecStorageIdentity](SpecStorageIdentity.md) | 22 |

---

## Conventions

**These apply to every test plan. An individual plan does not repeat them** — it
opens with a link to its specification and goes straight to its test cases.

- **Every requirement is cited by at least one case**, and every case cites the
  requirement it verifies.

- **Every test case cites the requirement it verifies**, by ID. A case citing no
  requirement tests something nobody asked for; a requirement no case cites is
  untested. Both are visible at a glance in the tables.
- Test case IDs mirror the specification prefix with a `T`: `STI-T1`, `STI-T2`…
  They are **permanent**: never renumbered, never reused, retired as `[Removed]`
  rather than deleted.
- A case states what to set up, what to do, and what must be true afterwards —
  enough to run it without reading the source.
- Each plan ends with a **Coverage** table naming any requirement with no case,
  and why, so the gap is deliberate rather than accidental.
- Pass/fail results belong here, never in the specification.
- Test plans are **English only**.
