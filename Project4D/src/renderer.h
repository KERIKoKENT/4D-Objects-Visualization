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

    void Time(); // Определение времени кадра
    void Clear();  // Очистка экрана
    void SwapBuffers(); // Переключение буферов
    bool ShouldClose(); // Проверка на выход
    void PollEvents();  // Обработка событий

    void InitBuffers();
    
    std::map<std::string, GLuint> shaderPrograms; // Шейдеры
    void InitShaders();
    void InitFrameBuffers();

    void AddModel(Model* model); // Добавление модели
    void DrawModels(); // Рендеринг всех моделей
    void PostProcessing(); // Дополнительная отрисовка перед выводом на экран
    void ProcessInput(GLFWwindow* window, float deltaTime); // Обработка ввода

	double GetDeltaTime() const { return deltaTime; } // Получение времени кадра
    GLFWwindow* GetWindow() const { return window; } // Получение указателя на окно
    Camera* GetCamera() const { return camera; } // Получение указателя на камеру
    Camera* GetScreenCamera() const { return screenCamera; } // Получение указателя на камеру отображения

private:
    GLFWwindow* window;
    float screenWidth, screenHeight;
    GLuint VAO, VBO, EBO, shaderProgram;
    GLuint quadVAO = 0, quadVBO;
    GLuint fbo, colorBufferTexture, depthBufferTexture;
    Camera* camera; // Добавляем указатель на камеру
    Camera* screenCamera;
    std::vector<Light*> lights; // Массив источников света
    std::vector<Model*> models; // Массив моделей

    double lastTime, currentTime, deltaTime;

  // Инициализация буферов
    void DrawModel(const Model* model); // Рендеринг одной модели
    void RenderFullscreenQuad(); // Рендеринг квадрата для хранения текстуры кадра и добавления постпроцессинга

    GLuint CompileShader(GLenum shaderType, const std::string& shaderSource);
    GLuint CreateProgram(GLuint vertexShader, GLuint fragmentShader);
    std::vector<std::string> GetShaderFiles(const std::string& directory);
    void CheckShaderCompileError(GLuint shader, const std::string& shaderType);
    void CheckProgramLinkError(GLuint program);
};

#endif