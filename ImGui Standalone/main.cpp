#include "UI.h"
#include <stdio.h>
#include <iostream>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{

    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, ("CONIN$"), ("r"), stdin);
    freopen_s(&fDummy, ("CONOUT$"), ("w"), stderr);
    freopen_s(&fDummy, ("CONOUT$"), ("w"), stdout);

    SetConsoleTitleA("PE Entropy");
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    std::cout << "[?] PE Entropy - Entropy Graph Tool By github.com/VeryCuteLookingCat" << std::endl;
    std::cout << "[<3] Thank you for using my tool" << std::endl;

    UI::Render();

    return 0;
}
