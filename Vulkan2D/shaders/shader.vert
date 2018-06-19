#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    vec3 position;
	vec3 cameraPos;
} ubo;

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
	gl_Position = vec4(inPosition.x + ubo.cameraPos.x + position.x, inPosition.y + ubo.cameraPos.y + position.y, inPosition.z + ubo.cameraPos.z + position.z, 1);
	//vec4(inPosition.x, inPosition.y, inPosition.z, 1);//
	fragColor = inColor;

	fragTexCoord = inTexCoord;
}
