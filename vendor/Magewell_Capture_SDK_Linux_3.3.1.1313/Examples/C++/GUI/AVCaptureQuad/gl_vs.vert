attribute vec4 qt_Vertex;
attribute vec2 qt_TexureIn;
varying vec2 qt_TexCoord0;

void main(void)
{
    gl_Position = qt_Vertex;
    qt_TexCoord0= qt_TexureIn;

}
