#include <stdio.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <string.h>
#include "utils.h"
#define BUFF_LEN 2048
#define true 1 
#define false 0 

typedef struct __attribute__((packed)){
    uint32_t len; 
    char* payload; 
} message; 


int resolve_request(int fd, message* msg) {
    if(full_read(fd, (char*)msg, sizeof(int32_t))) {
        return -1; 
    }
    char buff[BUFF_LEN]; 
    if(full_read(fd, buff, msg->len)) {
        return -1; 
    }
    msg->payload = malloc(msg->len);
    strcpy(msg->payload, buff);  
    return 0; 
} 

int serve_client(int client_sock_fd) {
    message msg = {};
    if(resolve_request(client_sock_fd, &msg)) {
        printf("voa: something went wrong during parsing request\n"); 
        char* err = "please, send a proper request following protocol";
        full_write(client_sock_fd, err, strlen(err)+1);
        return -1; 
    }
    full_write(client_sock_fd, (char*)&msg, sizeof(uint32_t));
    full_write(client_sock_fd, msg.payload, msg.len);
    printf("voa: got message %s\n", msg.payload); 
    free(msg.payload); 
    return 0; 
}

void accept_requests(int listening_sock_fd) {
    while(true) {

        struct sockaddr_in sockaddr = {}; 
        socklen_t len = sizeof(sockaddr); 

        int client_fd = accept(listening_sock_fd, 
            (struct sockaddr*)&sockaddr, &len);

        if(client_fd < 0) {
            continue; 
        }

        while(true) {
            int32_t cs = serve_client(client_fd); 
            if(!cs) {
                break; 
            }
        }

        close(client_fd); 
    }
}

int main() {
    // create raw socket  
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int val = 1; 
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(1234); 
    addr.sin_addr.s_addr = htonl(0);
    // bind socket to address 0.0.0.0/1234
    int rv = bind(fd, 
        (const struct sockaddr*)&addr, sizeof(addr)); 
    if (rv) {
        die("bind()"); 
    } 

    // socket is now listening for packets on port 1234 
    rv = listen(fd, SOMAXCONN); 
    
    if (rv) {
        die("listen()");
    }
    printf("voa: starting listening for requests\n");
    accept_requests(fd); 

    return 0; 
}