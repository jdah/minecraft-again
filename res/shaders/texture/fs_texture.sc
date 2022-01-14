$input v_texcoord0

#include "../common.sc"

SAMPLER2D(s_tex, 0);

void main() {
	gl_FragColor = vec4(texture2D(s_tex, v_texcoord0).rgb, 1.0);
}
