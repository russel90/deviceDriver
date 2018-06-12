#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 100

int main(int argc, char **argv)
{
    int fd =-1;
    int count;
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);

    printf("GPIO Set : %s\n", argv[1]);
    fd = open("/dev/gpioled", O_RDWR);
    if(fd < 0){
        printf("Error : Open()\n");
        return -1;
    }
    
    count = write(fd, argv[1], strlen(argv[1]));
    if(count < 0){
        printf("Error : write()\n");
        return -1;
    }

    count = read(fd, buffer, 10);
    if(count < 0){
        printf("Error : count()\n");
        return -1;
    }

    printf("/dev/gpioled opened\n");

    sleep(5);
    close(fd);

    printf("/dev/gpioled closed\n");

    return 0;
}
