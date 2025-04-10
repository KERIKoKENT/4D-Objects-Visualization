#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// �����������: ������ ���� � �������������� OpenGL
Renderer::Renderer(int width, int height, const char* title) { // ������������� �����
    if (!glfwInit()) {
        std::cerr << "������ ������������� GLFW!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // ��������� OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    screenWidth = width;
    screenHeight = height;

    window = glfwCreateWindow(screenWidth, screenHeight, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "������ �������� ����!" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }


    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "������ ������������� GLEW!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // �������� ����� �����������
    glEnable(GL_DEPTH_TEST);

    camera = new Camera(glm::vec3(0.0f, 1.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f); // ������������� ������
    screenCamera = new Camera(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0), 0.0f, 0.0f);
    lights.push_back(new Light(glm::vec3(1.2f, 3.0f, 2.0f), glm::vec3(1.0f, 0.0f, 1.0f)));
    lights.push_back(new Light(glm::vec3(-1.2f, 3.0f, -2.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

    // ������������� ������� � ��������
    InitShaders();
    InitFrameBuffers();
}

// ������� ������
void Renderer::Clear() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// ������������ �������
void Renderer::SwapBuffers() {
    glfwSwapBuffers(window);
}

// �������� �� �������� ����
bool Renderer::ShouldClose() {
    return glfwWindowShouldClose(window);
}

void Renderer::Time() {
	currentTime = glfwGetTime();
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;
}

// ��������� �������
void Renderer::PollEvents() {
    glfwPollEvents();
    float currentFrame = glfwGetTime();
    static float lastFrame = 0.0f;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    ProcessInput(window, deltaTime);
    camera->ProcessCursor(window); // ��� ����������� ���������� ������� �������
}

// ����������: ������� ����
Renderer::~Renderer() {
    if (camera) {
        delete camera;
    }

    for (auto light : lights) {
        delete light;
    }

    for (auto model : models) {
        delete model;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

std::string ReadFile(const char* filePath) {
    std::filesystem::path path = std::filesystem::current_path() / filePath;
    path = path.lexically_normal();
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "������ �������� �����: " << path << std::endl;
        exit(EXIT_FAILURE);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ������������� �������
void Renderer::InitBuffers() {
    for (auto& model : models) {
        model->InitBuffers();
    }
}

void Renderer::InitFrameBuffers() {
    // ������� FBO
    if (fbo == 0) {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    // ������� �������� ��� �����
    glGenTextures(1, &colorBufferTexture);
    glBindTexture(GL_TEXTURE_2D, colorBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTexture, 0);

    // ������� �������� ��� ������� � ��������� (���� �����)
    glGenTextures(1, &depthBufferTexture);
    glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthBufferTexture, 0);

    // �������� �� ������������
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "������: Framebuffer �� �����!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // ������������ � ��������� ������
}

// ������������� ��������
GLuint Renderer::CompileShader(GLenum shaderType, const std::string& shaderSource) {
    GLuint shader = glCreateShader(shaderType);
    const char* shaderCode = shaderSource.c_str();
    glShaderSource(shader, 1, &shaderCode, nullptr);
    glCompileShader(shader);
    CheckShaderCompileError(shader, shaderType == GL_VERTEX_SHADER ? "�����������" : "������������");
    return shader;
}

GLuint Renderer::CreateProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    if (vertexShader != 0) {
        glAttachShader(program, vertexShader);
    }
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    CheckProgramLinkError(program);
    return program;
}

std::vector<std::string> Renderer::GetShaderFiles(const std::string& directory) {
    std::vector<std::string> shaderFiles;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        shaderFiles.push_back(entry.path().string());
    }
    return shaderFiles;
}

void Renderer::CheckShaderCompileError(GLuint shader, const std::string& shaderType) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "������ ���������� " << shaderType << " �������: " << infoLog << std::endl;
    }
}

void Renderer::CheckProgramLinkError(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "������ ���������� ��������� �������: " << infoLog << std::endl;
    }
}

void Renderer::InitShaders() {
    // ������ ���� ������ � ��������
    std::vector<std::string> shaderFiles = GetShaderFiles("..\\..\\..\\..\\Project4D\\src\\shaders\\");

    std::unordered_map<std::string, std::string> vertexShaders;
    std::unordered_map<std::string, std::string> fragmentShaders;

    // ������ ���� �������� � ���������� �� vertex � fragment
    for (const auto& file : shaderFiles) {
        std::string extension = file.substr(file.find_last_of(".") + 1);
        std::string name = file.substr(file.find_last_of("\\") + 1, file.find_last_of(".") - file.find_last_of("\\") - 1);

        if (extension == "vert") {
            vertexShaders[name] = ReadFile(file.c_str());
        }
        else if (extension == "frag") {
            fragmentShaders[name] = ReadFile(file.c_str());
        }
    }

    // �������� �������� ��������: ��� ���� vert/frag ������� ���� ���������
    for (const auto& vertPair : vertexShaders) {
        const std::string& shaderName = vertPair.first;

        if (fragmentShaders.find(shaderName) != fragmentShaders.end()) {
            // ��������� ��������� � ����������� ������ � ���������� ������
            GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaders[shaderName]);
            GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaders[shaderName]);

            // ������� ��������� � ��������� �������
            GLuint program = CreateProgram(vertexShader, fragmentShader);
            shaderPrograms[shaderName] = program;

            // ����������� ������� ����� ���������� � ����������
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }
    }

    // �������� ��������� �������� ��� ������ ����������� ��������
    for (const auto& fragPair : fragmentShaders) {
        const std::string& shaderName = fragPair.first;

        if (shaderPrograms.find(shaderName) == shaderPrograms.end()) {
            // ��� ����������� ��������, � ������� ��� ���������������� ���������� �������
            GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragPair.second);

            // ������� ��������� ������ ��� ������������ �������
            GLuint program = CreateProgram(0, fragmentShader);
            shaderPrograms[shaderName] = program;

            // ����������� ����������� ������
            glDeleteShader(fragmentShader);
        }
    }
}

