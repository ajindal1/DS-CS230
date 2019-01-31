// server program

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define BACKLOG 10
#define MYPORT "3490"

int main( int argc, char *argv[] ) {
    struct sockaddr_storage their_addr;
    struct addrinfo hints, *servinfo, *p;
    int status;
    socklen_t addr_size;
    char ipstr[ INET_ADDRSTRLEN ];

    memset( &hints, 0, sizeof hints );
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM; // use TCP stream sockets
    hints.ai_flags = AI_PASSIVE;

    if ( ( status = getaddrinfo( NULL, "3490", &hints, &servinfo ) ) != 0 ) {
        fprintf( stderr, "getaddrinfo error: %s\n", gai_strerror( status ) );
        exit( 1 );
    }

    int sockfd;
    sockfd = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol );

    bind( sockfd, servinfo->ai_addr, servinfo->ai_addrlen );

    freeaddrinfo( servinfo );

    listen( sockfd, BACKLOG );

    addr_size = sizeof their_addr;
    int new_fd;
    new_fd = accept( sockfd, ( struct sockaddr* )&their_addr, &addr_size );

    char *msg = "Winter is Coming";
    int len, bytes_sent;

    len = strlen( msg );
    bytes_sent = send( new_fd, msg, len, 0 );
    if ( len != bytes_sent ) {
        fprintf( stderr, "send unable to sent all bytes: %s\n", strerror( errno ) );
        exit( 1 );
    }

    close( sockfd );
}
