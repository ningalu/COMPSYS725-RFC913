# RFC 913

This is an implementation of a client and server for RFC 913 - Simple File Transfer Protocol for Assignment 1 of COMPSYS725. No external dependencies were used to improve portability for marking.

## Setup

This system was written on WSL Ubuntu 20.04 with GCC 9.3 and CMake 3.22.1 and has not been tested with any other tools. Compilers supporting glibc and C++11, modern versions of CMake, and other GNU/Windows compatability layers like Cygwin or MinGW should all work, although they may require some changes to the project. This implementation will not work in a Windows environment as the underlying socket and filesystem libraries are non-portable; it must be run somewhere with access to the GNU C Library. In cases where I thought RFC 913 was ambiguous or did not specify any particular behaviour, I made an effort to make server responses as informative as possible

**General Setup**

To build the project from scratch:  
1. Start a WSL shell  
    - Ensure GCC and CMake are installed
2. Run `configure.sh`
3. Run `build.sh`

This will configure the CMake project and build the Server and Client artefacts, as well as copy the `resources` folder into the `build` folder that is generated for the programs to use.

**Server Setup**

To run the server, either:
1. Run `serve.sh`  

Or  

1. Navigate to `/build`
2. Run `Server`  

Note that the server will only accept one connection at a time. The server checks the directory it was run from for its resources, so it should be run from a shell in the `/build` folder or with the `serve.sh` script for consistent behaviour.

**Client Setup**

To run the client:

1. Navigate to `/build`
2. Run `Client`  

The client checks the directory it was run from for its resources, so it must be run from a shell in the `/build` folder.

**Resources**

Resources used by the client and server are contained in `/resources`, and are copied into the `/build` folder by the `build.bat` script. These resources are
- `/resources/users.txt` in `/resources`, which lists sets of user|acct|pass information
- `/resources/Server` which is the default root directory of Server and contains `big.txt`, `server1.txt`, `server2.txt`, and `server3.txt`
- `/resources/Client` which contains `send.txt`, `client1.txt`, `client2.txt`, and `client3.txt`

## Testing

**Testing Setup**

To run all tests:
1. Start the server
2. Run ./test.bat

To run any individual test:
1. Start the server
2. Pipe the output of the test to run to the Client, eg. `../tests/login/user_with_no_acct_or_password.sh | ./Client`
    - Keep in mind that `Client` should be run from the `/build` folder

The tests are categorised by the general feature they test, which is typically just the command. Note that:
- the literal path given by the server will change depending on the machine it runs on
- the `build/resources` folder is restored before every test that modifies it so the changes wont be reflected unless using the functionality manually

### Login tests
1. Commands don't work before logging in  

>+thol600-server SFTP Service  
\> TYPE random parameters  
-You must log in to use this command  
\> LIST random parameters  
-You must log in to use this command  
\> CDIR random parameters  
-You must log in to use this command  
\> KILL random parameters  
-You must log in to use this command  
\> NAME random parameters  
-You must log in to use this command  
\> DONE random parameters  
-You must log in to use this command  
\> RETR random parameters  
-You must log in to use this command  
\> STOR random parameters  
-You must log in to use this command  
\> "user 1"  
!1 logged in  
\> "done"  
\+  
Connection closed  

2. User with no account or password
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> done  
\+  
Connection closed

3. User with account and no password
>+thol600-server SFTP Service  
\> user 2  
+User-id valid, send account and password  
\> acct ningalu  
!Account valid, logged-in  
\> done  
\+  
Connection closed

4. User with account and password
>+thol600-server SFTP Service  
\> user 3  
+User-id valid, send account and password  
\> acct prendar  
+Account valid, send password  
\> pass password123  
! Logged in  
\> done  
\+  
Connection closed

### Done Tests
1. Done closes the client's connection
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> done  
\+  
Connection closed  

### Transmission Type Tests

1. Insufficient arguments fail
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> type  
-Insufficient parameters  
\> done  
\+  
Connection closed

2. Invalid transmission type
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> type ghfhdsgdfs  
-Type not valid  
\> done  
\+  
Connection closed

3. Ascii type
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> type A  
+Using Ascii mode  
\> done  
\+  
Connection closed

4. Binary type
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> type B  
+Using Binary mode  
\> done  
\+  
Connection closed

5. Continuous type
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> type C  
+Using Continuous mode  
\> done  
\+  
Connection closed

### Directory Listing Tests

1. Insufficient arguments fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> list  
-Insufficient parameters  
\> done  
\+  
Connection closed

2. Invalid format fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> list gfdsgsdfsgdsg  
-Unknown directory format  
\> done  
\+  
Connection closed

3. Invalid directory fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> list F ./not_a_real_directory/  
-File does not exist 
\> done  
\+  
Connection closed

