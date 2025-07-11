// ===========================================
// SISTEMA SOLAR 3D INTERACTIVO CON OPENGL
// ===========================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "Shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// ===========================================
// CONSTANTES Y CONFIGURACIÓN GLOBAL
// ===========================================

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 768;
const float maxPitch = 90.0f;
const float minPitch = -90.0f;
const int MAX_METEORITES = 6;

// ===========================================
// ESTRUCTURAS DE DATOS
// ===========================================

struct Planet {
    string name;
    float orbitRadius;
    float orbitSpeed;
    float orbitAngle;
    float rotationSpeed;
    float rotationAngle;
    float size;
    GLuint texture;
    bool hasMoon;
    float moonDistance;
    float moonSpeed;
    float moonAngle;
    GLuint moonTexture;
    bool hasRing;
    GLuint ringTexture;
};

struct Meteorite {
    glm::vec3 position;
    glm::vec3 velocity;
    bool isVisible;
    float timeToAppear;
    float initialDelay;
};

struct PlanetData {
    string name;
    float distanceFromSunAU;
    float distanceFromSunKM;
    float orbitPeriodDays;
    float rotationPeriodHours;
    float diameterKM;
    float massEarths;
    float mass;
    string planetType;
    string atmosphere;
    string funFact;
    ImVec4 highlightColor;
};

// ===========================================
// VARIABLES GLOBALES DE ESTADO
// ===========================================

float cameraPitch = 0.0f;
float cameraYaw = 0.0f;
float pitchSpeed = 30.0f;
const float pitchIncrement = pitchSpeed * 0.016f;

bool firstMouse = true;
bool mouseControleEnabled = false;
float lastMouseY = SCR_HEIGHT / 2.0f;
float lastMouseX = SCR_WIDTH / 2.0f;
float mouseSensitivity = 0.5f;

bool showNames = false;
bool animationPaused = false;
bool showOrbits = true;
bool showMeteorites = false;
int meteoriteCount = 3;

bool showEducationalTable = true;
bool showAdvancedData = false;
bool showOnlyRockyPlanets = false;
bool showOnlyGasGiants = false;
bool highlightEarthComparisons = false;
int selectedPlanetForComparison = 2;
bool showFunFacts = false;

// ===========================================
// BASE DE DATOS EDUCATIVA
// ===========================================

// Datos de los planetas
PlanetData planetEducationalData[] = {
    {"Mercurio", 0.39f, 57.9f, 88.0f, 1407.6f, 4879.0f, 0.055f, 0.330f, "Rocoso", "Sin atmósfera", "Un día dura más que un año", ImVec4(0.8f, 0.7f, 0.6f, 1.0f)},
    {"Venus", 0.72f, 108.2f, 225.0f, 5832.5f, 12104.0f, 0.815f, 4.87f, "Rocoso", "Dióxido de carbono denso (96%), Nitrógeno (3%)", "Rota al revés (retrógrado)", ImVec4(1.0f, 0.8f, 0.4f, 1.0f)},
    {"Tierra", 1.0f, 149.6f, 365.25f, 24.0f, 12756.0f, 1.0f, 5.97f, "Rocoso", "Nitrógeno (78%), Óxígeno (21%)", "Único planeta con vida conocida", ImVec4(0.4f, 0.8f, 1.0f, 1.0f)},
    {"Marte", 1.52f, 227.9f, 687.0f, 24.6f, 6792.0f, 0.107f, 0.642f, "Rocoso", "Dióxido de carbono (95%), Nitrógeno (3%)", "Tiene las montañas más altas del sistema solar", ImVec4(1.0f, 0.5f, 0.3f, 1.0f)},
    {"Júpiter", 5.20f, 778.5f, 4333.0f, 9.9f, 142984.0f, 317.8f, 1898.0f, "Gaseoso", "Hidrógeno (89%), Helio (10%)", "Tiene más masa que todos los otros planetas juntos", ImVec4(0.9f, 0.7f, 0.5f, 1.0f)},
    {"Saturno", 9.58f, 1432.0f, 10747.0f, 10.7f, 120536.0f, 95.2f, 568.0f, "Gaseoso", "Hidrógeno (96%), Helio (3%)", "Flotaría en agua", ImVec4(1.0f, 0.9f, 0.7f, 1.0f)},
    {"Urano", 19.20f, 2867.0f, 30589.0f, 17.2f, 51118.0f, 14.5f, 86.8f, "Gigante de hielo", "Hidrógeno (83%), Helio (15%), Metano (2%)", "Rota de lado", ImVec4(0.4f, 0.8f, 0.9f, 1.0f)},
    {"Neptuno", 30.05f, 4515.0f, 59800.0f, 16.1f, 49528.0f, 17.1f, 102.0f, "Gigante de hielo", "Hidrógeno (80%), Helio (19%), Metano (1%)", "Vientos más rápidos del sistema solar", ImVec4(0.2f, 0.4f, 1.0f, 1.0f)}
};

