$input v_texcoord0, v_normal, v_position, v_color0

#include "../common.sc"

SAMPLER2D(s_tex, 0);
SAMPLER2D(s_noise, 1);

uniform vec4 time;

float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = x;
  	vec2 uv = (p.xy+vec2(37.0,17.0)) + f.xy;
  	vec2 rg = texture2D(s_noise, (uv + 0.5) / 128.0 ).yx;
  	return mix(rg.x, rg.y, sin(f.z) * 0.5 + 0.5);
}


CONST(mat3 m3) =
    mtxFromCols(
        vec3(0.00, 0.80, 0.60),
        vec3(-0.80, 0.36, -0.48),
        vec3(-0.60, -0.48, 0.64));
float fbm(vec3 p) {
    float f = 0.0;
    f += 0.5000 * noise(p); p = mul(m3, p) * 2.02;
    f += 0.2500 * noise(p); p = mul(m3, p) * 2.03;
    f += 0.1250 * noise(p); p = mul(m3, p) * 2.01;
    f += 0.0625 * noise(p);
    return f / 0.9375;
}

CONST(mat2 m2) = mat2(0.60, 0.60, -0.80, 0.80);
float water(float t, vec2 pos) {
	return abs(fbm(vec3(8.0 * mul(pos, m2), t)) - 0.5) * 0.1;
}





void main() {
    float t = ((time.x / 60.0) / 2.0) + 256;

    vec3 normal;
    vec2 coord = v_position.xz / 8.0;

    float bump = 0.2;
    float EPS = 0.05;
    vec2 dx = vec2(EPS, 0.0);
    vec2 dz = vec2(0.0, EPS);
    normal = vec3(0.0, 1.0, 0.0);
    normal.x = -bump * (water(t, coord + dx) - water(t, coord - dx)) / (2.0 * EPS);
    normal.z = -bump * (water(t, coord + dz) - water(t, coord - dz)) / (2.0 * EPS);
    normal = normalize(normal) * 0.5 + 0.5;
    // v_normal = normal;

	gl_FragData[0] = vec4(texture2D(s_tex, v_texcoord0).rgb, v_color0.w);
	gl_FragData[1] = vec4(normal, encode_u8(FLAG_WATER));
}
