#ifndef _COMMON_SFTPSOCKETUTIL_H
#define _COMMON_SFTPSOCKETUTIL_H
#include <string>

int CreateSftpSocket();

void BindServerSocket(int socket_fd, struct sockaddr_in server_address);
void ListenToSocket(int socket_fd);

void ConnectSocket(int socket_fd, struct sockaddr_in server_address);

void SendMessage(int socket_fd, std::string message);
std::string RecieveMessage(int socket_fd);

void RecieveFile(int socket_fd, std::string filename, bool append = false);

#endif