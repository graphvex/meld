// view/view.c — 3-slot SceneImageRing and MeldView implementation.

#include "vex.h"
#include "meld.h"

;;OVERVIEW
;;DEFINITION

SceneImageRing SceneImageRing_2(u32 width, u32 height) {
    SceneImageRing ring = { 0 };
    for (u32 i = 0u; i < SCENE_IMAGE_RING_SLOTS; i++) {
        ring.slots[i].image = Image_3(width, height, PIXEL_FORMAT_RGBA16_FLOAT);
        ring.slots[i].ready = Fence_0();
        ring.slots[i].version = 0u;
        ring.slots[i].refCount = 0u;
        ring.slots[i].completed = false;
    }
    ring.newestSlot = 0u;
    ring.writeSlot = 0u;
    ring.nextVersion = 1u;
    ring.hasPublished = false;
    return ring;
}

;;GETTER
const Image *SceneImage_image(const SceneImage *self) {
    return self ? &(*self).image : nullptr;
}
;;GETTER
u64 SceneImage_version(const SceneImage *self) {
    return self ? (*self).version : 0u;
}
;;GETTER
static u32 SceneImage_refCount(const SceneImage *self) {
    return self ? (*self).refCount : 0u;
}
;;SETTER
static fn SceneImage_setRefCount(SceneImage *self, u32 count) {
    if (self) (*self).refCount = count;
}
;;GETTER
static bool SceneImage_completed(const SceneImage *self) {
    return self ? (*self).completed : false;
}
;;SETTER
static fn SceneImage_setCompleted(SceneImage *self, bool completed) {
    if (self) (*self).completed = completed;
}
;;SETTER
static fn SceneImage_setVersion(SceneImage *self, u64 version) {
    if (self) (*self).version = version;
}
;;GETTER
static Fence *SceneImage_readyPtr(SceneImage *self) {
    return self ? &(*self).ready : nullptr;
}

;;GETTER
static u32 SceneImageRing_writeSlot(const SceneImageRing *self) {
    return self ? (*self).writeSlot : 0u;
}
;;SETTER
static fn SceneImageRing_setWriteSlot(SceneImageRing *self, u32 slot) {
    if (self) (*self).writeSlot = slot;
}
;;GETTER
static u32 SceneImageRing_newestSlot(const SceneImageRing *self) {
    return self ? (*self).newestSlot : 0u;
}
;;SETTER
static fn SceneImageRing_setNewestSlot(SceneImageRing *self, u32 slot) {
    if (self) (*self).newestSlot = slot;
}
;;GETTER
static u64 SceneImageRing_nextVersion(const SceneImageRing *self) {
    return self ? (*self).nextVersion : 1u;
}
;;SETTER
static fn SceneImageRing_setNextVersion(SceneImageRing *self, u64 ver) {
    if (self) (*self).nextVersion = ver;
}
;;GETTER
static bool SceneImageRing_hasPublished(const SceneImageRing *self) {
    return self ? (*self).hasPublished : false;
}
;;SETTER
static fn SceneImageRing_setHasPublished(SceneImageRing *self, bool has) {
    if (self) (*self).hasPublished = has;
}
;;GETTER
static SceneImage *SceneImageRing_slotPtr(SceneImageRing *self, u32 idx) {
    return (self && idx < SCENE_IMAGE_RING_SLOTS) ? &(*self).slots[idx] : nullptr;
}

u64 SceneImageRing_publishNext(SceneImageRing *self) {
    if (!self) return 0u;
    // Pick a slot that is not pinned by a consumer.
    u32 candidate = SceneImageRing_writeSlot(self);
    bool hasPub = SceneImageRing_hasPublished(self);
    u32 newest = SceneImageRing_newestSlot(self);
    for (u32 i = 0u; i < SCENE_IMAGE_RING_SLOTS; i++) {
        u32 idx = (candidate + i) % SCENE_IMAGE_RING_SLOTS;
        SceneImage *slot = SceneImageRing_slotPtr(self, idx);
        if (SceneImage_refCount(slot) == 0u && (!hasPub || idx != newest)) {
            candidate = idx;
            break;
        }
    }
    u64 ver = SceneImageRing_nextVersion(self);
    SceneImageRing_setNextVersion(self, ver + 1u);
    SceneImage *target = SceneImageRing_slotPtr(self, candidate);
    SceneImage_setVersion(target, ver);
    SceneImage_setCompleted(target, true);
    Fence_setSignaled(SceneImage_readyPtr(target), true);
    SceneImageRing_setNewestSlot(self, candidate);
    SceneImageRing_setWriteSlot(self, (candidate + 1u) % SCENE_IMAGE_RING_SLOTS);
    SceneImageRing_setHasPublished(self, true);
    return ver;
}

const SceneImage *SceneImageRing_acquireNewest(SceneImageRing *self) {
    if (!self || !SceneImageRing_hasPublished(self)) return nullptr;
    SceneImage *slot = SceneImageRing_slotPtr(self, SceneImageRing_newestSlot(self));
    if (!SceneImage_completed(slot)) return nullptr;
    SceneImage_setRefCount(slot, SceneImage_refCount(slot) + 1u);
    return slot;
}

fn SceneImageRing_release(SceneImageRing *self, const SceneImage *acquired) {
    if (!self || !acquired) return;
    for (u32 i = 0u; i < SCENE_IMAGE_RING_SLOTS; i++) {
        SceneImage *slot = SceneImageRing_slotPtr(self, i);
        if (slot == acquired) {
            u32 rc = SceneImage_refCount(slot);
            if (rc > 0u) SceneImage_setRefCount(slot, rc - 1u);
            return;
        }
    }
}

MeldView MeldView_2(u32 width, u32 height) {
    MeldView view = { 0 };
    view.width = width;
    view.height = height;
    f32 aspect = (height > 0u) ? ((f32) width / (f32) height) : 1.0f;
    view.camera = Camera_3(1.04719755f, aspect, 0.1f);
    view.origin = FloatingOrigin_0();
    view.postChain = PostChain_0();
    view.ring = SceneImageRing_2(width, height);
    return view;
}

;;GETTER
SceneImageRing *MeldView_ring(MeldView *self) {
    return self ? &(*self).ring : nullptr;
}
