// ===========================================
// Sistema Solar 3D Interactivo con OpenGL
// ===========================================
// Este programa renderiza una simulación del sistema solar con planetas,
// órbitas, lunas y efectos visuales usando OpenGL 3.3 Core Profile.

// Librerías principales de OpenGL
#include <glad/glad.h>     // Cargador de funciones OpenGL
#include <GLFW/glfw3.h>    // Gestión de ventanas y entrada

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
// CONFIGURACIÓN GLOBAL Y VARIABLES DE ESTADO
// ===========================================

// Dimensiones de la ventana de renderizado
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 768;

// Variables de control de cámara
float cameraPitch = 0.0f;                          // Ángulo de inclinación actual de la cámara (grados)
float pitchSpeed = 30.0f;                          // Velocidad de rotación de la cámara (grados/segundo)
const float maxPitch = 90.0f;                      // Límite superior de inclinación para evitar gimbal lock
const float minPitch = -90.0f;                     // Límite inferior de inclinación
const float pitchIncrement = pitchSpeed * 0.016f;  // Incremento por frame (asumiendo ~60 FPS)

// variables para el movimiento del mouse
bool firstMouse = true;	// Primera vez que se mueve el mouse
bool mouseControleEnabled = false; // Inicializamos el control del mouse deshabilitado
float lastMouseY = SCR_HEIGHT / 2.0f; // Última posición Y del mouse
float lastMouseX = SCR_WIDTH / 2.0f; // ultima posición X del mouse
float mouseSensitivity = 0.5f;  // Sensibilidad del movimiento 
float cameraYaw = 0.0f; // rotación horizontal



// Variables de estado de la interfaz
bool showNames = false;        // Mostrar/ocultar nombres de planetas
bool animationPaused = false;  // Pausar/reanudar animación del sistema solar
bool showOrbits = true;        // Mostrar/ocultar líneas de órbita
bool showMeteorites = false;   // Activar/desactivar lluvia de meteoritos
int meteoriteCount = 3;        // Cantidad de meteoritos activos simultáneamente

/**
 * Variables para controlar la visualización y funcionalidad de la tabla educativa
 */
bool showEducationalTable = true;          // Mostrar/ocultar tabla principal
bool showAdvancedData = false;             // Mostrar datos avanzados (masa, atmósfera)
bool showOnlyRockyPlanets = false;         // Filtro: solo planetas rocosos
bool showOnlyGasGiants = false;            // Filtro: solo gigantes gaseosos
bool highlightEarthComparisons = false;    // Resaltar comparaciones con la Tierra
int selectedPlanetForComparison = 2;       // Planeta seleccionado para comparación (2 = Tierra)
bool showFunFacts = true;


// ===========================================
// ESTRUCTURAS DE DATOS
// ===========================================

/**
 * Estructura que representa un cuerpo celeste (planeta) en el sistema solar.
 * Contiene todas las propiedades necesarias para su renderizado y animación.
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
	
	// Propiedades de satélites (Luna)
	bool hasMoon;          // Indica si el planeta tiene luna
	float moonDistance;    // Distancia de la luna al planeta (unidades relativas)
	float moonSpeed;       // Velocidad orbital de la luna (grados/segundo)
	float moonAngle;       // Ángulo actual de la luna en su órbita
	GLuint moonTexture;    // ID de la textura para la luna
	
	// Propiedades de anillos (Saturno)
	bool hasRing;          // Indica si el planeta tiene anillos
	GLuint ringTexture;    // ID de la textura para los anillos
};

/*Información educativa de planetas*/
struct PlanetData {
	string name;               // Nombre del planeta
	float distanceFromSunAU;        // Distancia promedio del Sol (Unidades Astronómicas)
	float distanceFromSunKM;        // Distancia en millones de kilómetros
	float orbitPeriodDays;          // Período orbital (cuánto dura un "año")
	float rotationPeriodHours;      // Período de rotación (cuánto dura un "día")
	float diameterKM;               // Diámetro ecuatorial en kilómetros
	float massEarths;               // Masa relativa a la Tierra (Tierra = 1.0)
	string planetType;         // Tipo: "Rocoso" o "Gaseoso"
	string atmosphere;         // Composición atmosférica principal
	string funFact;            // Dato curioso para engagement estudiantil
	ImVec4 highlightColor;          // Color para resaltar en la tabla
};


