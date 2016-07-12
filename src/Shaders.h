#ifndef SHADERS_H_
#define SHADERS_H_

#include <glad/glad.h>

extern const char* GLSL_VERSION;
extern const char* GLSL_HEIGHT_FUNCTION;

int CreateShader(GLuint* dst, GLenum type, GLsizei count, const char** source);
int CreateProgram(GLuint* dst, GLsizei count, GLuint* shaders);

#endif
