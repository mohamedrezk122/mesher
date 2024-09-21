// Mesh/object vertex shader

#version 460 core
layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
layout(location=2) in vec3 normal;

out vec3 v_pos;
out vec3 v_color;
out vec3 v_normal;

uniform mat4 u_Proj;
uniform mat4 u_Model;
uniform mat4 u_View;

void main(){
    v_pos = position;
    v_color = color;
    v_normal = normal;
    gl_Position = u_Proj * u_View * u_Model * vec4(position, 1.0f);
}