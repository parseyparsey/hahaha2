#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
//nmap
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
    mat3 TBN;
} vs_out;

uniform mat4 model;
//uniform mat4 LightSpaceMatrix;

void main(){

    gl_Position = vec4(aPos.x * 0.1, aPos.y * 0.1, 0.0, 1.0);
    vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoord;
    vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    /*vs_out.fragPos = vec3(model * vec4(aPos, 1.0));  
    vs_out.TexCoords = aTexCoord;

    vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    //vs_out.fragPosLightSpace = LightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
    
    //nmap_start
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
    //nmap_end
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);*/
}
