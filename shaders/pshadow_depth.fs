/*#version 460 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);

    lightDistance = lightDistance / far_plane;

    gl_FragDepth = 0.5;//lightDistance;
}*/

#version 460 core
in vec3 FragPosOut;
uniform vec3 lightPos;
uniform float far_plane;

void main(){
    float lightDistance = length(FragPosOut - lightPos);
    gl_FragDepth = lightDistance / far_plane;
}
/*
#version 460 core
in vec3 FragPos;
uniform vec3 lightPos;
uniform float far_plane;

void main(){
    float lightDistance = length(FragPos - lightPos);
    gl_FragDepth = lightDistance / far_plane;
}*/