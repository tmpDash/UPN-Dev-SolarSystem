// ===========================================
// SISTEMA SOLAR 3D INTERACTIVO CON OPENGL
// ===========================================
// Simulador educativo del sistema solar con planetas, órbitas, lunas y efectos visuales
// Desarrollado para estudiantes de nivel primario/secundario
// Utiliza OpenGL 3.3 Core Profile con controles de cámara avanzados
// ===========================================

// ===========================================
// 1. INCLUDES Y LIBRERÍAS
// ===========================================

// Librerías principales de OpenGL
#include <glad/glad.h>     // Cargador de funciones OpenGL
#include <GLFW/glfw3.h>    // Gestión de ventanas y entrada

// Librerías matemáticas para gráficos 3D
#include <glm/glm.hpp>                     // Vectores y matrices
#include <glm/gtc/matrix_transform.hpp>    // Transformaciones (translate, rotate, scale)
#include <glm/gtc/type_ptr.hpp>            // Conversión de tipos GLM a punteros

// Librería para interfaz gráfica
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Librerías de C++
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

// Librerías personalizadas del proyecto
#include "Shader.h"        // Clase personalizada para manejo de shaders

// Librería para cargar texturas (implementación única)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// ===========================================
// 2. CONSTANTES Y CONFIGURACIÓN GLOBAL
// ===========================================

// Dimensiones de la ventana de renderizado
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 768;

// Límites de control de cámara (prevención gimbal lock)
const float maxPitch = 90.0f; // Límite superior de inclinación
const float minPitch = -90.0f; // Límite inferior de inclinación

// Configuración de meteoritos
const int MAX_METEORITES = 6; // Número máximo de meteoritos simultáneos

// ===========================================
// 3. ESTRUCTURAS DE DATOS
// ===========================================

/**
 * Estructura que representa un cuerpo celeste (planeta) en el sistema solar.
 * Contiene todas las propiedades necesarias para renderizado y animación.
 */
struct Planet {
    // Propiedades básicas
    string name;           // Nombre del planeta para mostrar en pantalla
    float orbitRadius;     // Radio de la órbita alrededor del sol (unidades arbitrarias)
    float orbitSpeed;      // Velocidad de traslación orbital (grados/segundo)
    float orbitAngle;      // Ángulo actual en la órbita (0-360 grados)
    float rotationSpeed;   // Velocidad de rotación sobre su propio eje (grados/segundo)
    float rotationAngle;   // Ángulo actual de rotación (0-360 grados)
    float size;            // Tamaño relativo del planeta (factor de escala)
    GLuint texture;        // ID de la textura OpenGL para la superficie del planeta

    // Propiedades de Luna
    bool hasMoon;          // Indica si el planeta tiene luna
    float moonDistance;    // Distancia de la luna al planeta (unidades relativas)
    float moonSpeed;       // Velocidad orbital de la luna (grados/segundo)
    float moonAngle;       // Ángulo actual de la luna en su órbita
    GLuint moonTexture;    // ID de la textura para la luna

    // Propiedades de anillos (Saturno)
    bool hasRing;          // Indica si el planeta tiene anillos
    GLuint ringTexture;    // ID de la textura para los anillos
};

/**
 * Estructura que representa un meteorito en el sistema de partículas.
 * Los meteoritos se mueven diagonalmente a través de la pantalla para efectos visuales.
 */
struct Meteorite {
    glm::vec3 position;     // Posición actual en coordenadas normalizadas (-1 a 1)
    glm::vec3 velocity;     // Vector de velocidad y dirección del movimiento
    bool isVisible;         // Estado de visibilidad actual
    float timeToAppear;     // Tiempo futuro en el que aparecerá (no usado actualmente)
    float initialDelay;     // Retraso inicial antes de la primera aparición
};

/**
 * Estructura que almacena información educativa real de los planetas.
 * Datos basados en fuentes astronómicas oficiales (NASA https://nssdc.gsfc.nasa.gov/planetary/factsheet/).
 */
struct PlanetData {
    string name;                    // Nombre del planeta
    float distanceFromSunAU;        // Distancia promedio del Sol (Unidades Astronómicas)
    float distanceFromSunKM;        // Distancia en millones de kilómetros
    float orbitPeriodDays;          // Período orbital - cuánto dura un "año"
    float rotationPeriodHours;      // Período de rotación - cuánto dura un "día"
    float diameterKM;               // Diámetro ecuatorial en kilómetros
    float massEarths;               // Masa relativa a la Tierra (Tierra = 1.0) https://nssdc.gsfc.nasa.gov/planetary/factsheet/planet_table_ratio.html
    float mass;                      // Masa del planeta
    string planetType;              // Tipo: "Rocoso", "Gaseoso", "Gigante de hielo"
    string atmosphere;              // Composición atmosférica principal
    string funFact;                 // Dato curioso para engagement estudiantil
    ImVec4 highlightColor;          // Color para resaltar en la tabla
};

// ===========================================
// 4. VARIABLES GLOBALES DE ESTADO
// ===========================================

// Variables de control de cámara - Sistema de coordenadas esféricas
float cameraPitch = 0.0f;                          // Ángulo de inclinación actual (grados) - PITCH
float cameraYaw = 0.0f;                            // Ángulo de rotación horizontal (grados) - YAW
float pitchSpeed = 30.0f;                          // Velocidad de rotación de la cámara (grados/segundo)
const float pitchIncrement = pitchSpeed * 0.016f;  // Incremento por frame (asumiendo ~60 FPS)

// Variables para control con mouse
bool firstMouse = true;                            // Primera vez que se mueve el mouse (evita salto inicial)
bool mouseControleEnabled = false;                 // Control de mouse deshabilitado por defecto
float lastMouseY = SCR_HEIGHT / 2.0f;             // Última posición Y del mouse
float lastMouseX = SCR_WIDTH / 2.0f;              // Última posición X del mouse
float mouseSensitivity = 0.5f;                    // Sensibilidad del movimiento del mouse

// Variables de estado de la interfaz y efectos visuales
bool showNames = false;                            // Mostrar/ocultar nombres de planetas
bool animationPaused = false;                      // Pausar/reanudar animación del sistema solar
bool showOrbits = true;                            // Mostrar/ocultar líneas de órbita
bool showMeteorites = false;                       // Activar/desactivar lluvia de meteoritos
int meteoriteCount = 3;                            // Cantidad de meteoritos activos simultáneamente

// Variables para controlar la tabla educativa
bool showEducationalTable = true;                 // Mostrar/ocultar tabla principal
bool showAdvancedData = false;                     // Mostrar datos avanzados (masa, atmósfera)
bool showOnlyRockyPlanets = false;                 // Filtro: solo planetas rocosos
bool showOnlyGasGiants = false;                    // Filtro: solo gigantes gaseosos
bool highlightEarthComparisons = false;            // Resaltar comparaciones con la Tierra
int selectedPlanetForComparison = 2;               // Planeta seleccionado para comparación (2 = Tierra)
bool showFunFacts = false;                         // Mostrar datos curiosos en la tabla

