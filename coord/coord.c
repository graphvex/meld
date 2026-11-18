// coord/coord.c — precision tiers, UnitScale, Cell, ChunkKey, and FloatingOrigin.

#include "vex.h"
#include "coord/coord.h"

;;OVERVIEW
;;DEFINITION

const char *CoordPrecision_name(CoordPrecision tier) {
    switch (tier) {
        case COORD_PRECISION_F32:   return "F32";
        case COORD_PRECISION_IF64:  return "IF64";
        case COORD_PRECISION_D64:   return "D64";
        case COORD_PRECISION_LD128: return "LD128";
        default:                    return "UNKNOWN";
    }
}

bytes CoordPrecision_positionStride(CoordPrecision tier) {
    switch (tier) {
        case COORD_PRECISION_F32:
        case COORD_PRECISION_IF64:
        case COORD_PRECISION_D64:   return 32u;
        case COORD_PRECISION_LD128: return 64u;
        default:                    return 32u;
    }
}

// ---------------------------------------------------------------- UnitScale

UnitScale UnitScale_1(d64 unitsPerMetre) {
    if (!isfinite(unitsPerMetre) || unitsPerMetre <= 0.0) {
        THROW("UnitScale: invalid unitsPerMetre %g (must be finite and > 0)", unitsPerMetre);
        unitsPerMetre = UNIT_SCALE_DEFAULT;
    }
    UnitScale s = { 0 };
    s.unitsPerMetre = unitsPerMetre;
    s.metresPerUnit = 1.0 / unitsPerMetre;
    return s;
}

UnitScale UnitScale_0() {
    return UnitScale_1(UNIT_SCALE_DEFAULT);
}

;;GETTER
d64 UnitScale_unitsPerMetre(const UnitScale *self) {
    return self ? (*self).unitsPerMetre : UNIT_SCALE_DEFAULT;
}
;;GETTER
d64 UnitScale_metresPerUnit(const UnitScale *self) {
    return self ? (*self).metresPerUnit : 1.0;
}

d64 UnitScale_fromMetres(const UnitScale *self, d64 metres) {
    return metres * UnitScale_unitsPerMetre(self);
}

d64 UnitScale_toMetres(const UnitScale *self, d64 sceneUnits) {
    return sceneUnits * UnitScale_metresPerUnit(self);
}

// ---------------------------------------------------------------- Cell & ChunkKey

Cell Cell_3(i64 h, i64 v, i64 d) {
    return (Cell){ .h = h, .v = v, .d = d, ._pad = 0 };
}

Cell Cell_0() {
    return Cell_3(0, 0, 0);
}

static i64 floorToI64(d64 x) {
    d64 f = floor(x);
    return (i64) f;
}

Cell Cell_fromVec3f(vec3f p) {
    return Cell_3(floorToI64((d64) vec3f_x(p)),
                  floorToI64((d64) vec3f_y(p)),
                  floorToI64((d64) vec3f_z(p)));
}

Cell Cell_fromVec3d(vec3d p) {
    return Cell_3(floorToI64(vec3d_x(p)),
                  floorToI64(vec3d_y(p)),
                  floorToI64(vec3d_z(p)));
}

bool Cell_equals(Cell a, Cell b) {
    return a.h == b.h && a.v == b.v && a.d == b.d;
}

ChunkKey ChunkKey_fromCell(Cell cell, u32 chunkShift) {
    if (chunkShift > 62u) chunkShift = CHUNK_SHIFT_DEFAULT;
    return (ChunkKey){
        .h = cell.h >> chunkShift,
        .v = cell.v >> chunkShift,
        .d = cell.d >> chunkShift,
        ._pad = 0
    };
}

bool ChunkKey_equals(ChunkKey a, ChunkKey b) {
    return a.h == b.h && a.v == b.v && a.d == b.d;
}

// ---------------------------------------------------------------- PositionIF64

PositionIF64 PositionIF64_3(if64 h, if64 v, if64 d) {
    return (PositionIF64){ .h = h, .v = v, .d = d, ._pad = if64_0() };
}

PositionIF64 PositionIF64_0() {
    return PositionIF64_3(if64_0(), if64_0(), if64_0());
}

PositionIF64 PositionIF64_add(PositionIF64 a, PositionIF64 b) {
    return PositionIF64_3(if64_add(a.h, b.h), if64_add(a.v, b.v), if64_add(a.d, b.d));
}

PositionIF64 PositionIF64_sub(PositionIF64 a, PositionIF64 b) {
    return PositionIF64_3(if64_sub(a.h, b.h), if64_sub(a.v, b.v), if64_sub(a.d, b.d));
}

static i64 cellAxisIF64(if64 x) {
    i64 i = (i64) if64_integer(x);
    f32 f = if64_fraction(x);
    return (f < 0.0f) ? (i - 1) : i;
}

