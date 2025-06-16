#version 330 core // Especificamos la versi�n de GLSL que vamos a usar

// Atributos de entrada que vienen del VBO (por cada v�rtice)
layout (location = 0) in vec3 aPos;      // Posici�n del v�rtice (x, y, z)
layout (location = 1) in vec3 aNormal;   // Normal del v�rtice (para la luz, lo usaremos despu�s)
layout (location = 2) in vec2 aTexCoord; // Coordenada de textura (u, v)

// Variables de salida que se pasar�n al Fragment Shader
out vec2 TexCoord;

// Uniforms: variables globales que podemos cambiar desde nuestro c�digo C++
uniform mat4 model;       // Matriz para transformar el objeto (moverlo, rotarlo, escalarlo)
uniform mat4 view;        // Matriz de la c�mara (d�nde estamos mirando)
uniform mat4 projection;  // Matriz de perspectiva (c�mo se proyecta la escena 3D en la pantalla 2D)

void main()
{
    // Calculamos la posici�n final del v�rtice en el espacio de la pantalla
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Pasamos la coordenada de textura directamente al Fragment Shader
    TexCoord = aTexCoord;
}