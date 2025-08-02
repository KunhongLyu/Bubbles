#version 330 core
layout (location = 0) in vec2 aPos;

uniform float left;
uniform float right;
uniform float top;
uniform float bottom;

void main() {
    float posx = (1-(aPos.x+1)/2) * left + (aPos.x+1)/2 * right;
    float posy = (1-(aPos.y+1)/2) * bottom + (aPos.y+1)/2 * top;
    gl_Position = vec4(posx, posy, 0.0, 1.0);
}