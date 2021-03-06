#pragma once

#include "common/Scene/Lights/Light.h"

class DirectionalLight : public Light
{
public:
    DirectionalLight(const glm::vec3& direction);
    
    virtual void ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const override;
    virtual float ComputeLightAttenuation(glm::vec3 origin) const override;

    virtual void GenerateRandomPhotonRay(Ray& ray) const override;
    
private:
    glm::vec3 direction;
};
