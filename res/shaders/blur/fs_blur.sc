$input v_texcoord0

#include "../common.sc"

SAMPLER2D(s_input, 0);

uniform vec4 u_params;

// lifted from: github.com/Jam3/glsl-fast-gaussian-blur
vec4 blur13(vec2 st, vec2 resolution, vec2 direction) {
    vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.411764705882353) * direction;
    vec2 off2 = vec2(3.2941176470588234) * direction;
    vec2 off3 = vec2(5.176470588235294) * direction;
    color += texture2D(s_input, st) * 0.1964825501511404;
    color += texture2D(s_input, st + (off1 / resolution)) * 0.2969069646728344;
    color += texture2D(s_input, st - (off1 / resolution)) * 0.2969069646728344;
    color += texture2D(s_input, st + (off2 / resolution)) * 0.09447039785044732;
    color += texture2D(s_input, st - (off2 / resolution)) * 0.09447039785044732;
    color += texture2D(s_input, st + (off3 / resolution)) * 0.010381362401148057;
    color += texture2D(s_input, st - (off3 / resolution)) * 0.010381362401148057;
    return color;
}

void main() {
    bool horizontal = u_params.x > 0.0;
    gl_FragColor = blur13(
        v_texcoord0, textureSize(s_input, 0),
        horizontal ? vec2(1, 0) : vec2(0, 1));
}
