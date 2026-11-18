// scene/scene.c — Transform and Camera (reversed-Z infinite projection, no TAA).

#include "vex.h"
#include "meld.h"

;;OVERVIEW
;;DEFINITION

Transform Transform_3(vec3f pos, quat32 rot, vec3f scale) {
    return (Transform){
        .localPosition = pos,
        .localRotation = quat32_normalize(rot),
        .localScale = scale,
        ._pad = { 0u, 0u, 0u, 0u }
    };
}

Transform Transform_0() {
    return Transform_3(vec3f_0(), quat32_0(), vec3f_3(1.0f, 1.0f, 1.0f));
}

;;GETTER
static quat32 Transform_localRotation(const Transform *self) {
    return self ? (*self).localRotation : quat32_0();
}
;;GETTER
static vec3f Transform_localScale(const Transform *self) {
    return self ? (*self).localScale : vec3f_3(1.0f, 1.0f, 1.0f);
}
;;GETTER
static vec3f Transform_localPosition(const Transform *self) {
    return self ? (*self).localPosition : vec3f_0();
}

mat4f Transform_toMat4(const Transform *self) {
    if (!self) return mat4f_identity();
    mat3f rot = quat32_toMat3(Transform_localRotation(self));
    vec3f s = Transform_localScale(self);
    vec3f p = Transform_localPosition(self);
    vec3f c0 = vec3f_scale(mat3f_col(&rot, 0), vec3f_x(s));
    vec3f c1 = vec3f_scale(mat3f_col(&rot, 1), vec3f_y(s));
    vec3f c2 = vec3f_scale(mat3f_col(&rot, 2), vec3f_z(s));
    return mat4f_4(
        vec4f_4(vec3f_x(c0), vec3f_y(c0), vec3f_z(c0), 0.0f),
        vec4f_4(vec3f_x(c1), vec3f_y(c1), vec3f_z(c1), 0.0f),
        vec4f_4(vec3f_x(c2), vec3f_y(c2), vec3f_z(c2), 0.0f),
        vec4f_4(vec3f_x(p),  vec3f_y(p),  vec3f_z(p),  1.0f)
    );
}

Camera Camera_3(f32 fovRadians, f32 aspect, f32 nearPlane) {
    if (fovRadians <= 0.0f) fovRadians = 1.04719755f;
    if (aspect <= 0.0f) aspect = 1.0f;
    if (nearPlane <= 0.0f) nearPlane = 0.1f;
    return (Camera){
        .fovRadians = fovRadians,
        .aspect = aspect,
        .nearPlane = nearPlane,
        .orthoHalfHeight = 10.0f,
        .orthographic = false,
        ._pad = { 0 }
    };
}

Camera Camera_0() {
    return Camera_3(1.04719755f, 16.0f / 9.0f, 0.1f);
}

;;GETTER
static f32 Camera_fovRadians(const Camera *self) {
    return self ? (*self).fovRadians : 1.04719755f;
}
;;GETTER
static f32 Camera_aspect(const Camera *self) {
    return self ? (*self).aspect : 1.0f;
}
;;GETTER
static f32 Camera_nearPlane(const Camera *self) {
    return self ? (*self).nearPlane : 0.1f;
}

mat4f Camera_reversedZInfiniteProj(const Camera *self) {
    if (!self) return mat4f_identity();
    f32 f = 1.0f / tanf(Camera_fovRadians(self) * 0.5f);
    f32 a = Camera_aspect(self);
    f32 n = Camera_nearPlane(self);
    // Left-handed (+Z forward), reversed-Z (near -> 1, infinite far -> 0)
    return mat4f_4(
        vec4f_4(f / a, 0.0f, 0.0f, 0.0f),
        vec4f_4(0.0f,  f,    0.0f, 0.0f),
        vec4f_4(0.0f,  0.0f, 0.0f, 1.0f),
        vec4f_4(0.0f,  0.0f, n,    0.0f)
    );
}
