#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "camera.h"
#include "light.h"
#include "model.h"
#include "model4D.h"

class Renderer {
public:
    Renderer(int width, int height, const char* title);
    ~Renderer();

    void Time(); // ����������� ������� �����
    void Clear();  // ������� ������
    void SwapBuffers(); // ������������ �������
    bool ShouldClose(); // �������� �� �����
    void PollEvents();  // ��������� �������

    void InitBuffers();
    
    std::map<std::string, GLuint> shaderPrograms; // �������
    void InitShaders();
    void InitFrameBuffers();

    void AddModel(Model* model); // ���������� ������
    void DrawModels(); // ��������� ���� �������
    void PostProcessing(); // �������������� ��������� ����� ������� �� �����
    void ProcessInput(GLFWwindow* window, float deltaTime); // ��������� �����

	double GetDeltaTime() const { return deltaTime; } // ��������� ������� �����
    GLFWwindow* GetWindow() const { return window; } // ��������� ��������� �� ����
    Camera* GetCamera() const { return camera; } // ��������� ��������� �� ������
    Camera* GetScreenCamera() const { return screenCamera; } // ��������� ��������� �� ������ �����������

private:
    GLFWwindow* window;
    float screenWidth, screenHeight;
    GLuint VAO, VBO, EBO, shaderProgram;
    GLuint quadVAO = 0, quadVBO;
    GLuint fbo, colorBufferTexture, depthBufferTexture;
    Camera* camera; // ��������� ��������� �� ������
    Camera* screenCamera;
    std::vector<Light*> lights; // ������ ���������� �����
    std::vector<Model*> models; // ������ �������

    double lastTime, currentTime, deltaTime;

  // ������������� �������
    void DrawModel(const Model* model); // ��������� ����� ������
    void RenderFullscreenQuad(); // ��������� �������� ��� �������� �������� ����� � ���������� ���������������

    GLuint CompileShader(GLenum shaderType, const std::string& shaderSource);
    GLuint CreateProgram(GLuint vertexShader, GLuint fragmentShader);
    std::vector<std::string> GetShaderFiles(const std::string& directory);
    void CheckShaderCompileError(GLuint shader, const std::string& shaderType);
    void CheckProgramLinkError(GLuint program);
};

#endif