// ===========================================
// DECLARACIONES DE FUNCIONES
// ===========================================

void createSphere(vector<float>& vertices, vector<unsigned int>& indices);
void createCircle(std::vector<float>& vertices, int numSegments);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLuint loadTexture(const char* path, GLuint fallbackTextureID);
void renderPlanet(Shader& shader, Planet& planet, unsigned int sphereVAO,
    const vector<unsigned int>& sphereIndices, float deltaTime,
    const glm::mat4& view, const glm::mat4& projection);
void renderTextIn3DSpace(const std::string& text, glm::vec3 worldPos,
    const glm::mat4& view, const glm::mat4& projection);
void renderEducationalInterface();
void renderPlanetDataTable();
void renderPlanetComparisonInfo();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// ===========================================
// FUNCIONES DE ENTRADA Y CONTROL
// ===========================================

// Maneja eventos de teclado para controlar la cámara y funciones especiales
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_UP) {
            cameraPitch += pitchIncrement;
            if (cameraPitch > maxPitch) cameraPitch = maxPitch;
        }
        else if (key == GLFW_KEY_DOWN) {
            cameraPitch -= pitchIncrement;
            if (cameraPitch < minPitch) cameraPitch = minPitch;
        }
        else if (key == GLFW_KEY_R) {
            cameraPitch = 0.0f;
            cameraYaw = 0.0f;
            firstMouse = true;
        }
        else if (key == GLFW_KEY_M) {
            mouseControleEnabled = !mouseControleEnabled;
        }
    }
}

// Maneja movimiento del mouse para control de cámara con pitch y yaw
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseControleEnabled) return;

    if (firstMouse) {
        lastMouseY = ypos;
        lastMouseX = xpos;
        firstMouse = false;
        return;
    }

    float yoffset = lastMouseY - ypos;
    float xoffset = lastMouseX - xpos;

    lastMouseY = ypos;
    lastMouseX = xpos;

    yoffset *= mouseSensitivity;
    xoffset *= mouseSensitivity;

    cameraPitch += yoffset;
    cameraYaw += xoffset;

    if (cameraPitch > maxPitch) cameraPitch = maxPitch;
    if (cameraPitch < minPitch) cameraPitch = minPitch;

    if (cameraYaw > 360.0f) cameraYaw -= 360.0f;
    if (cameraYaw < 0.0f) cameraYaw += 360.0f;
}

// ===========================================
// FUNCIONES DE INTERFAZ EDUCATIVA
// ===========================================

// Renderiza toda la interfaz educativa con controles, filtros y tabla de datos
void renderEducationalInterface() {
    ImGui::SeparatorText("Información Astronómica");

    ImGui::Checkbox("Mostrar tabla de datos", &showEducationalTable);
    if (!showEducationalTable) return;

    ImGui::SeparatorText("Filtros");

    if (ImGui::RadioButton("Todos los planetas", !showOnlyRockyPlanets && !showOnlyGasGiants)) {
        showOnlyRockyPlanets = false;
        showOnlyGasGiants = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Solo rocosos", showOnlyRockyPlanets)) {
        showOnlyRockyPlanets = true;
        showOnlyGasGiants = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Solo gaseosos", showOnlyGasGiants)) {
        showOnlyRockyPlanets = false;
        showOnlyGasGiants = true;
    }

    ImGui::SeparatorText("Comparaciones");
    ImGui::Checkbox("Resaltar comparaciones con la Tierra", &highlightEarthComparisons);

    const char* planetNames[] = { "Mercurio", "Venus", "Tierra", "Marte", "Júpiter", "Saturno", "Urano", "Neptuno" };
    ImGui::SetNextItemWidth(150);
    ImGui::Combo("Comparar con", &selectedPlanetForComparison, planetNames, 8);

    renderPlanetDataTable();

    if (selectedPlanetForComparison >= 0 && selectedPlanetForComparison < 8) {
        renderPlanetComparisonInfo();
    }
}

