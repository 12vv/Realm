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

//void PointLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
//{
//    origin += normal * LARGE_EPSILON;
//    const glm::vec3 lightPosition = glm::vec3(GetPosition());
//    const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
//    const float distanceToOrigin = glm::distance(origin, lightPosition);
//
//    for (int i = 0; i < samplesToUse; ++i) {
//        //        Ray tmpRay;
//
//        float u1 = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1)));
//        float u2 = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1)));
//
//        //        std::cout << u1 << std::endl;
//
//        float r = sqrt(u1);
//        float theta = 2 * M_PI * u2;
//
//        float x = r * cos(theta);
//        float y = r * sin(theta);
//        float z = sqrt(1 - u1);
//
//        glm::vec3 tmpDir = glm::normalize(glm::vec3(x, y, z));
//
//
//        // transform
//        glm::vec3 t = glm::cross(normal, glm::vec3(1, 0, 0));
//        glm::vec3 b = glm::cross(normal, t);
//
//        glm::mat3 trans(t, b, normal);
//        tmpDir = tmpDir * glm::transpose(trans);
//        //        diffuseReflectionDir = glm::transpose(trans) * diffuseReflectionDir;
//
//        //        tmpRay.SetRayDirection(tmpDir);
//        //        tmpRay.SetRayPosition(origin + LARGE_EPSILON * tmpDir);
//        output.emplace_back(origin, tmpDir);
//    }
//
//    //    origin += normal * LARGE_EPSILON;
//    //    const glm::vec3 lightPosition = glm::vec3(GetPosition());
//    //    const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
//    //    const float distanceToOrigin = glm::distance(origin, lightPosition);
//    output.emplace_back(origin, rayDirection, distanceToOrigin);
//}

float PointLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    const glm::vec3 lightPosition = glm::vec3(GetPosition());
    const float distanceToOrigin = glm::distance(origin, lightPosition);
    return 1.f / (distanceToOrigin * distanceToOrigin * 2);
//    return 1.f / (distanceToOrigin * 1.5);
//    return 1.f / (distanceToOrigin * static_cast<float>(samplesToUse));
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
