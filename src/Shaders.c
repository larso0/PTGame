#include "Shaders.h"
#include <stdlib.h>
#include <stdio.h>
#include "PT.h"

int CreateShader(GLuint* dst, GLenum type, GLsizei count, const char** source)
{
    GLuint handle = glCreateShader(type);
    glShaderSource(handle, count, source, NULL);
    glCompileShader(handle);
    GLint status = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
    {
        GLint len = 0;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &len);
        char* log = malloc(len);
        glGetShaderInfoLog(handle, len, &len, log);
        Message("Error when compiling shader", log);
        free(log);
        glDeleteShader(handle);
        return -1;
    }
    *dst = handle;
    return 0;
}

int LoadShader(GLuint* dst, GLenum type, const char* file)
{
    FILE* f = fopen(file, "r");
    if(!f)
    {
        Message("Error", "Could not open file.");
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* src = malloc(len + 1);
    if(!src)
    {
        fclose(f);
        Message("Error", "Memory allocation error.");
        return -2;
    }
    fread(src, 1, len, f);
    fclose(f);
    src[len] = 0;
    int r = CreateShader(dst, type, 1, (const char**)&src);
    free(src);
    if(r < 0) return -3;
    else return 0;
}

int CreateProgram(GLuint* dst, GLsizei count, GLuint* shaders)
{
    GLuint handle = glCreateProgram();
    int i;
    for(i = 0; i < count; i++)
    {
        glAttachShader(handle, shaders[i]);
        if(glGetError() != GL_NO_ERROR) break;
    }
    if(i < count)
    {
        Message("Error", "Error when attaching shader.");
        glDeleteProgram(handle);
        return -1;
    }
    glLinkProgram(handle);
    GLint status = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        GLint len = 0;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &len);
        char* log = malloc(len);
        glGetProgramInfoLog(handle, len, &len, log);
        Message("Error when linking program", log);
        free(log);
        glDeleteProgram(handle);
        return -2;
    }
    *dst = handle;
    return 0;
}
