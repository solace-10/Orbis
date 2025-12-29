#include <glm/gtx/rotate_vector.hpp>

#include "systems/weapon_system.hpp"

#include <pandora.hpp>
#include <render/debug_render.hpp>
#include <scene/components/ghost_component.hpp>
#include <scene/components/rigid_body_component.hpp>
#include <scene/components/transform_component.hpp>
#include <scene/scene.hpp>
#include <scene/systems/physics_simulation_system.hpp>

#include "components/hardpoint_component.hpp"
#include "components/weapon_component.hpp"
#include "entity_builder/entity_builder.hpp"
#include "game.hpp"
#include "sector/sector.hpp"
#include "systems/ammo_system.hpp"

namespace WingsOfSteel
{

WeaponSystem::~WeaponSystem()
{
    Scene* pScene = Game::Get()->GetSector();
    if (pScene)
    {
        pScene->GetRegistry().on_destroy<HardpointComponent>().disconnect<&WeaponSystem::OnHardpointsDestroyed>(this);
    }
}

void WeaponSystem::Initialize(Scene* pScene)
{
    pScene->GetRegistry().on_destroy<HardpointComponent>().connect<&WeaponSystem::OnHardpointsDestroyed>(this);
}

void WeaponSystem::Update(float delta)
{
    entt::registry& registry = GetActiveScene()->GetRegistry();
    auto view = registry.view<WeaponComponent, TransformComponent>();
    view.each([delta, this](const auto entity, WeaponComponent& weaponComponent, TransformComponent& transformComponent) {
        glm::mat4 rootWorldTransform{ 1.0f };

        EntitySharedPtr pWeaponEntity = weaponComponent.GetOwner().lock();
        if (pWeaponEntity)
        {
            EntitySharedPtr pParentEntity = pWeaponEntity->GetParent().lock();
            if (pParentEntity)
            {
                rootWorldTransform = pParentEntity->GetComponent<TransformComponent>().transform;
            }
        }

        const glm::mat4 hardpointWorldTransform = rootWorldTransform * weaponComponent.m_AttachmentPointTransform;
        UpdateTransform(hardpointWorldTransform, weaponComponent, transformComponent);
    });
}

void WeaponSystem::AttachWeapon(const std::string& resourcePath, EntitySharedPtr pParentEntity, const std::string& hardpointName, bool automatedTargeting, WeaponFriendOrFoe friendOrFoe)
{
    SceneWeakPtr pWeakScene = Game::Get()->GetSector()->GetWeakPtr();

    EntityBuilder::Build(
        pWeakScene,
        resourcePath,
        glm::mat4(1.0f),
        [pWeakScene, pParentEntity, hardpointName, automatedTargeting, friendOrFoe](EntitySharedPtr pWeaponEntity) {
            pWeaponEntity->SetParent(pParentEntity);

            WeaponComponent& weaponComponent = pWeaponEntity->GetComponent<WeaponComponent>();
            weaponComponent.SetOwner(pWeaponEntity);
            weaponComponent.m_FriendOrFoe = friendOrFoe;

            bool hardpointFound = false;
            HardpointComponent& hardpointComponent = pParentEntity->GetComponent<HardpointComponent>();
            for (auto& hardpoint : hardpointComponent.hardpoints)
            {
                if (hardpoint.m_Name == hardpointName)
                {
                    hardpoint.m_pEntity = pWeaponEntity;
                    weaponComponent.m_AttachmentPointName = hardpoint.m_Name;
                    weaponComponent.m_AttachmentPointTransform = hardpoint.m_AttachmentPointTransform;
                    weaponComponent.m_ArcMinDegrees = hardpoint.m_ArcMinDegrees;
                    weaponComponent.m_ArcMaxDegrees = hardpoint.m_ArcMaxDegrees;
                    weaponComponent.m_AngleDegrees = (hardpoint.m_ArcMinDegrees + hardpoint.m_ArcMaxDegrees) / 2.0f;
                    weaponComponent.m_AutomatedTargeting = automatedTargeting;
                    hardpointFound = true;
                    break;
                }
            }

            if (!hardpointFound)
            {
                Log::Error() << "Failed to attach weapon to hardpoint.";
            }
        });
}

void WeaponSystem::OnHardpointsDestroyed(entt::registry& registry, entt::entity entity)
{
    Sector* pSector = Game::Get()->GetSector();
    if (!pSector)
    {
        return;
    }

    HardpointComponent& hardpointComponent = registry.get<HardpointComponent>(entity);
    for (auto& hardpoint : hardpointComponent.hardpoints)
    {
        if (hardpoint.m_pEntity)
        {
            pSector->RemoveEntity(hardpoint.m_pEntity);
            hardpoint.m_pEntity.reset();
        }
    }
}

void WeaponSystem::FireWeapon(EntitySharedPtr pWeaponEntity, WeaponComponent& weaponComponent)
{
    if (weaponComponent.m_Ammo.empty())
    {
        return;
    }

    weaponComponent.m_FireTimer = 1.0f / weaponComponent.m_RateOfFire;

    AmmoSystem* pAmmoSystem = GetActiveScene()->GetSystem<AmmoSystem>();
    if (pAmmoSystem)
    {
        pAmmoSystem->Instantiate(pWeaponEntity, weaponComponent);
    }
}

void WeaponSystem::UpdateTransform(const glm::mat4& hardpointWorldTransform, const WeaponComponent& weaponComponent, TransformComponent& transformComponent)
{
    transformComponent.transform = glm::rotate(hardpointWorldTransform, glm::radians(weaponComponent.m_AngleDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
}

} // namespace WingsOfSteel
