#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
void *print_hello (void *arg)
{
   int i = (int)arg;

   printf("it's me, thread %d!\n", i);
   sleep(1);
   pthread_exit(NULL);                                      /*  线程退出               */

   return (NULL);
}

int  main (int  argc, char  *argv[])
{
   int       ret;
   int       i;
   pthread_t     tid[5];

   for (i = 0; i < 5; i++) {                                /*  创建 5 个线程          */
      printf("in main: creating thread %d\n", i);

      ret = pthread_create(&tid[i], NULL, print_hello, (void *)i);
      if (ret != 0) {
         printf("create thread failed\n");
         exit(-1);
      }
   }
   pthread_join(tid[0], NULL);// 等待线程0退出
   printf("in main: exit!\n");
   pthread_exit(NULL);                                      /*  创建线程后，主线程退出 */
   return (0);
}
