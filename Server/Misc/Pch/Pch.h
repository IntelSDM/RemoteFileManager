#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX
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
#include "json.hpp"
using json = nlohmann::json;
#pragma comment(lib, "ws2_32.lib")