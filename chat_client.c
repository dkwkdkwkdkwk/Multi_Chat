// chat_client.c

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

SOCKET sock;
char nickname[50];  // 닉네임 전역 저장

DWORD WINAPI ReceiveThread(LPVOID arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int len = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (len <= 0) break;

        buffer[len] = '\0';

        // 내 닉네임으로 시작하는 메시지는 무시
        if (strncmp(buffer, "[", 1) == 0 && strstr(buffer, nickname) == buffer + 1) {
            continue; // 내가 보낸 거니까 패스
        }

        printf("%s", buffer);
    }
    return 0;
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server = {0};
    server.sin_family = AF_INET;
    server.sin_port = htons(9000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("서버 연결 실패\n");
        return 1;
    }

    printf("닉네임을 입력하세요: ");
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = '\0';  // 개행 제거

    printf("서버에 연결됨. 채팅을 시작하세요!\n");

    CreateThread(NULL, 0, ReceiveThread, NULL, 0, NULL);

    char msg[BUFFER_SIZE];
    char send_buf[BUFFER_SIZE + 50];

    while (1) {
        fgets(msg, BUFFER_SIZE, stdin);
        msg[strcspn(msg, "\n")] = '\0'; // 개행 제거

        sprintf(send_buf, "[%s] %s\n", nickname, msg);
        send(sock, send_buf, strlen(send_buf), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