/**
 * Array con datos astronómicos reales de todos los planetas
 * Fuentes: NASA Planetary Fact Sheet, JPL, ESA
 */
PlanetData planetEducationalData[] = {
	{
		"☿ Mercurio",
		0.39f,                          // 0.39 UA del Sol
		57.9f,                          // 57.9 millones de km
		88.0f,                          // 88 días terrestres por órbita
		1407.6f,                        // 1407.6 horas por día (58.6 días terrestres)
		4879.0f,                        // 4,879 km de diámetro
		0.055f,                         // 5.5% de la masa terrestre
		"Rocoso",
		"Sin atmósfera",
		"Un día dura más que un año",
		ImVec4(0.8f, 0.7f, 0.6f, 1.0f) // Color gris-dorado
	},
	{
		"♀ Venus",
		0.72f,                          // 0.72 UA del Sol
		108.2f,                         // 108.2 millones de km
		225.0f,                         // 225 días terrestres
		5832.5f,                        // 5832.5 horas (243 días terrestres)
		12104.0f,                       // 12,104 km de diámetro
		0.815f,                         // 81.5% de la masa terrestre
		"Rocoso",
		"CO₂ denso (96%)",
		"Rota al revés (retrógrado)",
		ImVec4(1.0f, 0.8f, 0.4f, 1.0f) // Color amarillo-naranja
	},
	{
		"🌍 Tierra",
		1.0f,                           // 1.0 UA del Sol (definición de UA)
		149.6f,                         // 149.6 millones de km
		365.25f,                        // 365.25 días (año terrestre)
		24.0f,                          // 24 horas (día terrestre)
		12756.0f,                       // 12,756 km de diámetro
		1.0f,                           // 100% masa terrestre (referencia)
		"Rocoso",
		"N₂ (78%), O₂ (21%)",
		"Único planeta con vida conocida",
		ImVec4(0.4f, 0.8f, 1.0f, 1.0f) // Color azul terrestre
	},
	{
		"♂ Marte",
		1.52f,                          // 1.52 UA del Sol
		227.9f,                         // 227.9 millones de km
		687.0f,                         // 687 días terrestres
		24.6f,                          // 24.6 horas (similar a la Tierra)
		6792.0f,                        // 6,792 km de diámetro
		0.107f,                         // 10.7% de la masa terrestre
		"Rocoso",
		"CO₂ (95%), N₂ (3%)",
		"Tiene las montañas más altas del sistema solar",
		ImVec4(1.0f, 0.5f, 0.3f, 1.0f) // Color rojizo
	},
	{
		"♃ Júpiter",
		5.20f,                          // 5.20 UA del Sol
		778.5f,                         // 778.5 millones de km
		4333.0f,                        // 4,333 días terrestres (11.9 años)
		9.9f,                           // 9.9 horas (día más corto)
		142984.0f,                      // 142,984 km de diámetro
		317.8f,                         // 317.8 veces la masa terrestre
		"Gaseoso",
		"H₂ (89%), He (10%)",
		"Más masivo que todos los otros planetas juntos",
		ImVec4(0.9f, 0.7f, 0.5f, 1.0f) // Color marrón-naranja
	},
	{
		"♄ Saturno",
		9.58f,                          // 9.58 UA del Sol
		1432.0f,                        // 1,432 millones de km
		10747.0f,                       // 10,747 días terrestres (29.4 años)
		10.7f,                          // 10.7 horas
		120536.0f,                      // 120,536 km de diámetro
		95.2f,                          // 95.2 veces la masa terrestre
		"Gaseoso",
		"H₂ (96%), He (3%)",
		"Flotaría en agua (densidad < 1 g/cm³)",
		ImVec4(1.0f, 0.9f, 0.7f, 1.0f) // Color dorado claro
	},
	{
		"♅ Urano",
		19.20f,                         // 19.20 UA del Sol
		2867.0f,                        // 2,867 millones de km
		30589.0f,                       // 30,589 días terrestres (83.7 años)
		17.2f,                          // 17.2 horas
		51118.0f,                       // 51,118 km de diámetro
		14.5f,                          // 14.5 veces la masa terrestre
		"Gigante de hielo",
		"H₂ (83%), He (15%), CH₄ (2%)",
		"Rota de lado (inclinación 98°)",
		ImVec4(0.4f, 0.8f, 0.9f, 1.0f) // Color azul-verde
	},
	{
		"♆ Neptuno",
		30.05f,                         // 30.05 UA del Sol
		4515.0f,                        // 4,515 millones de km
		59800.0f,                       // 59,800 días terrestres (163.7 años)
		16.1f,                          // 16.1 horas
		49528.0f,                       // 49,528 km de diámetro
		17.1f,                          // 17.1 veces la masa terrestre
		"Gigante de hielo",
		"H₂ (80%), He (19%), CH₄ (1%)",
		"Vientos más rápidos del sistema solar (2,100 km/h)",
		ImVec4(0.2f, 0.4f, 1.0f, 1.0f) // Color azul profundo
	}
};




