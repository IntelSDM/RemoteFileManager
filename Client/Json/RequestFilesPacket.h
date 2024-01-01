#pragma once
#include "pch.h"
class RequestFilesPacket
{
private:
    const int ID = 3;
public:
    RequestFilesPacket()
    {

    }

    void ToJson(json& j) const
    {
        j = json{
            {"ID", ID},

        };
    }
};