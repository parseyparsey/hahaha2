#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 shadowMatrices[6];
out vec3 FragPos;

void main(){
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = shadowMatrices[0] * vec4(FragPos, 1.0);
}

/*// pshadow_depth.vs
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 shadowMatrices[6];  // use face 0 for a valid clip pos
out vec3 FragPos;

void main(){
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = shadowMatrices[0] * vec4(FragPos, 1.0); // valid clip space
}*/

/*
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;
out vec3 FragPos;

void main(){
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = lightSpaceMatrix * vec4(FragPos, 1.0);
}*/