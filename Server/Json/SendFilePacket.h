#pragma once
#include "pch.h"
class SendFilePacket
{
private:
    const int ID = 2;
public:
    SendFilePacket()
    {

    }

    std::string FileName;

    void ToJson(json& j) const
    {
        j = json{
            {"ID", ID},
            {"FileName", FileName},
        };
    }

    // Convert JSON to Entity object
    void FromJson(const json& j)
    {
        FileName = j["FileName"];
    }
};