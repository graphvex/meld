#ifndef MELD_PROC_H
#define MELD_PROC_H

// proc/proc.h — deterministic procedural generation (Seed, Rng, Noise, Chunk).
//
// Deterministic Procedural Law: output is a pure function of
// (worldSeed, Cell, localCoordinate); precision tier, chunk shift, machine and
// load order never change a cell's value.

#include "coord/coord.h"

;;CLASSNAME("Seed")

u64 Seed_forCell(u64 worldSeed, Cell cell, u32 streamId);
f32 Seed_unitFloat(u64 seed);

;;CLASSNAME("Noise")

// 3D value noise in [-1, 1] evaluated at (cell + localFrac in [0, 1)^3).
f32 Noise_value3D(u64 worldSeed, Cell cell, vec3f localFrac);

;;CLASSNAME("Chunk")

typedef struct Chunk {
    ChunkKey key;
    u32 chunkShift;
    u32 edgeCells;
    u64 worldSeed;
    f32 *values;
    u64 _pad;
} Chunk;

_Static_assert(sizeof(Chunk) == 64, "Chunk obeys 32-byte multiple (64B)");

Chunk Chunk_3(ChunkKey key, u32 chunkShift, u64 worldSeed);

#endif
