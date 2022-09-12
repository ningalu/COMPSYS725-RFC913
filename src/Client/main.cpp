#include "Addresses.h"
#include "Commands.h"
#include "NetworkHeaders.h"
#include "SftpSocketUtil.h"
#include <StringUtil.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#define MAX 255
#define SA struct sockaddr

int main(int argc, char **argv) {

  // Set up the socket, server address, and cwd
  int client_socket;
  struct sockaddr_in servaddr, cli;
  char temp_buf[255];
  bzero(temp_buf, 255);

  // The cwd is finnicky as it is the location the program is called from
  // There are ways to get the location of the executable but the test scripts
  // are set up to call the tests correctly and the usage instructions should be
  // clear in the readme
  std::string cwd = getcwd(temp_buf, 255);
  bzero(temp_buf, 255);
  if (cwd[cwd.size() - 1] != '/') {
    cwd.push_back('/');
  }
  cwd.append("resources/Client/");

  client_socket = CreateSftpSocket();

  // Initialise server socket address with expected port
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(ADDRESS);
  servaddr.sin_port = htons(PORT);

  // connect the client socket to server socket
  ConnectSocket(client_socket, servaddr);

  // Read standard initial SFTP response
  std::string initial_message = RecieveMessage(client_socket);
  std::cout << initial_message << "\n";

  // Set up some state
  bool done = false, logged_in = false;
  std::string message = "", command = "", prev_command = "";
  std::string retr_filename, stor_filename = "";
  char input_char = 0;

  // Server interaction loop
  while (!(done && logged_in)) {
    done = false;
    message = "";
    std::string response;

    // Collect user input and send it to the server
    // Should probably actually std::getline
    while (true) {
      input_char = getchar();
      if (input_char == '\n') {
        break;
      }
      message.push_back(input_char);
    }
    SendMessage(client_socket, message);

    // Figure out the command the user sent
    auto split_message = SplitString(message);
    prev_command = command;
    if (split_message.size() > 0) {
      command = ToUpper(split_message[0]);
    } else {
      command = "";
    }

    // Set up for recieving a file from RETR
    if (command == sftp::Commands::RETR) {
      if (split_message.size() > 1) {
        retr_filename = split_message[1];
      } else {
        // this should never actually happen
        retr_filename = "default_retrieval_filename.txt";
      }
    }

    // Set up for sending a file through STOR
    if (command == sftp::Commands::STOR) {
      if (split_message.size() > 1) {
        stor_filename = split_message[1];
      } else {
        // this should never actually happen
        retr_filename = "default_retrieval_filename.txt";
      }
    }

    // Retrieve the file after telling the server to SEND it
    if ((command == "SEND") && (prev_command == sftp::Commands::RETR)) {
      RecieveFile(client_socket, cwd + retr_filename);
    } else {
      response = RecieveMessage(client_socket);
      std::cout << response << "\n";
    }

    if (response.size() > 0) {
      if (response[0] == '!') {
        logged_in = true;
      }

      // Close the connection if the response to DONE is successful
      if (command == "DONE") {
        if (response[0] == '+') {
          break;
        }
      }

      // If the size of the file to send to the server is sent appropriately,
      // send the file and accept the expected messages
      if ((command == "SIZE") && (prev_command == sftp::Commands::STOR)) {
        if (response[0] == '+') {
          std::ifstream f(stor_filename);
          std::stringstream stor_buffer;
          stor_buffer << f.rdbuf();
          std::string stor_content = stor_buffer.str();
          SendMessage(client_socket, stor_content);
          std::cout << RecieveMessage(client_socket) << "\n";
        }
      }
    }
  }

  // close the socket
  close(client_socket);
  std::cout << "Connection closed\n";
}