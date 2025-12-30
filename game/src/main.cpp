#include "game.hpp"
#include "pandora.hpp"

int main()
{
    using namespace WingsOfSteel;

    WindowSettings windowSettings;
    windowSettings.SetSize(1440, 900);
    windowSettings.SetTitle("Physically Based Rendering");

    static Game game;  // Static storage ensures Game survives async WebGPU initialization
    Initialize(
        windowSettings,
        []() { game.Initialize(); },
        [](float delta) { game.Update(delta); },
        []() { game.Shutdown(); });
}
