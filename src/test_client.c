#include <stdio.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <stdlib.h>
#include <string.h>  
#include "utils.h"
#define BUFF_LEN 2048


void query(int fd, char* text) {
    char buff[BUFF_LEN]; 
    int len = strlen(text)+1; 
    memcpy(buff, &len, sizeof(uint32_t));
    strcpy(buff+sizeof(uint32_t), text);
    full_write(fd, buff, len + sizeof(uint32_t));

    char rbuf[BUFF_LEN]; 
    full_read(fd, rbuf, sizeof(uint32_t) + len); 
    printf("client: %s\n", rbuf + sizeof(uint32_t)); 
}

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
    query(sock_fd, "Hello voa!"); 
    return 0; 
}

