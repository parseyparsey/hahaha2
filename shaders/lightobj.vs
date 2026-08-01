#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

layout (std140) uniform matrices{
    uniform mat4 view;
    uniform mat4 projection;
};

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 TexCoords;
    vec4 fragPosLightSpace;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 model;
uniform mat4 LightSpaceMatrix;

//nmap
uniform vec3 lightPos;
uniform vec3 viewPos;

//out vec3 normal;
//out vec3 fragPos;
//out vec2 TexCoords;

void main(){
    vs_out.fragPos = vec3(model * vec4(aPos, 1.0));  
    vs_out.TexCoords = aTexCoord;

    vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.fragPosLightSpace = LightSpaceMatrix * vec4(vs_out.fragPos, 1.0);

    //nmap
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.fragPos;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