// Renderiza la tabla principal con datos planetarios incluyendo filtros dinámicos
void renderPlanetDataTable() {
    int columnCount = 5;

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Sortable;

    ImVec2 tableSize = ImVec2(0.0f, showAdvancedData ? 300.0f : 250.0f);

    if (ImGui::BeginTable("PlanetEducationalTable", columnCount, tableFlags, tableSize)) {

        ImGui::TableSetupColumn("Planeta", ImGuiTableColumnFlags_NoSort);
        ImGui::TableSetupColumn("Distancia", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Año (días)", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Día (horas)", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Masa", ImGuiTableColumnFlags_DefaultSort);

        ImGui::TableHeadersRow();

        for (int i = 0; i < 8; i++) {
            auto& planet = planetEducationalData[i];

            if (showOnlyRockyPlanets && planet.planetType != "Rocoso") continue;
            if (showOnlyGasGiants && (planet.planetType == "Rocoso")) continue;

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (highlightEarthComparisons && i == selectedPlanetForComparison) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(100, 200, 100, 50));
            }
            ImGui::TextColored(planet.highlightColor, "%s", planet.name.c_str());
            ImGui::TextDisabled("(%s)", planet.planetType.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%.2f UA", planet.distanceFromSunAU);
            ImGui::TextDisabled("(%.0f M km)", planet.distanceFromSunKM);
            if (highlightEarthComparisons && i != 2) {
                float ratio = planet.distanceFromSunAU / planetEducationalData[2].distanceFromSunAU;
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.1fx", ratio);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%.0f días", planet.orbitPeriodDays);
            if (planet.orbitPeriodDays >= 365) {
                float years = planet.orbitPeriodDays / 365.25f;
                ImGui::TextDisabled("(%.1f años)", years);
            }
            if (highlightEarthComparisons && i != 2) {
                float ratio = planet.orbitPeriodDays / planetEducationalData[2].orbitPeriodDays;
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.1fx", ratio);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%.1f h", planet.rotationPeriodHours);
            if (planet.rotationPeriodHours >= 24) {
                float days = planet.rotationPeriodHours / 24.0f;
                ImGui::TextDisabled("(%.1f días)", days);
            }
            if (highlightEarthComparisons && i != 2) {
                float ratio = planet.rotationPeriodHours / planetEducationalData[2].rotationPeriodHours;
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.1fx", ratio);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", planet.mass);
            if (highlightEarthComparisons && i != 2) {
                float ratio = planet.massEarths;
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.2fx", ratio);
            }
        }

        ImGui::EndTable();
    }
}

// Muestra información detallada del planeta seleccionado con comparaciones
void renderPlanetComparisonInfo() {
    if (selectedPlanetForComparison < 0 || selectedPlanetForComparison >= 8) return;

    auto& planet = planetEducationalData[selectedPlanetForComparison];
    auto& earth = planetEducationalData[2];

    ImGui::SeparatorText("Información Detallada");

    ImGui::Text("Planeta seleccionado:");
    ImGui::SameLine();
    ImGui::TextColored(planet.highlightColor, "%s", planet.name.c_str());

    if (ImGui::BeginTable("ComparisonTable", 2, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Característica");
        ImGui::TableSetupColumn("Valor y Comparación");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Distancia del Sol");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f UA (%.0f millones de km)", planet.distanceFromSunAU, planet.distanceFromSunKM);
        if (selectedPlanetForComparison != 2) {
            float ratio = planet.distanceFromSunAU / earth.distanceFromSunAU;
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), " → %.1fx más %s que la Tierra",
                abs(ratio), ratio > 1.0f ? "lejos" : "cerca");
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Tamaño");
        ImGui::TableNextColumn();
        ImGui::Text("%.0f km de diámetro", planet.diameterKM);
        if (selectedPlanetForComparison != 2) {
            float ratio = planet.diameterKM / earth.diameterKM;
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), " → %.1fx %s que la Tierra",
                ratio, ratio > 1.0f ? "más grande" : "más pequeño");
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Masa");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f veces la masa terrestre", planet.massEarths);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Tipo");
        ImGui::TableNextColumn();
        ImGui::Text("%s", planet.planetType.c_str());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Atmósfera");
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s", planet.atmosphere.c_str());

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.8f, 1.0f), "Dato curioso:");
    ImGui::TextWrapped("%s", planet.funFact.c_str());
}

