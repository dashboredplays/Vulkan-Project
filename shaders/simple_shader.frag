//version 4.5 of GLSL
#version 450

layout (location = 0) out vec4 outColor;

void main() {
    //red, green, blue, alpha
    //This will run on a per pixel basis
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}