// ��������� ���� �������
void Renderer::DrawModels() {
	shaderProgram = shaderPrograms["mainShader"];
    glUseProgram(shaderProgram);

    for (const auto& model : models) {
        DrawModel(model);
    }
}

// ���������� ������
void Renderer::AddModel(Model* model) {
    models.push_back(model);
    model->InitBuffers();
}

// ��������� ����� ������
void Renderer::DrawModel(const Model* model) {

    // ������������� ������� model, view � projection
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, model->GetPosition());
    modelMatrix = glm::scale(modelMatrix, model->GetSize());
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);

    // ���������� �������
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    // �������� ������ � ������
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["mainShader"], "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["mainShader"], "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["mainShader"], "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix3fv(glGetUniformLocation(shaderPrograms["mainShader"], "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // ������������� ��������� ���������
    int lightCount = std::min((int)lights.size(), 10);
    glUniform1i(glGetUniformLocation(shaderPrograms["mainShader"], "lightCount"), lightCount);

    for (int i = 0; i < lightCount; i++) {
        std::string lightPosStr = "lights[" + std::to_string(i) + "].position";
        std::string lightColorStr = "lights[" + std::to_string(i) + "].color";
        glUniform3f(glGetUniformLocation(shaderPrograms["mainShader"], lightPosStr.c_str()), lights[i]->GetPosition().x, lights[i]->GetPosition().y, lights[i]->GetPosition().z);
        glUniform3f(glGetUniformLocation(shaderPrograms["mainShader"], lightColorStr.c_str()), lights[i]->GetColor().x, lights[i]->GetColor().y, lights[i]->GetColor().z);
    }

    // ������� �������������� ���������
    glUniform3f(glGetUniformLocation(shaderPrograms["mainShader"], "viewPos"), camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);
    glUniform3f(glGetUniformLocation(shaderPrograms["mainShader"], "objectColor"), model->GetColor().x, model->GetColor().y, model->GetColor().z);

    // ��������� ������
    glBindVertexArray(model->GetVAO());
    glDrawElements(GL_TRIANGLES, model->GetIndices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::RenderFullscreenQuad() {
    // ��������� VAO � VBO ��� ��������, ���� ��� �� �������������
    if (quadVAO == 0) {
        GLfloat quadVertices[] = {
            // ����������    // ���������� ����������
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Renderer::PostProcessing() {

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // ������������ � ��������� ������
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderProgram = shaderPrograms["raymarching"];
    glUseProgram(shaderProgram);
    
	//InitFrameBuffers();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBufferTexture);
    glUniform1i(glGetUniformLocation(shaderPrograms["raymarching"], "sceneColor"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
    glUniform1i(glGetUniformLocation(shaderPrograms["raymarching"], "sceneDepth"), 1);
    

    // �������� ������� ������
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    glm::mat4 projInverse = glm::inverse(projection);

    glUniform3fv(glGetUniformLocation(shaderPrograms["raymarching"], "camPos"), 1, glm::value_ptr(view));
    glUniformMatrix3fv(glGetUniformLocation(shaderPrograms["raymarching"], "camRot"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderPrograms["raymarching"], "projInverse"), 1, GL_FALSE, glm::value_ptr(projInverse));


    // 2. ��������� �����
    glUniform1i(glGetUniformLocation(shaderPrograms["raymarching"], "numLights"), 3);
    for (int i = 0; i < 2; i++) {
        glUniform3fv(glGetUniformLocation(shaderPrograms["raymarching"], ("lights[" + std::to_string(i) + "].position").c_str()), 1, glm::value_ptr(lights[i]->GetPosition()));
        glUniform3fv(glGetUniformLocation(shaderPrograms["raymarching"], ("lights[" + std::to_string(i) + "].color").c_str()), 1, glm::value_ptr(lights[i]->GetColor()));
    }

    // 3. ���������
    glUniform1f(glGetUniformLocation(shaderPrograms["raymarching"], "reflectivity"), 0.5f);
    glUniform1f(glGetUniformLocation(shaderPrograms["raymarching"], "shadowIntensity"), 0.8f);

    RenderFullscreenQuad();

}

void Renderer::ProcessInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);

    
    static bool escPressed = false;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressed) {
        escPressed = true;
        glfwSetInputMode(window, GLFW_CURSOR, camera->switchCursor() ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
        escPressed = false;
    }
}