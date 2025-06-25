#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/gtc/matrix_transform.hpp"
#include "dependencies/glm/gtc/type_ptr.hpp"
#include <vector>
#include "Shader.h"
#include <iostream>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// Configuración de ventana
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Estructura para planetas
struct Planet {
    string name;        // Nombre del planeta
    float orbitRadius;  // Distancia desde el sol
    float orbitSpeed;   // Velocidad orbital (grados/segundo)
    float orbitAngle;   // Ángulo actual de órbita
    float rotationSpeed;// Velocidad de rotación (grados/segundo)
    float rotationAngle;// Ángulo actual de rotación
    float size;         // Escala del planeta
    GLuint texture;     // Textura del planeta
    bool hasMoon;       // Indica si tiene luna
    float moonDistance; // Distancia de la luna
    float moonSpeed;    // Velocidad orbital de la luna
    float moonAngle;    // Ángulo actual de la luna
    GLuint moonTexture; // Textura de la luna
    bool hasRing;       // Indica si tiene anillos
    GLuint ringTexture; // Textura de los anillos
};

// Prototipos de funciones
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void createSphere(vector<float>& vertices, vector<unsigned int>& indices);
GLuint loadTexture(const char* path);
void renderPlanet(Shader& shader, Planet& planet, unsigned int sphereVAO,
    const vector<unsigned int>& sphereIndices, float deltaTime);

int main() {
    // Inicialización GLFW y creación de ventana
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sistema Solar v5", NULL, NULL);
    if (window == NULL) {
        cout << "Fallo al crear la ventana de GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Carga GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Fallo al inicializar GLAD" << endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // Compilación de shaders
    Shader ourShader("shaders/shader.vert", "shaders/shader.frag");

    // Creación de la esfera
    vector<float> sphereVertices;
    vector<unsigned int> sphereIndices;
    createSphere(sphereVertices, sphereIndices);

    // Configuración de buffers
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);

    // Atributos del vértice
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Carga de texturas
    GLuint galaxyTexture = loadTexture("textures/galaxy.jpg");
    GLuint sunTexture = loadTexture("textures/sun.jpg");
    GLuint mercuryTexture = loadTexture("textures/mercury.jpg");
    GLuint venusTexture = loadTexture("textures/venus.jpg");
    GLuint earthTexture = loadTexture("textures/earth.jpg");
    GLuint moonTexture = loadTexture("textures/moon.jpg");
    GLuint marsTexture = loadTexture("textures/mars.jpg");
    GLuint jupiterTexture = loadTexture("textures/jupiter.jpg");
    GLuint saturnTexture = loadTexture("textures/saturn.jpg");
    GLuint saturnRingTexture = loadTexture("textures/saturn_ring.png");
    GLuint uranusTexture = loadTexture("textures/uranus.jpg");
    GLuint neptuneTexture = loadTexture("textures/neptune.jpg");

    // Configuración de planetas
    Planet mercury = {
        "Mercury",  // name
        1.5f,       // orbitRadius
        47.9f,      // orbitSpeed
        0.0f,       // orbitAngle
        0.017f,     // rotationSpeed
        0.0f,       // rotationAngle
        0.15f,      // size
        mercuryTexture, // texture
        false,      // hasMoon
        0.0f,       // moonDistance
        0.0f,       // moonSpeed
        0.0f,       // moonAngle
        0,          // moonTexture
        false,      // hasRing
        0           // ringTexture
    };

    Planet venus = {
        "Venus",    // name
        2.0f,       // orbitRadius
        35.0f,      // orbitSpeed
        0.0f,       // orbitAngle
        0.004f,     // rotationSpeed
        0.0f,       // rotationAngle
        0.25f,      // size
        venusTexture, // texture
        false,      // hasMoon
        0.0f,       // moonDistance
        0.0f,       // moonSpeed
        0.0f,       // moonAngle
        0,          // moonTexture
        false,      // hasRing
        0           // ringTexture
    };

    Planet earth = {
        "Earth",    // name
        3.5f,       // orbitRadius
        30.0f,      // orbitSpeed
        0.0f,       // orbitAngle
        60.0f,      // rotationSpeed
        0.0f,       // rotationAngle
        0.3f,       // size
        earthTexture, // texture
        true,       // hasMoon
        0.7f,       // moonDistance
        200.0f,     // moonSpeed
        0.0f,       // moonAngle
        moonTexture,// moonTexture
        false,      // hasRing
        0           // ringTexture
    };

    Planet mars = {
        "Mars",     // name
        4.5f,       // orbitRadius
        24.1f,      // orbitSpeed
        0.0f,       // orbitAngle
        31.0f,      // rotationSpeed
        0.0f,       // rotationAngle
        0.2f,       // size
        marsTexture, // texture
        false,      // hasMoon
        0.0f,       // moonDistance
        180.0f,     // moonSpeed
        0.0f,       // moonAngle
        0,          // moonTexture
        false,      // hasRing
        0           // ringTexture
    };

    Planet jupiter = {
        "Jupiter",  // name
        6.0f,       // orbitRadius
        13.1f,      // orbitSpeed
        0.0f,       // orbitAngle
        28.0f,      // rotationSpeed
        0.0f,       // rotationAngle
        0.5f,       // size
        jupiterTexture, // texture
        false,      // hasMoon
        0.0f,       // moonDistance
        0.0f,       // moonSpeed
        0.0f,       // moonAngle
        0,          // moonTexture
        false,      // hasRing
        0           // ringTexture
    };

    Planet saturn = {
        "Saturn",   // name
        7.5f,       // orbitRadius
        9.7f,       // orbitSpeed
        0.0f,       // orbitAngle
        22.0f,      // rotationSpeed
        0.0f,       // rotationAngle
        0.45f,      // size
        saturnTexture, // texture
        false,      // hasMoon
        0.0f,       // moonDistance
        0.0f,       // moonSpeed
        0.0f,       // moonAngle
        0,          // moonTexture
        true,       // hasRing
        saturnRingTexture // ringTexture
    };

    Planet uranus = {
        "Uranus",    // name
        9.0f,        // orbitRadius
        6.8f,        // orbitSpeed
        0.0f,        // orbitAngle
        17.0f,       // rotationSpeed
        0.0f,        // rotationAngle
        0.4f,        // size
        uranusTexture, // texture
        false,       // hasMoon
        0.0f,        // moonDistance
        0.0f,        // moonSpeed
        0.0f,        // moonAngle
        0,           // moonTexture
        false,       // hasRing 
        0            // ringTexture
    };

    Planet neptune = {
        "Neptune",   // name
        10.5f,       // orbitRadius
        5.4f,        // orbitSpeed
        0.0f,        // orbitAngle
        16.0f,       // rotationSpeed
        0.0f,        // rotationAngle
        0.38f,       // size
        neptuneTexture, // texture
        false,       // hasMoon
        0.0f,        // moonDistance
        0.0f,        // moonSpeed
        0.0f,        // moonAngle
        0,           // moonTexture
        false,       // hasRing
        0            // ringTexture
    };

    // Variables de tiempo
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        // Cálculo del tiempo
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Limpieza de buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        // Configuración de vista y proyección
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        if (display_h == 0) display_h = 1;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)display_w / (float)display_h, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 3.0f, 18.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //posición de la camara
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Renderizado del fondo (galaxia)
        glDepthMask(GL_FALSE);
        glm::mat4 model_background = glm::mat4(1.0f);
        model_background = glm::scale(model_background, glm::vec3(50.0f, 50.0f, 50.0f));
        ourShader.setMat4("model", model_background);
        glBindTexture(GL_TEXTURE_2D, galaxyTexture);
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);

        // Renderizado del sol
        glm::mat4 model_sun = glm::mat4(1.0f);
        model_sun = glm::scale(model_sun, glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("model", model_sun);
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // Renderizado de los planetas
        renderPlanet(ourShader, mercury, sphereVAO, sphereIndices, deltaTime);
        renderPlanet(ourShader, venus, sphereVAO, sphereIndices, deltaTime);
        renderPlanet(ourShader, earth, sphereVAO, sphereIndices, deltaTime);
        renderPlanet(ourShader, mars, sphereVAO, sphereIndices, deltaTime);
        renderPlanet(ourShader, jupiter, sphereVAO, sphereIndices, deltaTime);
        renderPlanet(ourShader, saturn, sphereVAO, sphereIndices, deltaTime);
        renderPlanet(ourShader, uranus, sphereVAO, sphereIndices, deltaTime);
        renderPlanet(ourShader, neptune, sphereVAO, sphereIndices, deltaTime);

        // Intercambio de buffers y eventos
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpieza
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteProgram(ourShader.ID);
    glfwTerminate();
    return 0;
}

