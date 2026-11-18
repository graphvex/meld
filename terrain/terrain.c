// terrain/terrain.c — bilinear heightmap sampling for terrain rendering and queries.

#include "vex.h"
#include "terrain/terrain.h"

;;OVERVIEW
;;DEFINITION

;;GETTER
static const f32 *Heightmap_samples(const Heightmap *self) {
    return self ? (*self).samples : nullptr;
}
;;GETTER
static u32 Heightmap_edgeSamples(const Heightmap *self) {
    return self ? (*self).edgeSamples : 0u;
}

f32 Heightmap_sampleBilinear(const Heightmap *self, f32 u, f32 v) {
    const f32 *s = Heightmap_samples(self);
    u32 n = Heightmap_edgeSamples(self);
    if (!self || !s || n < 2u) return 0.0f;
    if (u < 0.0f) u = 0.0f;
    if (u > 1.0f) u = 1.0f;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

    f32 maxIdx = (f32) (n - 1u);
    f32 gx = u * maxIdx;
    f32 gy = v * maxIdx;
    u32 x0 = (u32) gx;
    u32 y0 = (u32) gy;
    u32 x1 = (x0 + 1u < n) ? (x0 + 1u) : x0;
    u32 y1 = (y0 + 1u < n) ? (y0 + 1u) : y0;
    f32 tx = gx - (f32) x0;
    f32 ty = gy - (f32) y0;

    f32 h00 = s[y0 * n + x0];
    f32 h10 = s[y0 * n + x1];
    f32 h01 = s[y1 * n + x0];
    f32 h11 = s[y1 * n + x1];

    f32 hx0 = h00 + tx * (h10 - h00);
    f32 hx1 = h01 + tx * (h11 - h01);
    return hx0 + ty * (hx1 - hx0);
}
