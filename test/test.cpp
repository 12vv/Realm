#include "test/test.h"
#include "common/core.h"

// 0 -- Naive.
// 1 -- BVH.
// 2 -- Grid.
#define ACCELERATION_TYPE 1

std::shared_ptr<Camera> TestApp::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    camera->SetPosition(glm::vec3(0.f, -4.1469f, 0.73693f));
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    return camera;
}

std::shared_ptr<Scene> TestApp::CreateScene() const
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();
    
    // Material
    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
    cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
    cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
    
    // Objects
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox/CornellBox-Mirror.obj", &loadedMaterials);
    for (size_t i = 0; i < cubeObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        cubeObjects[i]->SetMaterial(materialCopy);
    }
    
    std::shared_ptr<SceneObject> sceneObject = std::make_shared<SceneObject>();
    sceneObject->AddMeshObject(cubeObjects);
    
    //    // add
    //    sceneObject->AddMeshObject(cubeObjects1);
    //    ///
    
    
    sceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    sceneObject->CreateAccelerationData(AccelerationTypes::BVH);
    sceneObject->ConfigureAccelerationStructure([](AccelerationStructure* genericAccelerator) {
        BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
        accelerator->SetMaximumChildren(2);
        accelerator->SetNodesOnLeaves(2);
    });
    
    sceneObject->ConfigureChildMeshAccelerationStructure([](AccelerationStructure* genericAccelerator) {
        BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
        accelerator->SetMaximumChildren(2);
        accelerator->SetNodesOnLeaves(2);
    });
    newScene->AddSceneObject(sceneObject);
    
    
    
#if ACCELERATION_TYPE == 0
    newScene->GenerateAccelerationData(AccelerationTypes::NONE);
#elif ACCELERATION_TYPE == 1
    newScene->GenerateAccelerationData(AccelerationTypes::BVH);
#else
    UniformGridAcceleration* accelerator = dynamic_cast<UniformGridAcceleration*>(newScene->GenerateAccelerationData(AccelerationTypes::UNIFORM_GRID));
    assert(accelerator);
    // Assignment 7 Part 2 TODO: Change the glm::ivec3(10, 10, 10) here.
    accelerator->SetSuggestedGridSize(glm::ivec3(10, 10, 10));
#endif
    
    
    // Lights
    std::shared_ptr<AreaLight> areaLight = std::make_shared<AreaLight>(glm::vec2(2.f, 2.f));
    areaLight->SetSamplerAttributes(glm::vec3(2.f, 2.f, 1.f), 4);
    areaLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
    areaLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
    newScene->AddLight(areaLight);
    
    return newScene;
    
}
std::shared_ptr<ColorSampler> TestApp::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    // ASSIGNMENT 5 TODO: Change the grid size to be glm::ivec3(X, Y, 1).
    jitter->SetGridSize(glm::ivec3(4, 4, 1));
    
    std::shared_ptr<SimpleAdaptiveSampler> sampler = std::make_shared<SimpleAdaptiveSampler>();
    sampler->SetInternalSampler(jitter);
    
    sampler->SetEarlyExitParameters(1.f * SMALL_EPSILON, 16);
    
    //    return jitter;
    return sampler;
}

std::shared_ptr<class Renderer> TestApp::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
    return std::make_shared<BackwardRenderer>(scene, sampler);
}

int TestApp::GetSamplesPerPixel() const
{
    return 1;
}

bool TestApp::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}

int TestApp::GetMaxReflectionBounces() const
{
    return 2;
}

int TestApp::GetMaxRefractionBounces() const
{
    return 4;
}

glm::vec2 TestApp::GetImageOutputResolution() const
{
    return glm::vec2(640.f, 480.f);
}

