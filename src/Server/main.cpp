#include "Addresses.h"
#include "Commands.h"
#include "FileSystemHelper.h"
#include "NetworkHeaders.h"
#include "Responses.h"
#include "SftpSocketUtil.h"
#include "StringUtil.h"
#include "User.h"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#define MAX 255
#define SA struct sockaddr

int main(int argc, char **argv) {

  std::string cwd;
  std::string type = "B";

  // Initialise CWD
  char temp_buf[MAX];
  cwd = getcwd(temp_buf, MAX);
  bzero(temp_buf, MAX);
  cwd.append("/resources/Server/");
  std::cout << "Server root directory: " << cwd << "\n";

  // Load users from file
  std::map<std::string, sftp::User> users;
  std::ifstream users_file("./resources/users.txt");
  std::string line;
  while (std::getline(users_file, line)) {
    sftp::User temp(line);
    users.emplace(temp.user, temp);
  }
  std::cout << "Users in the system\n";
  for (auto &it : users) {
    std::cout << it.second << "\n";
  }

  int sockfd, connfd, len;
  struct sockaddr_in servaddr, cli;

  // socket create and verification
  sockfd = CreateSftpSocket();

  // Initialise server socket address to accept all requests
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);

  // Binding newly created socket to given IP and verification
  BindServerSocket(sockfd, servaddr);

  // Now server is ready to listen and verification
  ListenToSocket(sockfd);
  len = sizeof(cli);

  std::cout
      << "SFTP server has initialised correctly; waiting for connections\n";

  // Listen for client connections one at a time
  while (true) {

    // Reinitialise CWD
    cwd = getcwd(temp_buf, MAX);
    bzero(temp_buf, MAX);
    cwd.append("/resources/Server/");

    // Accept the data packet from client
    connfd = accept(sockfd, (SA *)&cli, (socklen_t *)&len);
    if (connfd < 0) {
      throw std::logic_error("Could not accept incoming connection");
    }

    // Initialise incoming and outgoing buffers
    char incoming[MAX], outgoing[MAX];
    bzero(incoming, MAX);
    bzero(outgoing, MAX);

    // Send initial connection message to client
    sprintf(outgoing, "%s%s %s", sftp::SUCCESS, sftp::DUMMY_HOST_NAME,
            sftp::POSITIVE_GREETING);
    write(connfd, outgoing, 255);
    bzero(outgoing, MAX);

    // Initialise some state
    sftp::User client_user, server_user("some", "unguessable", "parameters");
    std::string command, prev_command;
    std::string file_to_rename, file_to_retr, file_to_stor, stor_method;
    int retr_file_size, stor_file_size;
    bool renaming = false, retrieving = false, storing = false;

    // Process and response to client commands
    while (true) {
      // Ensure the incoming and outgoing buffers are cleared every time
      bzero(incoming, MAX);
      bzero(outgoing, MAX);
      std::string message, argument, response;
      std::stringstream message_stream;
      std::vector<std::string> arguments;

      // Read client command
      read(connfd, incoming, MAX);
      message = std::string{incoming};
      std::cout << "Command recieved: " << message << "\n";

      // Check the message recieved is of valid length
      if (strlen(incoming) < 4) {
        response = sftp::ERROR;
        response.append("Invalid Message");
        SendMessage(connfd, response);
        continue;
      }

      // If the message is long enough, but the 5th character is not null or a
      // space, the command is invalid per the specification
      if (!((incoming[4] == ' ') || (incoming[4] == '\0'))) {
        sprintf(outgoing, "%sInvalid Command", sftp::ERROR);
        write(connfd, outgoing, MAX);
        continue;
      }

      // Split the message by spaces
      message_stream = std::stringstream{message};
      while (getline(message_stream, argument, ' ')) {
        arguments.push_back(argument);
      }
      prev_command = command;
      command = ToUpper(arguments[0]);

      // Handle second stage of NAME command
      if (renaming) {
        renaming = false;
        std::string rename_file = file_to_rename;
        file_to_rename = "";
        if (command != "TOBE") {
          response = sftp::ERROR;
          response.append("NAME aborted");
          SendMessage(connfd, response);
          continue;
        }

        // If USER was not called appropriately yet, fail
        if (server_user != client_user) {
          response = sftp::ERROR;
          response.append("You must log in to use this command");
          SendMessage(connfd, response);
          continue;
        }

        // If no name was sent, fail
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("Insufficient parameters");
          SendMessage(connfd, response);
          continue;
        }

        // Attempt to rename file
        std::string new_name = cwd + arguments[1];
        int rename_res = rename(rename_file.c_str(), new_name.c_str());
        if (rename_res == 0) {
          response = sftp::SUCCESS;
          response.append(rename_file);
          response.append(" renamed to ");
          response.append(new_name);
          SendMessage(connfd, response);
          continue;
        }

        response = sftp::ERROR;
        response.append(
            "File wasn't renamed because the provided name was invalid");
        SendMessage(connfd, response);
        continue;
      }

      // Handle second stage of RETR command
      if (retrieving) {
        retrieving = false;
        std::string retr = file_to_retr;
        file_to_retr = "";
        int retr_size = retr_file_size;
        retr_file_size = 0;

        // Abort the RETR if the command sent is incorrect
        if (command == "STOP") {
          response = sftp::SUCCESS;
          response.append("ok, RETR aborted");
          SendMessage(connfd, response);
          continue;
        }
        if (command != "SEND") {
          response = sftp::ERROR;
          response.append("RETR aborted");
          SendMessage(connfd, response);
          continue;
        }

        // The validity of the file was checked when RETR was processed so
        // checking again is usually unnecessary
        std::ifstream f(retr);
        std::stringstream retr_buffer;
        retr_buffer << f.rdbuf();
        std::string retr_content = retr_buffer.str();
        SendMessage(connfd, retr_content);
        continue;
      }

      // Handle the second stage of the STOR command
      if (storing) {
        storing = false;
        std::string stor = file_to_stor;
        std::string stor_type = stor_method;
        file_to_stor = "";
        stor_method = "";

        // Abort the STOR if the command is invalid
        if (command != "SIZE") {
          response = sftp::ERROR;
          response.append("STOR aborted");
          SendMessage(connfd, response);
          continue;
        }
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("STOR aborted, no size specified");
          SendMessage(connfd, response);
          continue;
        }

        // Size specified, response with affirmation and recieve a file
        std::string stor_size = arguments[1];
        response = sftp::SUCCESS;
        response.append("ok, waiting for file");
        SendMessage(connfd, response);
        if (stor_type == "APP") {
          std::cout << "appending\n";
          RecieveFile(connfd, stor, true);
        } else {
          std::cout << "creating\n";
          RecieveFile(connfd, stor);
        }
        response = sftp::SUCCESS;
        response.append("Saved ");
        response.append(stor);
        SendMessage(connfd, response);
        continue;
      }

      // Check the recieved command is valid
      if (!sftp::Commands::IsValid(command)) {
        response = sftp::ERROR;
        response.append("Invalid Command");
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::USER) {
        // No id is supplied
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("No user-id was specified");
          SendMessage(connfd, response);
          continue;
        }

        // User doesn't exist
        std::string user_id = arguments[1];
        auto it = users.find(user_id);
        if (it == users.end()) {
          response = sftp::ERROR;
          response.append("Invalid user-id, try again");
          SendMessage(connfd, response);
          continue;
        }

        // User doesn't have a username or password
        server_user = (*it).second;
        client_user = sftp::User{user_id, "", ""};
        if ((server_user.acct == "") && (server_user.pass == "")) {
          response = sftp::LOGGED_IN;
          response.append(server_user.user);
          response.append(" logged in");
          SendMessage(connfd, response);
          continue;
        }

        // User has a username and/or password
        response = sftp::SUCCESS;
        response.append("User-id valid, send account and password");
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::ACCT) {
        // If USER was not called appropriately yet, fail
        if (server_user.user != client_user.user) {
          response = sftp::ERROR;
          response.append("You must select a valid user-id");
          SendMessage(connfd, response);
          continue;
        }

        // If no account name was sent, fail
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("No account was specified");
          SendMessage(connfd, response);
          continue;
        }
        client_user.acct = arguments[1];

        // If the account name given doesn't match the account
        // associated with the id, fail
        if (client_user.acct != server_user.acct) {
          response = sftp::ERROR;
          response.append("Invalid account, try again");
          SendMessage(connfd, response);
          continue;
        }

        // If the program somehow reaches this point with a blank
        // password on the server-side user, update the client's
        // user and response with logged in
        if (server_user.pass == "") {
          client_user.pass = "";
          response = sftp::LOGGED_IN;
          response.append("Account valid, logged-in");
          SendMessage(connfd, response);
          continue;
        }

        // If the given passwords don't match, prompt for a password
        if (client_user.pass != server_user.pass) {
          response = sftp::SUCCESS;
          response.append("Account valid, send password");
          SendMessage(connfd, response);
          continue;
        }

        // USER has been called, an account name matching the USER call
        // was recieved, and the passwords match
        response = sftp::LOGGED_IN;
        response.append("Account valid, logged-in");
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::PASS) {
        // If USER was not called appropriately yet, fail
        if (server_user.user != client_user.user) {
          response = sftp::ERROR;
          response.append("You must select a valid user-id");
          SendMessage(connfd, response);
          continue;
        }

        // If no password was sent, fail
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("No password was specified");
          SendMessage(connfd, response);
          continue;
        }
        client_user.pass = arguments[1];

        // If password is incorrect, fail
        if (client_user.pass != server_user.pass) {
          response = sftp::ERROR;
          response.append("Wrong password, try again");
          SendMessage(connfd, response);
          continue;
        }

        // If accounts don't match, prompt for account
        if (client_user.acct != server_user.acct) {
          response = sftp::SUCCESS;
          response.append("Send account");
          SendMessage(connfd, response);
          continue;
        }

        response = sftp::LOGGED_IN;
        response.append(" Logged in");
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::TYPE) {
        // If USER was not called appropriately yet, fail
        if (server_user != client_user) {
          response = sftp::ERROR;
          response.append("You must log in to use this command");
          SendMessage(connfd, response);
          continue;
        }

        // If no transmission type was sent, fail
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("Insufficient parameters");
          SendMessage(connfd, response);
          continue;
        }
        std::string recieved_type = arguments[1];

        // If the transmission type sent was invalid, fail
        if ((recieved_type != "A") && (recieved_type != "B") &&
            (recieved_type != "C")) {
          response = sftp::ERROR;
          response.append("Type not valid");
          SendMessage(connfd, response);
          continue;
        }

        type = recieved_type;
        response = sftp::SUCCESS;
        response.append("Using ");
        if (type == "A") {
          response.append("Ascii");
        }
        if (type == "B") {
          response.append("Binary");
        }
        if (type == "C") {
          response.append("Continuous");
        }
        response.append(" mode");
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::LIST) {
        // If USER was not called appropriately yet, fail
        if (server_user != client_user) {
          response = sftp::ERROR;
          response.append("You must log in to use this command");
          SendMessage(connfd, response);
          continue;
        }

        // If no format was sent, fail
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("Insufficient parameters");
          SendMessage(connfd, response);
          continue;
        }

        // If the format argument is invalid, fail
        std::string format = arguments[1];
        if ((format != "F") && (format != "V")) {
          response = sftp::ERROR;
          response.append("Unknown directory format");
          SendMessage(connfd, response);
          continue;
        }

        // Check the current directory if no path argument was provided
        std::string path = "./";
        if (arguments.size() > 2) {
          path = arguments[2];
        }

        // If an error occurs while getting file information, spit out the
        // exception message
        std::vector<std::pair<std::string, std::string>> file_info;
        std::string search_dir;
        try {
          search_dir = cwd + path;
          file_info = GetFileInfoInDirectory(search_dir);
        } catch (std::logic_error &err) {
          response = sftp::ERROR;
          response.append(err.what());
          SendMessage(connfd, response);
          continue;
        }

        // Both successful cases start with the success token and the searched
        // file path
        response = sftp::SUCCESS;
        response.append(search_dir);
        response.append("\n");

        if (format == "F") {
          for (auto it : file_info) {
            response.append(it.first);
            response.append("\n");
          }
          SendMessage(connfd, response);
          continue;
        }

        // If the format was valid and was not F it must be V
        for (auto it : file_info) {
          response.append(it.first);
          response.append(" | ");
          response.append(it.second);
          response.append("\n");
        }
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::CDIR) {
        // If USER was not called appropriately yet, fail
        if (server_user != client_user) {
          response = sftp::ERROR;
          response.append("You must log in to use this command");
          SendMessage(connfd, response);
          continue;
        }

        // If no directory was sent, fail
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("Insufficient parameters");
          SendMessage(connfd, response);
          continue;
        }

        std::string new_dir = arguments[1];

        // If the directory starts with a ., assume its relative and append to
        // cwd
        if (new_dir[0] == '.') {
          new_dir = cwd + new_dir;
        }

        // If the directory does exist, modify cwd
        if (DirExists(new_dir)) {
          cwd = new_dir;
          if (cwd[cwd.size() - 1] != '/') {
            cwd.append("/");
          }
          response = sftp::LOGGED_IN;
          response.append("Changed working dir to ");
          response.append(cwd);
          SendMessage(connfd, response);
          continue;
        }

        response = sftp::ERROR;
        response.append(
            "Can't connect to directory because: directory doesn't exist");
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::KILL) {
        // If USER was not called appropriately yet, fail
        if (server_user != client_user) {
          response = sftp::ERROR;
          response.append("You must log in to use this command");
          SendMessage(connfd, response);
          continue;
        }

        // If no filename was sent, fail
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("Insufficient parameters");
          SendMessage(connfd, response);
          continue;
        }

        std::string del_file = arguments[1];
        // Path starts with ., assume relative and append to cwd
        if (del_file[0] == '.') {
          del_file = cwd + del_file;
        }

        // Try to delete the file
        if (FileExists(del_file)) {
          if (remove(del_file.c_str()) == 0) {
            response = sftp::SUCCESS;
            response.append(del_file);
            response.append(" deleted");
            SendMessage(connfd, response);
            continue;
          }

          // There are ways to tell why remove() fails but I didn't think it was
          // that important
          response = sftp::ERROR;
          response.append("Not deleted because file may be in use");
          SendMessage(connfd, response);
          continue;
        }

        // File doesn't exist
        response = sftp::ERROR;
        response.append("Not deleted because file could not be found");
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::NAME) {
        // If USER was not called appropriately yet, fail
        if (server_user != client_user) {
          response = sftp::ERROR;
          response.append("You must log in to use this command");
          SendMessage(connfd, response);
          continue;
        }

        // If no name was sent, fail
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("Insufficient parameters");
          SendMessage(connfd, response);
          continue;
        }

        // Provided name starts with a ., append to cwd
        std::string rename_file = arguments[1];
        if (rename_file.size() > 0) {
          if (rename_file[0] == '.') {
            rename_file = cwd + rename_file;
          }
        }

        // Set up flag to enable the second stage of the NAME command earlier in
        // this loop
        if (FileExists(rename_file)) {
          renaming = true;
          file_to_rename = rename_file;
          response = sftp::SUCCESS;
          response.append("File exists");
          SendMessage(connfd, response);
          continue;
        }

        response = sftp::ERROR;
        response.append("Can't find ");
        response.append(rename_file);
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::DONE) {
        if (server_user != client_user) {
          response = sftp::ERROR;
          response.append("You must log in to use this command");
          SendMessage(connfd, response);
          continue;
        }

        // Ends the loop instead of continuing it to allow for the next client
        // connection
        response = sftp::SUCCESS;
        SendMessage(connfd, response);
        break;
      }

      if (command == sftp::Commands::RETR) {
        // If USER was not called appropriately yet, fail
        if (server_user != client_user) {
          response = sftp::ERROR;
          response.append("You must log in to use this command");
          SendMessage(connfd, response);
          continue;
        }

        // If no filename was sent, fail
        if (arguments.size() < 2) {
          response = sftp::ERROR;
          response.append("Insufficient parameters");
          SendMessage(connfd, response);
          continue;
        }

        // File name stars with ., append to cwd
        std::string retr = arguments[1];
        if (retr[0] == '.') {
          retr = cwd + retr;
        }

        // Set up flag to process the second stage of RETR at the top of the
        // loop
        if (FileExists(retr)) {
          retrieving = true;
          file_to_retr = retr;
          retr_file_size = GetFileSize(retr);
          response = std::to_string(retr_file_size);
          SendMessage(connfd, response);
          continue;
        }

        response = sftp::ERROR;
        response.append("File doesn't exist");
        SendMessage(connfd, response);
        continue;
      }

      if (command == sftp::Commands::STOR) {
        // If USER was not called appropriately yet, fail
        if (server_user != client_user) {
          response = sftp::ERROR;
          response.append("You must log in to use this command");
          SendMessage(connfd, response);
          continue;
        }

        // If no storage type and/or file name was sent, fail
        if (arguments.size() < 3) {
          response = sftp::ERROR;
          response.append("Insufficient parameters");
          SendMessage(connfd, response);
          continue;
        }

        // File name stars with ., append to cwd
        std::string stor_type = arguments[1], stor_filename = arguments[2];
        if (stor_filename[0] == '.') {
          stor_filename = cwd + stor_filename;
        }

        storing = true;
        file_to_stor = stor_filename;
        stor_method = stor_type;

        if (stor_type == "NEW") {
          if (FileExists(stor_filename)) {
            // Assume generations aren't supported, must disable processing of
            // second stage of STOR since this fails
            storing = false;
            file_to_stor = "";
            stor_method = "";
            response = sftp::ERROR;
            response.append(
                "File exists, but system doesn't support generations");
            SendMessage(connfd, response);
            continue;
          }
          response = sftp::SUCCESS;
          response.append("File does not exist, will create new file");
          SendMessage(connfd, response);
          continue;
        }

        if (stor_type == "OLD") {
          if (FileExists(stor_filename)) {
            response = sftp::SUCCESS;
            response.append("Will write over old file");
            SendMessage(connfd, response);
            continue;
          }
          response = sftp::SUCCESS;
          response.append("Will create new file");
          SendMessage(connfd, response);
          continue;
        }

        if (stor_type == "APP") {
          if (FileExists(stor_filename)) {
            response = sftp::SUCCESS;
            response.append("Will append to file");
            SendMessage(connfd, response);
            continue;
          }
          response = sftp::SUCCESS;
          response.append("Will create file");
          SendMessage(connfd, response);
          continue;
        }

        // If the logic makes it here the STOR type must be invalid, so don't
        // bother trying to process the second stage of STOR
        storing = false;
        file_to_stor = "";
        stor_method = "";
        response = sftp::ERROR;
        response.append("Invalid STOR type, should be NEW, OLD, or APP");
        SendMessage(connfd, response);
        continue;
      }
      // Send fallback response
      response = sftp::SUCCESS;
      response.append("Fallthrough message; you shouldn't get here");
      SendMessage(connfd, response);
    }
  }

  // It would be nice to process the signal (Ctrl+C, etc) to the program to
  // allow it to close its socket connections gracefully when it's terminated
  // since thats the only way to do so at the moment

  // An easier method might just be to have a hidden command the client can send
  // to the server to close it correctly rather than killing the process
  close(sockfd);
}