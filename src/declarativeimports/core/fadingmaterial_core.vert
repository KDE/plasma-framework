#version 150 core
uniform highp mat4 qt_Matrix;
in highp vec4 qt_Vertex;
in highp vec2 qt_MultiTexCoord0;
out highp vec2 v_coord;
void main() {
    v_coord = qt_MultiTexCoord0;
    gl_Position = qt_Matrix * qt_Vertex;
}
