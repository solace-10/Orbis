#include <numeric>

#include <core/interpolation.hpp>
#include <core/log.hpp>
#include <pandora.hpp>
#include <render/debug_render.hpp>
#include <render/window.hpp>
#include <scene/components/camera_component.hpp>
#include <scene/components/orbit_camera_component.hpp>

#include "components/scenic_camera_component.hpp"
#include <scene/components/rigid_body_component.hpp>
#include <scene/components/transform_component.hpp>
#include <scene/entity.hpp>

#include "systems/camera_system.hpp"

namespace WingsOfSteel
{

CameraSystem::CameraSystem()
{
}

CameraSystem::~CameraSystem()
{
    InputSystem* pInputSystem = GetInputSystem();
    if (pInputSystem)
    {
        pInputSystem->RemoveMouseButtonCallback(m_RightMouseButtonPressedToken);
        pInputSystem->RemoveMouseButtonCallback(m_RightMouseButtonReleasedToken);
        pInputSystem->RemoveMousePositionCallback(m_MousePositionToken);
    }
}

void CameraSystem::Initialize(Scene* pScene)
{
    using namespace WingsOfSteel;
    m_RightMouseButtonPressedToken = GetInputSystem()->AddMouseButtonCallback([this]() { m_IsDragging = true; }, MouseButton::Right, MouseAction::Pressed);
    m_RightMouseButtonReleasedToken = GetInputSystem()->AddMouseButtonCallback([this]() { m_IsDragging = false; }, MouseButton::Right, MouseAction::Released);

    m_MousePositionToken = GetInputSystem()->AddMousePositionCallback([this](const glm::vec2& mousePosition, const glm::vec2& mouseDelta) {
        m_InputPending = true;
        m_MouseDelta = mouseDelta;
    });
}

void CameraSystem::Update(float delta)
{
    using namespace WingsOfSteel;
    EntitySharedPtr pCamera = GetActiveScene() ? GetActiveScene()->GetCamera() : nullptr;
    if (pCamera == nullptr)
    {
        return;
    }

    if (pCamera->HasComponent<CameraComponent>() && pCamera->HasComponent<OrbitCameraComponent>())
    {
        OrbitCameraComponent& orbitCameraComponent = pCamera->GetComponent<OrbitCameraComponent>();
        if (m_IsDragging && m_InputPending)
        {
            const float sensitivity = 0.15f;
            orbitCameraComponent.orbitAngle -= glm::radians(m_MouseDelta.x * sensitivity);
            orbitCameraComponent.pitch += glm::radians(m_MouseDelta.y * sensitivity);

            if (orbitCameraComponent.pitch < orbitCameraComponent.minimumPitch)
            {
                orbitCameraComponent.pitch = orbitCameraComponent.minimumPitch;
            }
            else if (orbitCameraComponent.pitch > orbitCameraComponent.maximumPitch)
            {
                orbitCameraComponent.pitch = orbitCameraComponent.maximumPitch;
            }
            m_InputPending = false;
        }

        glm::vec3 position(
            glm::cos(orbitCameraComponent.orbitAngle) * glm::cos(orbitCameraComponent.pitch),
            glm::sin(orbitCameraComponent.pitch),
            glm::sin(orbitCameraComponent.orbitAngle) * glm::cos(orbitCameraComponent.pitch));

        CameraComponent& cameraComponent = pCamera->GetComponent<CameraComponent>();
        cameraComponent.camera.LookAt(orbitCameraComponent.anchorPosition + position * orbitCameraComponent.distance, orbitCameraComponent.anchorPosition, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (pCamera->HasComponent<CameraComponent>() && pCamera->HasComponent<ScenicCameraComponent>())
    {
        ScenicCameraComponent& scenicCamera = pCamera->GetComponent<ScenicCameraComponent>();

        // Advance orbit phase and calculate orbit angle (smooth bounce until we fix the dome's discontinuity)
        scenicCamera.orbitAngle += scenicCamera.orbitSpeed * delta;
        float actualOrbitAngle = glm::sin(scenicCamera.orbitAngle) * glm::half_pi<float>() * 0.9f + glm::pi<float>();

        // Advance zoom phase and calculate current distance
        scenicCamera.zoomPhase += scenicCamera.zoomSpeed * delta;
        float currentDistance = scenicCamera.baseDistance + glm::sin(scenicCamera.zoomPhase) * scenicCamera.zoomAmplitude;

        // Advance pitch phase and calculate current pitch
        scenicCamera.pitchPhase += scenicCamera.pitchSpeed * delta;
        float currentPitch = scenicCamera.basePitch + glm::sin(scenicCamera.pitchPhase) * scenicCamera.pitchAmplitude;

        // Calculate camera position using spherical coordinates
        glm::vec3 position(
            glm::cos(actualOrbitAngle) * glm::cos(currentPitch),
            glm::sin(currentPitch),
            glm::sin(actualOrbitAngle) * glm::cos(currentPitch));

        CameraComponent& cameraComponent = pCamera->GetComponent<CameraComponent>();
        cameraComponent.camera.LookAt(scenicCamera.anchorPosition + position * currentDistance, scenicCamera.anchorPosition, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}


glm::vec3 CameraSystem::MouseToWorld(const glm::vec2& mousePos) const
{
    using namespace WingsOfSteel;
    EntitySharedPtr pCamera = GetActiveScene() ? GetActiveScene()->GetCamera() : nullptr;
    if (pCamera == nullptr || !pCamera->HasComponent<CameraComponent>())
    {
        return glm::vec3(0.0f);
    }

    const CameraComponent& cameraComponent = pCamera->GetComponent<CameraComponent>();
    const Camera& camera = cameraComponent.camera;

    // Get camera position and create ray direction
    const glm::vec3 cameraPos = camera.GetPosition();

    // Convert mouse to world coordinates at two different depths to create a ray
    const glm::vec3 nearPoint = camera.ScreenToWorld(mousePos, GetWindow()->GetWidth(), GetWindow()->GetHeight(), 0.0f);
    const glm::vec3 farPoint = camera.ScreenToWorld(mousePos, GetWindow()->GetWidth(), GetWindow()->GetHeight(), 1.0f);

    // Calculate ray direction
    const glm::vec3 rayDir = glm::normalize(farPoint - nearPoint);

    // Define the XZ plane (Y = 0)
    const glm::vec3 planeNormal(0.0f, 1.0f, 0.0f); // Up vector
    const glm::vec3 planePoint(0.0f, 0.0f, 0.0f); // Origin point on the plane

    // Perform ray-plane intersection
    const float denom = glm::dot(rayDir, planeNormal);

    // Check if ray is parallel to plane
    if (std::abs(denom) < std::numeric_limits<float>::epsilon())
    {
        return glm::vec3(0.0f); // No intersection
    }

    // Calculate intersection parameter
    const float t = glm::dot(planePoint - nearPoint, planeNormal) / denom;

    // Check if intersection is behind the camera
    if (t < 0.0f)
    {
        return glm::vec3(0.0f); // Intersection behind camera
    }

    // Calculate intersection point
    return nearPoint + rayDir * t;
}

} // namespace WingsOfSteel
