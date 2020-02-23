#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string>
#include <netinet/in.h>
#include <cstring>

bool BindPassiveSocket(const int portNum, int *boundSocket)
{
    struct sockaddr_in sin;
    int   newsock, optval;
    size_t optlen;
    memset(&sin.sin_zero, 0, 8);

    sin.sin_port = htons(portNum);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    if((newsock= socket(PF_INET, SOCK_STREAM, 0))<0)
        return false;

    optval = 1;
    optlen = sizeof(int);
    setsockopt(newsock, SOL_SOCKET, SO_REUSEADDR, &optval, optlen);
    if(bind(newsock, (struct sockaddr*) &sin, sizeof(struct sockaddr_in))<0)
        return false;

    if(listen(newsock,SOMAXCONN)<0)
        return false;

    *boundSocket = newsock;
    return true;
}

std::string ReadCommand (int sock)
{
    char readbuf[1025];
    const size_t buflen=1024;
    size_t bytesRead = recv(sock, readbuf, buflen, 0);
    if (bytesRead<0)
        return std::string("");

    readbuf[bytesRead] = '\0';
    return std::string(readbuf);
}

void WriteToSocket (int sock, std::string line)
{
    const char *buff = line.c_str();
    size_t bufflen = line.length();
    send (sock, buff, bufflen, 0);
}

int WaitConnection (const int master)
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(master, &rfds);
    struct timeval tv = { 0, 50000 };  // 50 ms
    int iResult = select(master+1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv);
    if (iResult == 0)   /// timeout
        return 0;

    if(iResult < 0) {
        syslog(LOG_ERR, "select() failed: %m\n");
        return -1;
    }
    struct sockaddr_in  client;
    socklen_t socklen = sizeof(client);
    int slave = accept(master,(struct sockaddr *)&client, &socklen);
    if(slave<0) {     /* ошибка accept() */
        if (errno == EINTR)
            return 0;

        syslog(LOG_ERR, "accept() failed: %m\n");
        return -1;
    }
    return slave;
}
