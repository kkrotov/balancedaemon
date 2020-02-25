#pragma once
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <libpq-fe.h>
#include <thread>
#include "sockutils.h"

class CBalanceService;
void HandleConnectionThread(const int slave, CBalanceService *lpService);

class CBalanceService
{
    volatile sig_atomic_t   gGracefulShutdown;
    PGconn *g_lpPgconn;
    int masterSocket;

public:
    CBalanceService() : gGracefulShutdown(0) {}
    ~CBalanceService() {
        closelog();
        close (masterSocket);
        PQfinish(g_lpPgconn);
    }
    void shutdown () { gGracefulShutdown=1;}
    bool init(int port, std::string conninfo) {
        openlog ("balancedaemon", LOG_PID, LOG_DAEMON);
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

            std::thread t(HandleConnectionThread, slave, this);
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
};