// ===========================================
// FUNCIONES DE GEOMETRÍA Y UTILIDADES
// ===========================================

// Genera vértices e índices para una esfera perfecta usando parametrización esférica
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
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

// Genera vértices para un círculo en el plano XZ usado para órbitas planetarias
void createCircle(std::vector<float>& vertices, int numSegments) {
    vertices.clear();
    float angleStep = 2.0f * 3.1415926535f / numSegments;

    for (int i = 0; i <= numSegments; ++i) {
        float angle = i * angleStep;
        vertices.push_back(cos(angle));
        vertices.push_back(0.0f);
        vertices.push_back(sin(angle));
    }
}

// Callback para redimensionamiento de ventana
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// ===========================================
// FUNCIONES DE CARGA Y MANEJO DE RECURSOS
// ===========================================

// Carga una textura desde archivo con fallback en caso de error
GLuint loadTexture(const char* path, GLuint fallbackTextureID) {
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

        glDeleteTextures(1, &textureID);
        return fallbackTextureID;
    }

    stbi_image_free(data);
    return textureID;
}

// ===========================================
// FUNCIONES DE RENDERIZADO
// ===========================================

// Renderiza texto en el espacio 3D proyectándolo a coordenadas de pantalla
void renderTextIn3DSpace(const std::string& text, glm::vec3 worldPos, const glm::mat4& view, const glm::mat4& projection) {
    int display_w, display_h;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &display_w, &display_h);

    glm::vec4 clipPos = projection * view * glm::vec4(worldPos, 1.0f);

    if (clipPos.w < 0.0f) return;

    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

    float screenX = (ndcPos.x + 1.0f) / 2.0f * display_w;
    float screenY = (1.0f - ndcPos.y) / 2.0f * display_h;

    ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenX, screenY), IM_COL32(255, 255, 255, 255), text.c_str());
}

