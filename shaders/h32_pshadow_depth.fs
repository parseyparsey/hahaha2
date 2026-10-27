#version 460 core
in vec3 FragPosOut;
uniform vec3 lightPos;
uniform float far_plane;

void main(){
    float lightDistance = length(FragPosOut - lightPos);
    gl_FragDepth = lightDistance / far_plane;
}