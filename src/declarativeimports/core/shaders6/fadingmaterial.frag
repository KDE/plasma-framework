#version 440

layout (std140, binding = 0) uniform buf {
    highp mat4 qt_Matrix;
    highp vec4 u_src_rect; // offset 64
    highp vec4 u_target_rect; // offset 80
    highp float u_transitionProgress; // offset 96
    lowp float qt_Opacity; // offset 100
} ubuf; // size 104

layout (binding = 1) uniform sampler2D u_src;
layout (binding = 2) uniform sampler2D u_target;

layout (location = 0) in highp vec2 v_coord;
layout (location = 0) out vec4 fragColor;

void main() {
    lowp vec4 tex1 = texture(u_target, ubuf.u_target_rect.xy + ubuf.u_target_rect.zw * v_coord);
    lowp vec4 tex2 = texture(u_src, ubuf.u_src_rect.xy + ubuf.u_src_rect.zw * v_coord);
    fragColor = mix(tex2, tex1, ubuf.u_transitionProgress) * ubuf.qt_Opacity;
}
