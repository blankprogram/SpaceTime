#include <GLFW/glfw3.h>
#include <algorithm>
#include <glad/glad.h>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

// Trail‐point struct
struct TrailPoint {
    glm::vec3 pos;
    float life;
};

// Constants and Globals
static constexpr float G_CONST = 0.5f;
static constexpr float SUN_MASS = 10000.0f;
static constexpr float EARTH_MASS = 1.0f;
static constexpr float EARTH_DISTANCE = 20.0f;
static const float EARTH_SPEED = std::sqrt(G_CONST * SUN_MASS / EARTH_DISTANCE);

static constexpr float POINT_LIFETIME = 5.0f;
static constexpr float SAMPLE_INTERVAL = 0.025f;
static constexpr size_t MAX_TRAIL_POINTS = 1000;

static std::vector<TrailPoint> earthTrail;
static GLuint trailVAO = 0, trailVBO = 0;
static float sampleAccumulator = 0.0f;

static glm::vec3 sunPos = glm::vec3(0.0f);
static glm::vec3 earthPos = glm::vec3(EARTH_DISTANCE, 0.0f, 0.0f);
static glm::vec3 earthVel = glm::vec3(0.0f, 0.0f, EARTH_SPEED);

static glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
static glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
static glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

static float yaw = -90.0f;
static float pitch = 0.0f;
static float lastX = 400.0f;
static float lastY = 300.0f;
static bool firstMouse = true;

static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

// Function Declarations
void processInput(GLFWwindow *window);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);

std::string loadShaderSource(const char *path);
GLuint compileShader(GLenum type, const std::string &source);
GLuint createShaderProgram(const char *vsPath, const char *fsPath);

void generateSphereMesh(std::vector<float> &vertices,
                        std::vector<unsigned int> &indices, float radius = 1.0f,
                        int sectorCount = 36, int stackCount = 18);

GLuint loadTexture(const char *filePath);

void setupTrailBuffers();
void updateTrail(float dt);

