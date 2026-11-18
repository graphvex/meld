#ifndef MELD_MATERIAL_H
#define MELD_MATERIAL_H

// material/material.h — PBR material record (albedo, normal, ORM, emissive, height).

#include "graphvex.h"

;;CLASSNAME("Material")

typedef struct Material {
    u32 albedoSlot;
    u32 normalSlot;
    u32 ormSlot;
    u32 emissiveSlot;
    u32 heightSlot;
    u32 albedoFactorRGBA;
    f32 roughness;
    f32 metallic;
    f32 emissiveStrength;
    f32 heightScale;
    bool doubleSided;
    u8 _pad[23];
} Material;

_Static_assert(sizeof(Material) == 64, "Material obeys 32-byte multiple (64B)");

#endif
