#include <stdio.h>
#include <winsock2.h>
#include <signal.h>
#include <unistd.h>
#include <windows.h>

SOCKET s;

void clean_up() {
    closesocket(s);
    WSACleanup();
    printf("server closed\n");
}

BOOL CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        // Handle the CTRL-C signal.
        case CTRL_C_EVENT:
            printf("Ctrl-C event\n\n");
            Beep(750, 300);
            exit(0);
        default:
            break;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    atexit(clean_up);
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE)) {
    }
    else {
        printf("\nERROR: Could not set control handler");
        return 1;
    }
    WSADATA wsa;
    SOCKET new_socket;
    struct sockaddr_in server, client;
    int c;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");

    //Create a socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    printf("Socket created.\n");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    //Bind
    if (bind(s, (struct sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
    }

    puts("Bind done");


    //Listen to incoming connections
    listen(s, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");

    c = sizeof(struct sockaddr_in);
    char *message;

    while ((new_socket = accept(s, (struct sockaddr *) &client, &c)) != INVALID_SOCKET) {
        puts("Connection accepted");
        int recv_size;
        char server_reply[2000];
        if ((recv_size = recv(new_socket, server_reply, 2000, 0)) == SOCKET_ERROR) {
            puts("recv failed");
            int error_code = WSAGetLastError();
            printf("%d\n", error_code);
        }
        server_reply[recv_size] = '\0';
        puts(server_reply);
        //Reply to the client
        message = "Hello Client , I have received your connection. But I have to go now, bye\n";
        send(new_socket, message, strlen(message), 0);
    }

    if (new_socket == INVALID_SOCKET) {
        printf("accept failed with error code : %d", WSAGetLastError());
        return 1;
    }

    closesocket(s);
    return 0;
}