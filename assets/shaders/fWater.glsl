#version 330 core
in vec3 FragPos;
out vec4 FragColor;

in float zPosOffset;

void main()
{
    FragColor = vec4(0.3f, 0.4f + cos(zPosOffset * .1f) / 10.f, 0.8f + (0.1f + zPosOffset / 10.f), 1.0f);
}