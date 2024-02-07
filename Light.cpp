#include "pch.h"
#include "Light.h"

void init_lights(std::vector<PointLight>& lightsData)
{
    lightsData.push_back({ glm::vec3(1.5f, 1.5f, 1.5f), 0.1f, 0.5f, 0.5f, glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f) });
    lightsData.push_back({ glm::vec3(-1.5f, -1.5f, 1.5f), 0.1f, 0.5f, 0.5f, glm::vec3(1.f, .9f, .8f), glm::vec3(.7f, .5f, .4f), glm::vec3(.3f, .2f, .1f) });
    lightsData.push_back({ glm::vec3(1.5f, -1.5f, 1.5f), 0.1f, 0.5f, 0.5f, glm::vec3(1.f, .9f, .8f), glm::vec3(.7f, .5f, .4f), glm::vec3(.3f, .2f, .1f) });
}

bool compare_lights(PointLight& old_light, PointLight& new_light) 
{
    return memcmp(&old_light, &new_light, sizeof(PointLight));
}

bool compare_lights(LightsList& old_light, LightsList& new_light) 
{
    if (old_light.size != new_light.size) return true;

    return memcmp(&old_light.list, &new_light.list, old_light.size * sizeof(PointLight));
}
