#include <stdio.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <string.h>
#include "utils.h" 
#include <errno.h> 

void die(char* str) {
    printf("voa: error %s\n", str); 
    exit(-1); 
}

int32_t full_read(int fd, char* buffer, size_t n) {
    while(n > 0) {
        int32_t num_read = read(fd, buffer, n);
        if(num_read <= 0) {
            if(errno == EINTR) {
                continue; 
            }
            return -1; 
        }
        n -= num_read; 
        buffer += num_read; 
    }
    return 0;
} 

int32_t full_write(int fd, char* buffer, size_t n) {
    while(n > 0) {
        int32_t num_written = write(fd, buffer, n); 
        if(num_written <= 0) {
            if(errno == EINTR) {
                continue; 
            }
            return -1; 
        } 
        n -= num_written; 
        buffer += num_written; 
    }
    return 0; 
}