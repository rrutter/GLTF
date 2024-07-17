#version 330 core
in vec4 VertexColor;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform bool useTexture;

void main() {
    //vec4 textureColor = useTexture ? texture(textureSampler, TexCoords) : vec4(1.0, 1.0, 1.0, 1.0); // Use white if no texture
    //FragColor = textureColor * VertexColor; // Combine texture color with vertex color
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
}
