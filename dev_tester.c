#include <fcntl.h> 
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>

int main()
{
    int fd = open("/dev/chardev", O_RDONLY);
    if(fd != -1)
    {
        ioctl(fd, _IO(0x55, 0x1));
        printf("IO succ");
    }
    printf("IO fail");
}