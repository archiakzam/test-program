#pragma once

#include<string>
#include<functional>

class HttpService
{
private:
    int port;
    std::function<std::string()> handler;
public:
    HttpService(int otherPort, std::function<std::string()> otherHandler);
    void start();
};