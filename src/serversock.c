#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <signal.h>
#include <process.h>

#define BACKLOG 5
#define PORT "1337"
#define BUFFER_SIZE 100
#define MESSAGE_BUFFER 512
#define HOST NULL
#define POLL_SOCKET_AMNT 1

const int poll_timeout = 2500; // 2.5 seconds
const int thread_timeout = 60000; // one minute

void handle_sigint(int sig);
void *get_sockaddr_in(struct sockaddr *sa);
SOCKET new_lisock(void);
LPTSTR Messageformat(int message_id);
unsigned int WINAPI accept_worker(void *arg);

int main(void)
{    
    signal(SIGINT, handle_sigint);

    WSADATA wsa_data;

    int WSAstatus, poll_status;
    int err;
    LPTSTR errstring;

    SOCKET listen_sock, con_sock;

    LPDWORD thread_id;
    HANDLE h_thread;

    struct pollfd sock_array[POLL_SOCKET_AMNT];
    
    WSAstatus = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (WSAstatus != 0)
    {
        fprintf(stderr, "WSA Error\n");
        exit(-1);
    }
    
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        fprintf(stderr, "Version 2.2 of Winsock not available.\n");
        WSACleanup();
        exit(-1);
    }
    
    listen_sock = new_lisock();
    if (listen_sock == SOCKET_ERROR)
    {
        fprintf(stderr, "socket error, shutting down...\n");
        WSACleanup();
        exit(-1);
    }

    while (1)
    {
        // polling socket
        sock_array[0].fd = listen_sock; // assinging our listening socket
        sock_array[0].events = POLLIN; // Tell me when ready to read
        
        poll_status = WSAPoll(sock_array, POLL_SOCKET_AMNT, poll_timeout);

        if (poll_status == SOCKET_ERROR) {
            int i_err = WSAGetLastError();
            char *str_err = Messageformat(i_err); 
            fprintf(stderr, "POLLING ERROR: %s\n", str_err);
            break;
        };
        if (poll_status == 0)
        {
            fprintf(stderr, "poll time out, trying again..\n");
            continue;
        }
        if (poll_status > 0)
        {
            fprintf(stderr, "poll triggered!\n");
            h_thread = (HANDLE) _beginthreadex(NULL, 0, accept_worker, &(sock_array[0].fd), 0, NULL);
            if (h_thread == 0)
            {
                fprintf(stderr,"THREAD CREATION ERROR.\n");
                closesocket(sock_array[0].fd);
                break;
            }
        }
    }

    WaitForSingleObject(h_thread, INFINITE);
    CloseHandle(h_thread);
    
    // cleanup
    fprintf(stderr,"closing listening socket...\n");
    if (closesocket(listen_sock) == SOCKET_ERROR)
    {
        err = WSAGetLastError();
        errstring = Messageformat(err);
        fprintf(stderr, "LISTEN SOCK CLOSE ERROR: %s\n", errstring);
        LocalFree(errstring);
    }
    fprintf(stderr,"exiting...\n");
    WSACleanup();
    return 0;
}

void handle_sigint(int sig)
{
    printf("\nExiting...\n");
    WSACleanup();

    exit(0); // exit program
}

