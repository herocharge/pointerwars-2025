#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>
// Function pointers to (potentially) custom malloc() and
// free() functions.
//
static void * (*malloc_fptr)(size_t size) = malloc;
static void   (*free_fptr)(void* addr)    = free;


// Creates a new linked_list.
// PRECONDITION: Register malloc() and free() functions via the
//               linked_list_register_malloc() and 
//               linked_list_register_free() functions.
// POSTCONDITION: An empty linked_list has its head point to NULL.
// Returns a new linked_list on success, NULL on failure.
//
struct linked_list * linked_list_create(void){
    struct linked_list * ll = malloc_fptr(sizeof(struct linked_list));
    if (ll == NULL) {
        return NULL;
    }
    ll->head = NULL;
    ll->size = 0;
    return ll;
}

// Deletes a linked_list.
// \param ll : Pointer to linked_list to delete
// POSTCONDITION : An empty linked_list has its head point to NULL.
// Returns TRUE on success, FALSE otherwise.
//
bool linked_list_delete(struct linked_list * ll){
    if(ll == NULL)
        return true;
    
    struct node* curr = ll->head;
    while(curr != NULL){
        struct node* next = curr->next;
        free_fptr(curr); // TODO: check for fail conditions
        curr = next;
    }

    return true;    
}

// Returns the size of a linked_list.
// \param ll : Pointer to linked_list.
// Returns size on success, SIZE_MAX on failure.
//
size_t linked_list_size(struct linked_list * ll){
    if(ll == NULL)
        return SIZE_MAX;
    
    return ll->size;
}

// Inserts an element at the end of the linked_list.
// \param ll   : Pointer to linked_list.
// \param data : Data to insert.
// Returns TRUE on success, FALSE otherwise.
//
bool linked_list_insert_end(struct linked_list * ll,
                            unsigned int data){
    if(ll == NULL) 
        return false;
    struct node* new_node = malloc_fptr(sizeof(struct node));
    if(new_node == NULL)
        return false;

    new_node->data = data;
    new_node->next = NULL;

    struct node* curr = ll->head;
    if(curr == NULL){
        ll->head = new_node;
        ll->size += 1;
        return true;
    }
    
    while(curr->next != NULL){
        curr = curr->next;
    }
    curr->next = new_node;
    ll->size += 1;
    return true;
}

// Inserts an element at the front of the linked_list.
// \param ll   : Pointer to linked_list.
// \param data : Data to insert.
// Returns TRUE on success, FALSE otherwise.
//
bool linked_list_insert_front(struct linked_list * ll,
                              unsigned int data){
    if(ll == NULL) 
        return false;
    struct node* new_node = malloc_fptr(sizeof(struct node));
    if(new_node == NULL)
        return false;

    new_node->data = data;
    new_node->next = ll->head;
    ll->head = new_node;
    ll->size += 1;
    return true;
}

// Inserts an element at a specified index in the linked_list.
// \param ll    : Pointer to linked_list.
// \param index : Index to insert data at.
// \param data  : Data to insert.
// Returns TRUE on success, FALSE otherwise.
//
bool linked_list_insert(struct linked_list * ll,
                        size_t index,
                        unsigned int data){
    if(ll == NULL) 
        return false;

    if(ll->size < index)
        return false;

    if(index == 0)
        return linked_list_insert_front(ll, data);

    struct node* new_node = malloc_fptr(sizeof(struct node));
    if(new_node == NULL)
        return false;
    

    new_node->data = data;
    new_node->next = NULL;

    struct node* curr = ll->head;

    // TODO: check for off by one error
    for(size_t i = 0; i < index-1; i++){
        curr = curr->next;
    }

    struct node* tmp = curr->next;
    curr->next = new_node;
    new_node->next = tmp;
    ll->size += 1;
    return true;
}

// Finds the first occurrence of data and returns its index.
// \param ll   : Pointer to linked_list.
// \param data : Data to find.
// Returns index of the first index with that data, SIZE_MAX otherwise.
//
size_t linked_list_find(struct linked_list * ll,
                        unsigned int data){
    if(ll == NULL)
        return SIZE_MAX;
    
    struct node* curr = ll->head;
    size_t index = 0;

    // TODO: should we use iterators?
    while(curr != NULL){
        if(curr->data == data){
            return index;
        }
        curr = curr->next;
        index++;
    }
    return SIZE_MAX;
}

// Removes a node from the linked_list at a specific index.
// \param ll    : Pointer to linked_list.
// \param index : Index to remove node.
// Returns TRUE on success, FALSE otherwise.
//
bool linked_list_remove(struct linked_list * ll,
                        size_t index){
    if(ll == NULL)
        return false;
    
    if(ll->size <= index)
        return false;

    if(index == 0){
        struct node* tmp = ll->head;
        if(tmp == NULL) return false;
        ll->head = tmp->next;
        free_fptr(tmp);
        ll->size -= 1;
        return true;
    }

    struct node* curr = ll->head;
    
    for(size_t i = 0; i < index - 1; i++){
        curr = curr->next;
    }

    struct node* node_to_remove = curr->next;
    curr->next = node_to_remove->next;
    free_fptr(node_to_remove);
    ll->size -= 1;
    return true;
}

// Creates an iterator struct at a particular index.
// \param linked_list : Pointer to linked_list.
// \param index       : Index of the linked list to start at.
// Returns pointer to an iterator on success, NULL otherwise.
//
struct iterator * linked_list_create_iterator(struct linked_list * ll,
                                              size_t index){
    if(ll == NULL)
        return NULL;
    
    struct node* curr = ll->head;

    for(size_t i = 0; i <= index; i++){
        if(curr == NULL){
            return NULL;
        }
        // TODO: find a better way of doing this
        if(i == index){
            break;
        }
        curr = curr->next;
    }

    struct iterator* iter = malloc_fptr(sizeof(struct iterator));
    if(iter == NULL)
        return NULL;

    iter->ll = ll;
    iter->current_node = curr;
    iter->current_index = index;
    iter->data = curr->data;

    return iter;
}

// Deletes an iterator struct.
// \param iterator : Iterator to delete.
// Returns TRUE on success, FALSE otherwise.
//
bool linked_list_delete_iterator(struct iterator * iter){
    if(iter == NULL)
        return true;

    free_fptr(iter);

    return true;
}

// Iterates to the next node in the linked_list.
// \param iterator: Iterator to iterate on.
// Returns TRUE when next node is present, FALSE once end of list is reached.
//
bool linked_list_iterate(struct iterator * iter){
    if(iter == NULL)
        return false;

    if(iter->current_node->next == NULL){
        return false;
    }

    iter->current_node = iter->current_node->next;
    iter->current_index++;
    iter->data = iter->current_node->data;

    return true;
}

// Registers malloc() function.
// \param malloc : Function pointer to malloc()-like function.
// Returns TRUE on success, FALSE otherwise.
//
bool linked_list_register_malloc(void * (*malloc)(size_t)){
    malloc_fptr = malloc;
    return true;
}

// Registers free() function.
// \param free : Function pointer to free()-like function.
// Returns TRUE on success, FALSE otherwise.
//
bool linked_list_register_free(void (*free)(void*)){
    free_fptr = free;
    return true;
}
