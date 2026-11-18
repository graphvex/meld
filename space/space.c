// space/space.c — Euclidean spaces, rigid transforms, portals and single-tick traversal.

#include "vex.h"
#include "space/space.h"

;;OVERVIEW
;;DEFINITION

bool SpaceId_equals(SpaceId a, SpaceId b) {
    return a.index == b.index && a.generation == b.generation;
}

Space Space_2(SpaceId id, CoordSpace coordSpace) {
    return (Space){
        .id = id,
        .portalCount = 0u,
        .entityCount = 0u,
        ._pad = { 0u, 0u },
        .coordSpace = coordSpace
    };
}

Space Space_0() {
    return Space_2((SpaceId){ 0u, 1u }, CoordSpace_0());
}

// ---------------------------------------------------------------- RigidTransform

RigidTransform RigidTransform_3(quat64 rotation, vec3d translation, d64 scale) {
    if (!isfinite(scale) || scale <= 0.0) scale = 1.0;
    return (RigidTransform){
        .rotation = quat64_normalize(rotation),
        .translation = translation,
        .scale = scale,
        ._pad = { 0.0, 0.0, 0.0 }
    };
}

RigidTransform RigidTransform_0() {
    return RigidTransform_3(quat64_0(), vec3d_0(), 1.0);
}

;;GETTER
static d64 RigidTransform_scale(const RigidTransform *self) {
    return self ? (*self).scale : 1.0;
}
;;GETTER
static quat64 RigidTransform_rotation(const RigidTransform *self) {
    return self ? (*self).rotation : quat64_0();
}
;;GETTER
static vec3d RigidTransform_translation(const RigidTransform *self) {
    return self ? (*self).translation : vec3d_0();
}

vec3d RigidTransform_applyPoint(const RigidTransform *self, vec3d p) {
    if (!self) return p;
    vec3d scaled = vec3d_scale(p, RigidTransform_scale(self));
    vec3d rotated = quat64_rotateVec3(RigidTransform_rotation(self), scaled);
    return vec3d_add(rotated, RigidTransform_translation(self));
}

vec3d RigidTransform_applyDirection(const RigidTransform *self, vec3d dir) {
    if (!self) return dir;
    vec3d rotated = quat64_rotateVec3(RigidTransform_rotation(self), dir);
    return vec3d_scale(rotated, RigidTransform_scale(self));
}

RigidTransform RigidTransform_inverse(const RigidTransform *self) {
    if (!self) return RigidTransform_0();
    d64 scale = RigidTransform_scale(self);
    d64 invScale = (scale != 0.0) ? (1.0 / scale) : 1.0;
    quat64 invRot = quat64_conjugate(RigidTransform_rotation(self));
    vec3d negTrans = vec3d_neg(RigidTransform_translation(self));
    vec3d invTrans = vec3d_scale(quat64_rotateVec3(invRot, negTrans), invScale);
    return RigidTransform_3(invRot, invTrans, invScale);
}

// ---------------------------------------------------------------- PortalFace & Traversal

PortalFace PortalFace_5(vec3d center, vec3d right, vec3d up, d64 halfWidth, d64 halfHeight) {
    vec3d nr = vec3d_normalize(right);
    vec3d nu = vec3d_normalize(up);
    vec3d n  = vec3d_normalize(vec3d_cross(nr, nu));
    return (PortalFace){
        .center = center,
        .right = nr,
        .up = nu,
        .normal = n,
        .halfWidth = halfWidth,
        .halfHeight = halfHeight,
        ._pad = { 0.0, 0.0 }
    };
}

;;GETTER
static SpaceId Portal_spaceA(const Portal *self) {
    return self ? (*self).spaceA : (SpaceId){ 0u, 0u };
}
;;GETTER
static SpaceId Portal_spaceB(const Portal *self) {
    return self ? (*self).spaceB : (SpaceId){ 0u, 0u };
}
;;GETTER
static bool Portal_twoWay(const Portal *self) {
    return self ? (*self).twoWay : false;
}
;;GETTER
static PortalFace Portal_faceA(const Portal *self) {
    return self ? (*self).faceA : (PortalFace){ 0 };
}
;;GETTER
static PortalFace Portal_faceB(const Portal *self) {
    return self ? (*self).faceB : (PortalFace){ 0 };
}
;;GETTER
static RigidTransform Portal_aToB(const Portal *self) {
    return self ? (*self).aToB : RigidTransform_0();
}
;;GETTER
static bool Portal_collapsesDepthTo2D(const Portal *self) {
    return self ? (*self).collapsesDepthTo2D : false;
}

bool Traversal_tryCross(const Portal *portal,
                        SpaceId currentSpace,
                        vec3d p0,
                        vec3d p1,
                        vec3d velocity,
                        d64 inDepthRestore,
                        SpaceId *outSpace,
                        vec3d *outPos,
                        vec3d *outVel,
                        d64 *outDepthRestore) {
    if (outSpace) *outSpace = currentSpace;
    if (outPos) *outPos = p1;
    if (outVel) *outVel = velocity;
    if (outDepthRestore) *outDepthRestore = inDepthRestore;
    if (!portal) return false;

    SpaceId spaceA = Portal_spaceA(portal);
    SpaceId spaceB = Portal_spaceB(portal);
    bool forward = SpaceId_equals(currentSpace, spaceA);
    bool backward = Portal_twoWay(portal) && SpaceId_equals(currentSpace, spaceB);
    if (!forward && !backward) return false;

    PortalFace face = forward ? Portal_faceA(portal) : Portal_faceB(portal);
    RigidTransform xform = Portal_aToB(portal);
    if (!forward) xform = RigidTransform_inverse(&xform);
    SpaceId destSpace = forward ? spaceB : spaceA;

    vec3d d0 = vec3d_sub(p0, face.center);
    vec3d d1 = vec3d_sub(p1, face.center);
    d64 side0 = vec3d_dot(d0, face.normal);
    d64 side1 = vec3d_dot(d1, face.normal);

    // Must cross from positive normal side (>= 0) to negative normal side (< 0)
    if (!(side0 >= 0.0 && side1 < 0.0)) return false;

    d64 denom = side0 - side1;
    d64 t = (denom > 1e-12) ? (side0 / denom) : 0.0;
    vec3d hit = vec3d_add(p0, vec3d_scale(vec3d_sub(p1, p0), t));
    vec3d relHit = vec3d_sub(hit, face.center);

    d64 u = fabs(vec3d_dot(relHit, face.right));
    d64 v = fabs(vec3d_dot(relHit, face.up));
    if (u > face.halfWidth || v > face.halfHeight) return false;

    vec3d mappedPos = RigidTransform_applyPoint(&xform, p1);
    vec3d mappedVel = RigidTransform_applyDirection(&xform, velocity);
    d64 nextRestore = inDepthRestore;

    if (Portal_collapsesDepthTo2D(portal)) {
        if (forward) {
            nextRestore = vec3d_z(mappedPos);
            mappedPos = vec3d_3(vec3d_x(mappedPos), vec3d_y(mappedPos), 0.0);
        } else {
            mappedPos = vec3d_3(vec3d_x(mappedPos), vec3d_y(mappedPos), inDepthRestore);
        }
    }

    if (outSpace) *outSpace = destSpace;
    if (outPos) *outPos = mappedPos;
    if (outVel) *outVel = mappedVel;
    if (outDepthRestore) *outDepthRestore = nextRestore;
    return true;
}
