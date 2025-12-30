#pragma once

#include <glm/vec3.hpp>

#include <scene/components/icomponent.hpp>
#include <scene/components/component_factory.hpp>

namespace WingsOfSteel
{

class ScenicCameraComponent : public IComponent
{
public:
    ScenicCameraComponent() = default;
    ~ScenicCameraComponent() = default;

    void Deserialize(const ResourceDataStore* pContext, const Json::Data& json) override {}

    // Base camera parameters
    glm::vec3 anchorPosition{ -10.0f };   // What the camera looks at (origin)
    float baseDistance{ 90.0f };        // Base distance from anchor
    float basePitch{ 0.3f };             // Base vertical angle (radians)

    // Orbit animation
    float orbitAngle{ 0.0f };            // Current horizontal angle (radians)
    float orbitSpeed{ 0.05f };           // Radians per second

    // Zoom oscillation
    float zoomAmplitude{ 20.0f };        // +/- distance from baseDistance
    float zoomSpeed{ 0.1f };             // Oscillation speed (radians per second)
    float zoomPhase{ 0.0f };             // Current phase (radians)

    // Pitch oscillation
    float pitchAmplitude{ 0.1f };        // +/- radians from basePitch
    float pitchSpeed{ 0.08f };           // Oscillation speed (radians per second)
    float pitchPhase{ 0.0f };            // Current phase (radians)
};

REGISTER_COMPONENT(ScenicCameraComponent, "scenic_camera")

} // namespace WingsOfSteel
