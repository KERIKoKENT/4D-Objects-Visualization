cmake_minimum_required(VERSION 3.12)
project(Project4D)

# Устанавливаем стандарт C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Создаём исполняемый файл
add_executable(Project4D "Project4D.cpp" "src/renderer.cpp" "src/renderer.h" "src/camera.h" "src/camera.cpp" "src/light.h" "src/light.cpp" "src/model.h" "src/model.cpp" "src/model4D.cpp")
set_target_properties(Project4D PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Project4D")

# Подключаем заголовочные файлы (GLM, GLFW, GLEW)
target_include_directories(Project4D PRIVATE
    ${CMAKE_SOURCE_DIR}/Project4D/libs/glfw/include
    ${CMAKE_SOURCE_DIR}/Project4D/libs/glew/include
    ${CMAKE_SOURCE_DIR}/Project4D/libs/glm
)

# Линкуем библиотеки (GLFW, GLEW, OpenGL)
target_link_libraries(Project4D PRIVATE
    ${CMAKE_SOURCE_DIR}/Project4D/libs/glfw/lib-vc2022/glfw3.lib
    ${CMAKE_SOURCE_DIR}/Project4D/libs/glew/lib/Release/x64/glew32.lib
    opengl32
)

add_custom_command(TARGET Project4D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/Project4D/libs/glew/bin/Release/x64/glew32.dll
    $<TARGET_FILE_DIR:Project4D>
)