// render/render.c — explicit PostChain (bloom, tonemap, opt-in Dither; no TAA).

#include "vex.h"
#include "render/render.h"

;;OVERVIEW
;;DEFINITION

PostChain PostChain_0() {
    PostChain chain = { 0 };
    chain.exposureEV = 0.0f;
    chain.bloomIntensity = 0.05f;
    chain.ditherStrength = 1.0f / 255.0f;
    // Default chain has tonemap only; Dither is never added implicitly.
    PostChain_addFilter(&chain, POST_FILTER_TONEMAP);
    return chain;
}

;;GETTER
static u32 PostChain_count(const PostChain *self) {
    return self ? (*self).count : 0u;
}
;;SETTER
static fn PostChain_setCount(PostChain *self, u32 count) {
    if (self) (*self).count = count;
}
;;GETTER
static PostFilterKind PostChain_step(const PostChain *self, u32 idx) {
    return (self && idx < (*self).count) ? (*self).steps[idx] : POST_FILTER_NONE;
}
;;SETTER
static fn PostChain_setStep(PostChain *self, u32 idx, PostFilterKind kind) {
    if (self && idx < POST_CHAIN_MAX_STEPS) (*self).steps[idx] = kind;
}

bool PostChain_addFilter(PostChain *self, PostFilterKind kind) {
    if (!self || kind == POST_FILTER_NONE) return false;
    u32 count = PostChain_count(self);
    if (count >= POST_CHAIN_MAX_STEPS) {
        THROW("PostChain: exceeded POST_CHAIN_MAX_STEPS (%u)", POST_CHAIN_MAX_STEPS);
        return false;
    }
    PostChain_setStep(self, count, kind);
    PostChain_setCount(self, count + 1u);
    return true;
}

bool PostChain_hasFilter(const PostChain *self, PostFilterKind kind) {
    if (!self) return false;
    u32 count = PostChain_count(self);
    for (u32 i = 0u; i < count; i++) {
        if (PostChain_step(self, i) == kind) return true;
    }
    return false;
}
