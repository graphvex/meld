#ifndef MELD_BUFFER_H
#define MELD_BUFFER_H

// buffer/buffer.h — retained G-buffer and HDR target (no temporal history).

#include "graphvex.h"

;;CLASSNAME("GBuffer")

typedef struct GBuffer {
    Image albedo;
    Image normalOct;
    Image materialOrm;
    Image emissive;
    Image velocity;
    Image depth;
} GBuffer;

_Static_assert(sizeof(GBuffer) % 32 == 0, "GBuffer obeys 32-byte floor");

;;CLASSNAME("HdrTarget")

// Explicit Filter Law: HdrTarget carries no temporal history buffer (no TAA).
typedef struct HdrTarget {
    Image color;
} HdrTarget;

_Static_assert(sizeof(HdrTarget) == 32, "HdrTarget obeys 32-byte floor");

#endif
