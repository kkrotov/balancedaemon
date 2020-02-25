#include <string>
#include <vector>
#include <sstream>
#include "balanceservice.h"
void daemonize();

std::vector<std::string> split(std::string commandline, char delimeter)
{
    std::stringstream ss(commandline);
    std::string item;
    std::vector<std::string> words;
    while (std::getline(ss, item, delimeter)) {
        if (item.length())
            words.push_back(item);
    }
    return words;
}

void HandleConnectionThread(const int slave, CBalanceService *lpService)
{
    std::string commandline = ReadCommand(slave);
    if (commandline.length() == 0) {
        close(slave);
        return;
    }
    std::vector<std::string> command = split(commandline, ' ');
    if (command[0] == "add") {
        bool res = lpService->add(command[1], command[2]);
        WriteToSocket(slave, res? "OK":"ERROR");
    } else
    if (command[0] == "sub") {
        bool res = lpService->sub(command[1], command[2]);
        WriteToSocket(slave, res? "OK":"ERROR");
    } else
    if (command[0] == "quit\n") {
        WriteToSocket(slave, "OK");
        lpService->shutdown();
    }
    else
        WriteToSocket(slave, "ERROR");

    close(slave);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
        exit(1);

    daemonize();

    int port = atoi(argv[1]);
    const char *conninfo = argv[2];

    CBalanceService service;
    if (!service.init(port, conninfo))
        return EXIT_FAILURE;

    service.run();
    return EXIT_SUCCESS;
}
