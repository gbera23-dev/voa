

#ifndef __UTILS__ 
#define __UTILS__


void die(char* str);
int32_t full_read(int fd, char* buffer, size_t n);
int32_t full_write(int fd, char* buffer, size_t n);


#endif 