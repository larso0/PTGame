#include "PT.h"
#include <stdio.h>

typedef enum
{
    STATE_RUNNING = 1,
    STATE_FULLSCREEN = 1 << 1,
    STATE_MOUSE_GRABBED = 1 << 2
} State;


static int SetupProgram(GLuint* dst)
{
    GLuint program;
    GLuint shaders[3];
    const char* vshader_src =
        "#version 140\n"
        "float Height(vec2 pos);\n"
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
        "#version 140\n"
        "in float distance;\n"
        "out vec3 fcolor;\n"
        "uniform vec3 color;\n"
        "uniform float viewdistance;\n"
        "void main()\n"
        "{\n"
        "    float start = viewdistance * 0.7f;\n"
        "    float fog = 1.0 - clamp((viewdistance - distance) /\n"
        "                            (viewdistance - start),\n"
        "                            0.0, 1.0);\n"
        "    fcolor = mix(color, vec3(0.5f, 0.5f, 0.5f), fog);\n"
        "}\n";
    if(LoadShader(shaders, GL_VERTEX_SHADER, "Noise.glsl") < 0) return -1;
    if(CreateShader(shaders + 1, GL_VERTEX_SHADER, 1, &vshader_src) < 0)
    {
        glDeleteShader(shaders[0]);
        return -2;
    }
    if(CreateShader(shaders + 2, GL_FRAGMENT_SHADER, 1, &fshader_src) < 0)
    {
        glDeleteShader(shaders[0]);
        glDeleteShader(shaders[1]);
        return -3;
    }
    int err = CreateProgram(&program, 3, shaders);
    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);
    glDeleteShader(shaders[2]);
    if(err < 0) return -3;
    *dst = program;
    return 0;
}

static GLuint CreateGridVertexBuffer(float vd)
{
    int n = 2*(int)ceil(vd);
    float co = -n/2.f; //Center offset
    size_t vs_size = n*n*2*sizeof(float);
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, vs_size, NULL, GL_STATIC_DRAW);
    float* vs = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    int i, j, front = 0;
    for(i = 0; i < n; i++)
    {
        for(j = 0; j < n; j++)
        {
            vs[front++] = i + co;
            vs[front++] = j + co;
        }
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    return buf;
}

static GLuint CreateGridIndexBuffer(int* count, float vd)
{
    int n = 2*(int)ceil(vd);
    size_t is_size = (2*(n-1) + 2*(n-1)*n)*sizeof(GLuint);
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, is_size, NULL, GL_STATIC_DRAW);
    GLuint* is = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    int i, j, front = 0;
    for(i = 0; i < n-1; i++)
    {
        is[front++] = i*n;
        for(j = 0; j < n; j++)
        {
            is[front++] = i*n+j;
            is[front++] = (i+1)*n+j;
        }
        is[front++] = (i+1)*n+n-1;
    }
    *count = front;
    glUnmapBuffer(GL_ARRAY_BUFFER);
    return buf;
}

