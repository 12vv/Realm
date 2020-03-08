#include "common/Scene/Lights/Directional/DirectionalLight.h"

DirectionalLight::DirectionalLight(const glm::vec3& direction):
direction(glm::normalize(direction))
{
    
}

void DirectionalLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;
    //    const glm::vec3 rayDirection = -1.f * glm::vec3(GetForwardDirection());
    const glm::vec3 rayDirection = -1.f * direction;
    output.emplace_back(origin, rayDirection);
}

float DirectionalLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    return 0.4f;
}

void DirectionalLight::GenerateRandomPhotonRay(Ray& ray) const
{
}
