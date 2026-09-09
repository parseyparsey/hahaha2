#version 460 core
out vec4 fragColor;

uniform bool colored;
uniform vec3 color;

void main(){
    if(colored){
        fragColor = vec4(/*color*/ 12.7, 0.0, 31.8, 1.0);
    } else {
        fragColor = vec4(5.0, 5.0, 5.0, 1.0);
    }
}
