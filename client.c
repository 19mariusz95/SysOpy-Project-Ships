//
// Created by Mariusz on 04.05.2016.
//


#include <stdio.h>
#include<stdio.h>
#include<winsock2.h>
#include <stdbool.h>
#include "main.h"

SOCKET s;
WSADATA wsa;
struct sockaddr_in server;
int id;

void draw_board();

void init();

void login();

void logout();

int set_ship();

void wait_for_opponent();

void send_ready();

int wait();

int attack();

BOOL CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:
            exit(0);
        default:
            break;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    atexit(logout);
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE)) {
        printf("\nERROR: Could not set control handler");
        return 1;
    }
    init();
    login();
    for (int i = 0; i < 6; i++) {
        draw_board();
        while (set_ship() != 1) {
            printf("bad args\n");
        }
    }
    draw_board();
    wait_for_opponent();
    printf("Ready to play a game.\n");
    draw_board();
    while (1) {
        int res;
        while ((res = wait()) != 0);
        if (res == LOST) {
            printf("Przegrales gre\n");
            break;
        }
        draw_board();
        while ((res = attack()) == AGAIN);
        switch (res) {
            case WIN:
                printf("Wygrales gre!!!\n");
                return 0;
            case HIT:
                printf("trafiony\n");
            default:
                break;
        }
        draw_board();
    }
}

int attack() {
    char message[10];
    message[0] = ATTACK;
    message[1] = (char) id;
    printf("Wprowadz namiar w formacie L N ; L-litera N-cyfra\n");
    char a;
    int b;
    scanf("%c %d", &a, &b);
    fflush(stdin);
    message[2] = (char) (a - 65);
    message[3] = (char) (b - 1);
    if (send(s, message, 4, 0) < 0) {
        puts("Send failed");
        return 0;
    }
    int recv_size;
    char string[2000];
    if ((recv_size = recv(s, string, 2000, 0)) == SOCKET_ERROR) {
        puts("recv failed");
    }
    string[recv_size] = '\0';
    int req = string[0];
    return req;
}

int wait() {
    char message[10];
    message[0] = WAIT;
    message[1] = (char) id;
    if (send(s, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return 1;
    }

    int recv_size;
    char string[2000];
    if ((recv_size = recv(s, string, 2000, 0)) == SOCKET_ERROR) {
        puts("recv failed");
    }
    string[recv_size] = '\0';
    return (int) string[0];
}

void wait_for_opponent() {
    send_ready();
    int res = -1;
    printf("waiting for opponent ...\n");
    do {
        if (res != -1) {
            Sleep(1000);
        }
        char message[10];
        message[0] = ISREADY;
        message[1] = (char) id;
        if (send(s, message, strlen(message), 0) < 0) {
            puts("Send failed");
            return;
        }

        int recv_size;
        char string[2000];
        if ((recv_size = recv(s, string, 2000, 0)) == SOCKET_ERROR) {
            puts("recv failed");
        }
        string[recv_size] = '\0';
        res = (int) string[0];
    } while (res == 0);
}

void send_ready() {
    char message[10];
    message[0] = SETREADY;
    message[1] = (char) id;
    if (send(s, message, 2, 0) < 0) {
        puts("Send failed");
        return;
    }
}

int set_ship() {
    char message[10];
    message[0] = SETSHIP;
    message[1] = (char) id;
    printf("Wprowadz namiar w formacie L N D P L-litera N-cyfra D - dlugosc P - 0-poziom 1-pion\n");
    char a;
    int b;
    int d;
    int p;
    scanf("%c %d %d %d", &a, &b, &d, &p);
    fflush(stdin);
    message[2] = (char) (a - 65);
    message[3] = (char) (b - 1);
    message[4] = (char) d;
    message[5] = (char) p;
    if (send(s, message, 6, 0) < 0) {
        puts("Send failed");
        return 0;
    }
    int recv_size;
    char string[2000];
    if ((recv_size = recv(s, string, 2000, 0)) == SOCKET_ERROR) {
        puts("recv failed");
    }
    string[recv_size] = '\0';
    int req = string[0];
    return req;
}

void login() {
    char message[10];
    message[0] = LOGIN;
    if (send(s, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return;
    }

    int recv_size;
    char string[2000];
    if ((recv_size = recv(s, string, 2000, 0)) == SOCKET_ERROR) {
        puts("recv failed");
    }
    string[recv_size] = '\0';
    id = (int) string[0];
    if (id == -1) {
        puts("Server rejected you");
        exit(2);
    }
    printf("Your id is %d\n", id);
}

void logout() {
    char message[10];
    message[0] = LOGOUT;
    message[1] = (char) id;
    if (send(s, message, 2, 0) < 0) {
        puts("Send failed");
        return;
    }
}

void init() {
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        exit(1);
    }

    printf("Initialised.\n");

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    printf("Socket created.\n");


    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0) {
        puts("connect error");
        exit(1);
    }

    puts("Connected");
}

void draw_board() {

    char message[10];
    message[0] = GETBOARD;
    message[1] = (char) id;
    if (send(s, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return;
    }

    int recv_size;
    char string[2000];
    if ((recv_size = recv(s, string, 2000, 0)) == SOCKET_ERROR) {
        puts("recv failed");
    }
    string[recv_size] = '\0';

    printf("%15s %30s\n", "client 0", "client 1");
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
                printf("%c ", string[k * 100 + j + 10 * i]);
            }
            printf("    ");
        }
        printf("\n");
    }
}

