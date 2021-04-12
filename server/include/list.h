#ifndef LIST_H
#define LIST_H

#include "string.h"

struct node_t
{
  struct node_t *next;
  void *data;
};

typedef struct node_t node_t;

typedef struct list_t
{
  node_t *head;
  void (*free_data)(void *);
  int (*compare_data)(void *, void *);
} list_t;

list_t *list_create();
list_t* list_create_from_array(void** array);
void** list_to_array(const list_t* list, const size_t data_size);

void list_add_start(void *data, list_t *list);
void list_add_last(void *token, list_t* list);

int list_find(void *data, list_t *list);
void* list_get(const int target_index, list_t *list);

int list_delete(int target_index, list_t *list);
void list_free(list_t*);
list_t *list_slice(int target_index, list_t *list);

#endif
