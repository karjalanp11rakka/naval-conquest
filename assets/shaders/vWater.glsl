#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
out vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 modelPosition = model * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
    position = modelPosition.xyz / modelPosition.w;

    gl_Position = projection * view * modelPosition;
}