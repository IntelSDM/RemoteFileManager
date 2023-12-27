#pragma once

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#ifdef DrawText
#undef DrawText
#endif
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
#ifdef GetObject
#undef GetObject
#endif
#ifdef SendMessage
#undef SendMessage
#endif
#ifdef GetUserName
#undef GetUserName
#endif
#ifdef CreateFont
#undef CreateFont
#endif
#ifdef Button
#undef Button
#endif
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <fstream>
#include <ppltasks.h>
#include <windowsx.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <TlHelp32.h>
#include <thread>
#include <filesystem>
#include <algorithm> 
#include <cctype>    
#include <iomanip>
#include <random>
#include <sstream>
#include <locale>
#include <d2d1_1.h>
#include <dwrite.h>
#include "json.hpp"
using json = nlohmann::json;
#pragma comment(lib, "ws2_32.lib")

#include "Colour.h"
#include "Graphics.h"
#include "font.h"
#include "Vector.h"
#include "Input.h"