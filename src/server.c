#include <stdio.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <stdlib.h> 
#define BUFF_LEN 2048

void die(char* str) {
    printf("voa: error %s\n", str); 
    exit(-1); 
}

//simple echo 
void serve_client(int client_sock_fd) {
    char buffer[BUFF_LEN]; 
    int num_read = read(client_sock_fd, buffer, BUFF_LEN);
    printf("voa: client sent \"%s\"\n", buffer); 
    write(client_sock_fd, buffer, num_read); 
}

void accept_requests(int listening_sock_fd) {
    while(1) {

        struct sockaddr_in sockaddr = {}; 
        int len = sizeof(sockaddr); 

        int client_fd = accept(listening_sock_fd, 
            (struct sockaddr*)&sockaddr, &len);

        if(client_fd < 0) {
            continue; 
        }

        serve_client(client_fd); 

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