// ===========================================
// 5. BASE DE DATOS EDUCATIVA
// ===========================================

/**
 * Array con datos astronómicos reales de todos los planetas del sistema solar.
 * Fuentes: 
 * NASA Planetary Fact Sheet  https://nssdc.gsfc.nasa.gov/planetary/factsheet/index.html
 * JPL https://ssd.jpl.nasa.gov/planets/phys_par.html
 * ESA https://www.esa.int/kids/es/Aprende/Nuestro_Universo/Planetas_y_lunas/El_Sistema_Solar
 * Incluye información educativa para estudiantes de nivel primario/secundario.
 */
PlanetData planetEducationalData[] = {
    {
        "Mercurio",
        0.39f,                          // 0.4 UA del Sol
        57.9f,                         // 57.9 millones de km
        88.0f,                         // 88 días terrestres por órbita
        1407.6f,                       // 1407.6 horas por día (58.6 días terrestres)
        4879.0f,                       // 4,879 km de diámetro
        0.055f,                        // 5.5% de la masa terrestre
        0.330f,                        // masa del planeta
        "Rocoso",
        "Sin atmósfera",
        "Un día dura más que un año",
        ImVec4(0.8f, 0.7f, 0.6f, 1.0f) // Color gris-dorado
    },
    {
        "Venus",
        0.72f,                         // 0.72 UA del Sol
        108.2f,                        // 108.2 millones de km
        225.0f,                        // 225 días terrestres
        5832.5f,                       // 5832.5 horas (243 días terrestres)
        12104.0f,                      // 12,104 km de diámetro
        0.815f,                        // 81.5% de la masa terrestre
        4.87f,                        // masa del planeta
        "Rocoso",
        "Dióxido de carbono denso (96%), Nitrógeno (3%)",
        "Rota al revés (retrógrado)",
        ImVec4(1.0f, 0.8f, 0.4f, 1.0f) // Color amarillo-naranja
    },
    {
        "Tierra",
        1.0f,                          // 1.0 UA del Sol (definición de UA)
        149.6f,                        // 149.6 millones de km
        365.25f,                       // 365.25 días (año terrestre)
        24.0f,                         // 24 horas (día terrestre)
        12756.0f,                      // 12,756 km de diámetro
        1.0f,                          // 100% masa terrestre (referencia)
        5.97f,                        // masa del planeta
        "Rocoso",
        "Nitrógeno (78%), Óxígeno (21%)",
        "Único planeta con vida conocida",
        ImVec4(0.4f, 0.8f, 1.0f, 1.0f) // Color azul terrestre
    },
    {
        "Marte",
        1.52f,                         // 1.52 UA del Sol
        227.9f,                        // 227.9 millones de km
        687.0f,                        // 687 días terrestres
        24.6f,                         // 24.6 horas (similar a la Tierra)
        6792.0f,                       // 6,792 km de diámetro
        0.107f,                        // 10.7% de la masa terrestre
        0.642f,                        // masa del planeta
        "Rocoso",
        "Dióxido de carbono (95%), Nitrógeno (3%)",
        "Tiene las montañas más altas del sistema solar",
        ImVec4(1.0f, 0.5f, 0.3f, 1.0f) // Color rojizo
    },
    {
        "Júpiter",
        5.20f,                         // 5.20 UA del Sol
        778.5f,                        // 778.5 millones de km
        4333.0f,                       // 4,333 días terrestres (11.9 años)
        9.9f,                          // 9.9 horas (día más corto)
        142984.0f,                     // 142,984 km de diámetro
        317.8f,                        // 317.8 veces la masa terrestre
        1898.0f,                        // masa del planeta
        "Gaseoso",
        "Hidrógeno (89%), Helio (10%)",
        "Tiene más masa que todos los otros planetas juntos (aprox 2.5 veces)",
        ImVec4(0.9f, 0.7f, 0.5f, 1.0f) // Color marrón-naranja
    },
    {
        "Saturno",
        9.58f,                         // 9.58 UA del Sol
        1432.0f,                       // 1,432 millones de km
        10747.0f,                      // 10,747 días terrestres (29.4 años)
        10.7f,                         // 10.7 horas
        120536.0f,                     // 120,536 km de diámetro
        95.2f,                         // 95.2 veces la masa terrestre
        568.0f,                        // masa del planeta
        "Gaseoso",
        "Hidrógeno (96%), Helio (3%)",
        "Flotaría en agua (densidad < 1 g/cm³)",
        ImVec4(1.0f, 0.9f, 0.7f, 1.0f) // Color dorado claro
    },
    {
        "Urano",
        19.20f,                        // 19.20 UA del Sol
        2867.0f,                       // 2,867 millones de km
        30589.0f,                      // 30,589 días terrestres (83.7 años)
        17.2f,                         // 17.2 horas
        51118.0f,                      // 51,118 km de diámetro
        14.5f,                         // 14.5 veces la masa terrestre
        86.8f,                        // masa del planeta
        "Gigante de hielo",
        "Hidrógeno (83%), Helio (15%), Metano (2%)",
        "Rota de lado (inclinación 98°)",
        ImVec4(0.4f, 0.8f, 0.9f, 1.0f) // Color azul-verde
    },
    {
        "Neptuno",
        30.05f,                        // 30.05 UA del Sol
        4515.0f,                       // 4,515 millones de km
        59800.0f,                      // 59,800 días terrestres (163.7 años)
        16.1f,                         // 16.1 horas
        49528.0f,                      // 49,528 km de diámetro
        17.1f,                         // 17.1 veces la masa terrestre
        102.0f,                 // masa del planeta
        "Gigante de hielo",
        "Higrógeno (80%), Helio (19%), Metano (1%)",
        "Vientos más rápidos del sistema solar (2,100 km/h)",
        ImVec4(0.2f, 0.4f, 1.0f, 1.0f) // Color azul profundo
    }
    //Pluton no se considera planeta desde el 2006, debido a que no cumple con los criterios de La Unión Astronómica Internacional UAI

};

// ===========================================
// 6. DECLARACIONES DE FUNCIONES
// ===========================================

// Funciones de geometría y utilidades OpenGL
void createSphere(vector<float>& vertices, vector<unsigned int>& indices);
void createCircle(std::vector<float>& vertices, int numSegments);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Funciones de carga y manejo de recursos
GLuint loadTexture(const char* path, GLuint fallbackTextureID);

// Funciones de renderizado
void renderPlanet(Shader& shader, Planet& planet, unsigned int sphereVAO,
    const vector<unsigned int>& sphereIndices, float deltaTime,
    const glm::mat4& view, const glm::mat4& projection);
void renderTextIn3DSpace(const std::string& text, glm::vec3 worldPos,
    const glm::mat4& view, const glm::mat4& projection);

// Funciones de interfaz educativa - Tabla informativa
void renderEducationalInterface();
void renderPlanetDataTable();
void renderPlanetComparisonInfo();