void *get_sockaddr_in(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

SOCKET new_lisock(void)
{
    int err, status_code;
    int yes = 1;
    u_long mode = 1;
    LPTSTR errstring;
    char ip_buffer[14];

    struct addrinfo *addr_res = NULL;
    struct addrinfo hint, *ptr;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    SOCKET listen_sock;


    // receiving IP addresses
    if ((status_code = getaddrinfo(HOST, PORT, &hint, &addr_res)) != 0 )
    {
        err = WSAGetLastError();
        errstring = Messageformat(err);
        fprintf(stderr, "ADDRESS ERROR: %s\n", errstring);
        LocalFree(errstring);
        return SOCKET_ERROR;
    };

    // loop over available ip addresses from getaddrfino()
    for (ptr = addr_res; ptr != NULL; ptr = ptr->ai_next)
    {   
        // creating socket
        if ((listen_sock = socket(addr_res->ai_family, addr_res->ai_socktype, addr_res->ai_protocol))
            == INVALID_SOCKET)
            {
                err = WSAGetLastError();
                errstring = Messageformat(err);
                fprintf(stderr, "SOCKET BIND ERROR: %s\n", errstring);
                LocalFree(errstring);
                continue;
            }
        // configures socket to be reusable 
        fprintf(stderr, "socket created!\n");
        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes))
            == INVALID_SOCKET)
            {
                err = WSAGetLastError();
                errstring = Messageformat(err);
                fprintf(stderr, "SOCKET OPT ERROR: %s\n", errstring);
                LocalFree(errstring);
                freeaddrinfo(addr_res);
                continue;
            }
        // // sets socket to non blocking
        // if (ioctlsocket(listen_sock, FIONBIO, &mode)
        //     == SOCKET_ERROR)
        //     {
        //         err = WSAGetLastError();
        //         errstring = Messageformat(err);
        //         fprintf(stderr, "SOCKET CONFIG ERROR: %s\n", errstring);
        //         LocalFree(errstring);
        //         freeaddrinfo(addr_res);
        //         continue;
        //     }
        // binding socket
        if (bind(listen_sock, addr_res->ai_addr, addr_res->ai_addrlen)
            == SOCKET_ERROR)
            {
                err = WSAGetLastError();
                errstring = Messageformat(err);
                fprintf(stderr, "SOCKET BIND ERROR: %s\n", errstring);
                LocalFree(errstring);
                continue;
            }
        break;
    } 
    
    if (ptr == NULL)
    {
        freeaddrinfo(addr_res);
        fprintf(stderr, "SOCKET ERROR\n");
        return SOCKET_ERROR;
    }

    void* addr = get_sockaddr_in(addr_res->ai_addr);
    LPCSTR ip = inet_ntop(addr_res->ai_family, addr, ip_buffer, sizeof ip_buffer);
    if (ip == NULL)
    {
        err = WSAGetLastError();
        errstring = Messageformat(err);
        fprintf(stderr, "IP PRINT ERROR: %s\n", errstring);
    }
    fprintf(stderr, "Hosting on:\nip: %s on port: %s\n", ip, PORT);


    // listening on socket
    status_code = listen(listen_sock, BACKLOG);
    if (status_code == SOCKET_ERROR)
    {
        err = WSAGetLastError();
        errstring = Messageformat(err);
        fprintf(stderr, "SOCKET LISTEN ERROR: %s\n", errstring);
        LocalFree(errstring);
        freeaddrinfo(addr_res);
        return SOCKET_ERROR;
    }
    fprintf(stderr, "listening...(cancel with CTRL+C)\n");

    freeaddrinfo(addr_res);
    return listen_sock;
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

unsigned int WINAPI accept_worker(void *arg)
{
    printf("thread start...\n");
    char *str_err;
    char *recv_buffer = malloc(BUFFER_SIZE * sizeof(char));
    int i_err, status, packets_received;
    SOCKET *listen_sock = (SOCKET *) arg;
    SOCKET con_sock;
    struct sockaddr_storage client_addr;
    char s[INET6_ADDRSTRLEN];

    // accept connections
    socklen_t addrlen = sizeof(client_addr);
    con_sock = accept(*listen_sock, (struct sockaddr*) &client_addr, &addrlen);
    if (con_sock == INVALID_SOCKET)
    {
        i_err = WSAGetLastError();
        str_err = Messageformat(i_err);
        fprintf(stderr, "SOCKET ACCEPT ERROR: %s\n", str_err);
        LocalFree(str_err);
        return -1;
    }

    inet_ntop(client_addr.ss_family, (struct sockaddr*) &client_addr, s, sizeof(s));
    fprintf(stderr,"connected to: %s\n", s);

    do {
        fprintf(stderr, "Waiting for data...\n");
        packets_received = recv(con_sock, recv_buffer, BUFFER_SIZE, 0);
        if (packets_received > 0)
        {
            fprintf(stderr, "Receiving data...%i bytes\n", packets_received);
        }
        else if (packets_received == 0)
        {
            fprintf(stderr, "Connection closed...\n");
        }
        else
        {
            i_err = WSAGetLastError();
            str_err = Messageformat(i_err);
            fprintf(stderr, "RECEIVE ERROR: %s, %i\n", str_err, i_err);
            LocalFree(str_err);
            free(recv_buffer);
            return -1;
        }
    } while (packets_received > 0);

    
    fprintf(stderr, "Data received:\n%s\n", recv_buffer);
    free(recv_buffer);
    closesocket(con_sock); // No longer needed
    return 0;
}