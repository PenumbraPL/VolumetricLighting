#include "pch.h"
#include "Light.h"

extern std::vector<PointLight> lightsData;


void init_lights(void) 
{
    lightsData.push_back({ glm::vec3(1.5, 1.5, 1.5), 0.1, 0.5, 0.5, glm::vec3(1., 1., 1.), glm::vec3(1., 1., 1.), glm::vec3(1., 1., 1.) });
    lightsData.push_back({ glm::vec3(-1.5, -1.5, 1.5), 0.1, 0.5, 0.5, glm::vec3(1., .9, .8), glm::vec3(.7, .5, .4), glm::vec3(.3, .2, .1) });
    lightsData.push_back({ glm::vec3(1.5, -1.5, 1.5), 0.1, 0.5, 0.5, glm::vec3(1., .9, .8), glm::vec3(.7, .5, .4), glm::vec3(.3, .2, .1) });
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


