#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "Dict.h"

typedef struct Node_t
{
    char *key;
    void *value;
    struct Node_t *next;
} Node;

struct Dict_t
{
    Node **array;
    size_t arraySize;
    size_t nbKeys;
};

static unsigned int h(Dict *d, const char *key);
static void terminate(char *m);

static unsigned int h(Dict *d, const char *key)
{
    unsigned long hash = 5381;
    while (*key != '\0')
    {
        hash = hash * 33 + *key;
        key++;
    }
    return hash % d->arraySize;
}

static void terminate(char *m)
{
    printf("%s\n", m);
    exit(EXIT_FAILURE);
}

Dict *dictCreate(size_t m)
{
    Dict *d = malloc(sizeof(Dict));
    if (d == NULL)
        terminate("Dict cannot be created");

    d->array = calloc(m, sizeof(Node));
    if (d->array == NULL)
        terminate("Dict cannot be created");

    d->arraySize = m;
    return d;
}

void dictFree(Dict *d)
{
    for (size_t i = 0; i < d->arraySize; i++)
    {
        Node *n = d->array[i];
        while (n != NULL)
        {
            Node *nn = n->next;
            free(n->key);
            free(n);
            n = nn;
        }
    }

    free(d->array);
    free(d);
}

void dictFreeValues(Dict *d, void (*freeData)(void *))
{
    for (size_t i = 0; i < d->arraySize; i++)
    {
        Node *n = d->array[i];
        while (n != NULL)
        {
            Node *nn = n->next;
            free(n->key);
            if (n->value != NULL)
                freeData(n->value);
            free(n);
            n = nn;
        }
    }

    free(d->array);
    free(d);
}

void dictIterate(Dict *d, void (*f)(const char *key, void *values))
{
    for (size_t i = 0; i < d->arraySize; i++)
    {
        Node *n = d->array[i];
        while (n != NULL)
        {
            f(n->key, n->value);
            n = n->next;
        }
    }
}

void *dictSearch(Dict *d, const char *key)
{
    Node *p = d->array[h(d, key)];
    while (p != NULL && strcmp(p->key, key) != 0)
        p = p->next;

    if (p != NULL)
        return p->value;
    else
        return NULL;
}

bool dictContains(Dict *d, const char *key)
{
    Node *p = d->array[h(d, key)];
    while (p != NULL && strcmp(p->key, key) != 0)
        p = p->next;

    return (p != NULL);
}

void dictInsert(Dict *d, const char *key, void *value)
{
    int hashval = h(d, key);
    Node *p = d->array[hashval];
    while (p != NULL && strcmp(p->key, key) != 0)
        p = p->next;

    if (p != NULL)
        p->value = value;

    else
    {
        Node *newNode = malloc(sizeof(Node));
        if (!newNode)
            terminate("New node cannot be created.");

        int len = strlen(key);
        char *k = malloc((len + 1) * sizeof(char));
        if (!k)
            terminate("New node cannot be created.");

        strcpy(k, key);

        newNode->key = k;
        newNode->value = value;
        newNode->next = d->array[hashval];
        d->array[hashval] = newNode;
        d->nbKeys++;
    }
}

size_t dictGetNbKeys(Dict *d)
{
    return d->nbKeys;
}
