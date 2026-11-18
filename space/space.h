#ifndef MELD_SPACE_H
#define MELD_SPACE_H

// space/space.h — flat Euclidean spaces connected by rigid-transform Portals.
//
// Portal Topology Law: every Space is flat Euclidean with its own CoordSpace;
// non-Euclidean behaviour comes purely from the SpaceGraph topology.

#include "coord/coord.h"

#define PORTAL_RECURSION_MAX 4u

;;CLASSNAME("SpaceId")

typedef struct SpaceId {
    u32 index;
    u32 generation;
} SpaceId;

bool SpaceId_equals(SpaceId a, SpaceId b);

;;CLASSNAME("Space")

typedef struct Space {
    SpaceId id;
    u32 portalCount;
    u32 entityCount;
    u64 _pad[2];
    CoordSpace coordSpace;
} Space;

_Static_assert(sizeof(Space) == 96, "Space obeys 32-byte multiple (96B)");

Space Space_0();
Space Space_2(SpaceId id, CoordSpace coordSpace);
#define Space(...) CONSTRUCTOR_DISPATCH(Space, __VA_ARGS__)

;;CLASSNAME("RigidTransform")

typedef struct RigidTransform {
    quat64 rotation;
    vec3d translation;
    d64 scale;
    d64 _pad[3];
} RigidTransform;

_Static_assert(sizeof(RigidTransform) == 96, "RigidTransform obeys 32-byte multiple (96B)");

RigidTransform RigidTransform_0();
RigidTransform RigidTransform_3(quat64 rotation, vec3d translation, d64 scale);
#define RigidTransform(...) CONSTRUCTOR_DISPATCH(RigidTransform, __VA_ARGS__)

vec3d          RigidTransform_applyPoint(const RigidTransform *self, vec3d p);
vec3d          RigidTransform_applyDirection(const RigidTransform *self, vec3d dir);
RigidTransform RigidTransform_inverse(const RigidTransform *self);

;;CLASSNAME("PortalFace")

typedef struct PortalFace {
    vec3d center;
    vec3d right;
    vec3d up;
    vec3d normal;
    d64 halfWidth;
    d64 halfHeight;
    d64 _pad[2];
} PortalFace;

_Static_assert(sizeof(PortalFace) == 160, "PortalFace obeys 32-byte multiple (160B)");

PortalFace PortalFace_5(vec3d center, vec3d right, vec3d up, d64 halfWidth, d64 halfHeight);

;;CLASSNAME("Portal")

typedef struct Portal {
    SpaceId spaceA;
    SpaceId spaceB;
    bool twoWay;
    bool collapsesDepthTo2D;
    u8 _pad[14];
    PortalFace faceA;
    PortalFace faceB;
    RigidTransform aToB;
} Portal;

_Static_assert(sizeof(Portal) % 32 == 0, "Portal obeys 32-byte floor");

;;CLASSNAME("Traversal")

// Tests segment p0 -> p1 against portal.faceA; if crossed in the same tick,
// transforms p1 and velocity into spaceB (and restores/collapses 2D depth if
// configured), writing outSpace, outPos, outVel and outDepthRestore.
bool Traversal_tryCross(const Portal *portal,
                        SpaceId currentSpace,
                        vec3d p0,
                        vec3d p1,
                        vec3d velocity,
                        d64 inDepthRestore,
                        SpaceId *outSpace,
                        vec3d *outPos,
                        vec3d *outVel,
                        d64 *outDepthRestore);

;;CLASSNAME("PortalView")

static inline bool PortalView_canRecurse(u32 currentDepth) {
    return currentDepth < PORTAL_RECURSION_MAX;
}

#endif
