#pragma once
#include "structs.h"
CRITICAL_SECTION cs;

Queue* CreateQueue(int cap) {
	Queue* queue = (Queue*)malloc(sizeof(Queue));
	queue->size = 0;
	queue->capacity = cap;
	queue->rear = 0;
	queue->front = 0;
	queue->racun = (Racun*)malloc(sizeof(Racun) * cap);
	return queue;
}

int enqueue(Queue** q, Racun* data)
{
	Queue* queue = *q;

	if (queue->capacity==queue->size)
		return false;
	queue->rear = (queue->rear + 1)	% queue->capacity;
	*(queue->racun + queue->rear) = *data;
	queue->size = queue->size + 1;
	printf("\n%d-%d enqueued to queue", (queue->racun + queue->rear)->meterId, (queue->racun+queue->rear)->stanjeTrenutno);
	return true;
}

Racun dequeue(Queue** q)
{
	Queue* queue = *q;

	//printf("\n\tVelicina reda %d", queue->size);
	if(queue->size!=0)
	{
		struct Racun item = *(queue->racun + queue->front);
		printf("\n\n\t\Skinuto sa reda %d %d",item.meterId, item.stanjeTrenutno);
		queue->front = (queue->front + 1) % queue->capacity;
		queue->size = queue->size - 1;
		return item;
	}
	struct Racun r;
	r.meterId = -1;
	r.stanjeTrenutno = -1;
	return r;
}

void ispisiRacune(Queue* q)
{
	Queue* queue = q;

	if (queue->size != 0)
	{
		for (int i = q->front; i <= queue->rear; i++) {
			struct Racun item = *(queue->racun + i);
			printf("\n\tRING - %d %d %d",item.meterId, item.stanjeStaro, item.stanjeTrenutno);
		}
	}
}

