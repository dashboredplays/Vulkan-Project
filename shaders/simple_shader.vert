#version 450

vec2 positions[3] = vec2[] (
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

void main() {
	//R^4 vector, gives square. X increses right, y increases down. 0.0 is the z value, 1.0 is what we divide everything by (used for normalizing).
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}