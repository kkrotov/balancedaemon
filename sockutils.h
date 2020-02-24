#pragma once
#include <string>

int BindPassiveSocket(const int portNum);
std::string ReadCommand (int sock);
void WriteToSocket (int sock, std::string line);
int WaitConnection (const int master);
