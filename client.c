//
// Created by Mariusz on 04.05.2016.
//
#include <stdio.h>
#include <winsock2.h>
#include "main.h"

SOCKET s;
WSADATA wsa;
struct sockaddr_in server;
int id;
char board[2000];

void draw_board();

void init(char *ip, int port);

void login(char string[50]);

void logout();

int set_ship();

void wait_for_opponent(char msg);

int wait();

void attack(char *string);

void save();

void show_history();

int the_same(char *string);

BOOL CtrlHandler(DWORD type) {
    switch (type) {
        case CTRL_C_EVENT:
            exit(0);
        default:
            break;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("You have to specify ip address and port in arguments\n");
        exit(1);
    }
    int port = atoi(argv[2]);
    if (port == 0) {
        printf("Wrong second argument\n");
        exit(1);
    }
    atexit(logout);
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE)) {
        printf("\nERROR: Could not set control handler");
        return 1;
    }
    board[0] = '\0';
    init(argv[1], port);
    printf("Play a game or show history [1/2]\n");
    int option;
    scanf("%d", &option);
    if (option == 2) {
        show_history();
        return 0;
    }
    char nick[50];
    printf("Enter your nick\n");
    scanf("%s", nick);
    fflush(stdin);
    login(nick);
    wait_for_opponent(ISOPP);
    for (int i = 0; i < 6; i++) {
        draw_board();
        while (set_ship() != 1) {
            printf("bad args\n");
        }
    }
    draw_board();
    wait_for_opponent(ISREADY);
    printf("Ready to play a game.\n");
    draw_board();
    while (1) {
        int res;
        while ((res = wait()) == WAIT) {
            draw_board();
            Sleep(1000);
        }
        if (res == LOST) {
            printf("Przegrales gre\n");
            break;
        } else if (res == -1) {
            printf("Przeciwnik odlaczyl sie\n");
            exit(0);
        }
        draw_board();
        char tmp[2000];
        do {
            attack(tmp);
            res = tmp[0];
        } while (res == AGAIN);
        switch (res) {
            case WIN:
                draw_board();
                printf("Wygrales gre!!!\n");
                int moves;
                sscanf(tmp + 1, "%d", &moves);
                printf("Ilosc ruchow: %d\n", moves);
                printf("Chcesz zapisac ten wynik? [y/n]\n");
                char c;
                scanf("%c", &c);
                if (c == 'y') {
                    save();
                }
                return 0;
            case HIT:
                printf("trafiony\n");
                break;
            case SUNK:
                printf("trafiony - zatopiony\n");
                break;
            default:
                printf("pudlo\n");
                draw_board();
                break;
        }
    }
}

void show_history() {
    char message[10];
    message[0] = GETHISTORY;
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
    puts(string);
}

void save() {
    char message[10];
    message[0] = SAVE;
    message[1] = (char) id;
    if (send(s, message, 2, 0) < 0) {
        puts("Send failed");
        return;
    }
}

void attack(char *string) {
    char message[10];
    message[0] = ATTACK;
    message[1] = (char) id;
    printf("Wprowadz namiar w formacie L N ; L-litera N-cyfra\n");
    char a;
    int b;
    fflush(stdin);
    scanf("%c %d", &a, &b);
    fflush(stdin);
    message[2] = (char) (a - 65);
    message[3] = (char) (b - 1);
    if (send(s, message, 4, 0) < 0) {
        puts("Send failed");
        return;
    }
    int recv_size;
    if ((recv_size = recv(s, string, 2000, 0)) == SOCKET_ERROR) {
        puts("recv failed");
    }
    string[recv_size] = '\0';
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

void wait_for_opponent(char msg) {
    int res = -1;
    printf("waiting for opponent ...\n");
    do {
        if (res != -1) {
            Sleep(1000);
        }
        char message[10];
        message[0] = msg;
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
        if (res == -1) {
            printf("Opponent disconected\n");
            exit(0);
        }
    } while (res == 0);
}

int set_ship() {
    char message[10];
    message[0] = SETSHIP;
    message[1] = (char) id;
    printf("Wprowadz namiar w formacie L N D P | L-litera N-cyfra D - dlugosc P - 0-poziom 1-pion\n");
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

void login(char nick[50]) {
    char message[52];
    message[0] = LOGIN;
    strcpy(message + 1, nick);
    if (send(s, message, 52, 0) < 0) {
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

void init(char *ip, int port) {
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


    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons((u_short) port);

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

    if (the_same(string))
        return;
    for (int i = 0; i < 2000; i++)
        board[i] = string[i];
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

int the_same(char *string) {
    if (strcmp(board, string) == 0)
        return 1;
    return 0;
}
