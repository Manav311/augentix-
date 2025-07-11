#include <stdio.h>
#include <stdlib.h>

#include "tutk_linked_list.h"
#include <pthread.h>

Node *createnode(const KeyPair *data, Node *next)
{
	Node *newNode = malloc(sizeof(Node));
	if (!newNode) {
		return NULL;
	}

	newNode->data = *data;
	newNode->next = next;
	return newNode;
}

List *list_create()
{
	List *list = malloc(sizeof(List));
	if (!list) {
		return NULL;
	}

	list->head = NULL;
	list->Keynum = 0;
	pthread_mutex_init(&list->mutex, NULL);

	return list;
}

int list_pop(List *list, KeyPair *val)
{
	Node *current = NULL;

	pthread_mutex_lock(&list->mutex);
	current = list->head;

	if (current != NULL) {
		list->head = current->next;
		*val = current->data;
		free(current);
		list->Keynum -= 1;
		pthread_mutex_unlock(&list->mutex);
		return 0;
	}

	/* list empty */
	list->Keynum = 0;
	pthread_mutex_unlock(&list->mutex);
	return -1;
}

int list_add(const KeyPair *val, List *list)
{
	Node *current = NULL;

	pthread_mutex_lock(&list->mutex);
	if (list->head == NULL) {
		list->head = createnode(val, NULL);
		list->Keynum = 1;
	} else {
		current = list->head;

		if (current->data.key > val->key) {
			list->head = createnode(val, current);
		} else {
			while (current->next != NULL) {
				if (current->next->data.key > val->key) {
					break;
				}

				current = current->next;
			}

			current->next = createnode(val, current->next);
		}
		list->Keynum += 1;
	}

	pthread_mutex_unlock(&list->mutex);
	return 0;
}

void list_reverse(List *list)
{
	pthread_mutex_lock(&list->mutex);
	Node *reversed = NULL;
	Node *current = list->head;
	Node *temp = NULL;

	while (current != NULL) {
		temp = current;
		current = current->next;
		temp->next = reversed;
		reversed = temp;
	}

	list->head = reversed;
	pthread_mutex_unlock(&list->mutex);
}

void list_display(List *list)
{
	pthread_mutex_lock(&list->mutex);
	Node *current = list->head;
	if (list->head == NULL)
		goto end;

	fprintf(stderr, "=== List of Upload Files ===");
	for (; current != NULL; current = current->next) {
		fprintf(stderr, "%ld, 0x%08lX, %d\n", current->data.key, (unsigned long)current->data.val,
		        current->data.status);
	}
	fprintf(stderr, "============================");
end:
	pthread_mutex_unlock(&list->mutex);
}

void list_destroy(List *list)
{
	Node *current = list->head;
	Node *next = current;
	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}

	pthread_mutex_destroy(&list->mutex);
	free(list);
}
