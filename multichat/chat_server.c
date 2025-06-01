// chat_server.c

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Winsock ���̺귯��

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

SOCKET clients[MAX_CLIENTS];
int client_count = 0;
CRITICAL_SECTION cs; // ����ȭ�� ���� �Ӱ迵��

DWORD WINAPI ClientHandler(LPVOID client_sock_ptr);

// �޽��� ��ε�ĳ��Ʈ �Լ�
void Broadcast(const char* msg, SOCKET sender) {
    EnterCriticalSection(&cs);
    int i;
    for (i = 0; i < client_count; i++) {
        if (clients[i] != sender) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }
    LeaveCriticalSection(&cs);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    InitializeCriticalSection(&cs);

    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server = {0};
    server.sin_family = AF_INET;
    server.sin_port = htons(9000);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server, sizeof(server));
    listen(server_sock, 5);

    printf("���� ���� �� (��Ʈ 9000)...\n");

    while (1) {
        SOCKET client_sock;
        struct sockaddr_in client;
        int client_size = sizeof(client);
        client_sock = accept(server_sock, (struct sockaddr*)&client, &client_size);

        EnterCriticalSection(&cs);
        clients[client_count++] = client_sock;
        LeaveCriticalSection(&cs);

        CreateThread(NULL, 0, ClientHandler, (LPVOID)client_sock, 0, NULL);
    }

    DeleteCriticalSection(&cs);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}

// Ŭ���̾�Ʈ ó�� �Լ�
DWORD WINAPI ClientHandler(LPVOID client_sock_ptr) {
    SOCKET client_sock = (SOCKET)client_sock_ptr;
    char buffer[BUFFER_SIZE];

    while (1) {
        int len = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (len <= 0) {
            printf("Ŭ���̾�Ʈ ���� ����� (���� ��ȣ: %d)\n", client_sock); // ? �α� ��� �߰�
            break;
        }

        buffer[len] = '\0';
        Broadcast(buffer, client_sock);
    }

    // Ŭ���̾�Ʈ ���� ó��
    EnterCriticalSection(&cs);
    int i;
    for (i = 0; i < client_count; i++) {
        if (clients[i] == client_sock) {
            int j;
            for (j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    LeaveCriticalSection(&cs);

    closesocket(client_sock);
    return 0;
}

