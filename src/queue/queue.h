#ifndef __QUEUE__H__
#define __QUEUE__H__

#include <pthread.h>

typedef struct _queue_item
{
	int type;
	void *data;
	size_t sz;
	
	struct _queue_item *next;
} QUEUE_ITEM;

typedef struct _queue
{
	size_t numitems;
	QUEUE_ITEM *items;
	
	pthread_mutex_t modify_mutex;
	pthread_mutex_t read_mutex;
} QUEUE;

enum {
    EVENT_TYPE_KEY_PREV = 0,
    EVENT_TYPE_KEY_NEXT = 1,
    EVENT_TYPE_KEY_PLAY = 2,
    EVENT_TYPE_KEY_STOP = 3
};


void queue_init(void);
int queue_add_item(int type, void *data, size_t	len);
//int queue_add_item(int type, void *data);
//int queue_add_item(void *data, size_t sz);
void queue_del_item(QUEUE_ITEM *queue_item);
QUEUE_ITEM * queue_get_item();
int get_queue_size();
void queue_deinit();

#endif
