#pragma once

#include <glm/vec3.hpp>

#include <core/signal.hpp>
#include <core/smart_ptr.hpp>
#include <scene/scene.hpp>

namespace WingsOfSteel
{

DECLARE_SMART_PTR(Sector);
class Sector : public Scene
{
public:
    Sector();
    ~Sector();

    void Initialize() override;
    void Update(float delta) override;

    void ShowCameraDebugUI(bool state);
    void ShowGrid(bool state);

    EntitySharedPtr GetPlayerCarrier() const;

private:
    void SpawnDome();
    void SpawnLight();
    void SpawnPlayerFleet();

    EntitySharedPtr m_pDome;
    EntitySharedPtr m_pCamera;
    EntitySharedPtr m_pLight;
    EntityWeakPtr m_pCarrier;
    bool m_ShowCameraDebugUI{ false };
    bool m_ShowGrid{ false };
};

inline EntitySharedPtr Sector::GetPlayerCarrier() const
{
    return m_pCarrier.lock();
}

} // namespace WingsOfSteel
