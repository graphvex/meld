# meld — Repo-Local Living Preferences

## Constitution Link

gist.md (`../../../../metadata/gists/preferences/gist.md`) is the compressed law list; all universal laws bind this repository. Read one on demand: `./tools/prefs law "<title>"`. Universal laws are inherited, never restated.

## Tier and Ownership

**Tier:** `core` — the retained renderer.
**Owns:** coordinates (precision tiers, floating origin, units, chunks), spaces and portals, entities, scenes, materials, terrain, procedural generation, `Mesh`/`Meshlet`, render passes, the scene-image output.
**Does not own:** the GPU API (graphvex), UI composition (collage), UI widgets (darling), physics response (ramon), the scalar and vector floor (lib), allocation (re), persistence storage (rangoon).
Includes point down only: graphvex API, vexspoke, re, lib, rangoon's C ABI. Never collage, darling, hotcwap or driver headers.

## Directory Headers

* One `.h` per directory, named after the directory; the repo root header is `meld.h`.

## Exclusive Laws

| Law Title | Scope | Enforcement |
| **Precision Tier Law** | `coord/` | Mandatory for `meld` |
| **Floating Origin Law** | `coord/`, `render/` | Mandatory for `meld` |
| **Portal Topology Law** | `space/` | Mandatory for `meld` |
| **Deterministic Procedural Law** | `proc/`, `terrain/` | Mandatory for `meld` |
| **Scene Image Producer Law** | `view/` | Mandatory for `meld` |
| **Explicit Filter Law** | `render/` | Mandatory for `meld` |

### Precision Tier Law

* Four tiers: `F32` (`vec3f`), `IF64` (`if64` = i32+f32), `D64` (`d64`), `LD128` (`ld128` = i64+d64). The floor owns the scalar types; meld composes them and never redefines them.
* A tier is chosen per `Space` at cold construction and never changes.
* Widening is explicit; narrowing is a named call that reports out-of-range and never wraps or truncates silently.
* The integer half is the cell. A chunk is `2^chunkShift` cells (default 5); the key is `Cell >> chunkShift`.
* Rows pad to the 32-byte floor; padding is zeroed and named `_pad`.
* Unit scale is developer-defined (`UnitScale`, units per metre, default 1.0); metric constants pass through it, never raw.
* Axes are `CoordFrame`-agnostic; transfers use the basis query, never hand-written sign flips.

### Floating Origin Law

* The GPU only ever sees `f32`.
* Any higher tier is rebased against the view's `FloatingOrigin` before upload; the subtraction happens in the high tier.

### Portal Topology Law

* Every `Space` is flat Euclidean. A `Portal` pairs two oriented rects by a rigid transform plus optional scale.
* Non-Euclidean behaviour comes only from the graph of spaces; no metric is curved.
* A crossing is resolved in one tick: the entity is never in neither or both spaces.
* Recursive portal views are capped (`PORTAL_RECURSION_MAX`); at the cap a flat fallback is drawn, never nothing.

### Deterministic Procedural Law

* Output is a pure function of `(worldSeed, Cell, local coordinate)`; precision tier, chunk shift, machine and load order never change a cell's value.
* GPU twins match CPU output within a stated tolerance and sit behind a `Capability` gate.

### Scene Image Producer Law

* meld publishes a `SceneImage` (api `Image` + version + ready fence) per `MeldView` into a 3-slot ring.
* Consumers take the newest completed image and never make meld wait; an image outlives its last consumer, including across resize and close.
* The image is linear-light premultiplied RGBA (RGBA16F by default).
* meld never presents and never composites UI.
* meld is retained: persistent GPU buffers, per-frame delta uploads only.

### Explicit Filter Law

* No TAA; nothing temporal or jittered is hidden in the renderer.
* Dithering is a deliberate `Dither` filter in the ordered post chain; meld never dithers on its own and never uses noise to mask an artifact.

## Repo-Local Extensions

* None — no managed exceptions to universal laws.

## Readiness Cross-Reference

* Feature readiness: meld readiness gist.
* Open blockers: ecosystem blockers gist.
* Proof lawbook: `tests/test-preferences.md` — `./tools/prefs test "<title>"`.
* Evidence ledger: `metadata/gists/test-checklist.md` — `python3 tools/test_checklist.py check`.
