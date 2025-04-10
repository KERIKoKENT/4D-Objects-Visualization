#include "src/renderer.h"
#include <thread>
#include <chrono>

int main() {
    Renderer renderer(800, 600, "Project4D");

    double deltaTime;

    Model* cube = new Model(ModelType::Cube, glm::vec3(1.0f, -0.5f, 1.0f), glm::vec3(10.0f, 0.02f, 10.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    renderer.AddModel(cube);


    // Добавление тессеракта
    Model4D* tesseract = new Model4D(Model4DType::Tesseract, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.87f, 0.1f, 0.12f, 1.0f), 0.1f);
    renderer.AddModel(tesseract);
    tesseract->GenerateSlice(tesseract->GetWSlice());


    while (!renderer.ShouldClose()) {
		renderer.Time();
        renderer.Clear();

        
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        
        tesseract->InitBuffers();

        deltaTime = renderer.GetDeltaTime();

        tesseract->Rotate(0.1f, 0.1f, 0.1f, deltaTime);

        renderer.GetCamera()->ProcessCursor(renderer.GetWindow());
        renderer.DrawModels();
        //renderer.PostProcessing();
        renderer.SwapBuffers();
        renderer.PollEvents();

    }

    return 0;
}
