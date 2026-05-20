#pragma once
#include<string>
#include<vector>

struct MediaInfo
{
    std::vector<std::string> audio;
    std::vector<std::string> video;
    std::vector<std::string> images;    
};

class Scaner
{
public:
    static MediaInfo scanPath(const std::string& path);
};