void createCircle(std::vector<float>& vertices, int numSegments);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void createSphere(vector<float>& vertices, vector<unsigned int>& indices);

/**
 * Estructura que representa un meteorito en el sistema de partículas.
 * Los meteoritos se mueven diagonalmente a través de la pantalla.
 */
struct Meteorite {
	glm::vec3 position;     // Posición actual en coordenadas normalizadas (-1 a 1)
	glm::vec3 velocity;     // Vector de velocidad y dirección del movimiento
	bool      isVisible;    // Estado de visibilidad actual
	float     timeToAppear; // Tiempo futuro en el que aparecerá (no usado actualmente)
	float     initialDelay; // Retraso inicial antes de la primera aparición
};

GLuint loadTexture(const char* path, GLuint fallbackTextureID);
void renderPlanet(Shader& shader, Planet& planet, unsigned int sphereVAO, const vector<unsigned int>& sphereIndices, float deltaTime, const glm::mat4& view, const glm::mat4& projection);
void renderTextIn3DSpace(const std::string& text, glm::vec3 worldPos, const glm::mat4& view, const glm::mat4& projection);
void renderUI();

/**
 * https://www.glfw.org/docs/3.3/input_guide.html#input_key
 * Callback para manejar eventos de teclado.
 * Controla la inclinación de la cámara usando las teclas de flecha.
 * 
 * @param window  Ventana GLFW que recibió el evento
 * @param key     Código de la tecla presionada
 * @param scancode Código de escaneo específico del sistema
 * @param action  Tipo de acción (GLFW_PRESS, GLFW_REPEAT)
 * @param mods    Modificadores activos (Shift, Ctrl, Alt)
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Solo procesar eventos de presión o repetición de tecla
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_UP) {
			// Inclinar cámara hacia arriba
			cameraPitch += pitchIncrement;
			// Limitar el ángulo máximo para evitar que la cámara se voltee
			if (cameraPitch > maxPitch) cameraPitch = maxPitch;
		}
		else if (key == GLFW_KEY_DOWN) {
			// Inclinar cámara hacia abajo
			cameraPitch -= pitchIncrement;
			// Limitar el ángulo mínimo
			if (cameraPitch < minPitch) cameraPitch = minPitch;
		}
		else if (key == GLFW_KEY_R) {
			// Tecla R: Resetear la vista a la posición horizontal
			cameraPitch = 0.0f;
			cameraYaw = 0.0f; // reiniciamos yaw en 0
			firstMouse = true; 
			
		}
		else if (key == GLFW_KEY_M) {
			// deshabilitamos el mov del mouse
			if (mouseControleEnabled == true) {
				mouseControleEnabled = false;
			}
			else
			{
				mouseControleEnabled = true;
			}
		}
	}
}


/*
https://www.glfw.org/docs/3.3/input_guide.html#input_mouse

*/
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (!mouseControleEnabled) return; // si está desactivado no aplicar movimiento

	// inicializamos la posición para evitar un salto brusco
	if (firstMouse) {
		lastMouseY = ypos;
		lastMouseX = xpos;
		firstMouse = false;
		return;
	}

	// calcular el offset vertical 
	float yoffset = lastMouseY - ypos; // Invertido: mover arriba 
	lastMouseY = ypos; //actualizamos la posicicón en lastMouseY
	// calcular el offset horizontal
	float xoffset = lastMouseX - xpos; // movimiento horizontal
	lastMouseX = xpos; // actualizamos la posición en lastMouseX

	
	// aplicamos la sensibilidad y actualizamos el pitch
	yoffset *= mouseSensitivity;
	cameraPitch += yoffset;

	xoffset *= mouseSensitivity; // aplicamos sesiblidad del mov para x
	cameraYaw += xoffset; // actualizamoz el yaw


	// aplicamos limite superior e inferior (pitch)
	if (cameraPitch > maxPitch) cameraPitch = maxPitch;
	if (cameraPitch < minPitch) cameraPitch = minPitch;

	// aplicamos limites para yaw 0° a 360°

	if (cameraYaw > 360.0f) cameraYaw -= 360.0f;
	if (cameraYaw < 0.0f) cameraYaw += 360.0f;
}