Cell PositionIF64_cell(PositionIF64 self) {
    return Cell_3(cellAxisIF64(self.h), cellAxisIF64(self.v), cellAxisIF64(self.d));
}

vec3f PositionIF64_toVec3fRelative(PositionIF64 self, PositionIF64 origin) {
    return vec3f_3(
        if64_toF32Relative(self.h, origin.h),
        if64_toF32Relative(self.v, origin.v),
        if64_toF32Relative(self.d, origin.d)
    );
}

// ---------------------------------------------------------------- PositionLD128

PositionLD128 PositionLD128_3(ld128 h, ld128 v, ld128 d) {
    return (PositionLD128){ .h = h, .v = v, .d = d, ._pad = ld128_0() };
}

PositionLD128 PositionLD128_0() {
    return PositionLD128_3(ld128_0(), ld128_0(), ld128_0());
}

PositionLD128 PositionLD128_fromIF64(PositionIF64 p) {
    return PositionLD128_3(
        ld128_2((i64) if64_integer(p.h), (d64) if64_fraction(p.h)),
        ld128_2((i64) if64_integer(p.v), (d64) if64_fraction(p.v)),
        ld128_2((i64) if64_integer(p.d), (d64) if64_fraction(p.d))
    );
}

PositionLD128 PositionLD128_fromVec3d(vec3d p) {
    return PositionLD128_3(
        ld128_fromD64(vec3d_x(p)),
        ld128_fromD64(vec3d_y(p)),
        ld128_fromD64(vec3d_z(p))
    );
}

PositionLD128 PositionLD128_add(PositionLD128 a, PositionLD128 b) {
    return PositionLD128_3(ld128_add(a.h, b.h), ld128_add(a.v, b.v), ld128_add(a.d, b.d));
}

PositionLD128 PositionLD128_sub(PositionLD128 a, PositionLD128 b) {
    return PositionLD128_3(ld128_sub(a.h, b.h), ld128_sub(a.v, b.v), ld128_sub(a.d, b.d));
}

static i64 cellAxisLD128(ld128 x) {
    i64 i = ld128_integer(x);
    d64 f = ld128_fraction(x);
    return (f < 0.0) ? (i - 1) : i;
}

Cell PositionLD128_cell(PositionLD128 self) {
    return Cell_3(cellAxisLD128(self.h), cellAxisLD128(self.v), cellAxisLD128(self.d));
}

vec3d PositionLD128_toVec3dRelative(PositionLD128 self, PositionLD128 origin) {
    return vec3d_3(
        ld128_toD64Relative(self.h, origin.h),
        ld128_toD64Relative(self.v, origin.v),
        ld128_toD64Relative(self.d, origin.d)
    );
}

vec3f PositionLD128_toVec3fRelative(PositionLD128 self, PositionLD128 origin) {
    return vec3f_3(
        ld128_toF32Relative(self.h, origin.h),
        ld128_toF32Relative(self.v, origin.v),
        ld128_toF32Relative(self.d, origin.d)
    );
}

// ---------------------------------------------------------------- CoordSpace & FloatingOrigin

CoordSpace CoordSpace_4(CoordPrecision precision, CoordFrame frame, u32 dimension, d64 unitsPerMetre) {
    if (dimension != 2u && dimension != 3u) dimension = 3u;
    if (!CoordFrame_isValid((u32) frame)) frame = COORD_FRAME_DEFAULT;
    return (CoordSpace){
        .precision = precision,
        .frame = frame,
        .dimension = dimension,
        .chunkShift = CHUNK_SHIFT_DEFAULT,
        .unitScale = UnitScale_1(unitsPerMetre),
        ._pad = { 0, 0 }
    };
}

CoordSpace CoordSpace_0() {
    return CoordSpace_4(COORD_PRECISION_IF64, COORD_FRAME_DEFAULT, 3u, UNIT_SCALE_DEFAULT);
}

FloatingOrigin FloatingOrigin_1(PositionLD128 anchor) {
    return (FloatingOrigin){ .anchor = anchor };
}

FloatingOrigin FloatingOrigin_0() {
    return FloatingOrigin_1(PositionLD128_0());
}

;;GETTER
static PositionLD128 FloatingOrigin_anchor(const FloatingOrigin *self) {
    return self ? (*self).anchor : PositionLD128_0();
}

vec3f FloatingOrigin_rebaseLD128(const FloatingOrigin *self, PositionLD128 worldPos) {
    PositionLD128 anchor = FloatingOrigin_anchor(self);
    return PositionLD128_toVec3fRelative(worldPos, anchor);
}

vec3f FloatingOrigin_rebaseIF64(const FloatingOrigin *self, PositionIF64 worldPos) {
    return FloatingOrigin_rebaseLD128(self, PositionLD128_fromIF64(worldPos));
}

vec3f FloatingOrigin_rebaseD64(const FloatingOrigin *self, vec3d worldPos) {
    return FloatingOrigin_rebaseLD128(self, PositionLD128_fromVec3d(worldPos));
}
