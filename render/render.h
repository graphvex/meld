#ifndef MELD_RENDER_H
#define MELD_RENDER_H

// render/render.h — explicit post-processing filter chain (bloom, tonemap,
// exposure, and opt-in Dither; never TAA).
//
// Explicit Filter Law: no TAA; dithering is a deliberate PostChain step that
// must be explicitly added by the developer.

#include "vex.h"

#define POST_CHAIN_MAX_STEPS 8u

;;CLASSNAME("PostFilterKind")

typedef enum PostFilterKind {
    POST_FILTER_NONE     = 0,
    POST_FILTER_BLOOM    = 1,
    POST_FILTER_EXPOSURE = 2,
    POST_FILTER_TONEMAP  = 3,
    POST_FILTER_DITHER   = 4
} PostFilterKind;

;;CLASSNAME("PostChain")

typedef struct PostChain {
    PostFilterKind steps[POST_CHAIN_MAX_STEPS];
    u32 count;
    f32 exposureEV;
    f32 bloomIntensity;
    f32 ditherStrength;
    u32 _pad[4];
} PostChain;

_Static_assert(sizeof(PostChain) == 64, "PostChain obeys 32-byte multiple (64B)");

PostChain PostChain_0();
#define PostChain(...) CONSTRUCTOR_DISPATCH(PostChain, __VA_ARGS__)

bool PostChain_addFilter(PostChain *self, PostFilterKind kind);
bool PostChain_hasFilter(const PostChain *self, PostFilterKind kind);

#endif
