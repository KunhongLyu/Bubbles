#version 330 core
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;

void main() {
    vec3 norm = normalize(Normal);
    vec3 light = normalize(-lightDir);
    
    vec3 ambient = ambientColor;
    
    float diff = max(dot(norm, light), 0.0);
    vec3 diffuse = diff * diffuseColor * lightColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfway = normalize(light + viewDir);
    float spec = pow(max(dot(norm, halfway), 0.0), shininess);
    vec3 specular = spec * specularColor * lightColor;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}