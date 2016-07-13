#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <stdlib.h>
#include "Shaders.h"
#include "Node.h"
#include "Camera.h"
#include "Util.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

typedef enum
{
    STATE_RUNNING = 1,
    STATE_FULLSCREEN = 1 << 1,
    STATE_MOUSE_GRABBED = 1 << 2
} State;

static SDL_Window* window;
static SDL_GLContext context;

static int Init()
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        Message("Error: Could not initialize SDL.", SDL_GetError());
        return -1;
    }

    if(SDL_GL_LoadLibrary(NULL) < 0)
    {
        Message("Error: Could not load GL library.", SDL_GetError());
        SDL_Quit();
        return -2;
    }

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    window = SDL_CreateWindow
    (
        "PTGame",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if(!window)
    {
        Message("Error: Could not create window.", SDL_GetError());
        SDL_Quit();
        return -3;
    }

    context = SDL_GL_CreateContext(window);
    if(!context)
    {
        Message("Error: Could not create GL context.", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -4;
    }

    if(SDL_GL_SetSwapInterval(-1) < 0)
    {
        if(SDL_GL_SetSwapInterval(1) < 0)
        {
            Message("Warning: VSync not supported.", SDL_GetError());
        }
    }

    if(!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        Message("Error: Could not load GL extensions.",
                "Unable to load OpenGL extensions with glad loader.");
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -5;
    }

    return 0;
}

static void Quit()
{
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static int SetupProgram(GLuint* dst)
{
    GLuint program;
    GLuint shaders[2];
    const char* vshader_src =
        "in vec2 grid_pos;\n"
        "out float distance;\n"
        "uniform mat4 world_mat;\n"
        "uniform mat4 view_mat;\n"
        "uniform mat4 proj_mat;\n"
        "void main()\n"
        "{\n"
        "    vec4 pos = world_mat * vec4(grid_pos.x, 0.f,\n"
        "                                grid_pos.y, 1.f);\n"
        "    pos.y = Height(pos.xz);\n"
        "    pos = view_mat * pos;\n"
        "    distance = length(pos.xyz);\n"
        "    gl_Position = proj_mat * pos;\n"
        "}\n";
    const char* fshader_src =
        "in float distance;\n"
        "out vec3 fcolor;\n"
        "uniform vec3 color;"
        "void main()\n"
        "{\n"
        "    const float start = 64.f, end = 100.f;\n"
        "    float fog = 1.0 - clamp((end - distance) /\n"
        "                            (end - start),\n"
        "                            0.0, 1.0);\n"
        "    fcolor = mix(color, vec3(0.5f, 0.5f, 0.5f), fog);\n"
        "}\n";
    const char* strings[3];
    strings[0] = GLSL_VERSION;
    strings[1] = GLSL_HEIGHT_FUNCTION;
    strings[2] = vshader_src;
    if(CreateShader(shaders, GL_VERTEX_SHADER, 3, strings) < 0) return -1;
    strings[1] = fshader_src;
    if(CreateShader(shaders + 1, GL_FRAGMENT_SHADER, 2, strings) < 0)
        return -2;
    int err = CreateProgram(&program, 2, shaders);
    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);
    if(err < 0) return -3;
    *dst = program;
    return 0;
}

static GLuint CreateGridVertexBuffer(int res)
{
    size_t vs_size = res*res*2*sizeof(float);
    float* vs = malloc(vs_size);
    int i, j, front = 0;
    for(i = 0; i < res; i++)
    {
        for(j = 0; j < res; j++)
        {
            vs[front++] = i - 96.f;
            vs[front++] = j - 96.f;
        }
    }
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, vs_size, vs, GL_STATIC_DRAW);
    free(vs);
    return buf;
}

static GLuint CreateGridIndexBuffer(int* count, int res)
{
    size_t is_size = (2*(res-1) + 2*(res-1)*res)*sizeof(GLushort);
    GLushort* is = malloc(is_size);
    int i, j, front = 0;
    for(i = 0; i < res-1; i++)
    {
        is[front++] = i*res;
        for(j = 0; j < res; j++)
        {
            is[front++] = i*res+j;
            is[front++] = (i+1)*res+j;
        }
        is[front++] = (i+1)*res+res-1;
    }
    *count = front;
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, is_size, is, GL_STATIC_DRAW);
    free(is);
    return buf;
}

