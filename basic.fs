in float fog;
out vec3 fcolor;
uniform vec3 color;
void main()
{
    fcolor = mix(color, vec3(0.5f, 0.5f, 0.5f), fog);
}
