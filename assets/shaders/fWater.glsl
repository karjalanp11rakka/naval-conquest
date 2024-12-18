#version 330 core
in vec3 FragPos;
out vec4 FragColor;

in float zPosOffset;

void main()
{
    FragColor = vec4(.3f, .4f + cos(zPosOffset / 10.f) / 10.f, .8f + (0.1f + zPosOffset / 10.f), 1.f);
}