// Funciones de entrada y control - Teclado y Mouse 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// ===========================================
// 7. FUNCIONES DE ENTRADA Y CONTROL
// ===========================================

/**
 * Callback para manejar eventos de teclado.
 * Controla la inclinación y rotación de la cámara usando las teclas de flecha y funciones especiales.
 * Referencia: https://www.glfw.org/docs/3.3/input_guide.html#input_key
 *
 * @param window   Ventana GLFW que recibió el evento
 * @param key      Código de la tecla presionada (GLFW_KEY_*)
 * @param scancode Código de escaneo específico del sistema (no usado)
 * @param action   Tipo de acción (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT)
 * @param mods     Modificadores activos (Shift, Ctrl, Alt)
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Solo procesar eventos de presión o repetición de tecla
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_UP) {
            // Tecla flecha hacia arriba: Inclinar cámara hacia arriba (aumentar pitch)
            cameraPitch += pitchIncrement;
            // Limitar el ángulo máximo para evitar gimbal lock
            if (cameraPitch > maxPitch) cameraPitch = maxPitch;
        }
        else if (key == GLFW_KEY_DOWN) {
            // Tecla flecha hacia abajo: Inclinar cámara hacia abajo (disminuir pitch)
            cameraPitch -= pitchIncrement;
            // Limitar el ángulo mínimo
            if (cameraPitch < minPitch) cameraPitch = minPitch;
        }
        else if (key == GLFW_KEY_R) {
            // Tecla R: Resetear la vista a la posición horizontal por defecto
            cameraPitch = 0.0f;
            cameraYaw = 0.0f;
            firstMouse = true;  // Reinicializar mouse para evitar saltos
        }
        else if (key == GLFW_KEY_M) {
            // Tecla M: Toggle (activar/desactivar) control con mouse
            mouseControleEnabled = !mouseControleEnabled;
        }
    }
}

/**
 * Callback para manejar movimiento del mouse.
 * Implementa control de cámara con pitch y yaw usando coordenadas esféricas.
 * Referencia: https://www.glfw.org/docs/3.3/input_guide.html#input_mouse
 *
 * @param window Ventana GLFW que recibió el evento
 * @param xpos   Posición X actual del cursor
 * @param ypos   Posición Y actual del cursor
 */
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // Si el control de mouse está desactivado, no procesar movimiento
    if (!mouseControleEnabled) return;

    // Primera vez: inicializar posición para evitar salto brusco al activar
    if (firstMouse) {
        lastMouseY = ypos;
        lastMouseX = xpos;
        firstMouse = false;
        return;
    }

    // Calcular offset de movimiento desde la última posición
    float yoffset = lastMouseY - ypos;  // Invertido: mover arriba = pitch positivo
    float xoffset = lastMouseX - xpos;  // Movimiento horizontal para yaw

    // Actualizar últimas posiciones
    lastMouseY = ypos;
    lastMouseX = xpos;

    // Aplicar sensibilidad y actualizar ángulos de cámara
    yoffset *= mouseSensitivity;
    xoffset *= mouseSensitivity;

    cameraPitch += yoffset;  // Controla inclinación arriba/abajo
    cameraYaw += xoffset;    // Controla rotación izquierda/derecha

    // Aplicar límites para pitch (evitar gimbal lock)
    if (cameraPitch > maxPitch) cameraPitch = maxPitch;
    if (cameraPitch < minPitch) cameraPitch = minPitch;

    // Mantener yaw en rango 0°-360° (opcional, para consistencia)
    if (cameraYaw > 360.0f) cameraYaw -= 360.0f;
    if (cameraYaw < 0.0f) cameraYaw += 360.0f;
}

// ===========================================
// 8. FUNCIONES DE INTERFAZ EDUCATIVA
// ===========================================

/**
 * Función principal que renderiza toda la interfaz educativa.
 * Incluye controles, filtros y la tabla de datos planetarios.
 */
void renderEducationalInterface() {
    // Sección principal de la tabla educativa
    ImGui::SeparatorText("Información Astronómica");

    // Control principal para mostrar/ocultar tabla
    ImGui::Checkbox("Mostrar tabla de datos", &showEducationalTable);
    if (!showEducationalTable) return; // Si está desactivada, no mostrar controles adicionales

    // Controles de visualización en la misma línea
    // Comentado por temas de espacio
    //ImGui::SameLine();
    //ImGui::Checkbox("Datos avanzados", &showAdvancedData);
    //ImGui::SameLine();
    //ImGui::Checkbox("Datos curiosos", &showFunFacts);

    // Sección de filtros por tipo de planeta
    ImGui::SeparatorText("Filtros");

    // Radio buttons para filtros mutuamente excluyentes
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

    // Sección de herramientas de comparación
    ImGui::SeparatorText("Comparaciones");
    ImGui::Checkbox("Resaltar comparaciones con la Tierra", &highlightEarthComparisons);

    // Selector de planeta para comparación detallada
    const char* planetNames[] = { "Mercurio", "Venus", "Tierra", "Marte", "Júpiter", "Saturno", "Urano", "Neptuno" };
    ImGui::SetNextItemWidth(150);
    ImGui::Combo("Comparar con", &selectedPlanetForComparison, planetNames, 8);

    // Renderizar la tabla principal con datos
    renderPlanetDataTable();

    // Mostrar información detallada del planeta seleccionado
    if (selectedPlanetForComparison >= 0 && selectedPlanetForComparison < 8) {
        renderPlanetComparisonInfo();
    }
}

/**
 * Renderiza la tabla principal con datos planetarios.
 * Incluye filtros dinámicos, colores identificativos.
 */
