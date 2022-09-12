#include "SftpSocketUtil.h"
#include "NetworkHeaders.h"
#include <iostream>
#include <stdexcept>
#include <stdio.h>

int CreateSftpSocket() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    throw std::logic_error("Socket could not be created");
  }
  return sockfd;
}

void BindServerSocket(int socket_fd, struct sockaddr_in server_address) {
  if ((bind(socket_fd, (struct sockaddr *)&server_address,
            sizeof(server_address))) != 0) {
    throw std::logic_error("Socket could not be bound to server address");
  }
}

void ListenToSocket(int socket_fd) {
  if ((listen(socket_fd, 5)) != 0) {
    throw std::logic_error("Could not listen to socket");
  }
}

void ConnectSocket(int socket_fd, struct sockaddr_in server_address) {
  if (connect(socket_fd, (struct sockaddr *)&server_address,
              sizeof(server_address)) != 0) {
    throw std::logic_error("Could not connect to server");
  }
}

void SendMessage(int socket_fd, std::string message) {
  write(socket_fd, message.c_str(), message.size() + 1);
}

std::string RecieveMessage(int socket_fd) {
  char buffer[255];
  int amount_read = 0, total_read = 0;
  std::string message = "";

  bzero(buffer, 255);

  while (true) {
    bzero(buffer, 255);
    int amount_read = read(socket_fd, buffer, 255);

    message.append(std::string{buffer});
    total_read += amount_read;

    if (amount_read < 255) {
      break;
    }

    if (amount_read == 255) {
      if (buffer[254] == 0) {
        break;
      }
    }
  }
  return message;
}

void RecieveFile(int socket_fd, std::string filename, bool append) {
  char buffer[255];
  int amount_read = 0, total_read = 0;
  std::string message = "";
  FILE *recieved;
  if (append) {
    recieved = fopen(filename.c_str(), "a");
  } else {
    recieved = fopen(filename.c_str(), "w");
  }
  bzero(buffer, 255);

  while (true) {
    bzero(buffer, 255);
    int amount_read = read(socket_fd, buffer, 255);

    total_read += amount_read;

    if (amount_read < 255) {
      fwrite(buffer, 1, amount_read - 1, recieved);
      break;
    }

    if (amount_read == 255) {
      if (buffer[254] == 0) {
        fwrite(buffer, 1, amount_read - 1, recieved);
        break;
      }
    }

    fwrite(buffer, 1, amount_read, recieved);
  }
  fclose(recieved);
}