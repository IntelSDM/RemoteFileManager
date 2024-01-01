#pragma once
#include "pch.h"
class RequestFileDownload
{
private:
    const int ID = 4;
public:
    RequestFileDownload()
    {
    }

    int FileID;

    void ToJson(json& j) const
    {
        j = json{
            {"ID", ID},
            {"FileID", FileID},
        };
    }

    // Convert JSON to Entity object
    void FromJson(const json& j)
    {
        FileID = j["FileID"];
    }
};