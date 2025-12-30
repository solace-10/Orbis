#include <imgui.h>

#include <core/log.hpp>
#include <pandora.hpp>
#include <physics/collision_shape.hpp>
#include <render/debug_render.hpp>
#include <resources/resource_data_store.hpp>
#include <resources/resource_system.hpp>
#include <scene/components/ambient_light_component.hpp>
#include <scene/components/camera_component.hpp>
#include <scene/components/debug_render_component.hpp>
#include <scene/components/directional_light_component.hpp>
#include <scene/components/model_component.hpp>
#include <scene/components/rigid_body_component.hpp>
#include <scene/components/transform_component.hpp>
#include <scene/systems/model_render_system.hpp>
#include <scene/systems/physics_simulation_system.hpp>

#include "components/scenic_camera_component.hpp"
#include "entity_builder/entity_builder.hpp"
#include "sector/sector.hpp"
#include "systems/ammo_system.hpp"
#include "systems/camera_system.hpp"
#include "systems/debug_render_system.hpp"
#include "systems/shield_system.hpp"
#include "systems/ship_navigation_system.hpp"
#include "systems/weapon_system.hpp"

namespace WingsOfSteel
{

Sector::Sector()
{
}

Sector::~Sector()
{
}

void Sector::Initialize()
{
    Scene::Initialize();

    AddSystem<ModelRenderSystem>();
    AddSystem<PhysicsSimulationSystem>();
    AddSystem<ShipNavigationSystem>();
    AddSystem<WeaponSystem>();
    AddSystem<AmmoSystem>();
    AddSystem<ShieldSystem>();

    // Make sure these systems are added after everything else that might modify transforms,
    // otherwise the camera and debug rendering will be offset by a frame.
    AddSystem<CameraSystem>();
    AddSystem<DebugRenderSystem>();

    m_pCamera = CreateEntity();
    m_pCamera->AddComponent<CameraComponent>(70.0f, 1.0f, 5000.0f);

    ScenicCameraComponent& scenicCamera = m_pCamera->AddComponent<ScenicCameraComponent>();
    scenicCamera.anchorPosition = glm::vec3(0.0f, -10.0f, 0.0f);
    scenicCamera.baseDistance = 90.0f;
    scenicCamera.basePitch = 0.4f;
    scenicCamera.orbitSpeed = 0.03f;
    scenicCamera.zoomAmplitude = 10.0f;
    SetCamera(m_pCamera);

    SpawnDome();
    SpawnLight();
    SpawnPlayerFleet();
}

void Sector::Update(float delta)
{
    Scene::Update(delta);

    if (m_ShowGrid)
    {
        GetDebugRender()->XZSquareGrid(-1000.0f, 1000.0f, -1.0f, 100.0f, Color::White);
    }
}

void Sector::ShowCameraDebugUI(bool state)
{
    m_ShowCameraDebugUI = state;
}

void Sector::ShowGrid(bool state)
{
    m_ShowGrid = state;
}

void Sector::SpawnDome()
{
    m_pDome = CreateEntity();
    m_pDome->AddComponent<TransformComponent>();

    ModelComponent& modelComponent = m_pDome->AddComponent<ModelComponent>();
    modelComponent.SetModel("/models/dome/dome.glb");
}

void Sector::SpawnLight()
{
    m_pLight = CreateEntity();

    DirectionalLightComponent& directionalLightComponent = m_pLight->AddComponent<DirectionalLightComponent>();
    directionalLightComponent.SetAngle(226.0f);
    directionalLightComponent.SetPitch(40.0f);
    directionalLightComponent.SetColor(1.0f, 0.96f, 0.68f);

    AmbientLightComponent& ambientLightComponent = m_pLight->AddComponent<AmbientLightComponent>();
    ambientLightComponent.SetColor(0.10f, 0.14f, 0.17f);
}

void Sector::SpawnPlayerFleet()
{
    SceneWeakPtr pWeakScene = weak_from_this();

    EntityBuilder::Build(pWeakScene, "/entity_prefabs/player/carrier.json", glm::mat4(1.0f), [pWeakScene](EntitySharedPtr pEntity) {
        SectorSharedPtr pScene = std::dynamic_pointer_cast<Sector>(pWeakScene.lock());
        if (pScene)
        {
            pScene->m_pCarrier = pEntity;
        }
    });
}

} // namespace WingsOfSteel
