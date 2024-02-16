#pragma once
#include "pch.h"

struct PointLight {
    alignas(16) glm::vec3 position = glm::vec3(0.);

    float constant = 0.5f;
    float linear = 0.5f;
    float quadratic = 0.5f;

    alignas(16) glm::vec3 ambient = glm::vec3(0.8f);
    alignas(16) glm::vec3 diffuse = glm::vec3(0.8f);
    alignas(16) glm::vec3 specular = glm::vec3(0.8f);
};
struct LightsList {
    unsigned int size;
    alignas(16) PointLight list[];
};
