#include"Scaner.h"
#include<filesystem>
#include<set>
#include<algorithm>

using namespace std;
string toLower(const string& str)
{
    string res(str);
    for(int iter=0;iter<(int)res.size();++iter)
    {
        res[iter]=tolower(res[iter]);        
    }
    return res;
}

bool isFileMultimedia(const filesystem::path& file,MediaInfo& info)
{
    if(!filesystem::is_regular_file(file))  return false;
    
    static const set<string> audioExt = {".mp3", ".wav", ".ogg", ".flac", ".m4a"};
    static const set<string> videoExt = {".mp4", ".avi", ".mkv", ".mov", ".wmv", ".flv"};
    static const set<string> imageExt = {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff"};

    string extension= toLower(file.extension().string());
    string fileName = file.filename().string();
    
    if(audioExt.count(extension))
    {   
        info.audio.push_back(fileName);
        return true;        
    }
    if(videoExt.count(extension))
    {   
        info.video.push_back(fileName);
        return true;        
    }
    if(imageExt.count(extension))
    {   
        info.images.push_back(fileName);
        return true;        
    }
    return false;
}

MediaInfo Scaner::scanPath(const string& path)
{
    MediaInfo result;
    if(!filesystem::exists(path) || !filesystem::is_directory(path)) return result;

    for (const auto& entry : filesystem::directory_iterator(path)) {
        isFileMultimedia(entry.path(), result);
    }
    return result;
}