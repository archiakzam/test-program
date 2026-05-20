#include "HttpService.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

HttpService::HttpService(int otherPort, function<string()> otherHandler)
    : port(otherPort), handler(otherHandler)
{
}

void HttpService::start()
{
    int servId = socket(AF_INET,SOCK_STREAM,0);
    if(servId<0) 
    {
        perror("socker init err");
        return;
    }

    int opt = 1;
    setsockopt(servId, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(servId, (struct sockaddr*)&addr, sizeof(addr))<0)
    {
        perror("bind err");
        close(servId);
        return;
    }

    if(listen(servId, 10)<0)
    {
        perror("listen err");
        close(servId);
        return;
    }
    cout << "HTTP server listening on port " << port << endl;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(servId, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("client accept err");
            continue;
        }

        char buffer[4096];
        ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            string request(buffer);
            if (request.find("GET /media_files") == 0) {
                string json = handler();
                ostringstream response;
                response << "HTTP/1.1 200 OK\r\n"
                         << "Content-Type: application/json\r\n"
                         << "Content-Length: " << json.size() << "\r\n"
                         << "Connection: close\r\n\r\n"
                         << json;
                string resp = response.str();
                send(client_fd, resp.c_str(), resp.size(), 0);
            } else {
                string not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                send(client_fd, not_found.c_str(), not_found.size(), 0);
            }
        }
        close(client_fd);
    }
    close(servId);
}
