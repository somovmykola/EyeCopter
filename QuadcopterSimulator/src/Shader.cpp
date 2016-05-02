#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "Shader.h"

#define VERBOSE_OUTPUT


GLuint LoadShaders(const char* vertex_filename, const char* fragment_filename){
	// Create the shaders
	GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	string vertShaderCode;
	ifstream vertShaderStream(vertex_filename, ios::in);
	if(vertShaderStream.is_open()){
		string line = "";
		while(getline(vertShaderStream, line))
			vertShaderCode += "\n" + line;
		vertShaderStream.close();
	} else {
		fprintf(stderr, "Could not open %s\n", vertex_filename);
		return 0;
	}

	// Read the Fragment Shader code from the file
	string fragShaderCode;
	ifstream fragShaderStream(fragment_filename, ios::in);
	if(fragShaderStream.is_open()){
		string line = "";
		while(getline(fragShaderStream, line))
			fragShaderCode += "\n" + line;
		fragShaderStream.close();
	} else {
		fprintf(stderr, "Could not open %s\n", fragment_filename);
		return 0;
	}

	GLint result = GL_FALSE;
	int infoLogLength;


	// Compile Vertex Shader
#ifdef VERBOSE_OUTPUT
	printf("Compiling shader : %s\n", vertex_filename);
#endif
	char const *vertSourcePointer = vertShaderCode.c_str();
	glShaderSource(vertShaderID, 1, &vertSourcePointer , NULL);
	glCompileShader(vertShaderID);

	// Check Vertex Shader
	glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		vector<char> vertShaderErrorMessage(infoLogLength+1);
		glGetShaderInfoLog(vertShaderID, infoLogLength, NULL, &vertShaderErrorMessage[0]);
		fprintf(stderr, "%s\n", &vertShaderErrorMessage[0]);
	}
	

	// Compile Fragment Shader
#ifdef VERBOSE_OUTPUT
	printf("Compiling shader : %s\n", fragment_filename);
#endif
	char const *fragSourcePointer = fragShaderCode.c_str();
	glShaderSource(fragShaderID, 1, &fragSourcePointer , NULL);
	glCompileShader(fragShaderID);

	// Check Fragment Shader
	glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		vector<char> fragShaderErrorMessage(infoLogLength+1);
		glGetShaderInfoLog(fragShaderID, infoLogLength, NULL, &fragShaderErrorMessage[0]);
		fprintf(stderr, "%s\n", &fragShaderErrorMessage[0]);
	}


	// Link the program
#ifdef VERBOSE_OUTPUT
	printf("Linking program\n");
#endif
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertShaderID);
	glAttachShader(programID, fragShaderID);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0){
		vector<char> programErrorMessage(infoLogLength+1);
		glGetProgramInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
		fprintf(stderr, "%s\n", &programErrorMessage[0]);
	}

	// Cleanup
	glDetachShader(programID, vertShaderID);
	glDetachShader(programID, fragShaderID);
	
	glDeleteShader(vertShaderID);
	glDeleteShader(fragShaderID);

	return programID;
}