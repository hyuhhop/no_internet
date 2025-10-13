#pragma once
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <stdexcept>

#pragma comment(lib, "wbemuuid.lib")


/// <summary>throw std_runtime </summary>/// <param name="disable"></param>
void disable_network(bool disable);