void renderPlanetDataTable() {
    // Calcular número de columnas según opciones activas
    int columnCount = 5; // Columnas básicas: Planeta, Distancia, Año, Día
    //if (showAdvancedData) columnCount += 3; // +Diámetro, +Masa, +Atmósfera
    //if (showFunFacts) columnCount += 1;     // +Dato Curioso

    // Configurar propiedades de la tabla
    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders |      // Bordes entre celdas
        ImGuiTableFlags_RowBg |          // Alternar color de filas
        ImGuiTableFlags_Resizable |      // Columnas redimensionables
        ImGuiTableFlags_ScrollY |        // Scroll vertical
        ImGuiTableFlags_Sortable;        // Ordenamiento por columnas

    // Altura fija para permitir scroll si hay muchos datos
    ImVec2 tableSize = ImVec2(0.0f, showAdvancedData ? 300.0f : 250.0f);

    if (ImGui::BeginTable("PlanetEducationalTable", columnCount, tableFlags, tableSize)) {

        // CONFIGURAR ENCABEZADOS DE COLUMNAS
        ImGui::TableSetupColumn("Planeta", ImGuiTableColumnFlags_NoSort);
        ImGui::TableSetupColumn("Distancia", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Año (días)", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Día (horas)", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Masa", ImGuiTableColumnFlags_DefaultSort);

        /* Comentado por temas de espacio
        if (showAdvancedData) {
            ImGui::TableSetupColumn("Diámetro (km)", ImGuiTableColumnFlags_DefaultSort);
            ImGui::TableSetupColumn("Masa (Tierras)", ImGuiTableColumnFlags_DefaultSort);
            ImGui::TableSetupColumn("Atmósfera", ImGuiTableColumnFlags_NoSort);
        }

        if (showFunFacts) {
            ImGui::TableSetupColumn("Dato Curioso", ImGuiTableColumnFlags_NoSort);
        }*/

        // Renderizar fila de encabezados
        ImGui::TableHeadersRow();

        // RENDERIZAR DATOS DE CADA PLANETA
        for (int i = 0; i < 8; i++) {
            auto& planet = planetEducationalData[i];

            // APLICAR FILTROS DE TIPO DE PLANETA
            if (showOnlyRockyPlanets && planet.planetType != "Rocoso") continue;
            if (showOnlyGasGiants && (planet.planetType == "Rocoso")) continue;

            ImGui::TableNextRow();

            // COLUMNA 1: NOMBRE DEL PLANETA (con color identificativo)
            ImGui::TableNextColumn();
            // Resaltar fila si está seleccionada para comparación
            if (highlightEarthComparisons && i == selectedPlanetForComparison) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(100, 200, 100, 50));
            }
            ImGui::TextColored(planet.highlightColor, "%s", planet.name.c_str());
            ImGui::TextDisabled("(%s)", planet.planetType.c_str());

            // COLUMNA 2: DISTANCIA DEL SOL
            ImGui::TableNextColumn();
            ImGui::Text("%.2f UA", planet.distanceFromSunAU);
            ImGui::TextDisabled("(%.0f M km)", planet.distanceFromSunKM);
            // Mostrar comparación con la Tierra si está activada
            if (highlightEarthComparisons && i != 2) { // No comparar Tierra consigo misma
                float ratio = planet.distanceFromSunAU / planetEducationalData[2].distanceFromSunAU;
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.1fx", ratio);
            }

            // COLUMNA 3: PERÍODO ORBITAL (AÑO)
            ImGui::TableNextColumn();
            ImGui::Text("%.0f días", planet.orbitPeriodDays);
            // Mostrar equivalencia en años terrestres para períodos largos
            if (planet.orbitPeriodDays >= 365) {
                float years = planet.orbitPeriodDays / 365.25f;
                ImGui::TextDisabled("(%.1f años)", years);
            }
            // Comparación con la Tierra
            if (highlightEarthComparisons && i != 2) {
                float ratio = planet.orbitPeriodDays / planetEducationalData[2].orbitPeriodDays;
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.1fx", ratio);
            }

            // COLUMNA 4: PERÍODO DE ROTACIÓN (DÍA)
            ImGui::TableNextColumn();
            ImGui::Text("%.1f h", planet.rotationPeriodHours);
            // Mostrar equivalencia en días terrestres para rotaciones lentas
            if (planet.rotationPeriodHours >= 24) {
                float days = planet.rotationPeriodHours / 24.0f;
                ImGui::TextDisabled("(%.1f días)", days);
            }
            // Comparación con la Tierra
            if (highlightEarthComparisons && i != 2) {
                float ratio = planet.rotationPeriodHours / planetEducationalData[2].rotationPeriodHours;
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.1fx", ratio);
            }

            // COLUMNA 5: MASA DEL PLANETA
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", planet.mass);
            //Comparacion con la tierra
            if (highlightEarthComparisons && i != 2) {
                //float ratio = planet.mass / planetEducationalData[2].mass;
                float ratio = planet.massEarths;
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.2fx", ratio);
            }

            // COLUMNAS AVANZADAS (mostradas solo si están activadas)
            /* Comentado por temas de espacio
            if (showAdvancedData) {
                // COLUMNA 5: DIÁMETRO
                ImGui::TableNextColumn();
                ImGui::Text("%.0f km", planet.diameterKM);
                if (highlightEarthComparisons && i != 2) {
                    float ratio = planet.diameterKM / planetEducationalData[2].diameterKM;
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.1fx", ratio);
                }

                // COLUMNA 6: MASA (relativa a la Tierra)
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", planet.massEarths);

                // COLUMNA 7: COMPOSICIÓN ATMOSFÉRICA
                ImGui::TableNextColumn();
                ImGui::TextWrapped("%s", planet.atmosphere.c_str());
            }

            // COLUMNA: DATO CURIOSO (si está activada)
            if (showFunFacts) {
                ImGui::TableNextColumn();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
                ImGui::TextWrapped("%s", planet.funFact.c_str());
                ImGui::PopStyleColor();
            }*/
        }

        ImGui::EndTable();
    }
}

/**
 * Muestra información detallada del planeta seleccionado.
 * Incluye comparaciones específicas y datos de contexto educativo.
 * Se actualiza dinámicamente según la selección del usuario.
 */
void renderPlanetComparisonInfo() {
    // Validar índice de selección
    if (selectedPlanetForComparison < 0 || selectedPlanetForComparison >= 8) return;

    // referencias para facilitar el acceso a los datos
    auto& planet = planetEducationalData[selectedPlanetForComparison];
    auto& earth = planetEducationalData[2]; // Tierra como referencia constante

    ImGui::SeparatorText("Información Detallada");

    // Mostrar nombre del planeta seleccionado con color identificativo
    ImGui::Text("Planeta seleccionado:");
    ImGui::SameLine();
    // se usa c_str() debido a que ImGUI::TextColored espera una cadena tipo C const char* y no un string como esta definido 
    ImGui::TextColored(planet.highlightColor, "%s", planet.name.c_str());

    // Crear tabla de dos columnas para comparaciones detalladas
    if (ImGui::BeginTable("ComparisonTable", 2, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Característica");
        ImGui::TableSetupColumn("Valor y Comparación");
        ImGui::TableHeadersRow();

        // Fila 1: Distancia del Sol
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Distancia del Sol");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f UA (%.0f millones de km)", planet.distanceFromSunAU, planet.distanceFromSunKM);
        // Agregar comparación con la Tierra (excepto si ES la Tierra)
        if (selectedPlanetForComparison != 2) {
            float ratio = planet.distanceFromSunAU / earth.distanceFromSunAU;
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), " → %.1fx más %s que la Tierra",
                abs(ratio), ratio > 1.0f ? "lejos" : "cerca");
        }

        // Fila 2: Tamaño comparativo
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

        // Fila 3: Masa relativa
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Masa");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f veces la masa terrestre", planet.massEarths);

        // Fila 4: Clasificación planetaria
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Tipo");
        ImGui::TableNextColumn();
        ImGui::Text("%s", planet.planetType.c_str());

        // Fila 5: Composición atmosférica
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Atmósfera");
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s", planet.atmosphere.c_str());

        ImGui::EndTable();
    }

    // Sección destacada con dato curioso para engagement estudiantil
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.8f, 1.0f), "Dato curioso:");
    ImGui::TextWrapped("%s", planet.funFact.c_str());
}

// ===========================================
// 9. FUNCIONES DE GEOMETRÍA Y UTILIDADES
// ===========================================

