#include "test/app1.h"
#include "common/core.h"

std::shared_ptr<Camera> Assignment5::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    camera->SetPosition(glm::vec3(0.f, -4.1469f, 0.73693f));
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    return camera;
}

// 0 -- Naive.
// 1 -- BVH.
// 2 -- Grid.
#define ACCELERATION_TYPE 1

std::shared_ptr<Scene> Assignment5::CreateScene() const
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

    // Material
    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
    cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
    cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);

    // Objects
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Empty-Squashed.obj", &loadedMaterials);
//    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("box_nf.obj", &loadedMaterials);
    for (size_t i = 0; i < cubeObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        cubeObjects[i]->SetMaterial(materialCopy);
    }
    
    std::vector<std::shared_ptr<MeshObject>> floor = MeshLoader::LoadMesh("floor.obj", &loadedMaterials);
    for (size_t i = 0; i < floor.size(); ++i) {
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        floor[i]->SetMaterial(materialCopy);
    }
    
    
    // Material_glass
    std::shared_ptr<BlinnPhongMaterial> glass = std::make_shared<BlinnPhongMaterial>();
    glass->SetTransmittance(0.98f);
    glass->SetIOR(1.5f);

    // Objects_sphere
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials1;
    std::vector<std::shared_ptr<MeshObject>> glass_sphere = MeshLoader::LoadMesh("glass1.obj", &loadedMaterials1);
    for (size_t i = 0; i < glass_sphere.size(); ++i) {
//        std::shared_ptr<Material> materialCopy = glossy->Clone();
//        materialCopy->LoadMaterialFromAssimp(loadedMaterials1[i]);
        glass_sphere[i]->SetMaterial(glass);
    }
    
    // Material_mirror
    std::shared_ptr<BlinnPhongMaterial> mirror = std::make_shared<BlinnPhongMaterial>();
    mirror->SetReflectivity(0.98f);
//    mirror->SetSpecular(glm::vec3(1.f, 1.f, 1.f), 150.f);
    
    // Objects_sphere
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials2;
    std::vector<std::shared_ptr<MeshObject>> mirror_sphere = MeshLoader::LoadMesh("mirror.obj", &loadedMaterials2);
    for (size_t i = 0; i < mirror_sphere.size(); ++i) {
//        std::shared_ptr<Material> materialCopy = mirror->Clone();
//        materialCopy->LoadMaterialFromAssimp(loadedMaterials2[i]);
        mirror_sphere[i]->SetMaterial(mirror);
    }
    
    // Material_glossy
    std::shared_ptr<BlinnPhongMaterial> glossy = std::make_shared<BlinnPhongMaterial>();
    glossy->SetDiffuse(glm::vec3(0.2f, 0.2f, 0.2f));
    glossy->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 20.f);
    glossy->SetAmbient(glm::vec3(0.2, 0.2, 0.2));
    
//    auto data = TextureLoader::LoadTexture("checkerboard.png");
//    glossy->SetTexture("checkerboard", data);
    
//    glossy->SetReflectivity(0.1f);
    
    // Objects_sphere
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials3;
    std::vector<std::shared_ptr<MeshObject>> glossy_obj1 = MeshLoader::LoadMesh("toy.obj", &loadedMaterials3);
    for (size_t i = 0; i < glossy_obj1.size(); ++i) {
//        std::shared_ptr<Material> materialCopy = glossy->Clone();
//        materialCopy->LoadMaterialFromAssimp(loadedMaterials3[i]);
//        glossy_obj1[i]->SetMaterial(materialCopy);
        glossy_obj1[i]->SetMaterial(glossy);
    }
    
    
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials4;
    std::shared_ptr<BlinnPhongMaterial> test = std::make_shared<BlinnPhongMaterial>();
    test->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
    test->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
    
    
    //    std::vector<std::shared_ptr<MeshObject>> cubeObjects1 = MeshLoader::LoadMesh("all.obj", &loadedMaterials1);
//    std::vector<std::shared_ptr<MeshObject>> cube = MeshLoader::LoadMesh("glossy.obj", &loadedMaterials4);
//    for (size_t i = 0; i < cube.size(); ++i) {
//        std::shared_ptr<Material> materialCopy = test->Clone();
//        materialCopy->LoadMaterialFromAssimp(loadedMaterials4[i]);
//        cube[i]->SetMaterial(materialCopy);
//    }
    

    std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
    cubeSceneObject->AddMeshObject(cubeObjects);
    
    cubeSceneObject->AddMeshObject(glass_sphere);
    cubeSceneObject->AddMeshObject(mirror_sphere);
    cubeSceneObject->AddMeshObject(glossy_obj1);
//    cubeSceneObject->AddMeshObject(floor);
    
//    cubeSceneObject->AddMeshObject(cube);
    
    cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    cubeSceneObject->CreateAccelerationData(AccelerationTypes::UNIFORM_GRID);
    newScene->AddSceneObject(cubeSceneObject);

    // Lights
    std::shared_ptr<Light> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 0.97028f));
    pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));

    std::shared_ptr<Light> dirLight = std::make_shared<DirectionalLight>(glm::vec3(0.f, 1.f, 0.f));
    dirLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 0.97028f));
    dirLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
    
//    // add directional light
//    std::shared_ptr<Light> directionalLight = std::make_shared<DirectionalLight>();
//    directionalLight->SetPosition(glm::vec3(-5.01909f, -15.0101f, 1.97028f));
//    pointLight->SetLightColor(glm::vec3(0.f, 1.f, 1.f));
//    newScene->AddLight(directionalLight);

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
    newScene->AddLight(pointLight);
    newScene->AddLight(dirLight);
    
    return newScene;

}
std::shared_ptr<ColorSampler> Assignment5::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    // ASSIGNMENT 5 TODO: Change the grid size to be glm::ivec3(X, Y, 1).
    jitter->SetGridSize(glm::ivec3(8, 8, 1));

    std::shared_ptr<SimpleAdaptiveSampler> sampler = std::make_shared<SimpleAdaptiveSampler>();
    sampler->SetInternalSampler(jitter);

    // ASSIGNMENT 5 TODO: Change the '1.f' in '1.f * SMALL_EPSILON' here to be higher and see what your results are. (Part 3)
    sampler->SetEarlyExitParameters(100.f * SMALL_EPSILON, 16);

    // ASSIGNMENT 5 TODO: Comment out 'return jitter;' to use the adaptive sampler. (Part 2)
//    return jitter;
    return sampler;
}

std::shared_ptr<class Renderer> Assignment5::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
    return std::make_shared<BackwardRenderer>(scene, sampler);
}

int Assignment5::GetSamplesPerPixel() const
{
    return 2;
}

bool Assignment5::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}

int Assignment5::GetMaxReflectionBounces() const
{
    return 2;
}

int Assignment5::GetMaxRefractionBounces() const
{
    return 4;
}

glm::vec2 Assignment5::GetImageOutputResolution() const
{
    return glm::vec2(640.f, 480.f);
}
