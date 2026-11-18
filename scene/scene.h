#ifndef MELD_SCENE_H
#define MELD_SCENE_H

// scene/scene.h — retained Mesh, Meshlet, Transform, Camera (no TAA jitter),
// Light and Sprite.

#include "entity/entity.h"
#include "graphvex.h"

#define MESHLET_MAX_VERTICES  64u
#define MESHLET_MAX_TRIANGLES 128u

;;CLASSNAME("Mesh")

typedef struct Mesh {
    GpuBuffer vertexBuffer;
    GpuBuffer indexBuffer;
    u32 vertexCount;
    u32 indexCount;
    u32 meshletCount;
    u32 _pad[5];
} Mesh;

_Static_assert(sizeof(Mesh) == 96, "Mesh obeys 32-byte multiple (96B)");

;;CLASSNAME("Meshlet")

typedef struct Meshlet {
    vec3f aabbMin;
    vec3f aabbMax;
    vec4f coneAxisAndCutoff;
    u32 vertexOffset;
    u32 triangleOffset;
    u16 vertexCount;
    u16 triangleCount;
    u32 _pad;
} Meshlet;

_Static_assert(sizeof(Meshlet) == 64, "Meshlet obeys 32-byte multiple (64B)");

;;CLASSNAME("Transform")

typedef struct Transform {
    vec3f localPosition;
    quat32 localRotation;
    vec3f localScale;
    u32 _pad[4];
} Transform;

_Static_assert(sizeof(Transform) == 64, "Transform obeys 32-byte multiple (64B)");

Transform Transform_0();
Transform Transform_3(vec3f pos, quat32 rot, vec3f scale);
#define Transform(...) CONSTRUCTOR_DISPATCH(Transform, __VA_ARGS__)

mat4f Transform_toMat4(const Transform *self);

;;CLASSNAME("Camera")

// Explicit Filter Law: Camera carries zero sub-pixel jitter (no TAA).
typedef struct Camera {
    f32 fovRadians;
    f32 aspect;
    f32 nearPlane;
    f32 orthoHalfHeight;
    bool orthographic;
    u8 _pad[15];
} Camera;

_Static_assert(sizeof(Camera) == 32, "Camera obeys 32-byte floor");

Camera Camera_0();
Camera Camera_3(f32 fovRadians, f32 aspect, f32 nearPlane);
#define Camera(...) CONSTRUCTOR_DISPATCH(Camera, __VA_ARGS__)

mat4f Camera_reversedZInfiniteProj(const Camera *self);

;;CLASSNAME("Sprite")

typedef struct Sprite {
    vec2f size;
    vec2f pivot;
    u32 imageBindlessSlot;
    u32 colorRGBA;
    i32 layer;
    u32 _pad;
} Sprite;

_Static_assert(sizeof(Sprite) == 32, "Sprite obeys 32-byte floor");

#endif
