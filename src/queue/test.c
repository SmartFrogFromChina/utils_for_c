#include<stdio.h>  
#include<sys/types.h>  
#include<stdlib.h>  
#include<unistd.h>  
#include<pthread.h>  
 #include "queue.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  
static pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;  
static pthread_mutex_t eventQueueMtx = PTHREAD_MUTEX_INITIALIZER; 
  
QUEUE_ITEM * item = NULL;  


void *decrement(void *arg) 
{  
    while(1)
    {
        pthread_mutex_lock(&mutex);
        printf("size:%d\n",get_queue_size());  
        while(get_queue_size() == 0)     //队列中没有数据就等待
        {
            pthread_cond_wait(&cond, &mutex);
        } 
        item = queue_get_item();
        pthread_mutex_unlock(&mutex); //获取到队列中的元素后就释放互斥锁，否则可能导致新的元素无法顺利进入队列
        
        printf("item->data:%s\n", (char *)item->data);  
        printf("end  ------------------------------\n");
        queue_del_item(item);
        sleep(2); //模拟：对获取到的元素进行后续处理
                 
    }
    return NULL; 
}  
  
void *increment(void *arg) 
{  
    while(1)
    {
        pthread_mutex_lock(&mutex); 
        queue_add_item(EVENT_TYPE_KEY_PLAY,"123456",6);

        if(get_queue_size() != 0)
        {
            printf("sending signal...........\n");
            pthread_cond_signal(&cond); 
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
        if(get_queue_size() > 10)
            break;
    } 
    return NULL;
}  
  
int main(int argc, char *argv[]) 
{  
    pthread_t tid_in, tid_de;
    queue_init();
    pthread_create(&tid_de, NULL, (void*)decrement, NULL);   
    pthread_create(&tid_in, NULL, (void*)increment, NULL);  
    pthread_join(tid_de, NULL);  
    pthread_join(tid_in, NULL);  
    pthread_mutex_destroy(&mutex);  
    pthread_cond_destroy(&cond);  
    queue_deinit();
    return 0;  
} 
