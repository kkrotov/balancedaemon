#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <string>
#include <vector>
#include <sstream>
#include <libpq-fe.h>
#include <iostream>
#include <thread>
#include "sockutils.h"
static void daemonize();

volatile sig_atomic_t   gGracefulShutdown=0;
PGconn *g_lpPgconn;

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

bool add (std::string userid, std::string amount) {
    std::string query = "select increment_balance('"+userid+"'::text, ("+amount+")::money);";
    PGresult *res = PQexec(g_lpPgconn, query.c_str());
    ExecStatusType stat = PQresultStatus(res);
    return stat != PGRES_FATAL_ERROR;
}

bool sub (std::string userid, std::string amount) {
    return add(userid, "-"+amount);
}

void HandleConnection(const int slave) {
    std::string commandline = ReadCommand(slave);
    if (commandline.length() == 0) {
        close(slave);
        return;
    }
    std::vector<std::string> command = split(commandline, ' ');
    if (command[0] == "add") {
        if (add(command[1], command[2]))
            WriteToSocket(slave, "OK");
        else {
            WriteToSocket(slave, "ERROR");
        }
    } else
    if (command[0] == "sub") {
        if (sub(command[1], command[2]))
            WriteToSocket(slave, "OK");
        else {
            WriteToSocket(slave, "ERROR");
        }
    } else
    if (command[0] == "quit\n") {
        WriteToSocket(slave, "QUIT");
        gGracefulShutdown = 1;
    }
    else
        WriteToSocket(slave, "ERROR");

    close(slave);
}

void AcceptConnections(const int master)
{
    while(!gGracefulShutdown) {
        int slave = WaitConnection(master);
        if (slave<=0)
            continue;

        std::thread t(HandleConnection, slave);
        t.detach();
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
        exit(1);

    int port = atoi(argv[1]);

    //daemonize();
    // echo quit | netcat localhost 8888
    syslog (LOG_NOTICE, "balancedaemon started.");

    int masterSocket=-1;
    if(!BindPassiveSocket(port, &masterSocket)) {
        syslog(LOG_INFO, "BindPassiveSocket failed, errno=%d", errno);
        exit(-1);
    }
    const char *conninfo = argv[2];
    int res;
    g_lpPgconn = PQconnectdb(conninfo);
    if (PQstatus(g_lpPgconn) == CONNECTION_OK) {
        AcceptConnections(masterSocket);
        res = EXIT_SUCCESS;
    }
    else {
        syslog(LOG_ERR,"Connection to database failed: %s", PQerrorMessage(g_lpPgconn));
        res = -1;
    }
    syslog (LOG_NOTICE, "balancedaemon terminated.");
    closelog();
    close (masterSocket);
    PQfinish(g_lpPgconn);

    return res;
}
