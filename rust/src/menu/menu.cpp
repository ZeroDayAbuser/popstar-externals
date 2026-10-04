#include <pawjob_umbrella.hpp>
#include <settings/settings.hpp>
#include <menu/menu.hpp>

void Menu::Init()
{
    Style::Update();

    Vector2 basePosition = {120.0f, 120.0f};
    Vector2 baseSize     = {650.0f, 550.0f};

    window = std::make_unique<CWindow>(HACK_NAME, basePosition, baseSize, HACK_TLD);

    if (auto Aimbot   = window->AddObject<CTab>(X("Aimbot")))   InitAimbotTab(Aimbot);
    if (auto Visuals  = window->AddObject<CTab>(X("Visuals")))  InitVisualsTab(Visuals);
    if (auto Misc     = window->AddObject<CTab>(X("Misc")))     InitMiscTab(Misc);
    if (auto Settings = window->AddObject<CTab>(X("Settings"))) InitSettingsTab(Settings);

    settings_io::Init();
    settings_io::LoadDefault();
}

void Menu::Tick()
{
    if (!window)
        return;

    Render::StartDrawing(ImGui::GetBackgroundDrawList());

    Menu::isOpen = window->GetOpened();
    window->Render();

    Render::EndDrawing();
}

void Menu::Shutdown()
{
    window.reset();
}
