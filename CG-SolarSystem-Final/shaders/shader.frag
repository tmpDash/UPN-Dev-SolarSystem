#version 330 core

// Variable de entrada que viene del Vertex Shader
in vec2 TexCoord;

// Variable de salida: el color final del píxel
out vec4 FragColor;

// Uniform para la textura que vamos a usar
uniform sampler2D ourTexture;

void main()
{
    // Tomamos el color de la textura en la coordenada correspondiente
    // y lo asignamos como el color final del píxel.
    FragColor = texture(ourTexture, TexCoord);
}