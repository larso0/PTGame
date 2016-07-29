#version 140
float Height(vec2 pos);
in vec2 grid_pos;
out float distance;
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;
void main()
{
    vec4 pos = world_mat * vec4(grid_pos.x, 0.f,
                                grid_pos.y, 1.f);
    pos.y = Height(pos.xz);
    pos = view_mat * pos;
    distance = length(pos.xyz);
    gl_Position = proj_mat * pos;
}