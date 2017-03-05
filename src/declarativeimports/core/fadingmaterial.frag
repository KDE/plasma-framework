varying highp vec2 v_coord;
uniform sampler2D u_src;
uniform sampler2D u_target;
uniform highp float u_transitionProgress;
uniform lowp float qt_Opacity;
void main() {
    lowp vec4 tex1 = texture2D(u_target, v_coord);
    lowp vec4 tex2 = texture2D(u_src, v_coord);
    gl_FragColor = mix(tex2, tex1, u_transitionProgress) * qt_Opacity;
}
