in vec2 grid_pos;
out float fog;
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;
uniform float viewdistance;
void main()
{
    vec4 pos = world_mat * vec4(grid_pos.x, 0.f,
                                grid_pos.y, 1.f);
    pos.y = Height(pos.xz);
    pos = view_mat * pos;
    float distance = length(pos.xyz);
    float start = viewdistance * 0.7f;
    fog = 1.0 - clamp((viewdistance - distance)
					  (viewdistance - start), 0.0, 1.0);
    gl_Position = proj_mat * pos;
}
