#include "PT.h"

typedef enum
{
    STATE_RUNNING = 1,
    STATE_FULLSCREEN = 1 << 1,
    STATE_MOUSE_GRABBED = 1 << 2
} State;

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

static GLuint CreateGridVertexBuffer()
{
    int n = 2*(int)ceil(settings.graphics.viewdistance);
    float co = -n/2.f; //Center offset
    size_t vs_size = n*n*2*sizeof(float);
    float* vs = malloc(vs_size);
    int i, j, front = 0;
    for(i = 0; i < n; i++)
    {
        for(j = 0; j < n; j++)
        {
            vs[front++] = i + co;
            vs[front++] = j + co;
        }
    }
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, vs_size, vs, GL_STATIC_DRAW);
    free(vs);
    return buf;
}

static GLuint CreateGridIndexBuffer(int* count)
{
    int n = 2*(int)ceil(settings.graphics.viewdistance);
    size_t is_size = (2*(n-1) + 2*(n-1)*n)*sizeof(GLuint);
    GLuint* is = malloc(is_size);
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

    GLuint program;
    if(SetupProgram(&program) < 0) return -1;

    GLuint grid_vbuf = CreateGridVertexBuffer();
    int grid_icount;
    GLuint grid_ibuf = CreateGridIndexBuffer(&grid_icount);

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
        glUniformMatrix4fv(grid_proj_mat_loc, 1, GL_FALSE,
                           (const float*)projection_matrix);
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
        tmp1[0] = floor(tmp1[0]);
        tmp1[1] = floor(tmp1[1]);
        grid_node.translation[0] += tmp1[0];
        grid_node.translation[2] += tmp1[1];
        UpdateNode(&grid_node);
    }

    return 0;
}
