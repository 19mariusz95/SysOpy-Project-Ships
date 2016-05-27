//
// Created by Mariusz on 04.05.2016.
//
#include <stdio.h>
#include <winsock2.h>
#include <unistd.h>
#include "main.h"

SOCKET s;

struct result {
    char nick[50];
    int moves;
};

struct result results[10];

int get_free_id();

int set_ship(int id, int a, int b, int c, int d);

int attack(int id, int a, int b);

void init_results();

void save(int id, int moves);

int board[2][100];
int ships[2][3] = {{2, 2, 2},
                   {2, 2, 2}};
int id[MAX_IDS];
char nick[2][50];
int ready[2] = {-1, -1};
int wait[2] = {0, WAIT};
int alive[2] = {12, 12};
int moves[2] = {0, 0};

void clean_up() {
    closesocket(s);
    WSACleanup();
    printf("server closed\n");
}

DWORD WINAPI ThreadFunc(void *data) {
    SOCKET new_socket = *((SOCKET *) data);
    char *message;
    int recv_size;
    char client_request[2000];
    int cl_id;
    int a, b, c, d;
    while ((recv_size = recv(new_socket, client_request, 2000, 0)) != SOCKET_ERROR) {
        client_request[recv_size] = '\0';
        switch (client_request[0]) {
            case LOGIN: {
                int free_id = get_free_id();
                message = malloc(2 * sizeof(char));
                message[0] = (char) (free_id);
                if (free_id != -1) {
                    id[free_id] = 1;
                }
                message[1] = '\0';
                send(new_socket, message, 1, 0);
                if (free_id != -1) {
                    ready[free_id] = 0;
                    ready[free_id] = 0;
                    for (int i = 0; i < 100; i++)
                        board[free_id][i] = ' ';
                    alive[free_id] = 12;
                    ships[free_id][0] = ships[free_id][1] = ships[free_id][2] = 2;
                    moves[free_id] = 0;
                    strncpy(nick[free_id], client_request + 1, 50);
                    printf("%s\n", nick[free_id]);
                }
                free(message);
                break;
            }
            case GETBOARD:
                cl_id = client_request[1];
                message = malloc(201 * sizeof(char));
                for (int j = 0; j < 2; j++) {
                    for (int i = 0; i < 100; i++) {
                        char tmp = (char) board[j][i];
                        if (j != cl_id && tmp == 's')
                            tmp = ' ';
                        message[i + j * 100] = tmp;
                    }
                }
                message[200] = '\0';
                send(new_socket, message, 200, 0);
                free(message);
                break;
            case SETSHIP:
                cl_id = client_request[1];
                a = (client_request[2]);
                b = (client_request[3]);
                c = client_request[4];
                d = client_request[5];
                int res = set_ship(cl_id, a, b, c, d);
                message = malloc(2 * sizeof(char));
                message[0] = (char) res;
                message[1] = '\0';
                send(new_socket, message, 1, 0);
                free(message);
                break;
            case LOGOUT:
                cl_id = client_request[1];
                id[cl_id] = -1;
                ready[cl_id] = -1;
                break;
            case SETREADY:
                cl_id = client_request[1];
                ready[cl_id] = 1;
                break;
            case WAIT:
                cl_id = client_request[1];
                message = malloc(2 * sizeof(char));
                message[0] = alive[cl_id] == 0 ? (char) LOST : (char) wait[cl_id];
                message[1] = '\0';
                send(new_socket, message, 1, 0);
                free(message);
                break;
            case ISREADY:
                cl_id = client_request[1];
                ready[cl_id] = 1;
                message = malloc(2 * sizeof(char));
                message[0] = (char) ready[(cl_id + 1) % 2];
                message[1] = '\0';
                send(new_socket, message, 1, 0);
                free(message);
                break;
            case ATTACK:
                cl_id = client_request[1];
                a = (client_request[2]);
                b = (client_request[3]);
                message = malloc(10 * sizeof(char));
                char result = (char) attack(cl_id, a, b);
                message[0] = result;
                if (result == WIN) {
                    sprintf(message + 1, "%d", moves[cl_id]);
                }
                message[9] = '\0';
                send(new_socket, message, 9, 0);
                free(message);
                break;
            case SAVE:
                cl_id = client_request[1];
                save(cl_id, moves[cl_id]);
                break;
            case GETHISTORY:
                message = calloc(1000, sizeof(char));
                for (int i = 0; i < 10; i++) {
                    sprintf(message + strlen(message), "%s %d\n", results[i].nick, results[i].moves);
                }
                send(new_socket, message, strlen(message), 0);
                free(message);
                break;
            case ISOPP:
                message = calloc(2000, sizeof(char));
                cl_id = client_request[1];
                message[0] = id[(cl_id + 1) % MAX_IDS] > 0 ? (char) 1 : (char) 0;
                message[1] = '\0';
                send(new_socket, message, 1, 0);
                free(message);
            default: {
                break;
            }
        }
    }
    return 0;
}

