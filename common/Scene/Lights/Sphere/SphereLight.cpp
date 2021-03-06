#include "common/Scene/Lights/Sphere/SphereLight.h"

SphereLight::SphereLight(float radius):
    samplesToUse(128), lightRadius(radius)
{
}

glm::vec3 sampleUnitSphere(){
    
    float u = (float)std::rand() / RAND_MAX * 2 -1; //uniform in [0, 1)
    float theta = (float)std::rand() / RAND_MAX *2 * PI; //uniform in [0, 2*PI)
    
    float x = std::sqrt(1-u*u) * std::cos(theta);
    float y = std::sqrt(1-u*u) * std::sin(theta);
    float z = u;
    
    glm::vec3 pos(x, y, z);
    
    return pos;
}


void SphereLight::ComputeSampleRays(std::vector<Ray> &output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;
    
    for (int i=0; i < samplesToUse; ++i) {
        const glm::vec3 lightPosition = glm::vec3(GetPosition());
        glm::vec3 sampleRayDirection = sampleUnitSphere();
        glm::vec3 lightPosOnSphere = sampleRayDirection * lightRadius + lightPosition;
        const glm::vec3 rayDirection = glm::normalize(lightPosOnSphere - origin);
        const float distanceToLight = glm::distance(origin, lightPosOnSphere);
        output.emplace_back(origin, rayDirection, distanceToLight);
    }
}


float SphereLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    return 1.f / static_cast<float>(samplesToUse);
//    const glm::vec3 lightPosition = glm::vec3(GetPosition());
//    const float distanceToOrigin = glm::distance(origin, lightPosition);
//    if(distanceToOrigin <= lightRadius){
//        return 5.f;
//    }
//    const float distanceToSpherePos = distanceToOrigin - lightRadius;
////    return 1.f / ((static_cast<float>(samplesToUse) + distanceToSpherePos * distanceToSpherePos));
//    return 1.f / (static_cast<float>(samplesToUse) + 0.01f * distanceToOrigin);
}


void SphereLight::GenerateRandomPhotonRay(Ray& ray) const
{
    // get random position on the sphere
    const glm::vec3 LightOrigin = glm::vec3(GetPosition());
    const glm::vec3 sampleDir = sampleUnitSphere();
    const glm::vec3 rayPos = sampleDir * lightRadius + LightOrigin;
    
    float x=1.f, y=1.f, z=1.f;
    do{
        x = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
        y = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
        z = -1 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(2)));
    }while((x*x + y*y + z*z) > 1);
    ray.SetRayPosition(rayPos);
    ray.SetRayDirection(glm::normalize(glm::vec3(x, y, z)));
}

