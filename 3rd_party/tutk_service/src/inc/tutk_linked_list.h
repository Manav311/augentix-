#ifndef LINKEDLIST_HEADER
#define LINKEDLIST_HEADER

#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct keypair {
	unsigned long key;
	void *val; /* pinter to cr cb structure */
	int status;
} KeyPair;

typedef struct node {
	KeyPair data;
	struct node *next;
} Node;

typedef struct list {
	pthread_mutex_t mutex;
	Node *head;
	int Keynum;
} List;

typedef struct list List;
typedef struct node Node;
typedef struct keypair KeyPair;

List *list_create();
int list_add(const KeyPair *val, List *list);
int list_pop(List *list, KeyPair *val);
void list_destroy(List *list);
void list_display(List *list);
void list_reverse(List *list);

#endif
