#version 330 core // Especificamos la versión de GLSL que vamos a usar

// Atributos de entrada que vienen del VBO (por cada vértice)
layout (location = 0) in vec3 aPos;      // Posición del vértice (x, y, z)
layout (location = 1) in vec3 aNormal;   // Normal del vértice (para la luz, lo usaremos después)
layout (location = 2) in vec2 aTexCoord; // Coordenada de textura (u, v)

// Variables de salida que se pasarán al Fragment Shader
out vec2 TexCoord;

// Uniforms: variables globales que podemos cambiar desde nuestro código C++
uniform mat4 model;       // Matriz para transformar el objeto (moverlo, rotarlo, escalarlo)
uniform mat4 view;        // Matriz de la cámara (dónde estamos mirando)
uniform mat4 projection;  // Matriz de perspectiva (cómo se proyecta la escena 3D en la pantalla 2D)

void main()
{
    // Calculamos la posición final del vértice en el espacio de la pantalla
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Pasamos la coordenada de textura directamente al Fragment Shader
    TexCoord = aTexCoord;
}