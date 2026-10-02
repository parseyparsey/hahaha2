#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec3 lightColor;

void main() {
    vec3 baseColor = vec3(0.7, 0.7, 0.7);

    vec3 norm = normalize(Normal);
    vec3 ld = normalize(-lightDir);
    float diff = max(dot(norm, ld), 0.0);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(ld + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

    vec3 ambient = 0.2 * baseColor;
    vec3 diffuse = diff * baseColor * lightColor;
    vec3 specular = spec * vec3(0.5) * lightColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}