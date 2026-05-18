#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

in vec3 FragPos[];
uniform mat4 shadowMatrices[6];
out vec3 FragPosOut;

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; ++i)
        {
            FragPosOut = FragPos[i];
            gl_Position = shadowMatrices[face] * vec4(FragPos[i], 1.0);
            EmitVertex();
        }
        EndPrimitive();
    }
}

/*#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

in vec3 test00[]; // test

uniform mat4 shadowMatrices[6];
out vec4 FragPos;

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; ++i)
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * vec4(test00[i], 1.0);//FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}*/

/*
// pshadow_depth.gs
#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

in vec3 FragPos[];
uniform mat4 shadowMatrices[6];
out vec3 FragPosOut;

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; ++i)
        {
            FragPosOut = FragPos[i];
            gl_Position = shadowMatrices[face] * vec4(FragPos[i], 1.0);
            EmitVertex();
        }
        EndPrimitive();
    }
}*/