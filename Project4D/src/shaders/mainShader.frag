#version 440 core

struct Light {
    vec3 position;
    vec3 color;
};

uniform Light lights[10]; // �������� 10 ���������� �����
uniform int lightCount;
uniform vec3 viewPos;
uniform vec3 objectColor;

in vec3 FragPos;
in vec3 Normal;
uniform mat3 normalMatrix;

out vec4 FragColor;

void main() {
    vec3 norm = normalize(normalMatrix * Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // ������� ������� ��������
    vec3 ambient = vec3(0.1) * objectColor;
    vec3 result = ambient;

    for (int i = 0; i < lightCount; i++) {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        
        // ��������� ���������
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lights[i].color * objectColor;
        
        // ���������� ��������� (Phong)
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // ������� 32 = ������ �����
        vec3 specular = spec * lights[i].color;

        result += diffuse + specular;
    }


    FragColor = vec4(result, 1.0);
}
