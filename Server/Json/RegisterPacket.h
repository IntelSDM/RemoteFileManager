#pragma once
#include "pch.h"
class RegisterPacket
{
private:
    const int ID = 0;
public:
    RegisterPacket()
    {

    }

    std::string Username;
    std::string Password;

    void ToJson(json& j) const
    {
        j = json{
            {"ID", ID},
            {"Username", Username},
            {"Password", Password},
        };
    }

    // Convert JSON to Entity object
    void FromJson(const json& j)
    {
        Username = j["Username"];
        Password = j["Password"];
    }
};