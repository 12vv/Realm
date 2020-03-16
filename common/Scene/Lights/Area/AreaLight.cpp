#include "common/Scene/Lights/Area/AreaLight.h"

AreaLight::AreaLight(const glm::vec2& size):
    samplesToUse(32), lightSize(size)
{
    sampler = make_unique<JitterColorSampler>();
    sampler->SetGridSize(glm::ivec3(2, 2, 1));
}

void AreaLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;
    std::random_device rd;
    std::unique_ptr<SamplerState> sampleState = sampler->CreateSampler(rd, samplesToUse, 2);
    for (int i = 0; i < samplesToUse; ++i) {
        glm::vec3 sample = sampler->ComputeSampleCoordinate(*sampleState.get()) - 0.5f;
        sample.x *= lightSize.x;
        sample.y *= lightSize.y;
        sample.z = 0.f;

        const glm::vec3 lightPosition = glm::vec3(GetObjectToWorldMatrix() * glm::vec4(sample, 1.f));
        const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
        const float distanceToOrigin = glm::distance(origin, lightPosition);
        output.emplace_back(origin, rayDirection, distanceToOrigin);
    }
    
}

float AreaLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    const glm::vec3 lightPosition = glm::vec3(GetPosition());
    const float distanceToOrigin = glm::distance(origin, lightPosition);
    const glm::vec3 lightToPoint = glm::normalize(origin - lightPosition);
    //SMALL_EPSILON
    if (glm::dot(lightToPoint, glm::vec3(GetForwardDirection())) < -SMALL_EPSILON) {
        return 0.f;
    }
    return 1.f / (static_cast<float>(samplesToUse) * distanceToOrigin * distanceToOrigin);
//    return 1.f / static_cast<float>(samplesToUse);
//    return 1.f;
//    return 1.f / (static_cast<float>(samplesToUse) * distanceToOrigin);
    
//    const glm::vec3 lightPosition = glm::vec3(GetPosition());
//    const float distanceToOrigin = glm::distance(origin, lightPosition);
//    //    return 1.f / (distanceToOrigin * distanceToOrigin * 8);
//    return 1.f / (distanceToOrigin * 4);
}

void AreaLight::GenerateRandomPhotonRay(Ray& ray) const
{
    float x, y, z;
    do{
        x = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
        y = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
        z = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
    }while((x*x + y*y + z*z) > 1);
    ray.SetRayDirection(glm::normalize(glm::vec3(x, y, z)));
}

void AreaLight::SetSamplerAttributes(glm::ivec3 inputGridSize, int numSamples)
{
    sampler->SetGridSize(inputGridSize);
    samplesToUse = numSamples;
}
