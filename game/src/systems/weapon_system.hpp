#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <scene/components/transform_component.hpp>
#include <scene/systems/system.hpp>
#include <scene/entity.hpp>

#include "components/weapon_component.hpp"

namespace WingsOfSteel
{

class WeaponSystem : public System
{
public:
    WeaponSystem() = default;
    ~WeaponSystem();

    void Initialize(Scene* pScene) override;
    void Update(float delta) override;

    void AttachWeapon(const std::string& resourcePath, EntitySharedPtr pParentEntity, const std::string& hardpointName, bool automatedTargeting, WeaponFriendOrFoe fof);

private:
    void OnHardpointsDestroyed(entt::registry& registry, entt::entity entity);
    void FireWeapon(EntitySharedPtr pWeaponEntity, WeaponComponent& weaponComponent);
    void UpdateTransform(const glm::mat4& hardpointWorldTransform, const WeaponComponent& weaponComponent, TransformComponent& transformComponent);
};

} // namespace WingsOfSteel
