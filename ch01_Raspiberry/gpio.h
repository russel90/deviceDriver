#define IOCTL_OUT _IOW( FNDIP_DRIVER_MAJOR_KEY, 0, test_data )

typedef struct 
{
 unsigned char  num;
 unsigned char  data;
}__attribute__((packed))test_data;