/**
 * Genera vértices e índices para una esfera perfecta usando parametrización esférica.
 * Utilizada para renderizar planetas y el Sol con geometría suave y coordenadas de textura.
 *
 * @param vertices Vector de salida con datos de vértices (posición + normal + UV)
 * @param indices  Vector de salida con índices para triangulación
 */
void createSphere(vector<float>& vertices, vector<unsigned int>& indices) {
    const float PI = 3.14159265359f;
    const int sectorCount = 36;   // Segmentos horizontales (longitud)
    const int stackCount = 18;    // Segmentos verticales (latitud)
    float radius = 1.0f;

    float x, y, z, xy;                               // Coordenadas del vértice
    float nx, ny, nz, lengthInv = 1.0f / radius;     // Coordenadas de la normal
    float s, t;                                      // Coordenadas de textura (UV)
    float sectorStep = 2 * PI / sectorCount;         // Paso angular horizontal
    float stackStep = PI / stackCount;               // Paso angular vertical
    float sectorAngle, stackAngle;

    // Generar vértices usando coordenadas esféricas
    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;        // Ángulo desde el polo norte
        xy = radius * cosf(stackAngle);             // Radio en el plano XY
        z = radius * sinf(stackAngle);              // Coordenada Z

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;           // Ángulo en el plano XY

            // Calcular posición del vértice
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Calcular normal (para iluminación)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);

            // Calcular coordenadas de textura
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }

    // Generar índices para triangulación (dos triángulos por quad)
    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // Triángulo superior (excepto en el polo norte)
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            // Triángulo inferior (excepto en el polo sur)
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

/**
 * Genera vértices para un círculo en el plano XZ (Y=0).
 * Utilizado para renderizar las órbitas planetarias como líneas circulares.
 *
 * @param vertices    Vector de salida con posiciones de vértices
 * @param numSegments Número de segmentos del círculo (más = más suave)
 */
void createCircle(std::vector<float>& vertices, int numSegments) {
    vertices.clear();
    float angleStep = 2.0f * 3.1415926535f / numSegments;

    // Generar puntos del círculo
    for (int i = 0; i <= numSegments; ++i) {
        float angle = i * angleStep;
        vertices.push_back(cos(angle));  // X
        vertices.push_back(0.0f);        // Y (siempre 0 para órbitas horizontales)
        vertices.push_back(sin(angle));  // Z
    }
}

/**
 * Callback para redimensionamiento de ventana.
 * Actualiza el viewport de OpenGL cuando cambian las dimensiones de la ventana.
 * Referencia: https://www.glfw.org/docs/3.3/window_guide.html#window_size
 *
 * @param window Ventana que cambió de tamaño
 * @param width  Nueva anchura en píxeles
 * @param height Nueva altura en píxeles
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// ===========================================
// 10. FUNCIONES DE CARGA Y MANEJO DE RECURSOS
// ===========================================

/**
 * Carga una textura desde archivo con fallback en caso de error.
 * Soporta formatos de imagen comunes (JPG, PNG) y maneja errores graciosamente.
 *
 * @param path              Ruta al archivo de imagen
 * @param fallbackTextureID ID de textura a usar si falla la carga
 * @return                  ID de la textura cargada o la textura fallback
 */
GLuint loadTexture(const char* path, GLuint fallbackTextureID) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;  // nrComponents = número de canales de color
    // Cargar imagen desde archivo
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

    if (data) {
        // Determinar formato según número de canales
        GLenum format;
        if (nrComponents == 1) format = GL_RED;        // Escala de grises
        else if (nrComponents == 3) format = GL_RGB;   // Color RGB
        else if (nrComponents == 4) format = GL_RGBA;  // Color RGBA (con transparencia)

        // Configurar textura en OpenGL
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);  // Generar niveles de detalle automáticamente

        // Configurar parámetros de filtrado y repetición
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);     // Repetir horizontalmente
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);     // Repetir verticalmente
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Filtrado para reducción
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtrado para ampliación

        cout << "Textura cargada con exito: " << path << endl;
    }
    else {
        // Error al cargar: usar textura fallback y reportar problema
        cout << "Error al cargar la textura: " << path << endl;
        cout << "Motivo del error (stb_image): " << stbi_failure_reason() << endl;

        glDeleteTextures(1, &textureID);  // Limpiar textura fallida
        return fallbackTextureID;         // Retornar textura de error
    }

    stbi_image_free(data);  // Liberar memoria de la imagen
    return textureID;
}

// ===========================================
// 11. FUNCIONES DE RENDERIZADO
// ===========================================

/**
 * Renderiza texto en el espacio 3D proyectándolo a coordenadas de pantalla.
 * Utiliza ImGui para dibujar texto 2D sobre la escena 3D.
 *
 * @param text       Texto a mostrar
 * @param worldPos   Posición en coordenadas del mundo 3D
 * @param view       Matriz de vista actual
 * @param projection Matriz de proyección actual
 */
void renderTextIn3DSpace(const std::string& text, glm::vec3 worldPos, const glm::mat4& view, const glm::mat4& projection) {
    // Obtener dimensiones actuales del framebuffer
    int display_w, display_h;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &display_w, &display_h);

    // Transformar posición 3D a coordenadas de clip
    glm::vec4 clipPos = projection * view * glm::vec4(worldPos, 1.0f);

    // Verificar si el punto está detrás de la cámara
    if (clipPos.w < 0.0f) return;

    // Convertir a coordenadas normalizadas de dispositivo (NDC)
    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

    // Convertir NDC a coordenadas de pantalla
    float screenX = (ndcPos.x + 1.0f) / 2.0f * display_w;
    float screenY = (1.0f - ndcPos.y) / 2.0f * display_h;  // Invertir Y

    // Dibujar texto usando ImGui
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenX, screenY), IM_COL32(255, 255, 255, 255), text.c_str());
}

/**
 * Renderiza un planeta completo con sus componentes (planeta, luna, anillos).
 * Maneja animaciones orbitales, rotaciones y efectos visuales específicos.
 *
 * @param shader       Shader a usar para el renderizado
 * @param planet       Estructura con datos del planeta
 * @param sphereVAO    VAO de la geometría esférica
 * @param sphereIndices Índices de la esfera
 * @param deltaTime    Tiempo transcurrido desde el último frame
 * @param view         Matriz de vista actual
 * @param projection   Matriz de proyección actual
 */
