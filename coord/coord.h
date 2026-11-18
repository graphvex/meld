#ifndef MELD_COORD_H
#define MELD_COORD_H

// coord/coord.h — precision tiers (F32, IF64, D64, LD128), UnitScale, Cell,
// ChunkKey, CoordSpace, and FloatingOrigin.

#include "vex.h"
#include "math/coord_frame.h"

#define CHUNK_SHIFT_DEFAULT 5u
#define UNIT_SCALE_DEFAULT  1.0

// ---------------------------------------------------------------- CoordPrecision

;;CLASSNAME("CoordPrecision")

typedef enum CoordPrecision {
    COORD_PRECISION_F32   = 0,
    COORD_PRECISION_IF64  = 1,
    COORD_PRECISION_D64   = 2,
    COORD_PRECISION_LD128 = 3,
    COORD_PRECISION_COUNT = 4
} CoordPrecision;

const char *CoordPrecision_name(CoordPrecision tier);
bytes CoordPrecision_positionStride(CoordPrecision tier);

// ---------------------------------------------------------------- UnitScale

;;CLASSNAME("UnitScale")

typedef struct UnitScale {
    d64 unitsPerMetre;
    d64 metresPerUnit;
    u64 _pad[2];
} UnitScale;

_Static_assert(sizeof(UnitScale) == 32, "UnitScale obeys 32-byte floor");

UnitScale UnitScale_0();
UnitScale UnitScale_1(d64 unitsPerMetre);
#define UnitScale(...) CONSTRUCTOR_DISPATCH(UnitScale, __VA_ARGS__)

;;GETTER
d64 UnitScale_unitsPerMetre(const UnitScale *self);
;;GETTER
d64 UnitScale_metresPerUnit(const UnitScale *self);

d64 UnitScale_fromMetres(const UnitScale *self, d64 metres);
d64 UnitScale_toMetres(const UnitScale *self, d64 sceneUnits);

// ---------------------------------------------------------------- Cell & ChunkKey

;;CLASSNAME("Cell")

typedef struct Cell {
    i64 h;
    i64 v;
    i64 d;
    i64 _pad;
} Cell;

_Static_assert(sizeof(Cell) == 32, "Cell obeys 32-byte floor");

Cell Cell_0();
Cell Cell_3(i64 h, i64 v, i64 d);
#define Cell(...) CONSTRUCTOR_DISPATCH(Cell, __VA_ARGS__)

Cell Cell_fromVec3f(vec3f p);
Cell Cell_fromVec3d(vec3d p);
bool Cell_equals(Cell a, Cell b);

;;CLASSNAME("ChunkKey")

typedef struct ChunkKey {
    i64 h;
    i64 v;
    i64 d;
    i64 _pad;
} ChunkKey;

_Static_assert(sizeof(ChunkKey) == 32, "ChunkKey obeys 32-byte floor");

ChunkKey ChunkKey_fromCell(Cell cell, u32 chunkShift);
bool ChunkKey_equals(ChunkKey a, ChunkKey b);

// ---------------------------------------------------------------- PositionIF64 & PositionLD128

;;CLASSNAME("PositionIF64")

typedef struct PositionIF64 {
    if64 h;
    if64 v;
    if64 d;
    if64 _pad;
} PositionIF64;

_Static_assert(sizeof(PositionIF64) == 32, "PositionIF64 obeys 32-byte floor");

PositionIF64 PositionIF64_0();
PositionIF64 PositionIF64_3(if64 h, if64 v, if64 d);
#define PositionIF64(...) CONSTRUCTOR_DISPATCH(PositionIF64, __VA_ARGS__)

PositionIF64 PositionIF64_add(PositionIF64 a, PositionIF64 b);
PositionIF64 PositionIF64_sub(PositionIF64 a, PositionIF64 b);
Cell         PositionIF64_cell(PositionIF64 self);
vec3f        PositionIF64_toVec3fRelative(PositionIF64 self, PositionIF64 origin);

;;CLASSNAME("PositionLD128")

typedef struct PositionLD128 {
    ld128 h;
    ld128 v;
    ld128 d;
    ld128 _pad;
} PositionLD128;

_Static_assert(sizeof(PositionLD128) == 64, "PositionLD128 obeys 32-byte multiple (64B)");

PositionLD128 PositionLD128_0();
PositionLD128 PositionLD128_3(ld128 h, ld128 v, ld128 d);
#define PositionLD128(...) CONSTRUCTOR_DISPATCH(PositionLD128, __VA_ARGS__)

PositionLD128 PositionLD128_fromIF64(PositionIF64 p);
PositionLD128 PositionLD128_fromVec3d(vec3d p);
PositionLD128 PositionLD128_add(PositionLD128 a, PositionLD128 b);
PositionLD128 PositionLD128_sub(PositionLD128 a, PositionLD128 b);
Cell          PositionLD128_cell(PositionLD128 self);
vec3d         PositionLD128_toVec3dRelative(PositionLD128 self, PositionLD128 origin);
vec3f         PositionLD128_toVec3fRelative(PositionLD128 self, PositionLD128 origin);

// ---------------------------------------------------------------- CoordSpace & FloatingOrigin

;;CLASSNAME("CoordSpace")

typedef struct CoordSpace {
    CoordPrecision precision;
    CoordFrame frame;
    u32 dimension;
    u32 chunkShift;
    UnitScale unitScale;
    u64 _pad[2];
} CoordSpace;

_Static_assert(sizeof(CoordSpace) == 64, "CoordSpace obeys 32-byte multiple (64B)");

CoordSpace CoordSpace_0();
CoordSpace CoordSpace_4(CoordPrecision precision, CoordFrame frame, u32 dimension, d64 unitsPerMetre);
#define CoordSpace(...) CONSTRUCTOR_DISPATCH(CoordSpace, __VA_ARGS__)

;;CLASSNAME("FloatingOrigin")

typedef struct FloatingOrigin {
    PositionLD128 anchor;
} FloatingOrigin;

_Static_assert(sizeof(FloatingOrigin) == 64, "FloatingOrigin obeys 32-byte multiple (64B)");

FloatingOrigin FloatingOrigin_0();
FloatingOrigin FloatingOrigin_1(PositionLD128 anchor);
#define FloatingOrigin(...) CONSTRUCTOR_DISPATCH(FloatingOrigin, __VA_ARGS__)

vec3f FloatingOrigin_rebaseLD128(const FloatingOrigin *self, PositionLD128 worldPos);
vec3f FloatingOrigin_rebaseIF64(const FloatingOrigin *self, PositionIF64 worldPos);
vec3f FloatingOrigin_rebaseD64(const FloatingOrigin *self, vec3d worldPos);

#endif
