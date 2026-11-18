#ifndef MELD_H
#define MELD_H

// meld.h — the one include door of the meld retained 2D/3D renderer.
//
// Tier: core. Owns coordinates (F32, IF64, D64, LD128, FloatingOrigin,
// UnitScale, Cell, ChunkKey), spaces & portals, entities, scenes, materials,
// terrain, deterministic procedural generation, retained GPU buffers, render
// passes (explicit post filters, no TAA), and the SceneImage producer ring.

#include "vex.h"
#include "graphvex.h"
#include "math/coord_frame.h"

// One Type Registry Law: meld owns 0x0180 - 0x01FF.
#define ID_MELD_COORD_SPACE     0x0180u
#define ID_MELD_FLOATING_ORIGIN 0x0181u
#define ID_MELD_SPACE           0x0182u
#define ID_MELD_PORTAL          0x0183u
#define ID_MELD_ENTITY_TABLE    0x0184u
#define ID_MELD_SCENE           0x0185u
#define ID_MELD_MESH            0x0186u
#define ID_MELD_MESHLET         0x0187u
#define ID_MELD_MATERIAL        0x0188u
#define ID_MELD_TERRAIN         0x0189u
#define ID_MELD_CHUNK           0x018Au
#define ID_MELD_GBUFFER         0x018Bu
#define ID_MELD_POST_CHAIN      0x018Cu
#define ID_MELD_VIEW            0x018Du

#include "coord/coord.h"
#include "space/space.h"
#include "entity/entity.h"
#include "scene/scene.h"
#include "material/material.h"
#include "terrain/terrain.h"
#include "proc/proc.h"
#include "buffer/buffer.h"
#include "render/render.h"
#include "view/view.h"

#endif
