//
// Created by Mariusz on 04.05.2016.
//


#include <stdio.h>
#include<stdio.h>
#include<winsock2.h>
#include "main.h"

SOCKET s;
WSADATA wsa;
struct sockaddr_in server;

void draw_board();

void init();

int main(int argc, char *argv[]) {

    init();
    draw_board();
    return 0;
}

void init() {
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        exit(1);
    }

    printf("Initialised.\n");

    //Create a socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    printf("Socket created.\n");


    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    //Connect to remote server
    if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
        puts("connect error");
        exit(1);
    }

    puts("Connected");
}

void draw_board() {

    char message[10];
    message[0] = GETBOARD;
    if (send(s, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return;
    }

    //Receive a reply from the server
    int recv_size;
    char string[2000];
    if ((recv_size = recv(s, string, 2000, 0)) == SOCKET_ERROR) {
        puts("recv failed");
    }

    puts("Reply received\n");

    //Add a NULL terminating character to make it a proper string before printing
    string[recv_size] = '\0';

    for (int k = 0; k < 2; k++) {
        printf("   ");
        for (int i = 0; i < 10; i++) {
            char a = (char) (65 + i);
            printf("%c ", a);
        }
        printf("    ");
    }
    printf("\n");
    for (int i = 0; i < 10; i++) {
        for (int k = 0; k < 2; k++) {
            printf("%2d ", i + 1);
            for (int j = 0; j < 10; j++) {
                printf("%d ", string[k * 100 + j + 10 * i]);
            }
            printf("    ");
        }
        printf("\n");
    }
}

