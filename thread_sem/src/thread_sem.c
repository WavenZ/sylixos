#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
int set_parity (int fd, int databits, int stopbits, int parity);
int set_speed (int fd, int speed);
void *uart_thread(void *argv);
sem_t sem;
sem_t* psem = &sem;
/******************************************************************************
** 函数名称: main
** 功能描述: 程序主函数
******************************************************************************/
int main (int argc, char *argv[])
{
    int     ret;
    pthread_t tid[5];
    int     fd_tty;
    int i;
    /*
     * 参数个数小于2则返回
     */
    if (argc < 2) {
        printf("useage: %s dev\n", argv[0]);
        return  (-1);
    }

    fd_tty = open(argv[1], O_RDWR);                     /*  打开串口               */
    if (fd_tty < 0) {
        printf("open device %s faild\n", argv[1]);
        return  (-1);
    }

    ret = set_speed(fd_tty, 115200);                    /*  设置串口波特率         */
    if (ret != 0) {
        close(fd_tty);
        return  (-1);
    }
    ret = set_parity(fd_tty, 8, 1, 'n');                /*数据位 8，停止位 1，无校验*/
    if (ret != 0) {
        close(fd_tty);
        return  (-1);
    }
    for (i = 0; i < 5; i++) {                                /*  创建 5 个线程          */
        ret = pthread_create(&tid[i], NULL, uart_thread, (void *)fd_tty);
          if (ret != 0) {
             printf("create thread failed\n");
             exit(-1);
          }
    }
    sem_init(&sem, 0, 1);
    /*
     * 在 while 循环中，不断的检查数据，若收到数据，将数据通过此串口发送回主机
     */
    while(1) {
    }
    sem_destroy(psem);
    close(fd_tty);
    return  (0);
}

void *uart_thread(void *argv){
    int fd_tty = (int)argv;
    int nread;
    char buff[512];
    char pid[20];
    sprintf(pid, "thread(%d):\0", ((int)pthread_self()) % 100);
    while(1){
        nread = read (fd_tty, buff ,512);
        if(nread != -1) {
            sem_wait(psem);
            write(fd_tty, pid, strlen(pid));
            write(fd_tty, buff, nread);
            sem_post(psem);
        }
    }
}

int set_parity (int fd, int databits, int stopbits, int parity)
{
    int                 ret;
    struct termios  options;

    ret = tcgetattr(fd, &options);
    if  (ret !=  0) {
        printf("Setup Serial failed");
        return(1);
    }

    options.c_cflag &= ~CSIZE;
    switch (databits) {                                     /*  设置数据位数           */
    case 5:
        options.c_cflag |= CS5;
        break;

    case 6:
        options.c_cflag |= CS6;
        break;

    case 7:
        options.c_cflag |= CS7;
        break;

    case 8:
        options.c_cflag |= CS8;
        break;

    default:
        printf("Unsupported data size\n");
        return (2);
    }

    switch (parity) {                                       /*  设置校验方式           */
    case 'n':
    case 'N':
        options.c_cflag &= ~PARENB;
        options.c_iflag &= ~INPCK;
        break;

    case 'o':
    case 'O':
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;

    case 'e':
    case 'E':
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;
        break;

    default:
        printf("Unsupported parity\n");
        return (3);
    }

    switch (stopbits) {                                     /* 设置停止位              */
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;

    case 2:
        options.c_cflag |= CSTOPB;
        break;

    default:
        printf("Unsupported stop bits\n");
        return (4);
    }
    tcflush(fd, TCIOFLUSH);                                 /* 清楚串口收发缓冲去      */
    /*
     * 用于设置终端的相关参数,TCSANOW：不等数据传输完毕就立即改变属性
     */
    ret = tcsetattr(fd, TCSANOW, &options);
    if (ret != 0) {
        printf("Setup Serial failed");
        return (5);
    }
    return (0);
}

int cfsetospeed (struct termios *tp, speed_t speed)
{
    if (speed & ~CBAUD) {
        errno = EINVAL;
        return  (PX_ERROR);
    }

    tp->c_cflag &= ~CBAUD;
    tp->c_cflag |= speed;

    return  (ERROR_NONE);
}
int set_speed (int fd, int speed)
{
     int                ret;
     struct termios     opt;

     ret = tcgetattr(fd, &opt);                                                 /*获取串口的 termios 信息   */
     if (ret !=  0) {
          printf("Get Serial speed failed");
          return (1);
     }
    tcflush(fd, TCIOFLUSH);                                                 /*  刷新输入输出数据           */

    cfsetispeed(&opt, speed);                             /*  设置输入波特率              */
    cfsetospeed(&opt, speed);                                 /*  设置输出波特率              */
    /* 用于设置终端的相关参数, TCSANOW 不等数据传输完毕就立即改变属性  */
    ret = tcsetattr(fd, TCSANOW, &opt);
    if (ret != 0) {
         printf("Setup Serial speed failed");
         return (2);
    }
    return (0);
}

