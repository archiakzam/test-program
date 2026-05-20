#include "Scaner.h"
#include "JsonBuilder.h"
#include <iostream>
#include<fstream>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include <thread>
#include <chrono>
#include <mutex>

#ifdef ENABLE_HTTP
#include "HttpService.h"
#endif

using namespace std;

static mutex jsonMutex;
static string currentJson;

void updateJson(const string& dirPath)
{
    MediaInfo media = Scaner::scanPath(dirPath);
    string json = JsonBuilder::buildJson(media);
    lock_guard<mutex> lock(jsonMutex);
    currentJson = json;
}

void scannerThreadFunc(const string& dirPath, int intervalSec) {
    while (true) {
        updateJson(dirPath);
        this_thread::sleep_for(chrono::seconds(intervalSec));
    }
}

void writeToFile(const string& json) {
    const char* home = getenv("HOME");
    if (!home) {
        cout << "Error: HOME environment variable not set" << endl;
        return;
    }
    string path = string(home) + "/.media_files";
    ofstream file(path);
    if (file.is_open()) {
        file << json;
        file.close();
        cout << "Written to " << path << endl;
    } else {
        cout << "Error: failed to write to " << path << endl;
    }
}

static void printUsage(const char* prog) {
    cout << "Usage: " << prog << " [options]\n"
              << "Options:\n"
              << "  --dir <path>        directory to scan\n"
              << "  --interval <sec>    scanning interval in seconds (default: 60)\n"
              << "  --http              Enable HTTP server mode (default: off)\n"
              << "  --port <port>       HTTP port (default: 1234, only with --http)\n"
              << "  --help              Show this help\n";
}

int main(int argc, char* argv[])
{
    string dirPath;
    int interval = 60;
    bool httpMode = false;
    int port = 1234;

    static struct option options[] = {
        {"dir",      required_argument, 0, 'd'},
        {"interval", required_argument, 0, 'i'},
        {"http",     no_argument,       0, 'H'},
        {"port",     required_argument, 0, 'p'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int c;  
    while ((c = getopt_long(argc, argv, "d:i:Hp:h", options, nullptr)) != -1) {
        switch (c) {
            case 'd': dirPath = optarg; break;
            case 'i': interval = stoi(optarg); break;
            case 'H': httpMode = true; break;
            case 'p': port = stoi(optarg); break;
            case 'h': printUsage(argv[0]); return 0;
            default: printUsage(argv[0]); return 1;
        }
    }

    if(dirPath.empty())
    {
        cout<<"Error: dirrectory path is empty!"<<endl;
        printUsage(argv[0]);
        return 1;
    }

    if(interval<1)
    {
        cout<<"Error: interval need to be positive!"<<endl;
        return 1;
    }
    updateJson(dirPath);

    if (httpMode) {
        #ifdef ENABLE_HTTP
        thread scanner(scannerThreadFunc, dirPath, interval);
        scanner.detach();
        HttpService server(port, []() -> string {
            lock_guard<mutex> lock(jsonMutex);
            return currentJson;
        });
        server.start();
        #else
        cout << "HTTP support not compiled. Rebuild with -DENABLE_HTTP=ON" << endl;
        return 1;
        #endif
    } else {
        while (true) {
            writeToFile(currentJson);
            this_thread::sleep_for(chrono::seconds(interval));
            updateJson(dirPath);
        }
    }

    return 0;
}