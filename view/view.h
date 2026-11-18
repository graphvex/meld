#ifndef MELD_VIEW_H
#define MELD_VIEW_H

// view/view.h — SceneImage, 3-slot SceneImageRing and MeldView producer handoff.
//
// Scene Image Producer Law / Independent Scene Cadence Law: meld publishes
// completed linear-premultiplied RGBA16F SceneImages into a 3-slot ring;
// consumers acquire the newest completed image without blocking meld.

#include "graphvex.h"
#include "coord/coord.h"
#include "render/render.h"

#define SCENE_IMAGE_RING_SLOTS 3u

;;CLASSNAME("SceneImage")

typedef struct SceneImage {
    Image image;
    Fence ready;
    u64 version;
    u32 refCount;
    bool completed;
    u8 _pad[19];
} SceneImage;

_Static_assert(sizeof(SceneImage) == 96, "SceneImage obeys 32-byte multiple (96B)");

;;CLASSNAME("SceneImageRing")

typedef struct SceneImageRing {
    SceneImage slots[SCENE_IMAGE_RING_SLOTS];
    u32 newestSlot;
    u32 writeSlot;
    u64 nextVersion;
    bool hasPublished;
    u8 _pad[15];
} SceneImageRing;

_Static_assert(sizeof(SceneImageRing) == 320, "SceneImageRing obeys 32-byte multiple (320B)");

SceneImageRing SceneImageRing_2(u32 width, u32 height);

// Producer (meld): publish the next completed frame and advance version.
u64 SceneImageRing_publishNext(SceneImageRing *self);

// Consumer (collage / darling): acquire the newest completed SceneImage without
// waiting on in-flight producer work. Increments refCount so the slot stays
// alive until SceneImageRing_release is called.
const SceneImage *SceneImageRing_acquireNewest(SceneImageRing *self);
fn SceneImageRing_release(SceneImageRing *self, const SceneImage *acquired);

;;CLASSNAME("MeldView")

typedef struct MeldView {
    u32 width;
    u32 height;
    u64 _pad[3];
    Camera camera;
    FloatingOrigin origin;
    PostChain postChain;
    SceneImageRing ring;
} MeldView;

_Static_assert(sizeof(MeldView) % 32 == 0, "MeldView obeys 32-byte floor");

const Image *SceneImage_image(const SceneImage *self);
u64 SceneImage_version(const SceneImage *self);

MeldView MeldView_2(u32 width, u32 height);
SceneImageRing *MeldView_ring(MeldView *self);

#endif