void renderPlanet(Shader& shader, Planet& planet, unsigned int sphereVAO,
    const vector<unsigned int>& sphereIndices, float deltaTime,
    const glm::mat4& view, const glm::mat4& projection) {

    // ACTUALIZAR ANIMACIONES DEL PLANETA
    // Avanzar ángulo orbital (traslación alrededor del Sol)
    planet.orbitAngle = std::fmod(planet.orbitAngle + planet.orbitSpeed * deltaTime, 360.0f);
    // Avanzar ángulo de rotación (rotación sobre su propio eje)
    planet.rotationAngle = std::fmod(planet.rotationAngle + planet.rotationSpeed * deltaTime, 360.0f);

    // Actualizar ángulo de la luna si el planeta tiene una
    if (planet.hasMoon) {
        planet.moonAngle = std::fmod(planet.moonAngle + planet.moonSpeed * deltaTime, 360.0f);
    }

    // CALCULAR SISTEMA DE COORDENADAS DEL PLANETA
    // 1. Crear transformación orbital (posición del planeta en su órbita)
    glm::mat4 planetSystem = glm::rotate(glm::mat4(1.0f),
        glm::radians(planet.orbitAngle),
        glm::vec3(0.0f, 1.0f, 0.0f));  // Rotar alrededor del eje Y
    planetSystem = glm::translate(planetSystem, glm::vec3(planet.orbitRadius, 0.0f, 0.0f));  // Mover a distancia orbital

    // 2. Crear modelo del planeta (incluye rotación propia)
    glm::mat4 planetModel = glm::rotate(planetSystem,
        glm::radians(planet.rotationAngle),
        glm::vec3(0.0f, 1.0f, 0.0f));  // Rotación sobre su eje
    planetModel = glm::scale(planetModel, glm::vec3(planet.size));    // Escalar al tamaño apropiado

    // RENDERIZAR EL PLANETA PRINCIPAL
    shader.setMat4("model", planetModel);
    glBindTexture(GL_TEXTURE_2D, planet.texture);
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

    // RENDERIZAR NOMBRE DEL PLANETA (si está activado)
    if (showNames) {
        glm::vec3 planetWorldPos = glm::vec3(planetSystem[3]);  // Extraer posición del planeta
        planetWorldPos.y += planet.size * 1.5f;                // Elevar texto sobre el planeta
        renderTextIn3DSpace(planet.name, planetWorldPos, view, projection);
    }

    // RENDERIZAR ANILLOS (solo Saturno)
    if (planet.hasRing && planet.ringTexture != 0) {
        // Activar transparencia para los anillos
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Crear modelo de anillos (esfera aplastada e inclinada)
        glm::mat4 ringModel = planetSystem;

        // CONFIGURACIONES ESPECÍFICAS POR PLANETA
        if (planet.name == "Saturno") {
            // Configuración actual de Saturno (mantener igual)
            ringModel = glm::rotate(ringModel, glm::radians(23.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.7f, planet.size * 0.05f, planet.size * 1.7f));
        }
        else if (planet.name == "Jupiter") {
            // Júpiter: anillos muy sutiles y delgados
            ringModel = glm::rotate(ringModel, glm::radians(3.0f), glm::vec3(1.0f, 0.0f, 0.0f));  // Poca inclinación
            ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.4f, planet.size * 0.02f, planet.size * 1.4f));  // Más pequeños y delgados
        }
        else if (planet.name == "Urano") {
            // Urano: anillos verticales (rotación de 90°)
            ringModel = glm::rotate(ringModel, glm::radians(98.0f), glm::vec3(1.0f, 0.0f, 0.0f));  // Casi vertical
            ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.3f, planet.size * 0.03f, planet.size * 1.3f));  // Delgados
        }
        else if (planet.name == "Neptuno") {
            // Neptuno: anillos débiles
            ringModel = glm::rotate(ringModel, glm::radians(29.0f), glm::vec3(1.0f, 0.0f, 0.0f));  // Inclinación moderada
            ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.5f, planet.size * 0.025f, planet.size * 1.5f));  // Tamaño medio
        }
        shader.setMat4("model", ringModel);
        glBindTexture(GL_TEXTURE_2D, planet.ringTexture);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        glDisable(GL_BLEND);  // Desactivar transparencia
    }

    // RENDERIZAR LUNA (solo la Tierra)
    if (planet.hasMoon && planet.moonTexture != 0) {
        // Crear modelo de la luna (orbita alrededor del planeta)
        glm::mat4 moonModel = planetSystem;  // Empezar desde la posición del planeta
        moonModel = glm::rotate(moonModel, glm::radians(planet.moonAngle), glm::vec3(0.0f, 1.0f, 0.0f));  // Órbita lunar
        moonModel = glm::translate(moonModel, glm::vec3(planet.moonDistance, 0.0f, 0.0f));  // Distancia de la luna
        moonModel = glm::scale(moonModel, glm::vec3(planet.size * 0.3f));  // Tamaño de la luna (30% del planeta)

        shader.setMat4("model", moonModel);
        glBindTexture(GL_TEXTURE_2D, planet.moonTexture);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
    }
}


// ===========================================
// FUNCIÓN DE CARGA DE TEXTURAS
// ===========================================

/**
 * Estructura para organizar todas las texturas del sistema solar
 */
struct SolarSystemTextures {
    GLuint error, galaxy, sun, mercury, venus, earth, moon, mars;
    GLuint jupiter, jupiterRing, saturn, saturnRing;
    GLuint uranus, uranusRing, neptune, neptuneRing;
};

/**
 * Carga todas las texturas del sistema solar
 * Incluye textura de error como fallback y todas las texturas planetarias
 */
