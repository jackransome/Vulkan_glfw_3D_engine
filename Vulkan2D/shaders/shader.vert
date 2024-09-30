#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} ubo;

layout(binding=2, set=0) readonly buffer TRANSFORM_DATA {
    mat4 transform[500];
} objects;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inDiffuse;
layout(location = 4) in vec3 inSpecular;
layout(location = 5) in vec3 inAmbient;
layout(location = 6) in float inShininess;
layout(location = 7) in float inOpacity;

layout(location = 0) out vec3 fragDiffuse;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 FragPos;  
layout(location = 3) out vec3 Normal;
layout(location = 4) out vec3 cameraPos;
layout(location = 5) out vec3 fragSpecular;
layout(location = 6) out vec3 fragAmbient;
layout(location = 7) out float fragShininess;
layout(location = 8) out float fragOpacity;

layout(push_constant) uniform transformData {
    layout(offset = 0) int index;               // Offset 0
    layout(offset = 4) float textureOffsetX;    // Offset 4
    layout(offset = 8) float textureOffsetY;    // Offset 8
    layout(offset = 12) float textureWidth;     // Offset 12
    layout(offset = 16) float textureHeight;    // Offset 16
} object;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.proj * ubo.view * objects.transform[object.index+gl_InstanceIndex] * vec4(inPosition, 1.0);
    fragDiffuse = inDiffuse;
    FragPos = vec3(objects.transform[object.index+gl_InstanceIndex] * vec4(inPosition, 1.0));
    Normal = mat3(transpose(inverse(objects.transform[object.index+gl_InstanceIndex]))) * inNormal;
    cameraPos = ubo.cameraPos;
    fragTexCoord = inTexCoord*vec2(object.textureWidth, object.textureHeight) + vec2(object.textureOffsetX, object.textureOffsetY);
    fragSpecular = inSpecular;
    fragAmbient = inAmbient;
    fragShininess = inShininess;
    fragOpacity = inOpacity;
}