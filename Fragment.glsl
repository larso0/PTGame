#version 140
in float distance;
out vec3 fcolor;
uniform vec3 color;
uniform float viewdistance;
void main()
{
    float start = viewdistance * 0.7f;
    float fog = 1.0 - clamp((viewdistance - distance) /
                            (viewdistance - start),
                            0.0, 1.0);
    fcolor = mix(color, vec3(0.5f, 0.5f, 0.5f), fog);
}