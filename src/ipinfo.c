#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>


int main(int argc, char *argv[])
{    
    WSADATA wsa_data;
    int WSAstatus, status_code;
    
    if (argc != 2)
    {
        fprintf(stderr, "please provide a host name/address and port\n");
        exit(1);
    }
    
    WSAstatus = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (WSAstatus != 0)
    {
        printf("Error\n");
        return 1;
    }
    
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        fprintf(stderr,"Version 2.2 of Winsock not available.\n");
        WSACleanup();
        exit(1);
    }
    
    PCSTR host = argv[1];
    PCSTR port = "3490";

    struct addrinfo *addr_res = NULL;
    struct addrinfo hint;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_canonname = NULL;
    hint.ai_next = NULL;
    
    if ((status_code = getaddrinfo(host, port, &hint, &addr_res)) != 0 )
    {
        int error_code = WSAGetLastError();
        fprintf(stderr, "ADDRINFO ERROR, CODE: %i\n", error_code);
        exit(1);
    };

    printf("IP Address for %s\n", argv[1]);

    
    for (struct addrinfo *ptr = addr_res; ptr != NULL; ptr = ptr->ai_next)
    {
        // if (ptr->ai_family == AF_INET)
        // {
        //     char ip[INET_ADDRSTRLEN]; 
        //     inet_ntop(AF_INET, ptr->ai_addr->sa_data, ip, INET_ADDRSTRLEN);
        //     printf("the ip address is: %s\n", ip);
        // }
        void *addr;
        char *ipver;
        struct sockaddr_in *ipv4;
        struct sockaddr_in6 *ipv6;
        char ipstr[INET6_ADDRSTRLEN];
        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (ptr->ai_family == AF_INET) { // IPv4
            ipv4 = (struct sockaddr_in *)ptr->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            ipv6 = (struct sockaddr_in6 *)ptr->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(ptr->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    };
    
    WSACleanup();
    freeaddrinfo(addr_res);
    return 0;
}