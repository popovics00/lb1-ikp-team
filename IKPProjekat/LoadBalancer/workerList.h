#pragma once
#include "structs.h"

void AddAtEndWorker(NodeW** head, Worker* new_data) {
	NodeW* new_node = (struct NodeW*)malloc(sizeof(struct NodeW));

	new_node->worker = (Worker*)malloc(sizeof(Worker));
	new_node->worker = new_data;
	new_node->next = NULL;

	if (*head == NULL) {
		*head = new_node;
		return;
	}

	NodeW* last = *head;
	while (last->next != NULL)
		last = last->next;
	last->next = new_node;
	return;
}

void deleteNodeWorker(NodeW** head_ref, int key)
{
	NodeW* temp = *head_ref, * prev = NULL;

	if (temp != NULL && temp->worker->acceptedSocket == key) {
		*head_ref = temp->next;
		free(temp);
		return;
	}

	while (temp != NULL && temp->worker->acceptedSocket != key) {
		prev = temp;
		temp = temp->next;
	}

	if (temp == NULL)
		return;

	prev->next = temp->next;

	free(temp);
}

void FreeListWorker(NodeW* head) {
	NodeW* temp;
	while (head != NULL) {
		temp = head;
		head = head->next;
		free(temp->worker->ipAdr);
		temp->worker->ipAdr = NULL;
		free(temp);
		temp = NULL;
	}
	return;
}

NodeW* FindMeterWorker(NodeW* head, int meterId) {
	NodeW* temp = head;
	while (temp != NULL) {
		if (temp->worker->id == meterId)
			break;
		temp = temp->next;
	}
	return temp;
}

Worker* VratiSlobodnogWorkera(NodeW* head) {
	NodeW* temp = head;
	while (temp != NULL) {
		if (temp->worker->zauzet == false)
		{
			Sleep(3000);
			return temp->worker;
		}
		if (temp->next == NULL)
			temp = head;
		temp = temp->next;
		Sleep(3000);
	}
	return NULL;
}

void IspisiListuWorker(NodeW* head) {
	NodeW* temp = head;
	printf("\n\nLISTA WORKERA:");
	while (temp != NULL) {
		printf("\n\tWorker-%d na portu %d %d", temp->worker->id, temp->worker->port);
		if (temp->next == NULL)
			break;
		temp = temp->next;
	}
}

int IzbrojWorkere(NodeW* head) {
	NodeW* temp = head;
	int tempBrojac = 0;
	while (temp != NULL) {
		tempBrojac++;
		if (temp->next == NULL)
			break;
		temp = temp->next;
	}
	return tempBrojac;
}