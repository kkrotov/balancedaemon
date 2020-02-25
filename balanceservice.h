#pragma once
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <libpq-fe.h>
#include <thread>
#include <sstream>
#include "sockutils.h"

class CBalanceService;

class CBalanceService
{
    volatile sig_atomic_t   gGracefulShutdown;
    PGconn *g_lpPgconn;
    int masterSocket;

public:
    CBalanceService() : gGracefulShutdown(0),masterSocket(-1),g_lpPgconn(0) {
        openlog ("balancedaemon", LOG_PID, LOG_DAEMON);
    }
    ~CBalanceService() {
        closelog();
        close (masterSocket);
        PQfinish(g_lpPgconn);
    }
    void shutdown () {
        gGracefulShutdown=1;
        sleep(1);
    }
    bool init(int port, std::string conninfo) {
        syslog (LOG_NOTICE, "balancedaemon started.");

        masterSocket = BindPassiveSocket(port);
        if(masterSocket<0) {
            syslog(LOG_ERR, "BindPassiveSocket failed, errno=%d", errno);
            return false;
        }
        g_lpPgconn = PQconnectdb(conninfo.c_str());
        if (PQstatus(g_lpPgconn) != CONNECTION_OK) {
            syslog(LOG_ERR,"Connection to database failed: %s", PQerrorMessage(g_lpPgconn));
            return false;
        }
        return true;
    }
    void run () {
        while(!gGracefulShutdown) {
            int slave = WaitConnection(masterSocket);
            if (slave<=0)
                continue;

            std::thread t(&CBalanceService::HandleConnectionThread, this, slave);
            t.detach();
        }
        syslog (LOG_NOTICE, "balancedaemon terminated.");
    }
    bool add (std::string userid, std::string amount)
    {
        std::string query = "select increment_balance('"+userid+"'::text, ("+amount+")::money);";
        PGresult *res = PQexec(g_lpPgconn, query.c_str());
        ExecStatusType stat = PQresultStatus(res);
        return stat != PGRES_FATAL_ERROR;
    }
    bool sub (std::string userid, std::string amount)
    {
        return add(userid, "-"+amount);
    }
    void HandleConnectionThread(const int slave)
    {
        std::string commandline = ReadCommand(slave);
        if (commandline.length() == 0) {
            close(slave);
            return;
        }
        std::vector<std::string> command = split(commandline, ' ');
        if (command[0] == "add") {
            bool res = add(command[1], command[2]);
            WriteToSocket(slave, res? "OK":"ERROR");
        } else
        if (command[0] == "sub") {
            bool res = sub(command[1], command[2]);
            WriteToSocket(slave, res? "OK":"ERROR");
        } else
        if (command[0] == "quit\n") {
            WriteToSocket(slave, "OK");
            shutdown();
        }
        else
            WriteToSocket(slave, "ERROR");

        close(slave);
    }
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
};
