#include <bgfx_shader.sh>

#define PI 3.1415926538
#define TAU (2 * PI)
#define EPSILON 1e-5

#define TICKS_PER_SECOND 60.0

#define DECL_CAMERA_UNIFORMS(prefix)        \
    uniform mat4 prefix ## _proj;           \
    uniform mat4 prefix ## _view;           \
    uniform mat4 prefix ## _viewProj;       \
    uniform mat4 prefix ## _invProj;        \
    uniform mat4 prefix ## _invView;        \
    uniform mat4 prefix ## _invViewProj;    \
    uniform vec4 prefix ## _position;

#define FLAG_WATER 0x01

float encode_u8(uint value) {
    return ((float) (value & 0xFF)) / 255.0;
}

uint decode_u8(float value) {
    return (uint) floor(value * 255.0);
}

float lindepth(float d, float near, float far) {
	return near * far / (far + d * (near - far));
}

// convert depth from NDC of either [0, 1] or [-1, 1] to [0, 1]
float to_normal_depth(float z) {
#if BGFX_SHADER_LANGUAGE_GLSL
	return (z * 0.5) + 0.5;
#else
	return z;
#endif
}

// converts clip coordinates to st coordinates
vec2 clip_to_st(vec2 clip) {
#if !BGFX_SHADER_LANGUAGE_GLSL
    clip.y = -clip.y;
#endif
    return clip;
}

float to_clip_depth(float z) {
#if BGFX_SHADER_LANGUAGE_GLSL
	return (z * 2.0) - 1.0;
#else
	return z;
#endif
}

vec2 clip_to_texture(vec2 clip) {
    clip = (clip * 0.5) + 0.5;
#if !BGFX_SHADER_LANGUAGE_GLSL
    clip.y = 1.0 - clip.y;
#endif
    return clip;
}

vec3 clip_from_st_depth(vec2 st, float depth) {
    vec3 clip = vec3((st * 2.0) - 1.0, to_clip_depth(depth));
#if !BGFX_SHADER_LANGUAGE_GLSL
    clip.y = -clip.y;
#endif
    return clip;
}

vec3 clip_to_view(mat4 inv_proj, vec3 clip) {
	vec4 pos_v = mul(inv_proj, vec4(clip, 1.0));
	return pos_v.xyz / pos_v.w;
}

vec3 clip_to_world(mat4 inv_view_proj, vec3 clip) {
	vec4 pos_w = mul(inv_view_proj, vec4(clip, 1.0));
	return pos_w.xyz / pos_w.w;
}

float noise(vec2 st) {
	return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

// adapted from: github.com/shff/opengl_sky/blob/master/main.c
float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}
