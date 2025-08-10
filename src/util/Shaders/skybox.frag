#version 330 core
in vec2 uvCoords;

out vec4 FragColor;


uniform sampler2D boxFace;

void main() {
    FragColor = texture(boxFace, uvCoords);
    //FragColor = vec4(FragColor.rgb, 1.0); // Ensure alpha is 1.0 for the skybox
    //FragColor = vec4(uvCoords, 0.0, 1.0);
}