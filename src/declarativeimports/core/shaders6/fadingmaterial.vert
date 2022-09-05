#version 440

layout (std140, binding = 0) uniform buf {
    highp mat4 qt_Matrix;
    highp vec4 u_src_rect;
    highp vec4 u_target_rect;
    highp float u_transitionProgress;
    lowp float qt_Opacity;
} ubuf;

layout (location = 0) in highp vec4 qt_Vertex;
layout (location = 1) in highp vec2 qt_MultiTexCoord0;
layout (location = 0) out highp vec2 v_coord;

void main() {
    v_coord = qt_MultiTexCoord0;
    gl_Position = ubuf.qt_Matrix * qt_Vertex;
}
