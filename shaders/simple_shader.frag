
#version 450

layout (location = 0) in vec3 fragColor;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
	mat4 transform;
	vec3 color;
} push;

void main() {
//R^4 vector, gives square. X increses right, y increases down. 0.0 is the z value, 1.0 is what we divide everything by (used for normalizing).
   outColor = vec4(fragColor, 1.0);
}