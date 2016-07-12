#include "Shaders.h"
#include "Util.h"
#include <stdlib.h>

const char* GLSL_VERSION = "#version 140\n";
const char* GLSL_HEIGHT_FUNCTION =
    "// Simplex 2D noise\n"
    "//\n"
    "vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }\n"
    "\n"
    "float snoise(vec2 v)"
    "{\n"
    "    const vec4 C = vec4(0.211324865405187, 0.366025403784439,\n"
    "                        -0.577350269189626, 0.024390243902439);\n"
    "    vec2 i  = floor(v + dot(v, C.yy) );\n"
    "    vec2 x0 = v -   i + dot(i, C.xx);\n"
    "    vec2 i1;\n"
    "    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);\n"
    "    vec4 x12 = x0.xyxy + C.xxzz;\n"
    "    x12.xy -= i1;\n"
    "    i = mod(i, 289.0);\n"
    "    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))\n"
    "             + i.x + vec3(0.0, i1.x, 1.0 ));\n"
    "    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),\n"
    "                 dot(x12.zw,x12.zw)), 0.0);\n"
    "    m = m*m;\n"
    "    m = m*m;\n"
    "    vec3 x = 2.0 * fract(p * C.www) - 1.0;\n"
    "    vec3 h = abs(x) - 0.5;\n"
    "    vec3 ox = floor(x + 0.5);\n"
    "    vec3 a0 = x - ox;\n"
    "    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );\n"
    "    vec3 g;\n"
    "    g.x  = a0.x  * x0.x  + h.x  * x0.y;\n"
    "    g.yz = a0.yz * x12.xz + h.yz * x12.yw;\n"
    "    return 130.0 * dot(m, g);\n"
    "}\n"
    "\n"
    "float Noise(uint l, vec2 v, float p, float f, float min, float max)\n"
    "{\n"
    "    float maxAmp = 0.f;\n"
    "    float amp = 1.f;\n"
    "    float noise = 0.f;\n"
    "    for(uint i = uint(0); i < l; i++)\n"
    "    {\n"
    "        noise += snoise(v*f)*amp;\n"
    "        maxAmp += amp;\n"
    "        amp *= p;\n"
    "        f *= 2.f;\n"
    "    }\n"
    "    noise /= maxAmp;\n"
    "    noise = noise * (max - min) / 2.f + (max + min) / 2.f;\n"
    "    return noise;\n"
    "}\n"
    "float Height(vec2 pos)\n"
    "{\n"
    "    return Noise(uint(16), pos*0.0005f, 5.f, 0.001f, 0.f, 20.f);\n"
    "}\n";

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
