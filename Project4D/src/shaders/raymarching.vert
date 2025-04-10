#version 440 core
layout (location = 2) in vec2 aPos;      // ������� � NDC (-1 �� 1)
layout (location = 3) in vec2 aTexCoord; // ���������� ���������� (0 �� 1)

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoord;
    gl_Position = vec4(aPos, 0.0, 1.0);
}