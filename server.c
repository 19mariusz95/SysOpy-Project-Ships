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
    new_socket = accept(s, (struct sockaddr *) &client, &c);
    if (new_socket == INVALID_SOCKET) {
        printf("accept failed with error code : %d", WSAGetLastError());
    }

    puts("Connection accepted");

    return 0;
}