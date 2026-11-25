#include <stdio.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <stdlib.h> 
#include "utils.h"
#include <fcntl.h> 
#include <errno.h> 
#include <stdbool.h>
#include <poll.h> 
#include "utils.h" 
#include <assert.h> 
//for now, it is limited 
#define MAX_CLIENTS 2048
#define MAX_MESSAGE_SIZE 1024
#define SERVER_TIMEOUT 45000

typedef struct {
  bool want_read; 
  bool want_write; 
  bool want_close; 

  uint8_t* incoming; 
  int incoming_len; 
  uint8_t* outgoing; 
  int outgoing_len; 
} client; 


void fd_set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0); 
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags); 
}

int32_t serve_client(int fd){
    return 0;
}

void init_client_data(client* c) {
    c->incoming = malloc(MAX_MESSAGE_SIZE); 
    c->incoming_len = 0;
    c->outgoing = malloc(MAX_MESSAGE_SIZE);
    c->outgoing_len = 0; 
    c->want_close = false; 
    c->want_read = true; 
    c->want_write = false;  
}

void init_pollfd_data(struct pollfd* pfd, int client_sock_fd) {
    pfd->fd = client_sock_fd; 
    pfd->events = POLLIN; 
    pfd->revents = 0; 
}

size_t accumulate_clients(int listening_sock_fd, 
    client* clients, struct pollfd* pollfds, int* offset) {
    int num_clients = 0; 
    while(true) {
        struct sockaddr_in sockaddr = {}; 
        socklen_t len = sizeof(sockaddr); 

        int client_fd = accept(listening_sock_fd, 
            (struct sockaddr*)&sockaddr, &len);

        if(client_fd == -1) {
            break; 
        }

        init_client_data(&clients[client_fd]);
        init_pollfd_data(pollfds+(*offset), client_fd);
        num_clients++; 
        (*offset)++; 
    }
    return num_clients; 
}

int wait_for_clients(struct pollfd* pollfds, int* offset) {
    int num_ready = -1; 
    while(true) {
        num_ready = poll(pollfds, (nfds_t)(*offset), -1);

        if(num_ready < 0) {
            if(errno == EINTR) continue; 
            return -1; 
        } 
        return num_ready;
    }  
}

int32_t consume_data(struct pollfd* pollfds, int* offset, client* clients) {
    for(int i = 1; i < (*offset); i++) {
        if((pollfds[i].revents & (POLLIN)) != 0) {
            int fd = pollfds[i].fd; 
            int bytes_read = read(fd, clients[fd].incoming + clients[fd].incoming_len, MAX_MESSAGE_SIZE); 
            if(bytes_read == 0) {
                clients[fd].want_close = true; 
                continue; 
            }
            if(bytes_read < 0 ) {
                if (errno == EINTR)continue; 
                else clients[fd].want_close = true; 
            } 
            clients[fd].incoming_len += bytes_read; 
        }
    }
    return 0; 
}

int32_t cleanup_resources(struct pollfd* pollfds, int* offset, client* clients) {
    for(int i = 1; i < (*offset); i++) {
        if(clients[pollfds[i].fd].want_close) {
            printf("voa: clean up of socket fd %d initiated\n", pollfds[i].fd);        
            int fd = pollfds[i].fd; 
            close(fd); 
            free(clients[fd].incoming); 
            free(clients[fd].outgoing); 
            clients[fd].incoming = 0; 
            clients[fd].outgoing = 0; 
            clients[fd].outgoing_len = 0; 
            clients[fd].incoming_len = 0; 
            clients[fd].want_close = clients[fd].want_read = clients[fd].want_write = 0;
            pollfds[i] = pollfds[(*offset) - 1];
            (*offset)--; i--; 
        }
    }
    return 0; 
}

void accept_requests(int listening_sock_fd) {
    //client map, fd -> client struct 
    client clients[MAX_CLIENTS];
    struct pollfd pollfds[MAX_CLIENTS]; 
    int offset = 1; 
    //pollfds[0] is our listening socket 
    pollfds[0].fd = listening_sock_fd; 
    pollfds[0].events = POLLIN; 
    pollfds[0].revents = 0; 

    while(true) {
        printf("voa: waiting for clients...\n"); 
        int num_ready = wait_for_clients(pollfds, &offset);
        
        // for now, this is enough 
        assert(num_ready >= 0);

        accumulate_clients(listening_sock_fd,
            clients, pollfds, &offset); 

        //read data from ready sockets 
        consume_data(pollfds, &offset, clients);

        cleanup_resources(pollfds, &offset, clients); 


        for(int i = 1; i < offset; i++) {
            if(clients[pollfds[i].fd].incoming_len > 0) {
                printf("voa: client %d sent a message \"%s\"\n", pollfds[i].fd, clients[pollfds[i].fd].incoming);
            }
        }
    }
}

int main() {
    // create raw socket  
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    fd_set_nonblock(fd); // make listening socket non - blocking
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