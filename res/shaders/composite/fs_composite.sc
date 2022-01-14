$input v_texcoord0

#include "../common.sc"

SAMPLER2D(s_gbuffer, 0);
SAMPLER2D(s_normal, 1);
SAMPLER2D(s_depth, 2);
SAMPLER2D(s_sun, 3);
SAMPLER2D(s_noise, 4);
SAMPLER2D(s_ssao, 5);
SAMPLER2D(s_light, 6);
SAMPLER2D(s_bloom, 7);

DECL_CAMERA_UNIFORMS(u_look);

uniform vec4 u_ticks;

uniform vec4 u_sky_color;
uniform vec4 u_fog_color;
uniform vec4 u_void_color;
uniform vec4 u_fog;

// TODO: remove this
uniform vec4 u_show_buffer;

#define HALF_FOG_HEIGHT 256.0

vec3 get_sky(float h) {
    return h > 0 ?
        mix(
            u_fog_color.rgb,
            u_sky_color.rgb,
            clamp((h - HALF_FOG_HEIGHT) / HALF_FOG_HEIGHT, 0.0, 1.0))
        : mix(
            u_fog_color.rgb,
            u_void_color.rgb,
            clamp(
                1.0 - ((h + HALF_FOG_HEIGHT) / HALF_FOG_HEIGHT),
                0.0, 1.0));
}

void main() {
	float d = texture2D(s_depth, v_texcoord0).r;
    vec3
        pos_v =
            clip_to_view(
                u_look_invProj,
                clip_from_st_depth(v_texcoord0, d)),
        pos_w = mul(u_look_invView, vec4(pos_v, 1.0)),
        sky = get_sky(pos_w.y),
        view_void =
            clip_to_view(
                u_look_invProj,
                clip_from_st_depth(v_texcoord0, 0.9999)),
        sky_void = get_sky(mul(u_look_invView, vec4(view_void, 1.0)).y);

    if (d < 1.0 - EPSILON) {
        vec3 light = texture2D(s_light, v_texcoord0).rgb;
        vec3 bloom = texture2D(s_bloom, v_texcoord0).rgb;
        float o = texture2D(s_ssao, v_texcoord0).r;
        o = clamp(o, 0.5, 1.0);
        vec3 c = (light.rgb + bloom) * o;
        c = mix(c, sky_void, smoothstep(u_fog.x, u_fog.y, length(pos_v)));
        gl_FragColor = vec4(c, 1.0);
    } else {
        gl_FragColor = vec4(sky, 1.0);
    }

    uint show_buffer = (uint) (u_show_buffer.x);

    if (show_buffer == 1) {
        gl_FragColor = vec4(texture2D(s_depth, v_texcoord0).rrr, 1.0);
    } else if (show_buffer == 2) {
        gl_FragColor = vec4(texture2D(s_gbuffer, v_texcoord0).rgb, 1.0);
    } else if (show_buffer == 3) {
        gl_FragColor = vec4(texture2D(s_normal, v_texcoord0).rgb, 1.0);
    } else if (show_buffer == 4) {
        gl_FragColor = vec4(texture2D(s_sun, v_texcoord0).rgb, 1.0);
    } else if (show_buffer == 5) {
        gl_FragColor = vec4(texture2D(s_ssao, v_texcoord0).rgb, 1.0);
    } else if (show_buffer == 6) {
        gl_FragColor = vec4(texture2D(s_light, v_texcoord0).rgb, 1.0);
    } else if (show_buffer == 7) {
        gl_FragColor = vec4(texture2D(s_bloom, v_texcoord0).rgb, 1.0);
    }
}