void save(int id, int moves) {
    for (int i = 9; i > 0; i--) {
        results[i].moves = results[i - 1].moves;
        strcpy(results[i].nick, results[i - 1].nick);
    }
    strcpy(results[0].nick, nick[id]);
    results[0].moves = moves;
}

//a - column b - row
int attack(int id, int a, int b) {
    if (a < 0 || a > 9 || b < 0 || b > 9)
        return AGAIN;
    moves[id]++;
    int tmp = board[(id + 1) % 2][b * 10 + a];
    wait[id] = WAIT;
    wait[(id + 1) % 2] = 0;
    if (tmp == 's') {
        alive[(id + 1) % 2]--;
        board[(id + 1) % 2][b * 10 + a] = 'x';
        return alive[(id + 1) % 2] == 0 ? WIN : HIT;
    }
    else if (tmp == ' ') {
        board[(id + 1) % 2][b * 10 + a] = 'o';
    } else if (tmp == 'x' || tmp == 'o') {
        return AGAIN;
    }
    return 0;
}

//c - length d - position a - column b - row
int set_ship(int id, int a, int b, int c, int d) {
    if (c < 1 || c > 3 || ships[id][c - 1] < 1 || a < 0 || a > 9 || b < 0 || b > 9 || d < 0 || d > 1) {
        return 0;
    }
    int tab[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    if (d == 0) {
        if (a + c > 10)
            return 0;
        for (int i = a; i < a + c; i++) {
            if (board[id][b * 10 + i] != ' ')
                return 0;
            for (int j = 0; j < 8; j++) {
                int tmp = (b + tab[(j + 6) % 8]) * 10 + i + tab[j];
                if (tmp >= 0 && tmp < 100 && board[id][(b + tab[(j + 6) % 8]) * 10 + i + tab[j]] != ' ') {
                    return 0;
                }
            }
        }
        for (int i = a; i < a + c; i++) {
            board[id][b * 10 + i] = 's';
        }
        ships[id][c - 1]--;
        return 1;
    } else if (d == 1) {
        if (b + c > 10)
            return 0;
        for (int i = b; i < b + c; i++) {
            if (board[id][i * 10 + a] != ' ') {
                return 0;
            }
            for (int j = 0; j < 8; j++) {
                int tmp = (i + tab[(j + 6) % 8]) * 10 + a + tab[j];
                if (tmp >= 0 && tmp < 100 && board[id][tmp] != ' ')
                    return 0;
            }
        }
        for (int i = b; i < b + c; i++) {
            board[id][i * 10 + a] = 's';
        }
        ships[id][c - 1]--;
        return 1;
    }
    return 0;
}

int get_free_id() {
    for (int i = 0; i < MAX_IDS; i++) {
        if (id[i] - 1)
            return i;
    }
    return -1;
}

BOOL CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:
            Beep(750, 300);
            exit(0);
        default:
            break;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    atexit(clean_up);
    init_results();
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 100; i++)
            board[j][i] = ' ';
    }
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE)) {
        printf("\nERROR: Could not set control handler");
        return 1;
    }
    WSADATA wsa;
    SOCKET new_socket;
    struct sockaddr_in server, client;
    int c;
    id[0] = id[1] = -1;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    printf("Initialised.\n");
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }
    printf("Socket created.\n");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);
    if (bind(s, (struct sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
    }
    puts("Bind done");
    listen(s, 3);
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((new_socket = accept(s, (struct sockaddr *) &client, &c)) != INVALID_SOCKET) {
        puts("Connection accepted");
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadFunc, &new_socket, 0, NULL);
    }
    return 0;
}

void init_results() {
    for (int i = 0; i < 10; i++) {
        results[i].moves = -1;
        strcpy(results[i].nick, "noname");
    }
}