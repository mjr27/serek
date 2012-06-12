#include <string.h>
#include <stdlib.h>

typedef struct stringbuilder_node_s 
{
    struct stringbuilder_node_s *next;
    char *value;
    unsigned long size;
} stringbuilder_node_s;

typedef struct stringbuilder_s
{
    stringbuilder_node_s *head;
    stringbuilder_node_s *tail;
    unsigned long size;
} stringbuilder_s;


stringbuilder_s*
stringbuilder_new(void)
{
    stringbuilder_s *sb;
    sb = (stringbuilder_s*)malloc(sizeof(stringbuilder_s));
    if (sb == NULL) {
        //return (stringbuilder_s*)PyErr_NoMemory();
        return NULL;
    }

    memset(sb, '\0', sizeof(stringbuilder_s));
    return sb;
}

void*
stringbuilder_push(stringbuilder_s *self, char *data)
{
    stringbuilder_node_s *node;
    node = (stringbuilder_node_s*)malloc(sizeof(stringbuilder_node_s));
    if (node == NULL) {
        //return (stringbuilder_s*)PyErr_NoMemory();
        return NULL;
    }

    memset(node, '\0', sizeof(stringbuilder_node_s));
    node->value = data;
    node->size = strlen(data);
    self->size += node->size;
    
    if (self->head == NULL) {
        self->head = node;
    } else {
        self->tail->next = node;
    }
    self->tail = node;

    return node;
}

char*
stringbuilder_build(const stringbuilder_s *self)
{
    char *result = (char*)malloc(self->size + 1);
    if (result == NULL) {
        return NULL;
    }
    char *cursor = result;
    stringbuilder_node_s *iterator = self->head;
    while (iterator != NULL) {
        memcpy(cursor, iterator->value, iterator->size);
        cursor += iterator->size;
        iterator = iterator->next;
    }

    *cursor = '\0';
    return result;
}

void
stringbuilder_free(stringbuilder_s *self, char free_vals)
{
    stringbuilder_node_s *iterator = self->head;
    stringbuilder_node_s *tmp = NULL;
    while (iterator != NULL) {
        tmp = iterator;
        iterator = iterator->next;
        if (free_vals)
            free(tmp->value);
        free(tmp);
    }

    free(self);
}