int main(int argc, char** argv)
{
    if(Init() < 0) return 1;
    atexit(Quit);

    GLuint program;
    if(SetupProgram(&program) < 0) return -1;

    GLuint grid_vbuf = CreateGridVertexBuffer(192);
    int grid_icount;
    GLuint grid_ibuf = CreateGridIndexBuffer(&grid_icount, 192);

    glUseProgram(program);
    GLint grid_pos_loc = glGetAttribLocation(program, "grid_pos");
    GLint grid_world_mat_loc = glGetUniformLocation(program, "world_mat");
    GLint grid_view_mat_loc = glGetUniformLocation(program, "view_mat");
    GLint grid_proj_mat_loc = glGetUniformLocation(program, "proj_mat");
    GLint grid_color_loc = glGetUniformLocation(program, "color");

    glUniform3f(grid_color_loc, 0.f, 0.6f, 0.f);

    GLuint grid_vao;
    glGenVertexArrays(1, &grid_vao);
    glBindVertexArray(grid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbuf);
    glEnableVertexAttribArray(grid_pos_loc);
    glVertexAttribPointer(grid_pos_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ibuf);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    Node grid_node;
    ConstructNode(&grid_node);
    Camera camera;
    ConstructCamera(&camera);
    float perspective_fov = 45.f, perspective_near = 0.01f, perspective_far = 1000.f;
    mat4x4 projection_matrix;
    mat4x4_perspective(projection_matrix, perspective_fov,
                       WINDOW_WIDTH/(float)WINDOW_HEIGHT,
                       perspective_near, perspective_far);

    Uint32 ticks = SDL_GetTicks();
    State state = STATE_RUNNING;
    while(state & STATE_RUNNING)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        glUniformMatrix4fv(grid_world_mat_loc, 1, GL_FALSE,
                           (const float*)grid_node.world_matrix);
        glUniformMatrix4fv(grid_view_mat_loc, 1, GL_FALSE,
                           (const float*)camera.view_matrix);
        glUniformMatrix4fv(grid_proj_mat_loc, 1, GL_FALSE,
                           (const float*)projection_matrix);
        glBindVertexArray(grid_vao);
        glDrawElements(GL_TRIANGLE_STRIP, grid_icount, GL_UNSIGNED_SHORT, NULL);
        SDL_GL_SwapWindow(window);

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
                state &= ~STATE_RUNNING;
                break;
            case SDL_KEYUP:
                switch(event.key.keysym.scancode)
                {
                case SDL_SCANCODE_F11:
                    state ^= STATE_FULLSCREEN;
                    SDL_SetWindowFullscreen
                    (
                        window,
                        state & STATE_FULLSCREEN ?
                        SDL_WINDOW_FULLSCREEN_DESKTOP : 0
                    );
                    break;
                case SDL_SCANCODE_ESCAPE:
                    state ^= STATE_MOUSE_GRABBED;
                    SDL_SetRelativeMouseMode(state & STATE_MOUSE_GRABBED);
                    break;
                default:
                    break;
                }
                break;
            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    int w = event.window.data1;
                    int h = event.window.data2;
                    glViewport(0, 0, w, h);
                    mat4x4_perspective(projection_matrix, perspective_fov,
                                       w/(float)h, perspective_near, perspective_far);
                }
                break;
            case SDL_MOUSEMOTION:
                if(state & STATE_MOUSE_GRABBED)
                {
                    camera.yaw -= 0.01f*event.motion.xrel;
                    camera.pitch -= 0.01f*event.motion.yrel;
                }
                break;
            default:
                break;
            }
        }
        Uint32 nticks = SDL_GetTicks();
        float delta = (nticks - ticks)/1000.f;
        ticks = nticks;

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        vec3 translation;
        if(keys[SDL_SCANCODE_W])
        {
            vec3_scale(translation, camera.direction, delta*10.f);
            vec3_add(camera.node.translation,
                     camera.node.translation,
                     translation);
        }
        if(keys[SDL_SCANCODE_S])
        {
            vec3_scale(translation, camera.direction, -delta*10.f);
            vec3_add(camera.node.translation,
                     camera.node.translation,
                     translation);
        }
        if(keys[SDL_SCANCODE_A])
        {
            vec3_scale(translation, camera.right, -delta*10.f);
            vec3_add(camera.node.translation,
                     camera.node.translation,
                     translation);
        }
        if(keys[SDL_SCANCODE_D])
        {
            vec3_scale(translation, camera.right, delta*10.f);
            vec3_add(camera.node.translation,
                     camera.node.translation,
                     translation);
        }
        if(keys[SDL_SCANCODE_Q])
        {
            vec3_scale(translation, camera.up, -delta*10.f);
            vec3_add(camera.node.translation,
                     camera.node.translation,
                     translation);
        }
        if(keys[SDL_SCANCODE_E])
        {
            vec3_scale(translation, camera.up, delta*10.f);
            vec3_add(camera.node.translation,
                     camera.node.translation,
                     translation);
        }
        UpdateCamera(&camera);

        vec2 tmp1; tmp1[0] = camera.node.position[0];
        tmp1[1] = camera.node.position[2];
        vec2 tmp2; tmp2[0] = grid_node.position[0];
        tmp2[1] = grid_node.position[2];
        vec2_sub(tmp1, tmp1, tmp2);
        tmp1[0] = floor(tmp1[0]/64.f)*64.f;
        tmp1[1] = floor(tmp1[1]/64.f)*64.f;
        grid_node.translation[0] += tmp1[0];
        grid_node.translation[2] += tmp1[1];
        UpdateNode(&grid_node);
    }

    return 0;
}
