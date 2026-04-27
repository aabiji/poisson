#version 460 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

uniform mat4 view;
uniform mat4 projection;
layout(binding = 1, std430) readonly buffer model_buffer {
    mat4 model_matrices[];
};

out vec2 texture_coords;
out vec3 normal;

void main() {
    mat4 m = model_matrices[gl_InstanceID];
    gl_Position = projection * view * m * vec4(in_pos, 1.0);
    texture_coords = in_uv;
    normal = in_normal;
}