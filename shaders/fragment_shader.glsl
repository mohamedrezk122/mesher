// Mesh/object fragment shader
#version 460 core
in vec3 v_pos;
in vec3 v_color;
in vec3 v_normal;
out vec4 color;

uniform vec3 u_LightSource1;
uniform vec3 u_LightSource2;

vec3 normal = normalize(v_normal);
vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
vec3 ambient = 0.1f * light_color;
float specular_strength = 0.1f;

vec3 compute_pixel_light(vec3 source){
    vec3 light_dir = normalize(source - v_pos);  
    float diff = max(0.0f, dot(normal, source));
    vec3 diffuse = diff * light_color * 0.25f;

    vec3 reflect_dir = reflect(-light_dir, normal);  
    float spec = pow(max(dot(light_dir, reflect_dir), 0.0), 8);
    vec3 specular = specular_strength * spec * light_color;  
    return diffuse + specular + ambient ;
}

void main(){
    vec3 result; 
    result = compute_pixel_light(u_LightSource1);
    result += compute_pixel_light(u_LightSource2);
    color = vec4(result * v_color, 1.0f);
}