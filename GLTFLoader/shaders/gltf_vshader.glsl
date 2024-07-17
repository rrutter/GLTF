#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec4 jointIndices; // Bone indices
layout(location = 4) in vec4 weights;      // Bone weights

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 jointMatrices[100]; // Array of bone transformation matrices

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

vec3 applyBoneTransform(vec4 pos) {
    vec3 transformedPos = vec3(0.0);
    for (int i = 0; i < 4; ++i) {
        int jointIndex = int(jointIndices[i]);
        if (jointIndex >= 0) {
            mat4 boneTransform = jointMatrices[jointIndex];
            transformedPos += weights[i] * vec3(boneTransform * pos);
        }
    }
    return transformedPos;
}

void main() {
    vec3 transformedPosition = applyBoneTransform(vec4(position, 1.0));
    vec3 transformedNormal = normalize(applyBoneTransform(vec4(normal, 0.0)));

    FragPos = vec3(model * vec4(transformedPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * transformedNormal;
    TexCoords = texCoords;

    gl_Position = projection * view * model * vec4(transformedPosition, 1.0);


}
