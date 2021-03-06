#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#define BUFFER_SIZE 100

int main(int argc, char **argv)
{
    int fd =-1;
    int count;
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);

    printf("GPIO Set : %s\n", argv[1]);
    while((fd = open("/dev/gpioirq_led1",O_RDWR)) < 0){
        switch(errno){
            case ENXIO:
                system("sudo insmod gpioirq_module.ko");
                break;
            case ENOENT:
                system("sudo mknod /dev/gpioirq_led1 c 200 0");        
                system("sudo chmod 666 /dev/gpioirq_led1");
                break;
			default:
                break;
        }
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

    printf("/dev/gpioirq_led1 opened\n");

	// sleep(2);
    close(fd);
    system("sudo rmmod gpioirq_module");
    printf("/dev/gpioirq_led1 closed\n");

    return 0;
}
