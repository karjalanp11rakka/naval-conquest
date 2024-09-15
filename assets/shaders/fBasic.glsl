#version 330 core
out vec4 FragColor;
in vec3 position;
uniform vec3 color;

void main()
{
    FragColor = vec4(color.x * clamp(position.x, 0.8f, 1.0f), color.y * clamp(position.y, 0.8f, 1.0f), color.z * clamp(position.z, 0.8f, 1.0f), 1.0);
}