// Renderiza un planeta completo con sus componentes (planeta, luna, anillos)
void renderPlanet(Shader& shader, Planet& planet, unsigned int sphereVAO,
    const vector<unsigned int>& sphereIndices, float deltaTime,
    const glm::mat4& view, const glm::mat4& projection) {

    planet.orbitAngle = std::fmod(planet.orbitAngle + planet.orbitSpeed * deltaTime, 360.0f);
    planet.rotationAngle = std::fmod(planet.rotationAngle + planet.rotationSpeed * deltaTime, 360.0f);

    if (planet.hasMoon) {
        planet.moonAngle = std::fmod(planet.moonAngle + planet.moonSpeed * deltaTime, 360.0f);
    }

    glm::mat4 planetSystem = glm::rotate(glm::mat4(1.0f),
        glm::radians(planet.orbitAngle),
        glm::vec3(0.0f, 1.0f, 0.0f));
    planetSystem = glm::translate(planetSystem, glm::vec3(planet.orbitRadius, 0.0f, 0.0f));

    glm::mat4 planetModel = glm::rotate(planetSystem,
        glm::radians(planet.rotationAngle),
        glm::vec3(0.0f, 1.0f, 0.0f));
    planetModel = glm::scale(planetModel, glm::vec3(planet.size));

    shader.setMat4("model", planetModel);
    glBindTexture(GL_TEXTURE_2D, planet.texture);
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

    if (showNames) {
        glm::vec3 planetWorldPos = glm::vec3(planetSystem[3]);
        planetWorldPos.y += planet.size * 1.5f;
        renderTextIn3DSpace(planet.name, planetWorldPos, view, projection);
    }

    if (planet.hasRing && planet.ringTexture != 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 ringModel = planetSystem;

        if (planet.name == "Saturno") {
            ringModel = glm::rotate(ringModel, glm::radians(23.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.7f, planet.size * 0.05f, planet.size * 1.7f));
        }
        else if (planet.name == "Jupiter") {
            ringModel = glm::rotate(ringModel, glm::radians(3.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.4f, planet.size * 0.02f, planet.size * 1.4f));
        }
        else if (planet.name == "Urano") {
            ringModel = glm::rotate(ringModel, glm::radians(98.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.3f, planet.size * 0.03f, planet.size * 1.3f));
        }
        else if (planet.name == "Neptuno") {
            ringModel = glm::rotate(ringModel, glm::radians(29.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.5f, planet.size * 0.025f, planet.size * 1.5f));
        }
        shader.setMat4("model", ringModel);
        glBindTexture(GL_TEXTURE_2D, planet.ringTexture);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        glDisable(GL_BLEND);
    }

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

// ===========================================
// FUNCIÓN DE CARGA DE TEXTURAS
// ===========================================

struct SolarSystemTextures {
    GLuint error, galaxy, sun, mercury, venus, earth, moon, mars;
    GLuint jupiter, jupiterRing, saturn, saturnRing;
    GLuint uranus, uranusRing, neptune, neptuneRing;
};

// Carga todas las texturas del sistema solar
SolarSystemTextures loadAllSolarSystemTextures() {
    SolarSystemTextures textures;

    textures.error = loadTexture("textures/error.png", 0);
    if (textures.error == 0) {
        cout << "ERROR CRÍTICO: No se pudo cargar la textura de error. Saliendo." << endl;
        return textures;
    }

    textures.galaxy = loadTexture("textures/galaxy.jpg", textures.error);
    textures.sun = loadTexture("textures/sun.jpg", textures.error);
    textures.mercury = loadTexture("textures/mercury.jpg", textures.error);
    textures.venus = loadTexture("textures/venus.jpg", textures.error);
    textures.earth = loadTexture("textures/earth.jpg", textures.error);
    textures.moon = loadTexture("textures/moon.jpg", textures.error);
    textures.mars = loadTexture("textures/mars.jpg", textures.error);
    textures.jupiter = loadTexture("textures/jupiter.jpg", textures.error);
    textures.jupiterRing = loadTexture("textures/jupiter_ring.png", textures.error);
    textures.saturn = loadTexture("textures/saturn.jpg", textures.error);
    textures.saturnRing = loadTexture("textures/saturn_ring.png", textures.error);
    textures.uranus = loadTexture("textures/uranus.jpg", textures.error);
    textures.uranusRing = loadTexture("textures/uranus_ring.png", textures.error);
    textures.neptune = loadTexture("textures/neptune.jpg", textures.error);
    textures.neptuneRing = loadTexture("textures/neptune_ring.png", textures.error);

    return textures;
}

// ===========================================
// FUNCIÓN PRINCIPAL (MAIN)
// ===========================================

// Función principal del programa - inicializa OpenGL y ejecuta el loop de renderizado
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sistema Solar v6 con UI", NULL, NULL);
    if (window == NULL) {
        cout << "Fallo al crear la ventana de GLFW" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Fallo al inicializar GLAD" << endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader ourShader("shaders/shader.vert", "shaders/shader.frag");
    Shader orbitShader("shaders/orbit.vert", "shaders/orbit.frag");

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    vector<float> circleVertices;
    const int orbitSegments = 100;
    createCircle(circleVertices, orbitSegments);

    unsigned int orbitVAO, orbitVBO;
    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);

    glBindVertexArray(orbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), &circleVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int meteoriteVAO, meteoriteVBO;
    float pointVertex[] = { 0.0f, 0.0f, 0.0f };

    glGenVertexArrays(1, &meteoriteVAO);
    glGenBuffers(1, &meteoriteVBO);
    glBindVertexArray(meteoriteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, meteoriteVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointVertex), &pointVertex, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    SolarSystemTextures textures = loadAllSolarSystemTextures();
    if (textures.error == 0) {
        glfwTerminate();
        return -1;
    }

    std::vector<Planet> planets;

    planets.push_back({ "Mercurio", 1.5f, 47.9f, 0.0f, 0.017f, 0.0f, 0.15f, textures.mercury,
                      false, 0.0f, 0.0f, 0.0f, 0, false, 0 });

    planets.push_back({ "Venus", 2.0f, 35.0f, 0.0f, 0.004f, 0.0f, 0.25f, textures.venus,
                      false, 0.0f, 0.0f, 0.0f, 0, false, 0 });

    planets.push_back({ "Tierra", 3.5f, 30.0f, 0.0f, 60.0f, 0.0f, 0.3f, textures.earth,
                      true, 0.7f, 200.0f, 0.0f, textures.moon, false, 0 });

    planets.push_back({ "Marte", 4.5f, 24.1f, 0.0f, 31.0f, 0.0f, 0.2f, textures.mars,
                      false, 0.0f, 0.0f, 0.0f, 0, false, 0 });

    planets.push_back({ "Jupiter", 6.0f, 13.1f, 0.0f, 28.0f, 0.0f, 0.5f, textures.jupiter,
                      false, 0.0f, 0.0f, 0.0f, 0,true, textures.jupiterRing });

    planets.push_back({ "Saturno", 7.5f, 9.7f, 0.0f, 22.0f, 0.0f, 0.45f, textures.saturn,
                      false, 0.0f, 0.0f, 0.0f, 0, true, textures.saturnRing });

    planets.push_back({ "Urano", 9.0f, 6.8f, 0.0f, 17.0f, 0.0f, 0.4f, textures.uranus,
                      false, 0.0f, 0.0f, 0.0f, 0, true, textures.uranusRing });

    planets.push_back({ "Neptuno", 10.5f, 5.4f, 0.0f, 16.0f, 0.0f, 0.38f, textures.neptune,
                      false, 0.0f, 0.0f, 0.0f, 0, true, textures.neptuneRing });

    std::vector<Meteorite> meteorites;
    for (int i = 0; i < MAX_METEORITES; ++i) {
        Meteorite m;
        m.isVisible = false;
        m.initialDelay = (float)(rand() % 5000) / 1000.0f;
        float randomX = -1.2f - (float)(rand() % 100) / 200.0f;
        float randomY = 1.2f + (float)(rand() % 100) / 200.0f;
        m.position = glm::vec3(randomX, randomY, 0.0f);
        float velX = 0.4f + (float)(rand() % 40) / 100.0f;
        float velY = -0.4f - (float)(rand() % 40) / 100.0f;
        m.velocity = glm::vec3(velX, velY, 0.0f);
        meteorites.push_back(m);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float totalTime = 0.0f;
    float sunRotationAngle = 0.0f;
    float sunRotationSpeed = 5.0f;

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        totalTime += deltaTime;

        float effectiveDeltaTime = animationPaused ? 0.0f : deltaTime;

        if (showMeteorites) {
            for (int i = 0; i < meteoriteCount; ++i) {
                if (!meteorites[i].isVisible) {
                    if (totalTime > meteorites[i].initialDelay) {
                        meteorites[i].isVisible = true;
                        float randomX = -1.2f - (float)(rand() % 100) / 200.0f;
                        float randomY = 1.2f + (float)(rand() % 100) / 200.0f;
                        meteorites[i].position = glm::vec3(randomX, randomY, 0.0f);
                        meteorites[i].initialDelay = totalTime + 3.0f + (float)(rand() % 3000) / 1000.0f;
                    }
                }
                else {
                    meteorites[i].position += meteorites[i].velocity * deltaTime;
                    if (meteorites[i].position.x > 1.2f || meteorites[i].position.y < -1.2f) {
                        meteorites[i].isVisible = false;
                    }
                }
            }
        }
        else {
            for (int i = 0; i < MAX_METEORITES; ++i) {
                meteorites[i].isVisible = false;
            }
        }

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(250, 280), ImGuiCond_FirstUseEver);
        ImGui::Begin("Tablero de controles");

        ImGui::SeparatorText("Personalizacion");
        ImGui::Checkbox("Mostrar nombres", &showNames);
        ImGui::Checkbox("Detener animacion", &animationPaused);
        ImGui::Checkbox("Mostrar orbitas", &showOrbits);

        ImGui::SeparatorText("Navegacion");
        ImGui::Checkbox("Habilitar mouse", &mouseControleEnabled);

        if (ImGui::Button("Resetear Vista", ImVec2(-1, 0))) {
            cameraPitch = 0.0f;
            cameraYaw = 0.0f;
            firstMouse = true;
        }

        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float buttonWidth = ImGui::GetFrameHeight();
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - (buttonWidth * 2 + spacing)) * 0.5f);

        if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
            cameraPitch += pitchIncrement;
            if (cameraPitch > maxPitch) cameraPitch = maxPitch;
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
            cameraPitch -= pitchIncrement;
            if (cameraPitch < minPitch) cameraPitch = minPitch;
        }

        ImGui::Text("*Usar tambien las teclas de navegacion.");

        ImGui::SeparatorText("Efectos");
        ImGui::Checkbox("Lluvia de meteoritos", &showMeteorites);

        if (showMeteorites) {
            ImGui::PushItemWidth(100);
            if (ImGui::InputInt("Cantidad", &meteoriteCount)) {
                if (meteoriteCount < 1) meteoriteCount = 1;
                if (meteoriteCount > 6) meteoriteCount = 6;
            }
            ImGui::PopItemWidth();
        }

        renderEducationalInterface();

        ImGui::End();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        if (display_h == 0) display_h = 1;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)display_w / (float)display_h, 0.1f, 100.0f);

        float cameraDistance = 22.0f;
        float pitchRad = glm::radians(cameraPitch);
        float yawRad = glm::radians(cameraYaw);

        glm::vec3 cameraPos;
        cameraPos.x = cameraDistance * cos(pitchRad) * sin(yawRad);
        cameraPos.y = cameraDistance * sin(pitchRad);
        cameraPos.z = cameraDistance * cos(pitchRad) * cos(yawRad);

        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        if (abs(cameraPitch) > 70.0f) {
            float factor = (90.0f - abs(cameraPitch)) / 20.0f;
            cameraUp.y = factor;
            cameraUp.z = (cameraPitch > 0) ? -(1.0f - factor) : (1.0f - factor);
            cameraUp = glm::normalize(cameraUp);
        }

        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glDepthMask(GL_FALSE);
        glm::mat4 model_background = glm::mat4(1.0f);
        model_background = glm::scale(model_background, glm::vec3(50.0f, 50.0f, 50.0f));
        ourShader.setMat4("model", model_background);
        glBindTexture(GL_TEXTURE_2D, textures.galaxy);
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);

        glm::mat4 model_sun = glm::mat4(1.0f);
        model_sun = glm::rotate(model_sun, glm::radians(sunRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model_sun = glm::scale(model_sun, glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("model", model_sun);
        glBindTexture(GL_TEXTURE_2D, textures.sun);
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        if (showOrbits) {
            orbitShader.use();
            orbitShader.setMat4("projection", projection);
            orbitShader.setMat4("view", view);
            orbitShader.setVec3("orbitColor", glm::vec3(0.4f, 0.4f, 0.4f));

            glBindVertexArray(orbitVAO);

            for (const auto& planet : planets) {
                glm::mat4 model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(planet.orbitRadius));
                orbitShader.setMat4("model", model_orbit);
                glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);
            }
        }

        ourShader.use();

        if (!animationPaused) {
            sunRotationAngle = std::fmod(sunRotationAngle + sunRotationSpeed * effectiveDeltaTime, 360.0f);
        }

        for (auto& planet : planets) {
            renderPlanet(ourShader, planet, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);
        }

        if (showNames) {
            renderTextIn3DSpace("Sol", glm::vec3(0.0f, 1.5f, 0.0f), view, projection);
        }

        if (showMeteorites) {
            orbitShader.use();
            glm::mat4 ortho_projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
            orbitShader.setMat4("projection", ortho_projection);
            orbitShader.setMat4("view", glm::mat4(1.0f));
            orbitShader.setVec3("orbitColor", glm::vec3(1.0f, 1.0f, 0.8f));

            glPointSize(5.0f);
            glBindVertexArray(meteoriteVAO);

            for (int i = 0; i < meteoriteCount; ++i) {
                if (meteorites[i].isVisible) {
                    glm::mat4 model_meteorite = glm::translate(glm::mat4(1.0f), meteorites[i].position);
                    orbitShader.setMat4("model", model_meteorite);
                    glDrawArrays(GL_POINTS, 0, 1);
                }
            }
            glPointSize(1.0f);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &meteoriteVAO);
    glDeleteBuffers(1, &meteoriteVBO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteVertexArrays(1, &orbitVAO);
    glDeleteBuffers(1, &orbitVBO);
    glDeleteProgram(ourShader.ID);
    glDeleteProgram(orbitShader.ID);

    glfwTerminate();
    return 0;
}