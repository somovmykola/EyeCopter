#version 330 core

out vec3 fragmentColor;

uniform mat4 MVP;

void main() {
	gl_Position = gl_Position;
	//gl_Position = MVP * gl_Position;
	fragmentColor = vec3(gl_Color);
}