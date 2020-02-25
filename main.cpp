#include <string>
#include <vector>
#include "balanceservice.h"
void daemonize();

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
