#version 150 core
in highp vec2 v_coord;
out vec4 fragColor;
uniform sampler2D u_src;
uniform sampler2D u_target;
uniform highp float u_transitionProgress;
uniform lowp float qt_Opacity;
void main() {
    lowp vec4 tex1 = texture2D(u_target, v_coord);
    lowp vec4 tex2 = texture2D(u_src, v_coord);
    fragColor = mix(tex2, tex1, u_transitionProgress) * qt_Opacity;
}
