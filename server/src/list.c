#include <stdlib.h>
#include <stdio.h>
#include "../include/list.h"

list_t *list_create()
{
  list_t *ret_list = (list_t *)calloc(1, sizeof(list_t));
  ret_list->head = (node_t *)calloc(1, sizeof(node_t));
  ret_list->head->next = NULL;
  return ret_list;
}

void list_add_start(void *data, list_t *list)
{
  node_t *new_node = (node_t *)calloc(1, sizeof(node_t));
  if (new_node == NULL) exit(EXIT_FAILURE);
  new_node->data = data;
  new_node->next = list->head;
  list->head = new_node;
}

void list_add_last(void *data, list_t* list)
{
  node_t *new_node = (node_t *) calloc(1, sizeof(node_t));
  new_node->data = data;
  if (list->head->next == NULL)
  {
    new_node->next = list->head;
    list->head = new_node;
    return;
  }

  node_t *curr = list->head;
  while (curr->next->next != NULL)
  {
    curr = curr->next;
  }
  new_node->next = curr->next;
  curr->next = new_node;
}

int list_find(void *data, list_t *list)
{
  int i = 0;
  for (node_t *curr = list->head; curr->next != NULL; curr = curr->next)
  {
    if (list->compare_data(data, curr->data) == 0)
    {
      return i;
    }
    i++;
  }
  return -1;
}

int list_delete(int target_index, list_t *list)
{
  if (target_index < 0)
    return -1;
  if (list->head->next == NULL)
  {
    fprintf(stderr, "Cannot delete in empty list");
    return -1;
  }

  if (target_index == 0)
  {
    node_t *old_head = list->head;
    list->head = old_head->next;
    list->free_data(old_head->data);
    free(old_head);
    return 1;
  }

  node_t *curr = list->head;
  node_t *prev;
  for (int i = 0; curr->next != NULL; i++)
  {
    if (i == target_index)
    {
      prev->next = curr->next;
      list->free_data(curr->data);
      free(curr);
      return 1;
    }
    prev = curr;
    curr = curr->next;
  }

  return -1;
}

list_t *list_slice(int target_index, list_t *list)
{
  if (target_index == 0) return NULL;

  int index = 0;
  node_t *curr_node = list->head;
  node_t *prev_node = NULL;

  while (curr_node->next != NULL)
  {
    if (index == target_index)
    {
      list_t *new_list = (list_t *) calloc(1, sizeof(list_t));
      node_t *new_last_empty = (node_t *) calloc(1, sizeof(node_t));
      new_list->head = curr_node;
      prev_node->next = new_last_empty;
      return new_list;
    }

    prev_node = curr_node;
    curr_node = curr_node->next;
    index++;
  }
  return NULL;
}

void list_free(list_t *list)
{
  node_t *curr = list->head;
  while (curr->next != NULL)
  {
    node_t *tmp = curr;
    list->free_data(curr->data);
    curr = curr->next;
    free(tmp);
  }

  free(curr);
  free(list);
}

void* list_get(const int target_index, list_t *list)
{
  if (target_index < 0) return NULL;

  int index = 0;
  for (node_t *node = list->head; node->next != NULL; node = node->next)
  {
    if (index == target_index) return node->data;
    index++;
  }
  return NULL;
}


list_t* list_create_from_array(void** array)
{
  list_t *ret_list = list_create();
  int i = 0;
  for (i = 0; array[i] != NULL; i++)
  {
    list_add_last(array[i], ret_list);
  }
  free(array[i]);
  free(array);
  return ret_list;
}

void** list_to_array(const list_t* list, const size_t data_size)
{
  void** array = (void**) malloc(sizeof(void*));
  node_t *curr_node = list->head;
  int i = 0;

  for (i = 0; curr_node->next != NULL; i++)
  {
    void** tmp = realloc(array, sizeof(void*) * ( (unsigned long) (i + 2)) );
    if (tmp == NULL)
    {
      perror("realloc");
      exit(EXIT_FAILURE);
    }
    else
    {
      array = tmp;
    }
    void* ptr = malloc(data_size);
    memcpy(ptr, curr_node->data, data_size);
    array[i] = ptr;
    curr_node = curr_node->next;
  }

  array[i] = NULL;
  return array;
}