// PASO 5: FUNCIÓN DE RENDERIZADO DE LA TABLA
// ===========================================
/**
 * Renderiza la tabla principal con datos planetarios
 * Incluye filtros, colores y formateo responsivo
 */
void renderPlanetDataTable() {
	// Calcular número de columnas según opciones activas
	int columnCount = 4; // Básicas: Planeta, Distancia, Año, Día
	if (showAdvancedData) columnCount += 3; // +Diámetro, +Masa, +Atmósfera
	if (showFunFacts) columnCount += 1;     // +Dato Curioso

	// Configurar tabla con scroll y redimensionamiento
	ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_ScrollY |
		ImGuiTableFlags_Sortable;

	// Altura fija para scroll si hay muchos datos
	ImVec2 tableSize = ImVec2(0.0f, showAdvancedData ? 300.0f : 250.0f);

	if (ImGui::BeginTable("PlanetEducationalTable", columnCount, tableFlags, tableSize)) {

		// CONFIGURAR COLUMNAS
		ImGui::TableSetupColumn("🪐 Planeta", ImGuiTableColumnFlags_NoSort);
		ImGui::TableSetupColumn("📏 Distancia", ImGuiTableColumnFlags_DefaultSort);
		ImGui::TableSetupColumn("🗓️ Año (días)", ImGuiTableColumnFlags_DefaultSort);
		ImGui::TableSetupColumn("⏰ Día (horas)", ImGuiTableColumnFlags_DefaultSort);

		if (showAdvancedData) {
			ImGui::TableSetupColumn("📐 Diámetro (km)", ImGuiTableColumnFlags_DefaultSort);
			ImGui::TableSetupColumn("⚖️ Masa (Tierras)", ImGuiTableColumnFlags_DefaultSort);
			ImGui::TableSetupColumn("🌬️ Atmósfera", ImGuiTableColumnFlags_NoSort);
		}

		if (showFunFacts) {
			ImGui::TableSetupColumn("💡 Dato Curioso", ImGuiTableColumnFlags_NoSort);
		}

		// RENDERIZAR ENCABEZADOS
		ImGui::TableHeadersRow();

		// RENDERIZAR DATOS DE CADA PLANETA
		for (int i = 0; i < 8; i++) {
			auto& planet = planetEducationalData[i];

			// APLICAR FILTROS
			if (showOnlyRockyPlanets && planet.planetType != "Rocoso") continue;
			if (showOnlyGasGiants && (planet.planetType == "Rocoso")) continue;

			ImGui::TableNextRow();

			// COLUMNA 1: NOMBRE DEL PLANETA (con color)
			ImGui::TableNextColumn();
			if (highlightEarthComparisons && i == selectedPlanetForComparison) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(100, 200, 100, 50));
			}
			ImGui::TextColored(planet.highlightColor, "%s", planet.name.c_str());
			ImGui::TextDisabled("(%s)", planet.planetType.c_str());

			// COLUMNA 2: DISTANCIA DEL SOL
			ImGui::TableNextColumn();
			ImGui::Text("%.2f UA", planet.distanceFromSunAU);
			ImGui::TextDisabled("(%.0f M km)", planet.distanceFromSunKM);
			if (highlightEarthComparisons && i != 2) { // No comparar Tierra consigo misma
				float ratio = planet.distanceFromSunAU / planetEducationalData[2].distanceFromSunAU;
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.1fx", ratio);
			}

			// COLUMNA 3: PERÍODO ORBITAL (AÑO)
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

			// COLUMNA 4: PERÍODO DE ROTACIÓN (DÍA)
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

			// COLUMNAS AVANZADAS (si están activadas)
			if (showAdvancedData) {
				// COLUMNA 5: DIÁMETRO
				ImGui::TableNextColumn();
				ImGui::Text("%.0f km", planet.diameterKM);
				if (highlightEarthComparisons && i != 2) {
					float ratio = planet.diameterKM / planetEducationalData[2].diameterKM;
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), "%.1fx", ratio);
				}

				// COLUMNA 6: MASA
				ImGui::TableNextColumn();
				ImGui::Text("%.2f 🌍", planet.massEarths);

				// COLUMNA 7: ATMÓSFERA
				ImGui::TableNextColumn();
				ImGui::TextWrapped("%s", planet.atmosphere.c_str());
			}

			// COLUMNA: DATO CURIOSO (si está activada)
			if (showFunFacts) {
				ImGui::TableNextColumn();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
				ImGui::TextWrapped("%s", planet.funFact.c_str());
				ImGui::PopStyleColor();
			}
		}

		ImGui::EndTable();
	}
}


