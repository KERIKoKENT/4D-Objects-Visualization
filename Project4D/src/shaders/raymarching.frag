#version 440 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;

uniform vec3 camPos;
uniform mat3 camRot;
uniform mat4 projInverse;

struct Light {
    vec3 position;
    vec3 color;
};
uniform Light lights[10];
uniform int numLights;

uniform float reflectivity;
uniform float shadowIntensity;

float sceneSDF(vec3 p) {
    float sphere = length(p - vec3(0, 0, -3)) - 1.0;
    return sphere;
}

vec3 calcNormal(vec3 p) {
    float h = 0.001;
    vec2 k = vec2(1, -1);
    return normalize(k.xyy * sceneSDF(p + k.xyy * h) +
                     k.yyx * sceneSDF(p + k.yyx * h) +
                     k.yxy * sceneSDF(p + k.yxy * h) +
                     k.xxx * sceneSDF(p + k.xxx * h));
}

vec3 traceRayIterative(vec3 ro, vec3 rd) {
    vec3 finalColor = vec3(0);
    float reflectAmount = 1.0;

    for (int bounce = 0; bounce < 2; bounce++) {
        float t = 0.0;
        bool hit = false;
        vec3 p;
        vec3 normal;

        for (int i = 0; i < 64; i++) {
            p = ro + rd * t;
            float d = sceneSDF(p);
            if (d < 0.001) {
                hit = true;
                normal = calcNormal(p);
                break;
            }
            t += d;
            if (t > 20.0) break;
        }

        if (!hit) break;

        // Освещение
        vec3 baseColor = vec3(0.5); // базовый серый
        vec3 lightSum = vec3(0);

        for (int j = 0; j < numLights; j++) {
            vec3 lightDir = normalize(lights[j].position - p);
            float diff = max(dot(normal, lightDir), 0.0);

            // Тени
            float shadow = 1.0;
            float tShadow = 0.01;
            for (int k = 0; k < 10; k++) {
                float dShadow = sceneSDF(p + lightDir * tShadow);
                if (dShadow < 0.001) {
                    shadow = shadowIntensity;
                    break;
                }
                tShadow += dShadow;
                if (tShadow > 5.0) break;
            }

            lightSum += diff * lights[j].color * shadow;
        }

        vec3 localColor = baseColor * lightSum;
        finalColor += localColor * reflectAmount;

        // Подготовка к следующему отражению
        ro = p + normal * 0.01;
        rd = reflect(rd, normal);
        reflectAmount *= reflectivity;

        if (reflectAmount < 0.01) break;
    }

    return finalColor;
}

void main() {

    float depth = texture(sceneDepth, TexCoords).r;
    vec4 ndc = vec4(TexCoords * 2.0 - 1.0, depth, 1.0);
    vec4 worldPos = projInverse * ndc;
    worldPos /= worldPos.w;

    vec3 rayDir = normalize(worldPos.xyz - camPos);
    vec3 tracedColor = traceRayIterative(camPos, rayDir);
    vec3 originalColor = texture(sceneColor, TexCoords).rgb;

    FragColor = vec4(mix(originalColor, tracedColor, 0.5), 1.0);
}