// Process keyboard input
void processInput(GLFWwindow *window) {
    float baseSpeed = 2.5f;
    float speedMultiplier =
        (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 3.0f : 1.0f;
    float cameraSpeed = baseSpeed * speedMultiplier * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos +=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
}

// Mouse movement callback
void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xOffset = static_cast<float>(xpos) - lastX;
    float yOffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    glm::vec3 direction;
    direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    direction.y = std::sin(glm::radians(pitch));
    direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// Load shader source from file
std::string loadShaderSource(const char *path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " +
                                 std::string(path));
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Compile a single shader
GLuint compileShader(GLenum type, const std::string &source) {
    GLuint shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Compilation Error ("
                  << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
                  << "):\n"
                  << infoLog << "\n";
    }
    return shader;
}

// Link vertex + fragment shaders into a program
GLuint createShaderProgram(const char *vsPath, const char *fsPath) {
    std::string vsSource = loadShaderSource(vsPath);
    std::string fsSource = loadShaderSource(fsPath);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Error:\n" << infoLog << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

// Generate a UV sphere mesh
void generateSphereMesh(std::vector<float> &vertices,
                        std::vector<unsigned int> &indices, float radius,
                        int sectorCount, int stackCount) {
    const float PI = glm::pi<float>();

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2 - i * PI / stackCount;
        float xy = radius * std::cos(stackAngle);
        float z = radius * std::sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * PI / sectorCount;
            float x = xy * std::cos(sectorAngle);
            float y = xy * std::sin(sectorAngle);
            float u = static_cast<float>(j) / sectorCount;
            float v = static_cast<float>(i) / stackCount;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;
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

// Load a 2D texture from file
GLuint loadTexture(const char *filePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filePath, &width, &height, &channels, 0);
    if (data) {
        GLenum format = (channels == 4 ? GL_RGBA : GL_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << filePath << "\n";
    }
    stbi_image_free(data);
    return textureID;
}

// Setup the VAO/VBO for the trail
void setupTrailBuffers() {
    glGenVertexArrays(1, &trailVAO);
    glGenBuffers(1, &trailVBO);

    glBindVertexArray(trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);

    glBufferData(GL_ARRAY_BUFFER,
                 MAX_TRAIL_POINTS * (sizeof(glm::vec3) + sizeof(float)),
                 nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec3) + sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec3) + sizeof(float),
                          (void *)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// Update trail: sample, age, remove expired, upload
void updateTrail(float dt) {
    sampleAccumulator += dt;
    while (sampleAccumulator >= SAMPLE_INTERVAL) {
        sampleAccumulator -= SAMPLE_INTERVAL;
        earthTrail.push_back({earthPos, POINT_LIFETIME});
    }

    for (auto &tp : earthTrail) {
        tp.life -= dt;
    }

    earthTrail.erase(
        std::remove_if(earthTrail.begin(), earthTrail.end(),
                       [](const TrailPoint &tp) { return tp.life <= 0.0f; }),
        earthTrail.end());

    std::vector<float> bufferData;
    bufferData.reserve(earthTrail.size() * 4);
    for (auto &tp : earthTrail) {
        float lifeFrac = tp.life / POINT_LIFETIME;
        lifeFrac = std::max(lifeFrac, 0.0f);
        bufferData.push_back(tp.pos.x);
        bufferData.push_back(tp.pos.y);
        bufferData.push_back(tp.pos.z);
        bufferData.push_back(lifeFrac);
    }

    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(bufferData.size() * sizeof(float)),
                    bufferData.data());
}

// Main Entry Point
int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window =
        glfwCreateWindow(800, 600, "Earth-Sun Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return -1;
    }
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    GLuint trailShader =
        createShaderProgram("shaders/trail.vert", "shaders/trail.frag");
    GLuint mainShader =
        createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLint trailMVP_Loc = glGetUniformLocation(trailShader, "u_MVP");
    GLint mainMVP_Loc = glGetUniformLocation(mainShader, "u_MVP");
    GLint mainTex_Loc = glGetUniformLocation(mainShader, "u_Texture");

    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    generateSphereMesh(sphereVertices, sphereIndices);

    GLuint sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float),
                 sphereVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sphereIndices.size() * sizeof(unsigned int),
                 sphereIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    GLuint sunTexture = loadTexture("textures/lava.png");
    GLuint earthTexture = loadTexture("textures/dirt.jpg");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);

    setupTrailBuffers();

    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        glViewport(0, 0, screenWidth, screenHeight);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = std::min(currentFrame - lastFrame, 0.05f);
        lastFrame = currentFrame;

        processInput(window);

        {
            glm::vec3 direction = sunPos - earthPos;
            float distance = glm::length(direction);
            glm::vec3 forceDir = glm::normalize(direction);
            float forceMag =
                G_CONST * SUN_MASS * EARTH_MASS / (distance * distance);
            glm::vec3 acceleration = forceDir * (forceMag / EARTH_MASS);
            earthVel += acceleration * deltaTime;
            earthPos += earthVel * deltaTime;
        }

        updateTrail(deltaTime);

        glm::mat4 view =
            glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 proj = glm::perspective(
            glm::radians(45.0f), static_cast<float>(screenWidth) / screenHeight,
            0.1f, 100.0f);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw Sun
        glUseProgram(mainShader);
        glBindVertexArray(sphereVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        {
            glm::mat4 modelSun = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
            glm::mat4 mvpSun = proj * view * modelSun;
            glUniformMatrix4fv(mainMVP_Loc, 1, GL_FALSE,
                               glm::value_ptr(mvpSun));
            glUniform1i(mainTex_Loc, 0);
            glDrawElements(GL_TRIANGLES,
                           static_cast<GLsizei>(sphereIndices.size()),
                           GL_UNSIGNED_INT, nullptr);
        }

        // Draw Earth
        {
            glBindTexture(GL_TEXTURE_2D, earthTexture);
            glm::mat4 modelEarth = glm::translate(glm::mat4(1.0f), earthPos);
            modelEarth = glm::scale(modelEarth, glm::vec3(0.5f));
            glm::mat4 mvpEarth = proj * view * modelEarth;
            glUniformMatrix4fv(mainMVP_Loc, 1, GL_FALSE,
                               glm::value_ptr(mvpEarth));
            glDrawElements(GL_TRIANGLES,
                           static_cast<GLsizei>(sphereIndices.size()),
                           GL_UNSIGNED_INT, nullptr);
        }

        // Draw Trail as fading dots
        glUseProgram(trailShader);
        glBindVertexArray(trailVAO);
        {
            glm::mat4 mvpTrail = proj * view * glm::mat4(1.0f);
            glUniformMatrix4fv(trailMVP_Loc, 1, GL_FALSE,
                               glm::value_ptr(mvpTrail));

            int count = static_cast<int>(earthTrail.size());
            float pointSize = 4.0f;
            glPointSize(pointSize);
            glDrawArrays(GL_POINTS, 0, count);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteVertexArrays(1, &trailVAO);
    glDeleteBuffers(1, &trailVBO);

    glDeleteTextures(1, &sunTexture);
    glDeleteTextures(1, &earthTexture);
    glDeleteProgram(mainShader);
    glDeleteProgram(trailShader);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
