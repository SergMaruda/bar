// bar_started.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <winbase.h>
#include <crtdbg.h>
#include <vector>

typedef int (*StartPlugin)(int, char* []);

int APIENTRY _tWinMain(HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPTSTR    lpCmdLine,
  int       nCmdShow)
  {
  ShowWindow (GetConsoleWindow(), SW_HIDE);

  _TCHAR dir[256];
  _wgetcwd(dir, 256);
  int res(0);

  do 
    {
    int argc;
    char** argv;
    std::vector<std::unique_ptr<char>> args_v;
      {
      LPWSTR* lpArgv = CommandLineToArgvW( GetCommandLineW(), &argc );
      argv = (char**)malloc( argc*sizeof(char*) );
      int size(0);
      for(int i = 0; i < argc; ++i)
        {
        size = wcslen( lpArgv[i] ) + 1;
        argv[i] = new char[size];
        args_v.push_back(std::unique_ptr<char>(argv[i]));
        size_t n(0);
        wcstombs_s(&n, argv[i], size, lpArgv[i], size - 1);
        }
      LocalFree( lpArgv );
      }

    HMODULE library = LoadLibrary(L"bar.dll");
    StartPlugin start = (StartPlugin)GetProcAddress(library, "StartPlugin");

    res = start(argc, argv);

    FreeLibrary(library);

    if(res == 100)
      {
      //PerformUpdate
      }

    } while (res != 0);
  return res;
  }