// PASO 6: FUNCIÓN DE INFORMACIÓN COMPARATIVA
// ===========================================
/**
 * Muestra información detallada del planeta seleccionado
 * Incluye comparaciones y datos de contexto
 */
void renderPlanetComparisonInfo() {
	if (selectedPlanetForComparison < 0 || selectedPlanetForComparison >= 8) return;

	auto& planet = planetEducationalData[selectedPlanetForComparison];
	auto& earth = planetEducationalData[2]; // Tierra como referencia

	ImGui::SeparatorText("🔍 Información Detallada");

	// Información del planeta seleccionado
	ImGui::Text("Planeta seleccionado:");
	ImGui::SameLine();
	ImGui::TextColored(planet.highlightColor, "%s", planet.name.c_str());

	// Crear dos columnas para comparaciones
	if (ImGui::BeginTable("ComparisonTable", 2, ImGuiTableFlags_Borders)) {
		ImGui::TableSetupColumn("Característica");
		ImGui::TableSetupColumn("Valor y Comparación");
		ImGui::TableHeadersRow();

		// Distancia del Sol
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::Text("Distancia del Sol");
		ImGui::TableNextColumn();
		ImGui::Text("%.2f UA (%.0f millones de km)", planet.distanceFromSunAU, planet.distanceFromSunKM);
		if (selectedPlanetForComparison != 2) {
			float ratio = planet.distanceFromSunAU / earth.distanceFromSunAU;
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), " → %.1fx más %s que la Tierra",
				abs(ratio), ratio > 1.0f ? "lejos" : "cerca");
		}

		// Tamaño
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::Text("Tamaño");
		ImGui::TableNextColumn();
		ImGui::Text("%.0f km de diámetro", planet.diameterKM);
		if (selectedPlanetForComparison != 2) {
			float ratio = planet.diameterKM / earth.diameterKM;
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), " → %.1fx %s que la Tierra",
				ratio, ratio > 1.0f ? "más grande" : "más pequeño");
		}

		// Masa
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::Text("Masa");
		ImGui::TableNextColumn();
		ImGui::Text("%.2f veces la masa terrestre", planet.massEarths);

		// Tipo de planeta
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::Text("Tipo");
		ImGui::TableNextColumn();
		ImGui::Text("%s", planet.planetType.c_str());

		// Atmósfera
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::Text("Atmósfera");
		ImGui::TableNextColumn();
		ImGui::TextWrapped("%s", planet.atmosphere.c_str());

		ImGui::EndTable();
	}

	// Dato curioso destacado
	ImGui::Spacing();
	ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.8f, 1.0f), "💡 Dato curioso:");
	ImGui::TextWrapped("%s", planet.funFact.c_str());
}

// PASO 4: FUNCIÓN PRINCIPAL DE RENDERIZADO DE LA TABLA
// =====================================================
/**
 * Función principal que renderiza toda la interfaz educativa
 * Incluye controles, filtros y la tabla de datos
 */
