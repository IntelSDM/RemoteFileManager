#pragma once
#include "pch.h"
class LoginPacket
{
private:
    const int ID = 1;
public:
    LoginPacket(std::string username, std::string password) : Username(username), Password(password)
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