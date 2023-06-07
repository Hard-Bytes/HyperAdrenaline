#ifdef __MINGW32__
    #include <winsock2.h>
    #include <windows.h>
#endif
#include "game.hpp"

int main(int argc, char** argv)
{
    #ifdef __MINGW32__
        FreeConsole();
    #endif
    Game game;
    game.start();

    return 0;
}
