#version 460 core
out vec4 fragColor;

uniform bool colored;
uniform vec3 color;

void main(){
    if(colored){
        fragColor = vec4(/*color*/0.6, 0.0, 1.0, 1.0);
    } else {
        fragColor = vec4(1.0);
    }
}
