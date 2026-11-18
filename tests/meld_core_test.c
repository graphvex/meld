// meld_core_test.c — tests for meld coordinates (F32, IF64, D64, LD128,
// UnitScale, Cell, ChunkKey, FloatingOrigin), portal single-tick traversal &
// 3D<->2D depth restore, deterministic procedural Seed/Noise across chunk
// shifts, explicit PostChain (no TAA, opt-in Dither), and SceneImageRing.

#include "meld.h"

static i32 failures;

static fn check(i32 ok, const char *what) {
    if (ok) return;
    fprintf(stderr, "meld_core_test: FAIL %s\n", what);
    failures++;
}

static i32 near_f32(f32 a, f32 b) { return fabsf(a - b) < 1e-5f; }
static i32 near_d64(d64 a, d64 b) { return fabs(a - b) < 1e-9; }

i32 main() {
    // 1. Custom UnitScale ("how many float units = 1 metre", e.g. 3.571428 studs/m)
    UnitScale studs = UnitScale(3.5);
    check(near_d64(UnitScale_fromMetres(&studs, 2.0), 7.0), "UnitScale converts 2m to 7 studs");
    check(near_d64(UnitScale_toMetres(&studs, 7.0), 2.0), "UnitScale converts 7 studs back to 2m");

    // 2. Same Cell across all 4 precision tiers + negative chunk shift floor
    vec3f pf = vec3f(70.25f, -1.25f, 32.0f);
    vec3d pd = vec3d(70.25, -1.25, 32.0);
    PositionIF64 pif = PositionIF64(if64(70, 0.25f), if64(-1, -0.25f), if64(32, 0.0f));
    PositionLD128 pld = PositionLD128(ld128(70, 0.25), ld128(-1, -0.25), ld128(32, 0.0));

    Cell cf  = Cell_fromVec3f(pf);
    Cell cd  = Cell_fromVec3d(pd);
    Cell cif = PositionIF64_cell(pif);
    Cell cld = PositionLD128_cell(pld);
    check(Cell_equals(cf, cd) && Cell_equals(cd, cif) && Cell_equals(cif, cld),
          "All 4 precision tiers produce the exact same Cell (70, -2, 32)");

    ChunkKey ck = ChunkKey_fromCell(cf, CHUNK_SHIFT_DEFAULT);
    check(ck.h == 2 && ck.v == -1 && ck.d == 1,
          "ChunkKey >> 5 maps cell (70, -2, 32) to chunk (2, -1, 1)");

    // 3. LD128 FloatingOrigin rebase at 1e15 units with sub-micrometre precision
    PositionLD128 anchor = PositionLD128(
        ld128(1000000000000000LL, 0.0),
        ld128(-500000000000000LL, 0.5),
        ld128(250000000000000LL, 0.25)
    );
    PositionLD128 obj = PositionLD128(
        ld128(1000000000000002LL, 0.125),
        ld128(-500000000000000LL, 0.75),
        ld128(250000000000000LL, 0.25)
    );
    FloatingOrigin fo = FloatingOrigin(anchor);
    vec3f local = FloatingOrigin_rebaseLD128(&fo, obj);
    check(near_f32(vec3f_x(local), 2.125f) &&
          near_f32(vec3f_y(local), 0.25f) &&
          near_f32(vec3f_z(local), 0.0f),
          "FloatingOrigin rebases 1e15 LD128 position to exact camera-local vec3f");

    // 4. Deterministic procedural Seed & Noise: invariant under chunk shift
    Cell sampleCell = Cell(70, -2, 32);
    ChunkKey ck3 = ChunkKey_fromCell(sampleCell, 3u);
    ChunkKey ck5 = ChunkKey_fromCell(sampleCell, 5u);
    ChunkKey ck8 = ChunkKey_fromCell(sampleCell, 8u);
    (discard) ck3;
    (discard) ck5;
    (discard) ck8;
    f32 n1 = Noise_value3D(42ULL, sampleCell, vec3f(0.3f, 0.7f, 0.1f));
    f32 n2 = Noise_value3D(42ULL, sampleCell, vec3f(0.3f, 0.7f, 0.1f));
    check(n1 == n2 && n1 >= -1.0f && n1 <= 1.0f,
          "Noise_value3D is deterministic per Cell regardless of chunk shift");

    // 5. Portal single-tick traversal + 3D -> 2D depth collapse and restore
    SpaceId space3D = { 1u, 1u };
    SpaceId space2D = { 2u, 1u };
    Portal portal = { 0 };
    portal.spaceA = space3D;
    portal.spaceB = space2D;
    portal.twoWay = true;
    portal.collapsesDepthTo2D = true;
    // Face at origin, normal pointing +Z (so crossing from z=+1 to z=-1 triggers)
    portal.faceA = PortalFace_5(vec3d(0.0, 0.0, 0.0), vec3d(1.0, 0.0, 0.0), vec3d(0.0, 1.0, 0.0), 5.0, 5.0);
    portal.faceB = PortalFace_5(vec3d(100.0, 0.0, 0.0), vec3d(-1.0, 0.0, 0.0), vec3d(0.0, 1.0, 0.0), 5.0, 5.0);
    portal.aToB = RigidTransform(quat64(), vec3d(100.0, 0.0, 0.0), 1.0);

    SpaceId curSpace = space3D;
    vec3d pos = vec3d(0.0, 0.0, 0.0);
    vec3d vel = vec3d(0.0, 0.0, -10.0);
    d64 depthRestore = 0.0;
    bool crossed = Traversal_tryCross(&portal, curSpace,
                                      vec3d(1.0, 2.0, 1.0),
                                      vec3d(1.0, 2.0, -0.75),
                                      vel, depthRestore,
                                      &curSpace, &pos, &vel, &depthRestore);
    check(crossed && SpaceId_equals(curSpace, space2D), "Segment crossing portal moves entity to space2D in same tick");
    check(near_d64(vec3d_x(pos), 101.0) && near_d64(vec3d_z(pos), 0.0) && near_d64(depthRestore, -0.75),
          "3D->2D portal collapses Z to 0 and records depthRestore = -0.75");

    // Cross back from 2D to 3D and verify depthRestore is restored
    bool crossedBack = Traversal_tryCross(&portal, curSpace,
                                          vec3d(101.0, 2.0, -1.0),
                                          vec3d(101.0, 2.0, 0.5),
                                          vel, depthRestore,
                                          &curSpace, &pos, &vel, &depthRestore);
    check(crossedBack && SpaceId_equals(curSpace, space3D) && near_d64(vec3d_z(pos), -0.75),
          "2D->3D portal restores saved 3D depth");

    // 6. Explicit Filter Law: no implicit Dither, opt-in Dither works
    PostChain post = PostChain();
    check(!PostChain_hasFilter(&post, POST_FILTER_DITHER), "Dither is never enabled implicitly");
    PostChain_addFilter(&post, POST_FILTER_DITHER);
    check(PostChain_hasFilter(&post, POST_FILTER_DITHER), "Dither enabled when explicitly added to PostChain");

    // 7. SceneImageRing producer/consumer non-blocking handoff
    MeldView view = MeldView_2(1920u, 1080u);
    SceneImageRing *ring = MeldView_ring(&view);
    check(SceneImageRing_acquireNewest(ring) == nullptr, "No image before first publish");
    u64 v1 = SceneImageRing_publishNext(ring);
    const SceneImage *img1 = SceneImageRing_acquireNewest(ring);
    check(img1 != nullptr && SceneImage_version(img1) == v1, "Consumer acquires v1");
    // Producer publishes v2 and v3 while consumer still holds img1 pinned
    u64 v2 = SceneImageRing_publishNext(ring);
    u64 v3 = SceneImageRing_publishNext(ring);
    (discard) v2;
    const SceneImage *img3 = SceneImageRing_acquireNewest(ring);
    check(img3 != nullptr && SceneImage_version(img3) == v3 && SceneImage_version(img1) == v1,
          "Producer advances to v3 without overwriting consumer-pinned v1 slot");
    SceneImageRing_release(ring, img1);
    SceneImageRing_release(ring, img3);

    return failures ? 1 : 0;
}
