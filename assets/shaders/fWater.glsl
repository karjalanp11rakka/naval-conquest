#version 330 core
in vec3 FragPos;
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.3f, 0.4f, 1.0f * (abs(cos(FragPos.x * FragPos.z)) / 1.5f +.3f), 1.0f);
}