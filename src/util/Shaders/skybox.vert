#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;

out vec2 uvCoords;

uniform mat4 view;
uniform mat4 projection;

void main() {
    // Last coordinate is 0 for avoiding translation
    vec4 worldPos = vec4(vertex, 1.0);
    gl_Position = projection * view * worldPos;
    uvCoords = uv;
}
