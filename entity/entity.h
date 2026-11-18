#ifndef MELD_ENTITY_H
#define MELD_ENTITY_H

// entity/entity.h — generation-checked EntityId and Anchor component.

#include "space/space.h"

;;CLASSNAME("EntityId")

typedef struct EntityId {
    u32 index;
    u32 generation;
} EntityId;

static inline bool EntityId_isValid(EntityId id) { return id.generation != 0u; }
static inline bool EntityId_equals(EntityId a, EntityId b) {
    return a.index == b.index && a.generation == b.generation;
}

;;CLASSNAME("Anchor")

typedef struct Anchor {
    PositionLD128 position;
    SpaceId space;
    d64 depthRestore;
    u64 _pad[2];
} Anchor;

_Static_assert(sizeof(Anchor) == 96, "Anchor obeys 32-byte multiple (96B)");

#endif
