#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <signal.h>

#define BACKLOG 5
#define PORT "1337"
#define BUFFER_SIZE 512
#define MESSAGE_BUFFER 100
#define HOST NULL

void handle_sigint(int sig);
LPTSTR Messageformat(int message_id);

int main(void)
{    
    WSADATA wsa_data;
    int WSAstatus, status_code;
    signal(SIGINT, handle_sigint);
    int err;
    LPTSTR errstring;
    
    WSAstatus = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (WSAstatus != 0)
    {
        fprintf(stderr,"Error\n");
        return 1;
    }
    
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        fprintf(stderr,"Version 2.2 of Winsock not available.\n");
        WSACleanup();
        exit(1);
    }
    
    struct addrinfo *addr_res = NULL;
    struct addrinfo hint;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    
    // receiving IP 
    if ((status_code = getaddrinfo(HOST, PORT, &hint, &addr_res)) != 0 )
    {
        fprintf(stderr, "ADDRINFO ERROR: %s\n", Messageformat(WSAGetLastError()));
        WSACleanup();
        freeaddrinfo(addr_res);
        exit(1);
    };

    char ip_buffer[14];
    void* addr = &(((struct sockaddr_in *)addr_res->ai_addr)->sin_addr);
    LPCSTR ip = inet_ntop(addr_res->ai_family, addr, ip_buffer, sizeof ip_buffer);
    if (ip == NULL)
    {
        fprintf(stderr,"ADDRESS ERROR: %s\n", Messageformat(WSAGetLastError()));
    }
    fprintf(stderr,"address info received, connecting to:\n");
    fprintf(stderr,"ip: %s on port: %s\n", ip, PORT);

    // creating socket
    SOCKET sockd = socket(addr_res->ai_family, addr_res->ai_socktype, addr_res->ai_protocol);
    if (sockd == INVALID_SOCKET)
    {
        fprintf(stderr, "SOCKET OPEN ERROR: %s\n", Messageformat(WSAGetLastError()));
        exit(1);
    }
    fprintf(stderr,"socket created!\n");
    fprintf(stderr,"connecting...\n");
    
    // connect to server
    status_code = connect(sockd, addr_res->ai_addr, addr_res->ai_addrlen);
    if (status_code == SOCKET_ERROR)
    {
        fprintf(stderr, "SOCKET CONNECT ERROR: %s\n", Messageformat(WSAGetLastError()));
        closesocket(sockd);
        WSACleanup();
        freeaddrinfo(addr_res);
        exit(1);
    }

    // sending message
    fprintf(stderr,"connected!\n");
    fprintf(stderr,"press enter so send message...\n");
    getchar();


    char *msg = "hello from the other side!";
    int len = strlen(msg);
    int bytes_sent;
    fprintf(stderr,"sending: %s\n", msg);
    bytes_sent = send(sockd, msg, len, MSG_OOB);
    if (bytes_sent == 0)
    {
        fprintf(stderr, "SEND ERROR: %s\n", Messageformat(WSAGetLastError()));
        closesocket(sockd);
        WSACleanup();
        freeaddrinfo(addr_res);
        exit(1);
    }
    fprintf(stderr,"message sent, exiting...\n");
    
    // cleanup
    status_code = closesocket(sockd);
    if (status_code == SOCKET_ERROR)
    {
        fprintf(stderr, "SOCKET CLOSE ERROR: %s\n", Messageformat(WSAGetLastError()));
        WSACleanup();
        freeaddrinfo(addr_res);
        exit(1);
    }
    WSACleanup();
    freeaddrinfo(addr_res);
    return 0;

}

void handle_sigint(int sig) {
    printf("\nExiting...\n");
    WSACleanup();

    exit(0); // exit program
}



LPTSTR Messageformat(int message_id)
{
    LPTSTR formatted_message = NULL;
    if (FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM     |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        message_id,
                        0,
                        (LPTSTR)&formatted_message,
                        MESSAGE_BUFFER,
                        NULL
        ) == 0)
    {
        fprintf(stderr, "Message Format Error\n");
        exit(1);
    }
    return formatted_message;
}