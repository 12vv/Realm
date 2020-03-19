#include "common/Scene/Lights/Point/PointLight.h"

PointLight::PointLight():
samplesToUse(32)
{
}

void PointLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;
    const glm::vec3 lightPosition = glm::vec3(GetPosition());
    const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
    const float distanceToOrigin = glm::distance(origin, lightPosition);
    output.emplace_back(origin, rayDirection, distanceToOrigin);
}

float PointLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    const glm::vec3 lightPosition = glm::vec3(GetPosition());
    const float distanceToOrigin = glm::distance(origin, lightPosition);
//    return 1.f / (distanceToOrigin * distanceToOrigin * 4);
    return 1.f / (distanceToOrigin * 1.5);
}

void PointLight::GenerateRandomPhotonRay(Ray& ray) const
{
    float x, y, z;
    do{
        x = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
        y = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
        z = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
    }while((x*x + y*y + z*z) > 1);
    ray.SetRayPosition(glm::vec3(GetPosition()));
    ray.SetRayDirection(glm::normalize(glm::vec3(x, y, z)));
}
