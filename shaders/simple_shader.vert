//version 4.5 of GLSL
#version 450

//in: takes value from vertex buffer
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec3 fragColor;

//ordering important
layout(push_constant) uniform Push {
	mat4 transform; //projection * view * model
	mat4 normalMatrix;
} push;

//when working with light, always normalize vectors
const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.02;

void main() {
	//push.transform * position -> remember matrix multiplication order matters!
	// gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
	gl_Position = push.transform * vec4(position, 1.0);

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	float lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0);

	fragColor = lightIntensity * color;
}
//can represent translation with a higher dimension matrix (offsets in last column, (0, 0, 1) at bottom row multiplied by 1). This is called 2d affine transformation.
//homogeneous coordinates: 3d coordinates with 4th component (w) that is 1. This allows for translation with matrix multiplication.
//first translate object so anchor point goes to origin, rotate, then translate back to original position.
//relative offsets / distances are affected by LT
//example: v = [3, 2, 1]. which represents a point and is affected by the translation.
///		   u = [3, 2, 0]. which represents a distance / offset and is not affected by the translation.
//this is why 4d matrices are used