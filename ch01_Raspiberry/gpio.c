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

void test_out(void)
{
    test_data param;

    param.num = 1;
    param.data = 'a';

    ioctl( test_dev.fd, IOTCL_OUT, &param );
}

int main(int argc, char **argv)
{
    int fd =-1;
    int count;
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);

    printf("GPIO Set : %s\n", argv[1]);
    while((fd = open("/dev/gpioled",O_RDWR)) < 0){
        switch(errno){
            case ENXIO:
                system("sudo insmod gpio_module.ko");
                break;
            case ENOENT:
                system("sudo mknod /dev/gpioled c 200 0");        
                system("sudo chmod 666 /dev/gpioled");
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

    printf("/dev/gpioled opened\n");

    close(fd);
    system("sudo rmmod gpio_module");
    printf("/dev/gpioled closed\n");

    return 0;
}
