#pragma once
#include <string>

bool BindPassiveSocket(const int portNum, int *boundSocket);
std::string ReadCommand (int sock);
void WriteToSocket (int sock, std::string line);
int WaitConnection (const int master);
