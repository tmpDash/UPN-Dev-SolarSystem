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

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 768;

float cameraPitch = 0.0f;
float pitchSpeed = 30.0f;
const float maxPitch = 80.0f;
const float minPitch = -80.0f;
const float pitchIncrement = pitchSpeed * 0.016f;

bool showNames = false;
bool animationPaused = false;
bool showOrbits = true;
bool showMeteorites = false;
int meteoriteCount = 3;

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

void createCircle(std::vector<float>& vertices, int numSegments);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void createSphere(vector<float>& vertices, vector<unsigned int>& indices);

struct Meteorite {
	glm::vec3 position;
	glm::vec3 velocity;
	bool      isVisible;
	float     timeToAppear;
	float     initialDelay;
};

GLuint loadTexture(const char* path, GLuint fallbackTextureID);
void renderPlanet(Shader& shader, Planet& planet, unsigned int sphereVAO,
	const vector<unsigned int>& sphereIndices, float deltaTime, const glm::mat4& view, const glm::mat4& projection);
void renderTextIn3DSpace(const std::string& text, glm::vec3 worldPos, const glm::mat4& view, const glm::mat4& projection);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_UP) {
			cameraPitch += pitchSpeed * 0.016f;
			if (cameraPitch > maxPitch) cameraPitch = maxPitch;
		}
		else if (key == GLFW_KEY_DOWN) {
			cameraPitch -= pitchSpeed * 0.016f;
			if (cameraPitch < minPitch) cameraPitch = minPitch;
		}
		else if (key == GLFW_KEY_R) {
			cameraPitch = 0.0f;
		}
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
		// Podríamos terminar el programa o intentar crear una proceduralmente.
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

	Planet mercury = {
		"Mercurio", 1.5f, 47.9f, 0.0f, 0.017f, 0.0f, 0.15f, mercuryTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
	};

	Planet venus = {
		"Venus", 2.0f, 35.0f, 0.0f, 0.004f, 0.0f, 0.25f, venusTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
	};

	Planet earth = {
		"Tierra", 3.5f, 30.0f, 0.0f, 60.0f, 0.0f, 0.3f, earthTexture,
		true, 0.7f, 200.0f, 0.0f, moonTexture, false, 0
	};

	Planet mars = {
		"Marte", 4.5f, 24.1f, 0.0f, 31.0f, 0.0f, 0.2f, marsTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
	};

	Planet jupiter = {
		"Jupiter", 6.0f, 13.1f, 0.0f, 28.0f, 0.0f, 0.5f, jupiterTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
	};

	Planet saturn = {
		"Saturno", 7.5f, 9.7f, 0.0f, 22.0f, 0.0f, 0.45f, saturnTexture,
		false, 0.0f, 0.0f, 0.0f, 0, true, saturnRingTexture
	};

	Planet uranus = {
		"Urano", 9.0f, 6.8f, 0.0f, 17.0f, 0.0f, 0.4f, uranusTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
	};

	Planet neptune = {
		"Neptuno", 10.5f, 5.4f, 0.0f, 16.0f, 0.0f, 0.38f, neptuneTexture,
		false, 0.0f, 0.0f, 0.0f, 0, false, 0
	};

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

		glm::vec3 cameraPos;
		cameraPos.x = 0.0f;
		cameraPos.y = cameraDistance * sin(pitchRad);
		cameraPos.z = cameraDistance * cos(pitchRad);

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

			glm::mat4 model_orbit = glm::mat4(1.0f);

			model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(mercury.orbitRadius));
			orbitShader.setMat4("model", model_orbit);
			glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);

			model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(venus.orbitRadius));
			orbitShader.setMat4("model", model_orbit);
			glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);

			model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(earth.orbitRadius));
			orbitShader.setMat4("model", model_orbit);
			glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);

			model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(mars.orbitRadius));
			orbitShader.setMat4("model", model_orbit);
			glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);

			model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(jupiter.orbitRadius));
			orbitShader.setMat4("model", model_orbit);
			glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);

			model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(saturn.orbitRadius));
			orbitShader.setMat4("model", model_orbit);
			glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);

			model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(uranus.orbitRadius));
			orbitShader.setMat4("model", model_orbit);
			glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);

			model_orbit = glm::scale(glm::mat4(1.0f), glm::vec3(neptune.orbitRadius));
			orbitShader.setMat4("model", model_orbit);
			glDrawArrays(GL_LINE_STRIP, 0, orbitSegments + 1);
		}

		ourShader.use();

		if (!animationPaused) {
			sunRotationAngle = std::fmod(sunRotationAngle + sunRotationSpeed * effectiveDeltaTime, 360.0f);
		}

		renderPlanet(ourShader, mercury, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);
		renderPlanet(ourShader, venus, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);
		renderPlanet(ourShader, earth, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);
		renderPlanet(ourShader, mars, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);
		renderPlanet(ourShader, jupiter, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);
		renderPlanet(ourShader, saturn, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);
		renderPlanet(ourShader, uranus, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);
		renderPlanet(ourShader, neptune, sphereVAO, sphereIndices, effectiveDeltaTime, view, projection);

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