// Implementación de funciones

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
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
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
    return textureID;
}

void renderPlanet(Shader& shader, Planet& planet, unsigned int sphereVAO,
    const vector<unsigned int>& sphereIndices, float deltaTime) {
    // Actualizar ángulos
    planet.orbitAngle = std::fmod(planet.orbitAngle + planet.orbitSpeed * deltaTime, 360.0f);
    planet.rotationAngle = std::fmod(planet.rotationAngle + planet.rotationSpeed * deltaTime, 360.0f);

    if (planet.hasMoon) {
        planet.moonAngle = std::fmod(planet.moonAngle + planet.moonSpeed * deltaTime, 360.0f);
    }

    // Sistema de coordenadas del planeta
    glm::mat4 planetSystem = glm::rotate(glm::mat4(1.0f),
        glm::radians(planet.orbitAngle),
        glm::vec3(0.0f, 1.0f, 0.0f));
    planetSystem = glm::translate(planetSystem, glm::vec3(planet.orbitRadius, 0.0f, 0.0f));

    // Renderizar planeta principal
    glm::mat4 planetModel = glm::rotate(planetSystem,
        glm::radians(planet.rotationAngle),
        glm::vec3(0.0f, 1.0f, 0.0f));
    planetModel = glm::scale(planetModel, glm::vec3(planet.size));

    shader.setMat4("model", planetModel);
    glBindTexture(GL_TEXTURE_2D, planet.texture);
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

    // Renderizar anillos si existen
    if (planet.hasRing && planet.ringTexture != 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 ringModel = planetSystem;
        ringModel = glm::rotate(ringModel, glm::radians(23.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Inclinación de 23°
        ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.7f, planet.size * 0.05f, planet.size * 1.7f));

        shader.setMat4("model", ringModel);
        glBindTexture(GL_TEXTURE_2D, planet.ringTexture);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        glDisable(GL_BLEND);
    }

    // Renderizar luna si existe
    if (planet.hasMoon && planet.moonTexture != 0) {
        glm::mat4 moonModel = planetSystem;
        moonModel = glm::rotate(moonModel, glm::radians(planet.moonAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        moonModel = glm::translate(moonModel, glm::vec3(planet.moonDistance, 0.0f, 0.0f));
        moonModel = glm::scale(moonModel, glm::vec3(planet.size * 0.3f));

        shader.setMat4("model", moonModel);
        glBindTexture(GL_TEXTURE_2D, planet.moonTexture);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
    }
}