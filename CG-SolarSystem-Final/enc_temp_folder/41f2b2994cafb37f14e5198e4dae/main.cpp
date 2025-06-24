#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include "Shader.h"
#include <iostream>

using namespace std;
// --- Variables Globales y Funciones Auxiliares ---
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// --- FUNCIÓN DE CALLBACK PARA EL REDIMENSIONAMIENTO DE LA VENTANA ---
// Esta función será llamada por GLFW cada vez que la ventana cambie de tamaño.

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Le decimos a OpenGL que el nuevo tamaño del área de dibujo (viewport)
    // debe coincidir con el nuevo tamaño de la ventana.
    glViewport(0, 0, width, height);
}

void createSphere(vector<float>& vertices, vector<unsigned int>& indices) {
    const float PI = 3.14159265359f;
    const int sectorCount = 36;
    const int stackCount = 18;
    float radius = 1.0f;
    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;
    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);
        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
            nx = x * lengthInv; ny = y * lengthInv; nz = z * lengthInv;
            vertices.push_back(nx); vertices.push_back(ny); vertices.push_back(nz);
            s = (float)j / sectorCount; t = (float)i / stackCount;
            vertices.push_back(s); vertices.push_back(t);
        }
    }
    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1); k2 = k1 + sectorCount + 1;
        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) { indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1); }
            if (i != (stackCount - 1)) { indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1); }
        }
    }
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 3);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        cout << "Textura cargada con exito: " << path << endl;
    }
    else {
        cout << "Error al cargar la textura: " << path << endl;
        cout << "Motivo del error (stb_image): " << stbi_failure_reason() << endl;
    }
    stbi_image_free(data);
    return data ? textureID : 0;
}

int main() {
    // ---- 1. Inicialización ----
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sistema Solar v1", NULL, NULL);
    if (window == NULL) {
        cout << "Fallo al crear la ventana de GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Fallo al inicializar GLAD" << endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // ---- 2. Construir y compilar shaders ----
    Shader ourShader("shaders/shader.vert", "shaders/shader.frag");

    // ---- 3. Geometría y Buffers ----
    vector<float> sphereVertices;
    vector<unsigned int> sphereIndices;

    createSphere(sphereVertices, sphereIndices);

    unsigned int sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // location = 0 (aPos)
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // location = 1 (aNormal)
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); // location = 2 (aTexCoord)

    GLuint galaxyTexture = loadTexture("textures/galaxy.jpg");
    GLuint sunTexture = loadTexture("textures/sun.jpg");
    GLuint earthTexture = loadTexture("textures/earth.jpg");
    GLuint moonTexture = loadTexture("textures/moon.jpg");

    // ---- 5. Bucle de Renderizado ----
    float earthOrbitAngle = 0.0f;
    float earthOrbitSpeed = 30.0f; // Velocidad en grados por segundo

    float earthRotationAngle = 0.0f; // <-- NUEVO: Ángulo de rotación axial de la Tierra
    float earthRotationSpeed = 60.0f; // <-- NUEVO: Velocidad de rotación axial de la Tierra

    float moonOrbitAngle = 0.0f; // <-- NUEVO: Ángulo de órbita de la Luna
    float moonOrbitSpeed = 200.0f; // <-- NUEVO: Velocidad de órbita de la Luna

    // Variables para calcular el delta time
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // ---- 5. Bucle de Renderizado ----
    while (!glfwWindowShouldClose(window)) {

        // --- CÁLCULO DEL DELTA TIME ---
        float currentFrame = glfwGetTime(); // Tiempo desde que se inició GLFW
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        // --- OBTENER EL TAMAÑO ACTUAL DE LA VENTANA ---
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // --- CALCULAR MATRICES CON EL TAMAÑO ACTUAL ---
        // Asegurarse de no dividir por cero si la ventana se minimiza
        if (display_h == 0) display_h = 1;

        // Usa display_w y display_h para calcular la relación de aspecto correcta
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)display_w / (float)display_h, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 3.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // --- DIBUJAR EL FONDO (GALAXY) ---
        glDepthMask(GL_FALSE);
        glm::mat4 model_background = glm::mat4(1.0f);
        model_background = glm::scale(model_background, glm::vec3(50.0f, 50.0f, 50.0f));
        ourShader.setMat4("model", model_background);
        glBindTexture(GL_TEXTURE_2D, galaxyTexture);
        glBindVertexArray(sphereVAO); // Usamos la misma esfera para el fondo
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);

        // --- DIBUJAR EL SOL ---
        glm::mat4 model_sun = glm::mat4(1.0f);
        // Para mantener una proporción, digamos que el radio del Sol es ~109 veces el de la Tierra.
        // Usaremos una escala más visual: Sol = 1.0, Tierra = 0.3
        model_sun = glm::scale(model_sun, glm::vec3(1.0f, 1.0f, 1.0f)); // Tamaño base del Sol
        ourShader.setMat4("model", model_sun);
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // Matriz base que nos lleva a la POSICIÓN de la Tierra
        glm::mat4 earth_system_matrix = glm::mat4(1.0f);
        earth_system_matrix = glm::rotate(earth_system_matrix, glm::radians(earthOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Órbita de la Tierra
        earth_system_matrix = glm::translate(earth_system_matrix, glm::vec3(3.5f, 0.0f, 0.0f)); // Distancia al Sol

        // -- Dibujar la Tierra --
        glm::mat4 model_earth = earth_system_matrix; // Partimos de la posición de la Tierra
        model_earth = glm::rotate(model_earth, glm::radians(earthRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación axial de la Tierra
        model_earth = glm::scale(model_earth, glm::vec3(0.3f, 0.3f, 0.3f)); // Escalado final de la Tierra
        ourShader.setMat4("model", model_earth);
        glBindTexture(GL_TEXTURE_2D, earthTexture);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // -- Dibujar la Luna --
        glm::mat4 model_moon = earth_system_matrix; // Partimos de la misma posición de la Tierra
        model_moon = glm::rotate(model_moon, glm::radians(moonOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Órbita de la Luna alrededor de la Tierra
        model_moon = glm::translate(model_moon, glm::vec3(0.7f, 0.0f, 0.0f)); // Distancia de la Luna a la Tierra
        model_moon = glm::scale(model_moon, glm::vec3(0.1f, 0.1f, 0.1f)); // Escalado final de la Luna
        ourShader.setMat4("model", model_moon);
        glBindTexture(GL_TEXTURE_2D, moonTexture);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // --- ACTUALIZAR ÁNGULOS DE ANIMACIÓN ---
        earthOrbitAngle += earthOrbitSpeed * deltaTime;
        if (earthOrbitAngle >= 360.0f) earthOrbitAngle -= 360.0f;

        earthRotationAngle += earthRotationSpeed * deltaTime; // <-- NUEVO
        if (earthRotationAngle >= 360.0f) earthRotationAngle -= 360.0f; // <-- NUEVO

        moonOrbitAngle += moonOrbitSpeed * deltaTime; // <-- NUEVO
        if (moonOrbitAngle >= 360.0f) moonOrbitAngle -= 360.0f; // <-- NUEVO

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- 6. Limpieza ----
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteProgram(ourShader.ID);

    glfwTerminate();
    return 0;
}