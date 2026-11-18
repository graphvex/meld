#ifndef MELD_TERRAIN_H
#define MELD_TERRAIN_H

// terrain/terrain.h — chunk-aligned heightmaps (2^chunkShift + 1 shared-border
// samples), TerrainTile, and bilinear CPU height query.

#include "coord/coord.h"

;;CLASSNAME("Heightmap")

typedef struct Heightmap {
    const f32 *samples;
    u32 edgeSamples;
    f32 minHeight;
    f32 maxHeight;
    u32 _pad[3];
} Heightmap;

_Static_assert(sizeof(Heightmap) == 32, "Heightmap obeys 32-byte floor");

// Returns bilinearly interpolated height for normalized tile UV in [0, 1].
f32 Heightmap_sampleBilinear(const Heightmap *self, f32 u, f32 v);

;;CLASSNAME("TerrainTile")

typedef struct TerrainTile {
    ChunkKey key;
    Heightmap heightmap;
    u32 normalMapSlot;
    u32 splatMapSlot;
    u64 _pad[3];
} TerrainTile;

_Static_assert(sizeof(TerrainTile) == 96, "TerrainTile obeys 32-byte multiple (96B)");

#endif
