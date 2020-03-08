#include "common/Rendering/Renderer/Photon/PhotonMappingRenderer.h"
#include "common/Scene/Scene.h"
#include "common/Sampling/ColorSampler.h"
#include "common/Scene/Lights/Light.h"
#include "common/Scene/Geometry/Primitives/Primitive.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "common/Intersection/IntersectionState.h"
#include "common/Scene/SceneObject.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "glm/gtx/component_wise.hpp"

#define VISUALIZE_PHOTON_MAPPING 0

PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(1000000),
    maxPhotonBounces(1000)
{
    srand(static_cast<unsigned int>(time(NULL)));
}

void PhotonMappingRenderer::InitializeRenderer()
{
    // Generate Photon Maps
    GenericPhotonMapGeneration(diffuseMap, diffusePhotonNumber);
    diffuseMap.optimise();
}

void PhotonMappingRenderer::GenericPhotonMapGeneration(PhotonKdtree& photonMap, int totalPhotons)
{
    float totalLightIntensity = 0.f;
    size_t totalLights = storedScene->GetTotalLights();
    for (size_t i = 0; i < totalLights; ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }
        totalLightIntensity += glm::length(currentLight->GetLightColor());
    }

    // Shoot photons -- number of photons for light is proportional to the light's intensity relative to the total light intensity of the scene.
    for (size_t i = 0; i < totalLights; ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }

        const float proportion = glm::length(currentLight->GetLightColor()) / totalLightIntensity;
        const int totalPhotonsForLight = static_cast<const int>(proportion * totalPhotons);
        const glm::vec3 photonIntensity = currentLight->GetLightColor() / static_cast<float>(totalPhotonsForLight);
        for (int j = 0; j < totalPhotonsForLight; ++j) {
            Ray photonRay;
            std::vector<char> path;
            path.push_back('L');
            currentLight->GenerateRandomPhotonRay(photonRay);
            TracePhoton(photonMap, &photonRay, photonIntensity, path, 1.f, maxPhotonBounces);
        }
    }
}

void PhotonMappingRenderer::TracePhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, std::vector<char>& path, float currentIOR, int remainingBounces)
{
    assert(photonRay);
    IntersectionState state(0, 0);
    state.currentIOR = currentIOR;
    // coding
    Ray diffuseReflectionRay;
    if(remainingBounces < 0 || !storedScene->Trace(photonRay, &state)){
        return;
    }
    const glm::vec3 intersectionPoint = state.intersectionRay.GetRayPosition(state.intersectionT);
    
    const MeshObject* hitMeshObject = state.intersectedPrimitive->GetParentMeshObject();
    const Material* hitMaterial = hitMeshObject->GetMaterial();
    
    if(path.size() > 1 && hitMaterial->HasDiffuseReflection()){
        Photon tmpPhoton;
        Ray tmpRay;
        tmpRay.SetRayDirection(-photonRay->GetRayDirection());
        tmpRay.SetRayPosition(intersectionPoint);
        
        tmpPhoton.intensity = lightIntensity;
        tmpPhoton.position = intersectionPoint;
        tmpPhoton.toLightRay = tmpRay;
        photonMap.insert(tmpPhoton);
    }
    
    glm::vec3 diffuseColor = hitMaterial->GetBaseDiffuseReflection();
    //    std::cout << glm::to_string(hitMaterial->GetBaseDiffuseReflection()) << std::endl;
    float maxColor = diffuseColor.x > diffuseColor.y ? (diffuseColor.x > diffuseColor.z ? diffuseColor.x : diffuseColor.z) : (diffuseColor.y > diffuseColor.z ? diffuseColor.y : diffuseColor.z);
    float tmpPr = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1)));
    if(tmpPr < maxColor){
        // scatter the photon
        
        float u1 = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1)));
        float u2 = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1)));
        
        //        std::cout << u1 << std::endl;
        
        float r = sqrt(u1);
        float theta = 2 * M_PI * u2;
        
        float x = r * cos(theta);
        float y = r * sin(theta);
        float z = sqrt(1 - u1);
        
        glm::vec3 diffuseReflectionDir = glm::normalize(glm::vec3(x, y, z));
        
        
        // transform
        glm::vec3 normal = state.ComputeNormal();
        glm::vec3 t = glm::cross(normal, glm::vec3(1, 0, 0));
        glm::vec3 b = glm::cross(normal, t);
        
        glm::mat3 trans(normal, t, b);
        diffuseReflectionDir = diffuseReflectionDir * glm::transpose(trans);
        
        //        Ray tmpRay(intersectionPoint, diffuseReflectionDir);
        diffuseReflectionRay.SetRayPosition(intersectionPoint);
        diffuseReflectionRay.SetRayDirection(diffuseReflectionDir);
        
        //        photonRay = &tmpRay;
        //        photonRay->SetRayDirection(diffuseReflectionDir);
        //        photonRay->SetRayPosition(intersectionPoint);
        
    }
    
    remainingBounces--;
    path.emplace_back('L');
    
    //    TracePhoton(photonMap, photonRay, lightIntensity, path, currentIOR, remainingBounces);
    TracePhoton(photonMap, &diffuseReflectionRay, lightIntensity, path, currentIOR, remainingBounces);
}

glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
    glm::vec3 finalRenderColor = BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay);
#if VISUALIZE_PHOTON_MAPPING
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);

    std::vector<Photon> foundPhotons;
    diffuseMap.find_within_range(intersectionVirtualPhoton, 0.003f, std::back_inserter(foundPhotons));
    if (!foundPhotons.empty()) {
//        finalRenderColor += glm::vec3(1.f, 0.f, 0.f);
        for(auto i: foundPhotons){
            finalRenderColor += BackwardRenderer::ComputeSampleColor(intersection, i.toLightRay);
        }
        //        finalRenderColor += BackwardRenderer::ComputeSampleColor(intersection, intersectionVirtualPhoton.toLightRay);
        finalRenderColor /= foundPhotons.size();
    }
#endif
    return finalRenderColor;
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}
