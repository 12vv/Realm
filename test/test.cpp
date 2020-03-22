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
    //    cubeMaterial->SetReflectivity(0.3f);
    
    // Objects
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> objects = MeshLoader::LoadMesh("CornellBox/CornellBox-Sphere.obj", &loadedMaterials);
    
    //    objects.erase(objects.begin() + 1);
    for (size_t i = 0; i < objects.size(); ++i)
    {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        materialCopy->SetAmbient(glm::vec3(0.0f, 0.0f, 0.0f));
        
        if (i == 0)
        {
            materialCopy->SetReflectivity(0.7f);
        }
        
        if (i == 1)
        {
            
            materialCopy->SetTransmittance(0.8f);
            materialCopy->SetIOR(1.5f);
        }
        
        objects[i]->SetMaterial(materialCopy);
    }
    
    std::shared_ptr<SceneObject> sceneObject = std::make_shared<SceneObject>();
    sceneObject->AddMeshObject(objects);
    sceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    sceneObject->CreateAccelerationData(AccelerationTypes::BVH);    
    
    newScene->AddSceneObject(sceneObject);
    
    /*
     std::vector<std::shared_ptr<aiMaterial>> glassMaterials;
     std::vector<std::shared_ptr<MeshObject>> glassObjects = MeshLoader::LoadMesh("Project/Glass2/glass.obj", &glassMaterials);
     
     for (size_t i = 0; i < glassObjects.size(); ++i)
     {
     std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
     materialCopy->LoadMaterialFromAssimp(glassMaterials[i]);
     materialCopy->SetAmbient(glm::vec3(0.0f, 0.0f, 0.0f));
     
     materialCopy->SetReflectivity(0.2f);
     materialCopy->SetTransmittance(0.7f);
     materialCopy->SetIOR(1.1f);
     
     glassObjects[i]->SetMaterial(materialCopy);
     }
     
     std::shared_ptr<SceneObject> glassObject = std::make_shared<SceneObject>();
     glassObject->AddMeshObject(objects);
     glassObject->MultScale(0.2f);
     glassObject->Translate(glm::vec3(-0.3, -0.5, 0.f));
     glassObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
     glassObject->CreateAccelerationData(AccelerationTypes::BVH);
     newScene->AddSceneObject(glassObject);
     */
    // Lights
    std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(glm::vec3(-0.005f, -0.01f, 1.5028f)); // glm::vec3(0.01909f, 0.0101f, 1.5328f)
    pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
//    newScene->AddLight(pointLight);
    
    std::shared_ptr<PointLight> pointLight2 = std::make_shared<PointLight>();
    pointLight2->SetPosition(glm::vec3(0.05909f, 0.0501f, 1.5628f));
    pointLight2->SetLightColor(glm::vec3(0.1f, 0.3f, 0.1f));
//        newScene->AddLight(pointLight2);

    std::shared_ptr<SphereLight> sphereLight = std::make_shared<SphereLight>(0.2f);
    sphereLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.5f));
    sphereLight->SetLightColor(glm::vec3(0.98f, 1.f, 0.94f));
    newScene->AddLight(sphereLight);
    
    std::shared_ptr<AreaLight> areaLight = std::make_shared<AreaLight>(glm::vec2(2.f, 2.f));
    areaLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.45f));
    areaLight->SetLightColor(glm::vec3(0.98f, 1.f, 0.94f));
//    newScene->AddLight(areaLight);
    
    std::shared_ptr<Light> dirLight = std::make_shared<DirectionalLight>(glm::vec3(0.f, 1.f, 0.f));
    //    dirLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 0.97028f));
    dirLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
//    newScene->AddLight(dirLight);
    
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
//    return std::make_shared<BackwardRenderer>(scene, sampler);
    return std::make_shared<PhotonMappingRenderer>(scene, sampler);
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