4. Standard format
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> list F  
+/mnt/a/Programming/725a1/resources/Server/./  
.  
..  
big.txt  
server1.txt  
server2.txt  
server3.txt  
\
\> done  
\+  
Connection closed

5. Verbose format
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> list V  
+/mnt/a/Programming/725a1/resources/Server/./  
. | Dir  
.. | Dir  
big.txt | 2984B  
server1.txt | 1B  
server2.txt | 1B  
server3.txt | 1B    
\
\> done  
\+  
Connection closed

6. Different directory
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> list V ../  
+/mnt/a/Programming/725a1/resources/Server/../  
.  
..  
Client  
Server  
users.txt  
\
\> done  
\+  
Connection closed

### Change Directory Tests

1. Insufficient arguments fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> cdir  
-Insufficient parameters  
\> done  
\+  
Connection closed

2. Invalid directory name
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> cdir ./directory_that_doesnt_exist/  
-Can't connect to directory because: directory doesn't exist  
\> done  
\+  
Connection closed

3. Valid directory name  
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> cdir ../  
!Changed working dir to /mnt/a/Programming/725a1/build/resources/Server/../  
\> list V  
+/mnt/a/Programming/725a1/build/resources/Server/.././  
. | Dir  
.. | Dir  
Client | Dir  
Server | Dir  
users.txt | 36B  
\
\> done  
\+  
Connection closed

### Delete File Tests

1. Insufficient arguments fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> kill  
-Insufficient parameters  
\> done  
\+  
Connection closed

2. Invalid file fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> kill ./gfdhgsbosgd  
-Not deleted because file could not be found  
\> done  
\+  
Connection closed

3. Valid file
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> kill ./server1.txt  
+/mnt/a/Programming/725a1/build/resources/Server/./server1.txt deleted  
\> done  
\+  
Connection closed

### Rename File Tests

1. Insufficient arguments fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> name  
-Insufficient parameters  
\> done  
\+  
Connection closed

2. Invalid file fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> name ./invalid  
-Can't find /mnt/a/Programming/725a1/build/resources/Server/./invalid  
\> done  
\+  
Connection closed

3. Valid file with invalid new name
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> name ./server1.txt  
+File exists  
\> tobe //////////  
-File wasn't renamed because the provided name was invalid  
\> done  
\+  
Connection closed

4. Valid file with valid new name
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> name ./server1.txt  
+File exists  
\> tobe server4  
+/mnt/a/Programming/725a1/build/resources/Server/./server1.txt renamed to /mnt/a/Programming/725a1/build/resources/Server/server4.txt    
\> done  
\+  
Connection closed

### Retrieve File Tests

1. Insufficient arguments fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> retr  
-Insufficient parameters  
\> done  
\+  
Connection closed

2. Invalid file fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> retr ./not_real  
-File doesn't exist  
\> done  
\+  
Connection closed

3. Stop valid file
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> retr ./server1.txt  
1  
\> stop  
+ok, RETR aborted  
\> done  
\+  
Connection closed

4. Send valid file
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> retr ./big.txt  
2984  
\> send  
\> done  
\+  
Connection closed

### Retrieve File Tests

1. Insufficient arguments fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> stor  
-Insufficient parameters  
\> done  
\+  
Connection closed

2. STOR without SIZE fails
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> stor OLD ./server1.txt  
+Will write over old file    
\> done  
-STOR aborted  
\> done  
\+  
Connection closed

3. NEW on old file
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> stor NEW ./server1.txt  
-File exists, but system doesn't support generations    
\> done   
\+  
Connection closed

4. NEW on new file
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> stor NEW ./client1.txt  
+File does not exist, will create new file    
\> size 1  
+ok, waiting for file  
+Saved /mnt/a/Programming/725a1/build/resources/Server/./client1.txt  
\+  
Connection closed

5. OLD on old file
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> stor OLD ./server1.txt  
+Will write over old file    
\> send 1   
+ok, waiting for file  
+Saved /mnt/a/Programming/725a1/build/resources/Server/./server1.txt  
\+  
Connection closed

6. OLD on new file
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> stor OLD ./client1.txt  
+Will create new file    
\> send 1   
+ok, waiting for file  
+Saved /mnt/a/Programming/725a1/build/resources/Server/./client1.txt  
\+  
Connection closed

7. APP on old file
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> stor APP ./server1.txt  
+Will append to file    
\> send 1   
+ok, waiting for file  
+Saved /mnt/a/Programming/725a1/build/resources/Server/./server1.txt  
\+  
Connection closed

8. APP on old file
>+thol600-server SFTP Service  
\> user 1  
!1 logged in  
\> stor APP ./client1.txt  
+Will create file    
\> send 1   
+ok, waiting for file  
+Saved /mnt/a/Programming/725a1/build/resources/Server/./server1.txt  
\+  
Connection closed
