# meld — the renderer

> Nothing is implemented yet. Work order: [MANIFEST.md](MANIFEST.md). Laws: [meld-preferences.md](meld-preferences.md).

meld renders 2D and 3D scenes: coordinates, spaces and portals, entities, materials (albedo, normal, ORM, emissive, height), terrain, procedural worlds. It is **retained**: the scene and its GPU buffers persist, and each frame uploads only what changed. Its output is a `SceneImage` that collage places as a rect.

```text
graphvex (api) → collage | meld → darling (UI, incl. the 3D panel)
```

## Deliberate seams

1. **meld renders; it never composites UI or presents.** Output is a `SceneImage`.
2. **Dependencies point down only:** graphvex API, vexspoke, re, lib, rangoon's C ABI. Never collage or darling.
3. **Four precision tiers** — `F32`, `IF64` (i32+f32), `D64`, `LD128` (i64+d64) — chosen per `Space`. The GPU sees `f32` only, rebased against a floating origin.
4. **Portals are graph topology, not curved space.** Deliberate: physics, culling and procedural code stay Euclidean while the world can still loop, nest and stack.
5. **Unit scale is the developer's.** One float unit can be a metre or a stud.
6. **Chunks are `2^shift` cells** (default 32). Changing the shift never changes the generated world.
7. **Shaders are GLSL | MSL | WGSL → SPIR-V.** MSL-only modules are Metal-only by design.
8. **No TAA. Dithering is an explicit filter, never a hidden cheat.**
9. **Queries only.** Collision response belongs to ramon (physics).
10. **Deliberate gaps:** lights and shadows do not cross portals in v1; ray tracing only behind a `Capability`.
