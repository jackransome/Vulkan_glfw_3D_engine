#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float time;
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;
void main() {
	
	if (fragTexCoord.x >= 0.99) {
		outColor = vec4(fragColor.x, fragColor.y, fragColor.z, 1);
	}
	else{
		if (texture(texSampler, fragTexCoord).w > 0.0){
			outColor = texture(texSampler, fragTexCoord);
		}
		else{
			discard;
		}
	}
}

