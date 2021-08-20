#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject{
  mat4 proj;
  mat4 view;
  mat4 model;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

out gl_PerVertex{
  vec4 gl_Position;
};

void main(){
  gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition.xyz, 1.0);
  fragColor = inColor;
}