void renderEducationalInterface() {
	// Sección de la tabla educativa
	ImGui::SeparatorText("📚 Información Astronómica");

	// Controles principales
	ImGui::Checkbox("Mostrar tabla de datos", &showEducationalTable);

	if (!showEducationalTable) return; // Si está desactivada, no mostrar nada más

	// Controles de visualización
	ImGui::SameLine();
	ImGui::Checkbox("Datos avanzados", &showAdvancedData);
	ImGui::SameLine();
	ImGui::Checkbox("Datos curiosos", &showFunFacts);

	// Filtros de tipo de planeta
	ImGui::SeparatorText("🔍 Filtros");

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

	// Herramientas de comparación
	ImGui::SeparatorText("⚖️ Comparaciones");
	ImGui::Checkbox("Resaltar comparaciones con la Tierra", &highlightEarthComparisons);

	// Selector de planeta para comparación
	const char* planetNames[] = { "Mercurio", "Venus", "Tierra", "Marte", "Júpiter", "Saturno", "Urano", "Neptuno" };
	ImGui::SetNextItemWidth(150);
	ImGui::Combo("Comparar con", &selectedPlanetForComparison, planetNames, 8);

	// Renderizar la tabla principal
	renderPlanetDataTable();

	// Sección de información adicional
	if (selectedPlanetForComparison >= 0 && selectedPlanetForComparison < 8) {
		renderPlanetComparisonInfo();
	}
}




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

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

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

	GLuint errorTexture = loadTexture("textures/error.png", 0);
	if (errorTexture == 0) {
		// Si ni siquiera la textura de error se puede cargar, es un problema grave.
		// Podr�amos terminar el programa o intentar crear una proceduralmente.
		cout << "CRITICAL ERROR: Could not load the placeholder error texture. Exiting." << endl;
		glfwTerminate();
		return -1;
	}

	GLuint galaxyTexture = loadTexture("textures/galaxy.jpg", errorTexture);
	GLuint sunTexture = loadTexture("textures/sun.jpg", errorTexture);
	GLuint mercuryTexture = loadTexture("textures/mercury.jpg", errorTexture);
	GLuint venusTexture = loadTexture("textures/venus.jpg", errorTexture);
	GLuint earthTexture = loadTexture("textures/earth.jpg", errorTexture);
	GLuint moonTexture = loadTexture("textures/moon.jpg", errorTexture);
	GLuint marsTexture = loadTexture("textures/mars.jpg", errorTexture);
	GLuint jupiterTexture = loadTexture("textures/jupiter.jpg", errorTexture);
	GLuint saturnTexture = loadTexture("textures/saturn.jpg", errorTexture);
	GLuint saturnRingTexture = loadTexture("textures/saturn_ring.png", errorTexture);
	GLuint uranusTexture = loadTexture("textures/uranus.jpg", errorTexture);
	GLuint neptuneTexture = loadTexture("textures/neptune.jpg", errorTexture);

	std::vector<Planet> planets;

	planets.push_back({
		"Mercurio", 1.5f, 47.9f, 0.0f, 0.017f, 0.0f, 0.15f, mercuryTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
		});

	planets.push_back({
		"Venus", 2.0f, 35.0f, 0.0f, 0.004f, 0.0f, 0.25f, venusTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
		});

	planets.push_back({
		"Tierra", 3.5f, 30.0f, 0.0f, 60.0f, 0.0f, 0.3f, earthTexture,
		true, 0.7f, 200.0f, 0.0f, moonTexture, false, 0
		});

	planets.push_back({
		"Marte", 4.5f, 24.1f, 0.0f, 31.0f, 0.0f, 0.2f, marsTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
		});

	planets.push_back({
		"Jupiter", 6.0f, 13.1f, 0.0f, 28.0f, 0.0f, 0.5f, jupiterTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
		});

	planets.push_back({
		"Saturno", 7.5f, 9.7f, 0.0f, 22.0f, 0.0f, 0.45f, saturnTexture,
		false, 0.0f, 0.0f, 0.0f, 0, true, saturnRingTexture
		});

	planets.push_back({
		"Urano", 9.0f, 6.8f, 0.0f, 17.0f, 0.0f, 0.4f, uranusTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
		});

	planets.push_back({
		"Neptuno", 10.5f, 5.4f, 0.0f, 16.0f, 0.0f, 0.38f, neptuneTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
		});

	std::vector<Meteorite> meteorites;
	const int MAX_METEORITES = 6;
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
		}

		float spacing = ImGui::GetStyle().ItemSpacing.x;
		float buttonWidth = ImGui::GetFrameHeight();
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - (buttonWidth * 2 + spacing)) * 0.5f);

		if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
			cameraPitch += pitchIncrement;
			if (cameraPitch > maxPitch) {
				cameraPitch = maxPitch;
			}
		}

		ImGui::SameLine();

		if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
			cameraPitch -= pitchIncrement;
			if (cameraPitch < minPitch) {
				cameraPitch = minPitch;
			}
		}

		ImGui::Text("*Usar tambien las teclas de navegacion.");

		/* Efectos de meteoritos*/
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

		/*CONTROL DE CAMARA*/
		float cameraDistance = 22.0f;
		float pitchRad = glm::radians(cameraPitch);
		float yawRad = glm::radians(cameraYaw); // agregamos el calculo de la posición con Yaw

		glm::vec3 cameraPos; // vector tridimencional
		/*cameraPos.x = 0.0f; // Siempre en X = 0
		cameraPos.y = cameraDistance * sin(pitchRad); // Altura según el pitch
		cameraPos.z = cameraDistance * cos(pitchRad); // profundidad según el pitch*/
		cameraPos.x = cameraDistance * cos(pitchRad) * sin(yawRad); // ahora X varia también con yaw el mov en x
		cameraPos.y = cameraDistance * sin(pitchRad); // Altura según el pitch
		cameraPos.z = cameraDistance * cos(pitchRad) * cos(yawRad); // profundidad según el pitch

		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //Vector Up - (0, 1, 0) significa: "arriba es la dirección Y positiva"
		//prevencion del gimbalLock
		if (abs(cameraPitch) > 70.0f) { // angulo peligroso?
			float factor = (90.0f - abs(cameraPitch)) / 20.0f; // calculo del factor de transición
			cameraUp.y = factor; // ajustamos el componente Y 
			cameraUp.z = (cameraPitch > 0) ? -(1.0f - factor) : (1.0f - factor);
			cameraUp = glm::normalize(cameraUp); // Normaliza el vector up, es decir se establece la longitud en 1. https://stackoverflow.com/questions/17327906/what-glmnormalize-does
		}

		// glm::mat4 se usa para transformaciones geometricas en gráficos 3D, rotaciones, traslaciones y escalas
		// glm::lookAt define la orientación de la camara en el espacio 3D, recibe 3 parametros (Posciion de la camara, punto objetivo, vector up)
		glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp); 
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		glDepthMask(GL_FALSE);
		glm::mat4 model_background = glm::mat4(1.0f);
		model_background = glm::scale(model_background, glm::vec3(50.0f, 50.0f, 50.0f));
		ourShader.setMat4("model", model_background);
		glBindTexture(GL_TEXTURE_2D, galaxyTexture);
		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
		glDepthMask(GL_TRUE);

		glm::mat4 model_sun = glm::mat4(1.0f);
		model_sun = glm::rotate(model_sun, glm::radians(sunRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		model_sun = glm::scale(model_sun, glm::vec3(1.0f, 1.0f, 1.0f));
		ourShader.setMat4("model", model_sun);
		glBindTexture(GL_TEXTURE_2D, sunTexture);
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
	glDeleteProgram(ourShader.ID);
	glfwTerminate();
	return 0;
}

//Función que actualiza el tamaño de la vetnana cuando cambian las dimensiones https://www.glfw.org/docs/3.3/window_guide.html#window_full_screen
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

GLuint loadTexture(const char* path, GLuint fallbackTextureID) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents; //nrComponentes = Canales
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0); //Carga la imagen que carga en la ruta 
																			 // extraera la información del ancho, alto, canales y 0 es un flag; & permite el cambio de valores
	// si hay daa
	if (data) {
		GLenum format;
		if (nrComponents == 1) format = GL_RED;
		//3 canales (RGB)
		else if (nrComponents == 3) format = GL_RGB;
		//4 canales (RGBA)
		else if (nrComponents == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		//Configuramos los parametros de la textura, es decir cómo se va a comportar dentro del programa 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Linea vertical
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Linea Horizontal
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // MIN_FILTER es cuando la imagen se reduce o scala mas pequeño 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // MAG_FILTER es cuando la imagen se agranda o scala mas grande

		cout << "Textura cargada con exito: " << path << endl;
	}
	// si no hay data
	else {
		cout << "Error al cargar la textura: " << path << endl;
		cout << "Motivo del error (stb_image): " << stbi_failure_reason() << endl;

		glDeleteTextures(1, &textureID);

		return fallbackTextureID;
	}
	// liberamos la memoria usada por la imagen
	stbi_image_free(data);
	return textureID;
}

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

void renderPlanet(Shader& shader, Planet& planet, unsigned int sphereVAO,
	const vector<unsigned int>& sphereIndices, float deltaTime, const glm::mat4& view, const glm::mat4& projection) {

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
		ringModel = glm::rotate(ringModel, glm::radians(23.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		ringModel = glm::scale(ringModel, glm::vec3(planet.size * 1.7f, planet.size * 0.05f, planet.size * 1.7f));

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