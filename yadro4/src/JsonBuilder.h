#pragma once

#include"Scaner.h"
#include<string>

class JsonBuilder {
public:
    static std::string buildJson(const MediaInfo& media);
};