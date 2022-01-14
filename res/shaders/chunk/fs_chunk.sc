$input v_texcoord0, v_normal, v_position, v_color0

#include "../common.sc"

SAMPLER2D(s_tex, 0);

void main() {
    vec4 color = texture2D(s_tex, v_texcoord0);

    if (color.a < EPSILON) {
        discard;
    }

	gl_FragData[0] = vec4(color.rgb, v_color0.w);
	gl_FragData[1] = vec4(v_normal, encode_u8(0));
}
