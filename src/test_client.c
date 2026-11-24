#include <stdio.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>  
#define BUFF_LEN 2048


int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr = {};
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(1234); 
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // connect socket to address localhost:1234
    int rv = connect(sock_fd, (const struct sockaddr*)&addr, sizeof(addr));

    if(rv < 0) {
        printf("client: connection refused\n"); 
        exit(-1); 
    }
    char buff[BUFF_LEN]; 
    strcpy(buff, "hello server!"); 
    write(sock_fd, buff, BUFF_LEN); 

    char rbuf[BUFF_LEN]; 

    int num_read = read(sock_fd, rbuf, BUFF_LEN);

    printf("client: %s\n", rbuf); 

    return 0; 
}

