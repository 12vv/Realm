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

#define VISUALIZE_PHOTON_MAPPING 1

PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler), 
    diffusePhotonNumber(10000000),
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
    
    if(remainingBounces < 0 || !storedScene->Trace(photonRay, &state)){
        return;
    }
    const glm::vec3 intersectionPoint = state.intersectionRay.GetRayPosition(state.intersectionT);
    //    std::cout << glm::to_string(intersectionPoint) << std::endl;
    
    const MeshObject* hitMeshObject = state.intersectedPrimitive->GetParentMeshObject();
    const Material* hitMaterial = hitMeshObject->GetMaterial();
    
    if(path.size() > 1 && hitMaterial->HasDiffuseReflection()){
        // create photon
        Photon tmpPhoton;

        glm::vec3 tmpRayDir = -photonRay->GetRayDirection();
        Ray tmpRay(intersectionPoint, tmpRayDir);

        // set photon properties
        tmpPhoton.intensity = lightIntensity;
        tmpPhoton.position = intersectionPoint;
        tmpPhoton.toLightRay = tmpRay;
        photonMap.insert(tmpPhoton);
    }

    
    glm::vec3 diffuseColor = hitMaterial->GetBaseDiffuseReflection();

    const float maxPr = glm::max(diffuseColor.x, glm::max(diffuseColor.y, diffuseColor.z));
    //    float tmpPr = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1)));
    const float thresh = rand() / (RAND_MAX + 1.);
    if(thresh < maxPr){
        // scatter the photon
        float u1 = rand() / (RAND_MAX + 1.);
        float u2 = rand() / (RAND_MAX + 1.);
        
        float r = std::sqrt(u1);
        float theta = 2.f * PI * u2;
        
        float x = r * std::cos(theta);
        float y = r * std::sin(theta);
        float z = std::sqrt(1.f - u1);
        
        glm::vec3 diffuseReflectionDir = glm::normalize(glm::vec3(x, y, z));
        //        diffuseReflectionDir = glm::normalize(glm::vec3(x, y, z));
        //        std::cout << glm::to_string(diffuseReflectionDir) << std::endl;
        
        // transform
        const glm::vec3 n = state.ComputeNormal();
        glm::vec3 t;
        glm::vec3 b;
        
        if (std::fabs(glm::dot(n, glm::vec3(1.f, 0.f, 0.f))) < 0.8f){
            t = glm::cross(n, glm::vec3(1.f, 0.f, 0.f));
            b = glm::cross(n, t);
        }
        else if (std::fabs(glm::dot(n, glm::vec3(0.f, 1.f, 0.f))) < 0.8f){
            t = glm::cross(n, glm::vec3(0.f, 1.f, 0.f));
            b = glm::cross(n, t);
        }
        else {
            t = glm::cross(n, glm::vec3(0.f, 0.f, 1.f));
            b = glm::cross(n, t);
        }
        
        // normalize
        t = glm::normalize(t);
        b = glm::normalize(b);
        
        
        // create transform matrix
        const glm::mat3 T = glm::mat3(t, b, n);
        diffuseReflectionDir = T*diffuseReflectionDir;
        
//        Ray diffuseReflectionRay(intersectionPoint + SMALL_EPSILON * diffuseReflectionDir, diffuseReflectionDir);
        Ray diffuseReflectionRay(intersectionPoint, diffuseReflectionDir);
        
//        remainingBounces--;
        path.emplace_back('L');
        
        //    TracePhoton(photonMap, photonRay, lightIntensity, path, currentIOR, remainingBounces);
        TracePhoton(photonMap, &diffuseReflectionRay, lightIntensity, path, currentIOR, remainingBounces-1);
    }
}

glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay) const
{
    glm::vec3 finalRenderColor = BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay);
    
#if VISUALIZE_PHOTON_MAPPING
    Photon intersectionVirtualPhoton;
    glm::vec3 indirectLighting;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
    
    const MeshObject* parentObject = intersection.intersectedPrimitive->GetParentMeshObject();
    const Material* objectMaterial = parentObject->GetMaterial();
    
    std::vector<Photon> foundPhotons;
    float r = 0.003f;
    diffuseMap.find_within_range(intersectionVirtualPhoton, r, std::back_inserter(foundPhotons));
    
    if (!foundPhotons.empty()) {
        finalRenderColor += glm::vec3(1.f, 0.f, 0.f);
    }
#else
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);
    
    const MeshObject* intersectionMeshObject = intersection.intersectedPrimitive->GetParentMeshObject();
    const Material* intersectionMaterial = intersectionMeshObject->GetMaterial();
    
    // find photons that are near the intersection (within constant radius)
    std::vector<Photon> foundPhotons;
    float r = 0.005; // minimum r.
    diffuseMap.find_within_range(intersectionVirtualPhoton, r, std::back_inserter(foundPhotons));
    
    // try a different search approach (more similar to KNN)
    int k = 1000;
    float growStep = 0.0f;
    int p = 0;
    while(int(foundPhotons.size()) < k){
        // grow radius more if few photons are found. just a runtime optimization.
        p = (k + int(foundPhotons.size())) / 2;
        if (int(foundPhotons.size()) == 0){
            growStep = 0.005;
        }
        else{
            growStep = std::sqrt(float(p) / float(foundPhotons.size())) * r - r;
        }
        growStep = std::sqrt(float(p) / float(foundPhotons.size())) * r - r;
        r = r + std::min(std::max(growStep, 0.005f), 0.05f); // grow by grow step, but at least 0.005, never more than 0.05
        foundPhotons.clear();
        diffuseMap.find_within_range(intersectionVirtualPhoton, r, std::back_inserter(foundPhotons));
    }
    
    // calculate the contribution of each near photon to the pixel. Compute the BRDF coming from that photon
    if (!foundPhotons.empty()) {
        for (uint p = 0; p < std::min(int(foundPhotons.size()), k); ++p) {
            const glm::vec3 brdfColor = intersectionMaterial->ComputeBRDF(intersection,
                                                                          foundPhotons[p].intensity,
                                                                          foundPhotons[p].toLightRay,
                                                                          fromCameraRay,
                                                                          1.f); // light attenuation (1 = no attenuation)
            // calculate weights based on distance
            float dist = glm::length(foundPhotons[p].position-intersectionVirtualPhoton.position);
            float weight = std::max((r - dist) / r, 0.f);
            float photonDensityAdjust = std::max(float(foundPhotons.size()) / float(k), 1.f);
            
            finalRenderColor += brdfColor / (r*r) * 60.0f * weight * photonDensityAdjust;
        }
    }
#endif
    
    return finalRenderColor;
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}
