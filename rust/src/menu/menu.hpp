#pragma once


#include <pawjob_umbrella.hpp>
#include <settings/settings.hpp>

#define HACK_NAME X("Un-Named")
#define HACK_TLD  X("")

class CTab;
class CListbox;

namespace Menu
{
    void Init();
    void Tick();
    void Shutdown();

    void InitAimbotTab(CTab* tab);
    void InitVisualsTab(CTab* tab);
    void InitMiscTab(CTab* tab);
    void InitSettingsTab(CTab* tab);

    inline bool isOpen = false;
    inline std::unique_ptr<CWindow> window;

    inline int          currentConfigIndex = 0;
    inline CListbox*    configsListbox     = nullptr;
}
