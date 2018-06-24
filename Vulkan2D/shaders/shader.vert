#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
	vec3 cameraPos;
	mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
layout(binding = 1) uniform DynamicUniformBufferObject {
    vec3 position;
} dynamicUbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out float time;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	gl_Position = vec4(inPosition + ubo.cameraPos + dynamicUbo.position, 1.0);
	//gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	fragColor = inColor;

	fragTexCoord = inTexCoord;
}
