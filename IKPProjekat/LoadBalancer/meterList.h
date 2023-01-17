#pragma once
#include "structs.h"

void AddAtEnd(Node** head, Meter* new_data) {
	Node* new_node = (struct Node*)malloc(sizeof(struct Node));

	new_node->meter = (Meter*)malloc(sizeof(Meter));
	new_node->meter = new_data;
	new_node->next = NULL;

	if (*head == NULL) {
		*head = new_node;
		return;
	}

	Node* last = *head;
	while (last->next != NULL)
		last = last->next;
	last->next = new_node;
	return;
}

void deleteNode(Node** head_ref, int key)
{
	Node* temp = *head_ref, * prev = NULL;

	if (temp != NULL && temp->meter->acceptedSocket == key) {
		*head_ref = temp->next;
		free(temp);
		return;
	}

	while (temp != NULL && temp->meter->acceptedSocket != key) {
		prev = temp;
		temp = temp->next;
	}

	if (temp == NULL)
		return;

	prev->next = temp->next;

	free(temp);
}

void FreeList(Node* head) {
	Node* temp;
	while (head != NULL) {
		temp = head;
		head = head->next;
		free(temp->meter->ipAdr);
		temp->meter->ipAdr = NULL;
		free(temp);
		temp = NULL;
	}
	return;
}

Node* FindMeter(Node* head, int meterId) {
	Node* temp = head;
	while (temp != NULL) {
		if (temp->meter->id == meterId)
			break;
		temp = temp->next;
	}
	return temp;
}

void UvecajDug(Node** head_ref, int meterId, int noviMesecDug) {
	Node* temp = *head_ref;
	while (temp != NULL) {
		if (temp->meter->id == meterId)
		{
			temp->meter->debt += noviMesecDug;
			break;
		}
		else
		{
			temp = temp->next;
		}
	}
}

void SetajTrenutnoStanje(Node** head_ref, int meterId, int stanje) {
	Node* temp = *head_ref;
	while (temp != NULL) {
		if (temp->meter->id == meterId)
		{
			temp->meter->lastMonth = stanje;
			break;
		}
		else
		{
			temp = temp->next;
		}
	}
}

void IspisiListu(Node* head) {
	Node* temp = head;
	while (temp != NULL) {
		printf("%d %d %d\n",temp->meter->id, temp->meter->lastMonth,temp->meter->debt);
		if (temp->next == NULL)
			break;
		temp = temp->next;
	}
}