$input v_texcoord0

#include "../common.sc"

SAMPLER2D(s_gbuffer, 0);
SAMPLER2D(s_normal, 1);
SAMPLER2D(s_depth, 2);
SAMPLER2D(s_sun, 3);
SAMPLER2D(s_noise, 4);

uniform vec4 u_sun_direction;
uniform vec4 u_sun_ambient;
uniform vec4 u_sun_diffuse;

DECL_CAMERA_UNIFORMS(u_look_light);
DECL_CAMERA_UNIFORMS(u_sun);

CONST(uint) SHADOW_SAMPLES = 32;
CONST(vec2 POISSON[32]) = {
    { -0.94201624,  -0.39906216 },
    {  0.94558609,  -0.76890725 },
    { -0.094184101, -0.92938870 },
    {  0.34495938,   0.29387760 },
    { -0.91588581,   0.45771432 },
    { -0.81544232,  -0.87912464 },
    { -0.38277543,   0.27676845 },
    {  0.97484398,   0.75648379 },
    {  0.44323325,  -0.97511554 },
    {  0.53742981,  -0.47373420 },
    { -0.26496911,  -0.41893023 },
    {  0.79197514,   0.19090188 },
    { -0.24188840,   0.99706507 },
    { -0.81409955,   0.91437590 },
    {  0.19984126,   0.78641367 },
    {  0.14383161,  -0.14100790 },
    {  0.94201624,   0.39906216 },
    { -0.94558609,   0.76890725 },
    {  0.094184101,  0.92938870 },
    { -0.34495938,  -0.29387760 },
    {  0.91588581,  -0.45771432 },
    {  0.81544232,   0.87912464 },
    {  0.38277543,  -0.27676845 },
    { -0.97484398,  -0.75648379 },
    { -0.44323325,   0.97511554 },
    { -0.53742981,   0.47373420 },
    {  0.26496911,   0.41893023 },
    { -0.79197514,  -0.19090188 },
    {  0.24188840,  -0.99706507 },
    { -0.81409955,  -0.91437590 },
    { -0.19984126,  -0.78641367 },
    { -0.14383161,   0.14100790 },
};

float shadow(vec3 pos_w, float bias, float cos_theta) {
    vec4 pos_s = mul(u_sun_viewProj, vec4(pos_w, 1.0));
    pos_s.xyz /= pos_s.w;
    vec2 st_s = clip_to_texture(pos_s.xy);
    pos_s.z = to_normal_depth(pos_s.z);

    // slope-scaled depth bias: as cos_theta approaches zero, bias the shadow
    // more
    float b = bias * tan(acos(cos_theta));
    float s = 0.0;
    vec2 texel = 1.0 / textureSize(s_sun, 0);
    for (int i = 0; i < SHADOW_SAMPLES; i++) {
        float d =
            texture2D(
                s_sun,
                st_s + (POISSON[i] * 1.2 * texel)).r;
        s += pos_s.z > d ? 1.0 : 0.0;
    }

    s = 1.0 - (s / (float) (SHADOW_SAMPLES));
    return s;
}

vec3 sunlight(vec3 pos_w, vec3 n, float shine) {
    vec3 pos_l = mul(u_sun_view, vec4(pos_w, 1.0)).xyz;
    vec3 dir_l = normalize(u_sun_direction.xyz);
    vec3 dir_v = normalize(u_look_light_position.xyz - pos_w);
    vec3 dir_h = normalize(-dir_l + dir_v);

    float spec = shine < EPSILON ? 0.0 : pow(max(dot(n, dir_h), 0.0), shine);

    // cos_theta ranges from [-1, 1]: -1 when 'most' light, 1 when least light
    // if dir_l and n are complete opposites (dot is -1), then the surface is
    // getting maximum light
    float cos_theta = dot(dir_l, n);
    float d = max(-cos_theta, 0.0);
    float s = d > 0.0 ? shadow(pos_w, 0.0005, cos_theta) : 0.0;

    return (s * d * u_sun_diffuse.rgb)
        + (s * u_sun_diffuse.rgb * spec)
        + u_sun_ambient.rgb;
}

void main() {
	float d = texture2D(s_depth, v_texcoord0).r;
    vec4 t_g = texture2D(s_gbuffer, v_texcoord0);
    vec3 color = t_g.rgb;
    float shine = 64.0 * t_g.a;
    vec3 pos_v =
        clip_to_view(
            u_look_light_invProj,
            clip_from_st_depth(v_texcoord0, d));
    vec3 pos_w = mul(u_look_light_invView, vec4(pos_v, 1.0));
    vec4 t_n = texture2D(s_normal, v_texcoord0);
    uint flags = decode_u8(t_n.w);
    vec3 n_w = normalize((t_n.xyz * 2.0) - 1.0);

    if (d < 1.0 - EPSILON) {
        vec3 s = sunlight(pos_w, n_w, shine);
        vec3 l = color.rgb * s;
        gl_FragData[0] = vec4(vec3(l), 1.0);
        gl_FragData[1] = vec4(l - vec3(1.0), 1.0);
    } else {
        gl_FragData[0] = vec4(vec3(0.0), 1.0);
        gl_FragData[1] = vec4(vec3(0.0), 1.0);
    }
}
