#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
out float zPosOffset;

float random(float seed)
{
    return fract(sin(seed) * 43758.5453);
}

void main()
{
    zPosOffset = sin(random(aPos.x * aPos.y) * 100.f + time * 2.f);
    vec4 modelPosition = model * vec4(aPos.x, aPos.y, aPos.z + .0075f + zPosOffset / 150.f, 1.f);
    FragPos = modelPosition.xyz / modelPosition.w;

    gl_Position = projection * view * modelPosition;
}