#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

static QUEUE *g_queue=NULL;

static QUEUE *Initialize_Queue(void)
{
	QUEUE *q;
	
	q = calloc(1, sizeof(QUEUE));
	if (!(q))
		return NULL;
	
	pthread_mutex_init(&(q->modify_mutex), NULL);
	pthread_mutex_init(&(q->read_mutex), NULL);
	pthread_mutex_lock(&(q->read_mutex));
	
	return q;
}

static void Add_Queue_Item(QUEUE *queue, int type, void *data, size_t sz)
//void Add_Queue_Item(QUEUE *queue, int type, void *data)
//void Add_Queue_Item(QUEUE *queue,  void *data, size_t sz)
{
	QUEUE_ITEM *qi;
	
	qi = calloc(1, sizeof(QUEUE_ITEM));
	if (!(qi))
		return;
	
	qi->data = data;
	qi->sz = sz;
	qi->type = type;
	
	pthread_mutex_lock(&(queue->modify_mutex));
	
	qi->next = queue->items;
	queue->items = qi;
	queue->numitems++;
	
	pthread_mutex_unlock(&(queue->modify_mutex));
	pthread_mutex_unlock(&(queue->read_mutex));
}

static QUEUE_ITEM *Get_Queue_Item(QUEUE *queue)
{
	QUEUE_ITEM *qi, *pqi;
	
	pthread_mutex_lock(&(queue->read_mutex));
	pthread_mutex_lock(&(queue->modify_mutex));
	
  if(!(qi = pqi = queue->items)) {
      pthread_mutex_unlock(&(queue->modify_mutex));
      return (QUEUE_ITEM *)NULL;
  } 

	qi = pqi = queue->items;
	while ((qi->next))
	{
		pqi = qi;
		qi = qi->next;
	}
	
	pqi->next = NULL;
	if (queue->numitems == 1)
		queue->items = NULL;
	
	queue->numitems--;
	
	if (queue->numitems > 0)
		pthread_mutex_unlock(&(queue->read_mutex));
	
	pthread_mutex_unlock(&(queue->modify_mutex));
	
	return qi;
}

static void Free_Queue_Item(QUEUE_ITEM *queue_item)
{
	if(queue_item)
		free(queue_item);
}

static void Free_Queue(QUEUE*queue)
{
	if(queue)
		free(queue);
}

static int Get_Queue_Sizes(QUEUE *queue)
{
	int size;
	pthread_mutex_lock(&(queue->modify_mutex));
	size = queue->numitems;
	pthread_mutex_unlock(&(queue->modify_mutex));
	return size;
}


//下面对队列函数进行进一步封装

void queue_init(void)
{
	if(g_queue == NULL) {
		g_queue = Initialize_Queue();
		if(!g_queue) {
			printf("Initialize_Queue failed");
		}
	}
}

int queue_add_item(int type, void *data, size_t sz)
{
	if(g_queue == NULL) {
		printf("queue is not Init...");
		return -1;
	}
	Add_Queue_Item(g_queue,type, data, sz);
	return 0;
}

QUEUE_ITEM * queue_get_item()
{
	if(g_queue == NULL) {
		printf("queue is not Init...");
		return NULL;
	}
	return Get_Queue_Item(g_queue);
}

int get_queue_size()
{
	if(g_queue)
		return Get_Queue_Sizes(g_queue);
}

void queue_del_item(QUEUE_ITEM *queue_item)
{
	if(g_queue == NULL) {
		printf("queue is not Init...");
		return ;
	}
	Free_Queue_Item(queue_item);
}

void queue_deinit()
{
	if(g_queue) {
		Free_Queue(g_queue);

			g_queue = NULL;
	}
}

