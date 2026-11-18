// proc/proc.c — deterministic Cell-seeded hashes, value noise and Chunk descriptors.

#include "vex.h"
#include "proc/proc.h"

;;OVERVIEW
;;DEFINITION

static u64 splitmix64(u64 x) {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

u64 Seed_forCell(u64 worldSeed, Cell cell, u32 streamId) {
    u64 h = splitmix64(worldSeed ^ (u64) streamId);
    h = splitmix64(h ^ (u64) cell.h);
    h = splitmix64(h ^ (u64) cell.v);
    h = splitmix64(h ^ (u64) cell.d);
    return h;
}

f32 Seed_unitFloat(u64 seed) {
    // 24 bits of mantissa in [0.0f, 1.0f)
    u32 bits = (u32) (seed >> 40);
    return (f32) bits * (1.0f / 16777216.0f);
}

static f32 smoothstep5(f32 t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static f32 latticeValue(u64 worldSeed, i64 h, i64 v, i64 d) {
    u64 s = Seed_forCell(worldSeed, Cell_3(h, v, d), 0u);
    return Seed_unitFloat(s) * 2.0f - 1.0f;
}

f32 Noise_value3D(u64 worldSeed, Cell cell, vec3f localFrac) {
    f32 fx = vec3f_x(localFrac);
    f32 fy = vec3f_y(localFrac);
    f32 fz = vec3f_z(localFrac);
    f32 u = smoothstep5(fx);
    f32 v = smoothstep5(fy);
    f32 w = smoothstep5(fz);

    i64 x0 = cell.h, x1 = cell.h + 1;
    i64 y0 = cell.v, y1 = cell.v + 1;
    i64 z0 = cell.d, z1 = cell.d + 1;

    f32 c000 = latticeValue(worldSeed, x0, y0, z0);
    f32 c100 = latticeValue(worldSeed, x1, y0, z0);
    f32 c010 = latticeValue(worldSeed, x0, y1, z0);
    f32 c110 = latticeValue(worldSeed, x1, y1, z0);
    f32 c001 = latticeValue(worldSeed, x0, y0, z1);
    f32 c101 = latticeValue(worldSeed, x1, y0, z1);
    f32 c011 = latticeValue(worldSeed, x0, y1, z1);
    f32 c111 = latticeValue(worldSeed, x1, y1, z1);

    f32 x00 = c000 + u * (c100 - c000);
    f32 x10 = c010 + u * (c110 - c010);
    f32 x01 = c001 + u * (c101 - c001);
    f32 x11 = c011 + u * (c111 - c011);

    f32 y0v = x00 + v * (x10 - x00);
    f32 y1v = x01 + v * (x11 - x01);

    return y0v + w * (y1v - y0v);
}

Chunk Chunk_3(ChunkKey key, u32 chunkShift, u64 worldSeed) {
    if (chunkShift > 16u) chunkShift = CHUNK_SHIFT_DEFAULT;
    return (Chunk){
        .key = key,
        .chunkShift = chunkShift,
        .edgeCells = 1u << chunkShift,
        .worldSeed = worldSeed,
        .values = nullptr,
        ._pad = 0u
    };
}