int main(int argc, char** argv)
{
    Settings settings;
    ConstructSettings(&settings);
    LoadSettingsFile(&settings, "settings.ini");
    if(Init(&settings) < 0) return -1;

    GLuint program;
    if(SetupProgram(&program) < 0) return -2;

    GLuint grid_vbuf = CreateGridVertexBuffer(settings.graphics.viewdistance);
    int grid_icount;
    GLuint grid_ibuf = CreateGridIndexBuffer(&grid_icount,
                                             settings.graphics.viewdistance);

    glUseProgram(program);
    GLint grid_pos_loc = glGetAttribLocation(program, "grid_pos");
    GLint grid_world_mat_loc = glGetUniformLocation(program, "world_mat");
    GLint grid_view_mat_loc = glGetUniformLocation(program, "view_mat");
    GLint grid_proj_mat_loc = glGetUniformLocation(program, "proj_mat");
    GLint grid_color_loc = glGetUniformLocation(program, "color");
    GLint grid_viewdistance_loc = glGetUniformLocation(program, "viewdistance");

    glUniform3f(grid_color_loc, 0.f, 0.6f, 0.f);
    glUniform1f(grid_viewdistance_loc, settings.graphics.viewdistance);

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
    glViewport(0, 0, settings.video.width, settings.video.height);

    Node grid_node;
    ConstructNode(&grid_node);
    Camera camera;
    ConstructCamera(&camera);
    mat4x4 projection_matrix;
    mat4x4_perspective(projection_matrix,
                       settings.video.pfov,
                       settings.video.width/(float)settings.video.height,
                       settings.video.pnear,
                       settings.video.pfar);
    glUniformMatrix4fv(grid_proj_mat_loc, 1, GL_FALSE,
                       (const float*)projection_matrix);

    float speed = 10.f;
    Uint32 ticks = SDL_GetTicks();
    State state = STATE_RUNNING | (settings.video.fullscreen ? STATE_FULLSCREEN : 0);
    while(state & STATE_RUNNING)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        glUniformMatrix4fv(grid_world_mat_loc, 1, GL_FALSE,
                           (const float*)grid_node.world_matrix);
        glUniformMatrix4fv(grid_view_mat_loc, 1, GL_FALSE,
                           (const float*)camera.view_matrix);
        glBindVertexArray(grid_vao);
        glDrawElements(GL_TRIANGLE_STRIP, grid_icount, GL_UNSIGNED_INT, NULL);
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
                case SDL_SCANCODE_LSHIFT:
                    speed = settings.controls.speed1;
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.scancode)
                {
                case SDL_SCANCODE_LSHIFT:
                    speed = settings.controls.speed2;
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
                    mat4x4_perspective(projection_matrix,
                                       settings.video.pfov,
                                       w/(float)h,
                                       settings.video.pnear,
                                       settings.video.pfar);
                    glUniformMatrix4fv(grid_proj_mat_loc, 1, GL_FALSE,
                                       (const float*)projection_matrix);
                }
                break;
            case SDL_MOUSEMOTION:
                if(state & STATE_MOUSE_GRABBED)
                {
                    camera.yaw -= settings.controls.xsensitivity *
                                  event.motion.xrel;
                    camera.pitch -= settings.controls.xsensitivity *
                                    event.motion.yrel;
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
        vec3 movement;
        int i;
        for(i = 0; i < 3; i++) movement[i] = 0.f;
        SDL_bool moved = SDL_FALSE;
        if(keys[SDL_SCANCODE_W] && !keys[SDL_SCANCODE_S])
        {
            vec3_add(movement, movement, camera.direction);
            moved = SDL_TRUE;
        }
        else if(keys[SDL_SCANCODE_S] && !keys[SDL_SCANCODE_W])
        {
            vec3_sub(movement, movement, camera.direction);
            moved = SDL_TRUE;
        }
        if(keys[SDL_SCANCODE_A] && !keys[SDL_SCANCODE_D])
        {
            vec3_sub(movement, movement, camera.right);
            moved = SDL_TRUE;
        }
        else if(keys[SDL_SCANCODE_D] && !keys[SDL_SCANCODE_A])
        {
            vec3_add(movement, movement, camera.right);
            moved = SDL_TRUE;
        }
        if(keys[SDL_SCANCODE_Q] && !keys[SDL_SCANCODE_E])
        {
            vec3_sub(movement, movement, camera.up);
            moved = SDL_TRUE;
        }
        else if(keys[SDL_SCANCODE_E] && !keys[SDL_SCANCODE_Q])
        {
            vec3_add(movement, movement, camera.up);
            moved = SDL_TRUE;
        }

        if(moved)
        {
            vec3_norm(movement, movement);
            vec3_scale(movement, movement, delta * speed);
            vec3_add(camera.node.translation, camera.node.translation,
                     movement);
        }

        UpdateCamera(&camera);

        if(moved)
        {
            vec2 tmp1;
            tmp1[0] = camera.node.position[0];
            tmp1[1] = camera.node.position[2];
            vec2 tmp2;
            tmp2[0] = grid_node.position[0];
            tmp2[1] = grid_node.position[2];
            vec2_sub(tmp1, tmp1, tmp2);
            tmp1[0] = floor(tmp1[0]);
            tmp1[1] = floor(tmp1[1]);
            grid_node.translation[0] += tmp1[0];
            grid_node.translation[2] += tmp1[1];
            UpdateNode(&grid_node);
        }
    }

    glDeleteVertexArrays(1, &grid_vao);
    glDeleteBuffers(1, &grid_vbuf);
    glDeleteBuffers(1, &grid_ibuf);
    glDeleteProgram(program);

    return 0;
}
