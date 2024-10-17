#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

struct LightData {
    vec3 position;
    vec3 color;
    float intensity;
};

layout(std430, binding = 3) readonly buffer LightBuffer {
    LightData lights[500];
} lightBuffer;

layout(push_constant) uniform LightCount {
    layout(offset = 40) int lightCount;
} lightCount;

layout(location = 0) in vec3 fragDiffuse;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 FragPos;  
layout(location = 3) in vec3 Normal;
layout(location = 4) in vec3 cameraPos;
layout(location = 5) in vec3 fragSpecular;
layout(location = 6) in vec3 fragAmbient;
layout(location = 7) in float fragShininess;
layout(location = 8) in float fragOpacity;
layout(location = 9) in vec2 fragNormalTexCoord;
layout(location = 10) flat in int hasNormalMap;
layout(location = 0) out vec4 outColor;

void main() {

    vec4 texColor = texture(texSampler, fragTexCoord); 
    vec3 objectColor = texColor.rgb * fragDiffuse;

    vec3 totalLighting = vec3(0,0,0);

    // Extract normal from the normal map
    vec3 norm;
    if (hasNormalMap == 1) {
        vec3 normalMap = texture(texSampler, fragNormalTexCoord).rgb;
        normalMap = normalMap * 2.0 - 1.0;  // Transform from [0,1] to [-1,1]
        norm = normalize(Normal + normalMap);
    } else {
        norm = normalize(Normal);
    }

    for (int i = 0; i < lightCount.lightCount; i++) {
        LightData light = lightBuffer.lights[i];
        vec3 lightColor = light.color;
        vec3 lightPos = light.position;

        // Calculate distance to light
        float distance = length(lightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
        if (attenuation < 0.01) {
            continue;
        }   
        
        // diffuse 
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        if (diff == 0.0) {
            continue;
        }
        vec3 diffuse = diff * lightColor * fragDiffuse;

        // specular
        vec3 viewDir = normalize(cameraPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4);
        vec3 specular = spec * lightColor;  

        // Apply distance attenuation to diffuse and specular
        diffuse *= attenuation;
        specular *= attenuation;
        totalLighting += (diffuse + specular);
    }
    vec3 result = (totalLighting + vec3(0.1, 0.1, 0.1)) * objectColor;
    outColor = vec4(result, fragOpacity);
}