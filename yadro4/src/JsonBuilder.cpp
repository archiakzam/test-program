#include"JsonBuilder.h"
#include<vector>
#include<sstream>

using namespace std;

string produceJson(const string& s) {
    ostringstream o;
    for (char c : s) {
        switch (c) {
            case '"':  o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b";  break;
            case '\f': o << "\\f";  break;
            case '\n': o << "\\n";  break;
            case '\r': o << "\\r";  break;
            case '\t': o << "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                    o << "\\u" << hex << static_cast<int>(c);
                else
                    o << c;
        }
    }
    return o.str();
}

string strArrToJson(const vector<string>& arr)
{
    ostringstream strStream;
    strStream<<"[";
    for(int i=0;i<(int)arr.size();++i)
    {
        if(i>0)strStream<<",";
        strStream<<"\""<<produceJson(arr[i])<<"\"";
    }
    strStream<<"]";
    return strStream.str();

}

string JsonBuilder::buildJson(const MediaInfo& media)
{   
    ostringstream json;
    json 
        << "{"
         << "\"audio\": " << strArrToJson(media.audio) << ", "
         << "\"video\": " << strArrToJson(media.video) << ", "
         << "\"image\": " << strArrToJson(media.images)
         << "}";
    return json.str();
}
