#ifndef SHADERS_H_
#define SHADERS_H_

#include <glad/glad.h>

int CreateShader(GLuint* dst, GLenum type, GLsizei count, const char** source);
int LoadShader(GLuint* dst, GLenum type, const char* file);
int CreateProgram(GLuint* dst, GLsizei count, GLuint* shaders);

#endif
