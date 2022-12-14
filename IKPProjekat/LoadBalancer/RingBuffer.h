#pragma once
#include "structs.h"

Queue* CreateQueue(int kapacitet) {
	Queue* queue = (Queue*)malloc(sizeof(Queue));
	queue->size = 0;
	queue->capacity = kapacitet;
	queue->rear = 0;
	queue->front = 0;
	queue->array = (char*)malloc(sizeof(char) * kapacitet);
	return queue;
}
bool IsEmpty(Queue* queue) {
	if (queue->size == 0)
		return true;
	return false;
}
bool IsFull(Queue* queue, int strlenMessage) {
	//if ((queue->size + strlenMessage + sizeof(int)) >= (queue->capacity * 0.6) && queue == primaryQueue)
	//	ReleaseSemaphore(CreateQueueSemaphore, 1, NULL);
	if ((queue->size + strlenMessage + sizeof(int)) >= (queue->capacity * 0.7)) { //upisom poruke bi popunjenost presla 70%, ne dozvoljavamo!
		return true;
	}
	return false;
}
/*
* 
bool Enqueue(Queue* queue, char* message, int strlenMessage) {
	bool success = false;
	if (!IsFull(queue, strlenMessage)) {
		AddHeaderLength(queue, strlenMessage);
		AddMessage(queue, message, strlenMessage);
		queue->size += (strlenMessage + sizeof(int));
		success = true;
	}
	else {
		//WaitForSingleObject(CreatedQueueSemaphore, INFINITE);
		if (!IsFull(tempQueue, strlenMessage)) {
			AddHeaderLength(tempQueue, strlenMessage);
			AddMessage(tempQueue, message, strlenMessage);
			tempQueue->size += (strlenMessage + sizeof(int));
			success = true;
		}
		else { //napunio nam se i privremeni, dodati logiku da pauziramo klijente ili tako nesto
			printf("\n!!!Queue is full!!!\n");
		}
		PrimaryToSecondary();
		TempToPrimary();
	}
	return success;
}
bool AddHeaderLength(Queue* queue, int strlenMess) {
	for (int i = 0; i < 4; i++)
	{
		queue->array[queue->rear] = ((char*)(&strlenMess))[i];
		queue->rear = (queue->rear + 1) % queue->capacity;
	}
	return true;
}
bool AddMessage(Queue* queue, char* message, int strlenMess) {
	for (int i = 0; i < strlenMess; i++)
	{
		queue->array[queue->rear] = message[i];
		queue->rear = (queue->rear + 1) % queue->capacity;
	}
	return true;
}
char* Dequeue(Queue* queue) {
	char* message;
	if (!IsEmpty(queue)) {
		char strlenMessageString[4];
		for (int i = 0; i < 4; i++)
		{
			strlenMessageString[i] = queue->array[queue->front];
			queue->array[queue->front] = 'x';
			queue->front = (queue->front + 1) % queue->capacity;
		}
		int strlenMessageInt = *(int*)strlenMessageString;
		if (strlenMessageInt < 0 || strlenMessageInt > 10000) // uslovi su potrebni jer ne moze da uhavatimo kad se zapravo desi problem
			return NULL;
		message = (char*)malloc((sizeof(char) * strlenMessageInt) + 4);
		for (int i = 0; i < 4; i++)
		{
			message[i] = strlenMessageString[i];
		}
		for (int i = 0; i < strlenMessageInt; i++)
		{
			message[i + 4] = queue->array[queue->front];
			queue->array[queue->front] = 'x';
			queue->front = (queue->front + 1) % queue->capacity;
		}
		queue->size -= strlenMessageInt + sizeof(int);
		return message;
	}
	return NULL;
}
bool PrimaryToSecondary() {
	int secondaryIndex = 0;

	for (int i = 0; i < primaryQueue->size; i++)
	{
		secondaryQueue->array[secondaryIndex++] = primaryQueue->array[primaryQueue->front];
		primaryQueue->front = (primaryQueue->front + 1) % primaryQueue->capacity;
	}
	secondaryQueue->front = 0;
	secondaryQueue->size = primaryQueue->size;
	secondaryQueue->rear = secondaryQueue->size;

	DestroyQueue(primaryQueue);
	primaryQueue = secondaryQueue;
	secondaryQueue = NULL;

	return true;
}
bool TempToPrimary() {
	int secondaryIndex = 0;
	for (int i = 0; i < tempQueue->size; i++)
	{
		primaryQueue->array[primaryQueue->rear++] = tempQueue->array[i];
	}
	primaryQueue->size = primaryQueue->size + tempQueue->size;
	DestroyQueue(tempQueue);
	tempQueue = NULL;
	return true;
}
void DestroyQueue(Queue* queue) {
	if (queue != NULL) {
		free(queue->array);
		free(queue);
	}
}

*/