SolarSystemTextures loadAllSolarSystemTextures() {
    SolarSystemTextures textures;

    // Cargar textura de error como fallback
    textures.error = loadTexture("textures/error.png", 0);
    if (textures.error == 0) {
        cout << "ERROR CRÍTICO: No se pudo cargar la textura de error. Saliendo." << endl;
        // El main manejará este error
        return textures;
    }

    // Cargar todas las texturas del sistema solar usando la textura de error como fallback
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
// 12. FUNCIÓN PRINCIPAL (MAIN)
// ===========================================

/**
 * Función principal del programa.
 * Inicializa OpenGL, crea recursos, configura la escena y ejecuta el loop principal.
 * Maneja la simulación completa del sistema solar con controles interactivos.
 */
int main() {
    // INICIALIZACIÓN DE GLFW Y OPENGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Core Profile (moderno)

    // Crear ventana principal
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sistema Solar v6 con UI", NULL, NULL);
    if (window == NULL) {
        cout << "Fallo al crear la ventana de GLFW" << endl;
        glfwTerminate();
        return -1;
    }

    // Configurar contexto y callbacks
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // Redimensionamiento
    glfwSetKeyCallback(window, key_callback);                           // Teclado
    glfwSetCursorPosCallback(window, mouse_callback);                   // Mouse

    // Cargar funciones de OpenGL usando GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Fallo al inicializar GLAD" << endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);  // Activar test de profundidad para 3D

    // CARGA DE SHADERS
    Shader ourShader("shaders/shader.vert", "shaders/shader.frag");       // Shader para objetos 3D
    Shader orbitShader("shaders/orbit.vert", "shaders/orbit.frag");       // Shader para órbitas y efectos

    // GENERACIÓN DE GEOMETRÍA - ESFERA
    vector<float> sphereVertices;
    vector<unsigned int> sphereIndices;
    createSphere(sphereVertices, sphereIndices);

    // Configurar VAO/VBO para esferas (planetas, Sol)
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);

    // Configurar atributos de vértice
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);                    // Posición
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // UV
    glEnableVertexAttribArray(2);

    // GENERACIÓN DE GEOMETRÍA - CÍRCULOS (ÓRBITAS)
    vector<float> circleVertices;
    const int orbitSegments = 100;
    createCircle(circleVertices, orbitSegments);

    // Configurar VAO/VBO para órbitas
    unsigned int orbitVAO, orbitVBO;
    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);

    glBindVertexArray(orbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), &circleVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // CONFIGURACIÓN DE METEORITOS
    unsigned int meteoriteVAO, meteoriteVBO;
    float pointVertex[] = { 0.0f, 0.0f, 0.0f };

    glGenVertexArrays(1, &meteoriteVAO);
    glGenBuffers(1, &meteoriteVBO);
    glBindVertexArray(meteoriteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, meteoriteVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointVertex), &pointVertex, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);  // Desvincular VAO

    // CARGA DE TEXTURAS
    // Cargar textura de error como fallback
    // CARGA DE TEXTURAS
    SolarSystemTextures textures = loadAllSolarSystemTextures();
    if (textures.error == 0) {
        glfwTerminate();
        return -1;
    }

    // CONFIGURACIÓN DE PLANETAS
    // Crear vector con todos los planetas del sistema solar
    std::vector<Planet> planets;

    // Inicializar cada planeta con sus parámetros específicos
    planets.push_back({ "Mercurio", 1.5f, 47.9f, 0.0f, 0.017f, 0.0f, 0.15f, textures.mercury,
                      false, 0.0f, 0.0f, 0.0f, 0, false, 0 });

    planets.push_back({ "Venus", 2.0f, 35.0f, 0.0f, 0.004f, 0.0f, 0.25f, textures.venus,
                      false, 0.0f, 0.0f, 0.0f, 0, false, 0 });

    planets.push_back({ "Tierra", 3.5f, 30.0f, 0.0f, 60.0f, 0.0f, 0.3f, textures.earth,
                      true, 0.7f, 200.0f, 0.0f, textures.moon, false, 0 });

    planets.push_back({ "Marte", 4.5f, 24.1f, 0.0f, 31.0f, 0.0f, 0.2f, textures.mars,
                      false, 0.0f, 0.0f, 0.0f, 0, false, 0 });

    planets.push_back({ "Jupiter", 6.0f, 13.1f, 0.0f, 28.0f, 0.0f, 0.5f, textures.jupiter,
                      false, 0.0f, 0.0f, 0.0f, 0,true, textures.jupiterRing});

    planets.push_back({ "Saturno", 7.5f, 9.7f, 0.0f, 22.0f, 0.0f, 0.45f, textures.saturn,
                      false, 0.0f, 0.0f, 0.0f, 0, true, textures.saturnRing });

    planets.push_back({ "Urano", 9.0f, 6.8f, 0.0f, 17.0f, 0.0f, 0.4f, textures.uranus,
                      false, 0.0f, 0.0f, 0.0f, 0, true, textures.uranusRing });

    planets.push_back({ "Neptuno", 10.5f, 5.4f, 0.0f, 16.0f, 0.0f, 0.38f, textures.neptune,
                      false, 0.0f, 0.0f, 0.0f, 0, true, textures.neptuneRing });

    // CONFIGURACIÓN DE METEORITOS
    // Inicializar sistema de partículas para efectos visuales
    std::vector<Meteorite> meteorites;
    for (int i = 0; i < MAX_METEORITES; ++i) {
        Meteorite m;
        m.isVisible = false;                                            // Inicialmente invisible
        m.initialDelay = (float)(rand() % 5000) / 1000.0f;            // Retraso aleatorio 0-5 segundos
        float randomX = -1.2f - (float)(rand() % 100) / 200.0f;       // Posición inicial X aleatoria
        float randomY = 1.2f + (float)(rand() % 100) / 200.0f;        // Posición inicial Y aleatoria
        m.position = glm::vec3(randomX, randomY, 0.0f);
        float velX = 0.4f + (float)(rand() % 40) / 100.0f;            // Velocidad X aleatoria
        float velY = -0.4f - (float)(rand() % 40) / 100.0f;           // Velocidad Y aleatoria (hacia abajo)
        m.velocity = glm::vec3(velX, velY, 0.0f);
        meteorites.push_back(m);
    }

    // INICIALIZACIÓN DE IMGUI
    // Configurar interfaz gráfica de usuario
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();                                         // Tema oscuro
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // VARIABLES DE CONTROL DE TIEMPO Y ANIMACIÓN
    float deltaTime = 0.0f;                 // Tiempo transcurrido entre frames
    float lastFrame = 0.0f;                 // Tiempo del frame anterior
    float totalTime = 0.0f;                 // Tiempo total transcurrido
    float sunRotationAngle = 0.0f;          // Ángulo de rotación del Sol
    float sunRotationSpeed = 5.0f;          // Velocidad de rotación del Sol (grados/segundo)

    // ===========================================
    // LOOP PRINCIPAL DE RENDERIZADO
    // ===========================================
    while (!glfwWindowShouldClose(window)) {

        // CÁLCULO DE TIEMPO
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        totalTime += deltaTime;

        // Tiempo efectivo (se puede pausar la animación)
        float effectiveDeltaTime = animationPaused ? 0.0f : deltaTime;

        // ACTUALIZACIÓN DE METEORITOS
        if (showMeteorites) {
            for (int i = 0; i < meteoriteCount; ++i) {
                if (!meteorites[i].isVisible) {
                    // Verificar si es momento de hacer aparecer el meteorito
                    if (totalTime > meteorites[i].initialDelay) {
                        meteorites[i].isVisible = true;
                        // Reposicionar en zona de aparición
                        float randomX = -1.2f - (float)(rand() % 100) / 200.0f;
                        float randomY = 1.2f + (float)(rand() % 100) / 200.0f;
                        meteorites[i].position = glm::vec3(randomX, randomY, 0.0f);
                        // Programar próxima aparición
                        meteorites[i].initialDelay = totalTime + 3.0f + (float)(rand() % 3000) / 1000.0f;
                    }
                }
                else {
                    // Mover meteorito según su velocidad
                    meteorites[i].position += meteorites[i].velocity * deltaTime;
                    // Verificar si salió de pantalla (ocultar para reciclaje)
                    if (meteorites[i].position.x > 1.2f || meteorites[i].position.y < -1.2f) {
                        meteorites[i].isVisible = false;
                    }
                }
            }
        }
        else {
            // Si los meteoritos están desactivados, ocultar todos
            for (int i = 0; i < MAX_METEORITES; ++i) {
                meteorites[i].isVisible = false;
            }
        }

        // PROCESAR EVENTOS DE ENTRADA
        glfwPollEvents();

        // INICIALIZAR FRAME DE IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // INTERFAZ DE USUARIO - PANEL DE CONTROL
        ImGui::SetNextWindowSize(ImVec2(250, 280), ImGuiCond_FirstUseEver);
        ImGui::Begin("Tablero de controles");

        // Sección de personalización visual
        ImGui::SeparatorText("Personalizacion");
        ImGui::Checkbox("Mostrar nombres", &showNames);
        ImGui::Checkbox("Detener animacion", &animationPaused);
        ImGui::Checkbox("Mostrar orbitas", &showOrbits);

        // Sección de navegación y control de cámara
        ImGui::SeparatorText("Navegacion");
        ImGui::Checkbox("Habilitar mouse", &mouseControleEnabled);

        if (ImGui::Button("Resetear Vista", ImVec2(-1, 0))) {
            cameraPitch = 0.0f;
            cameraYaw = 0.0f;
            firstMouse = true;
        }

        // Botones de control manual de cámara
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

        // Sección de efectos visuales
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

        // Renderizar interfaz educativa
        renderEducationalInterface();

        ImGui::End();

        // CONFIGURACIÓN DE RENDERIZADO 3D
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);                    // Color de fondo oscuro
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       // Limpiar buffers

        ourShader.use();

        // Obtener dimensiones actuales de la ventana
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        if (display_h == 0) display_h = 1;  // Evitar división por cero

        // CONFIGURACIÓN DE MATRICES DE PROYECCIÓN Y VISTA
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)display_w / (float)display_h, 0.1f, 100.0f);

        // SISTEMA DE CÁMARA CON COORDENADAS ESFÉRICAS (PITCH + YAW)
        float cameraDistance = 22.0f;                            // Distancia fija del Sol
        float pitchRad = glm::radians(cameraPitch);               // Convertir pitch a radianes
        float yawRad = glm::radians(cameraYaw);                   // Convertir yaw a radianes

        // Calcular posición de cámara usando trigonometría esférica
        glm::vec3 cameraPos;    
        cameraPos.x = cameraDistance * cos(pitchRad) * sin(yawRad); // ahora X varia también con yaw el mov en x
        cameraPos.y = cameraDistance * sin(pitchRad); // Altura según el pitch
        cameraPos.z = cameraDistance * cos(pitchRad) * cos(yawRad); // profundidad según el pitch

        // PREVENCIÓN DE GIMBAL LOCK
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);       // Vector Up por defecto
        if (abs(cameraPitch) > 70.0f) {                          // ¿Ángulo peligroso?
            float factor = (90.0f - abs(cameraPitch)) / 20.0f;   // Factor de transición suave
            cameraUp.y = factor;                                 // Ajustar componente Y
            cameraUp.z = (cameraPitch > 0) ? -(1.0f - factor) : (1.0f - factor);  // Compensar en Z
            cameraUp = glm::normalize(cameraUp);                 // Normaliza el vector up, es decir se establece la longitud en 1. https://stackoverflow.com/questions/17327906/what-glmnormalize-does
        }

        // glm::mat4 se usa para transformaciones geometricas en gráficos 3D, rotaciones, traslaciones y escalas
        // glm::lookAt define la orientación de la camara en el espacio 3D, recibe 3 parametros (Posciion de la camara, punto objetivo, vector up)
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);

        // Enviar matrices a shaders
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // RENDERIZADO DEL FONDO (GALAXIA)
        glDepthMask(GL_FALSE);  // Desactivar escritura en depth buffer
        glm::mat4 model_background = glm::mat4(1.0f);
        model_background = glm::scale(model_background, glm::vec3(50.0f, 50.0f, 50.0f));  // Esfera gigante
        ourShader.setMat4("model", model_background);
        glBindTexture(GL_TEXTURE_2D, textures.galaxy);
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);   // Reactivar depth buffer

        // RENDERIZADO DEL SOL
        glm::mat4 model_sun = glm::mat4(1.0f);
        model_sun = glm::rotate(model_sun, glm::radians(sunRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model_sun = glm::scale(model_sun, glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("model", model_sun);
        glBindTexture(GL_TEXTURE_2D, textures.sun);
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // RENDERIZADO DE ÓRBITAS PLANETARIAS
        if (showOrbits) {
            orbitShader.use();
            orbitShader.setMat4("projection", projection);
            orbitShader.setMat4("view", view);
            orbitShader.setVec3("orbitColor", glm::vec3(0.4f, 0.4f, 0.4f));  // Color gris

            glBindVertexArray(orbitVAO);

            // Renderizar órbita de cada planeta
            for (const auto& planet : planets) {
                glm::mat4 model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(planet.orbitRadius));
                orbitShader.setMat4("model", model_orbit);
                glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);
            }
        }

        // VOLVER AL SHADER PRINCIPAL PARA PLANETAS
        ourShader.use();

        // ACTUALIZAR ROTACIÓN DEL SOL
        if (!animationPaused) {
            sunRotationAngle = std::fmod(sunRotationAngle + sunRotationSpeed * effectiveDeltaTime, 360.0f);
        }

        // RENDERIZADO DE TODOS LOS PLANETAS
        for (auto& planet : planets) {
            renderPlanet(ourShader, planet, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);
        }

        // RENDERIZADO DE NOMBRES (SI ESTÁ ACTIVADO)
        if (showNames) {
            renderTextIn3DSpace("Sol", glm::vec3(0.0f, 1.5f, 0.0f), view, projection);
        }

        // RENDERIZADO DE METEORITOS
        if (showMeteorites) {
            orbitShader.use();
            // Configurar proyección ortogonal para meteoritos (efecto 2D sobre 3D)
            glm::mat4 ortho_projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
            orbitShader.setMat4("projection", ortho_projection);
            orbitShader.setMat4("view", glm::mat4(1.0f));
            orbitShader.setVec3("orbitColor", glm::vec3(1.0f, 1.0f, 0.8f));  // Color amarillo-blanco

            glPointSize(5.0f);  // Tamaño de puntos
            glBindVertexArray(meteoriteVAO);

            // Renderizar cada meteorito visible
            for (int i = 0; i < meteoriteCount; ++i) {
                if (meteorites[i].isVisible) {
                    glm::mat4 model_meteorite = glm::translate(glm::mat4(1.0f), meteorites[i].position);
                    orbitShader.setMat4("model", model_meteorite);
                    glDrawArrays(GL_POINTS, 0, 1);
                }
            }
            glPointSize(1.0f);  // Restaurar tamaño de punto por defecto
        }

        // RENDERIZADO DE INTERFAZ IMGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // INTERCAMBIAR BUFFERS Y CONTINUAR LOOP
        glfwSwapBuffers(window);
    }

    // ===========================================
    // LIMPIEZA Y FINALIZACIÓN
    // ===========================================

    // Limpiar recursos de ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Limpiar recursos de OpenGL
    glDeleteVertexArrays(1, &meteoriteVAO);
    glDeleteBuffers(1, &meteoriteVBO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteVertexArrays(1, &orbitVAO);
    glDeleteBuffers(1, &orbitVBO);
    glDeleteProgram(ourShader.ID);
    glDeleteProgram(orbitShader.ID);

    // Finalizar GLFW
    glfwTerminate();
    return 0;
}
