#include <stdio.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <stdlib.h>
#include <string.h>  
#include<unistd.h> 
#include "utils.h"
#include <pthread.h> 

#define BUFF_LEN 2048
#define TEST_LOAD 1000


void* run(void* args) {
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

    char buffer[BUFF_LEN]; 

    snprintf(buffer, BUFF_LEN, "hello from socket with fd %d", sock_fd);  

    write(sock_fd, buffer, strlen(buffer) + 1);
    printf("write complete! socket fd: %d\n", sock_fd);
    return NULL;      
}

int main() {
    pthread_t threads[TEST_LOAD]; 
    for(int i = 0; i < TEST_LOAD; i++) {
        pthread_create(&threads[i], NULL, run, NULL);
    }
    for(int i = 0; i < TEST_LOAD; i++) {
        pthread_join(threads[i], NULL); 
    }
    return 0; 
}

