//version 4.5 of GLSL
#version 450

//in: takes value from vertex buffer
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

//ordering important
layout(push_constant) uniform Push {
	mat2 transform;
	vec2 offset;
	vec3 color;
} push;

void main() {
	//push.transform * position -> remember matrix multiplication order matters!
	gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
}