/**
 * @file linked_list.c
 * @author herocharge
 * @brief 
 * @version 0.1
 * @date 2025-09-01
 * 
 * 
 *  MIT License

    Copyright (c) 2025 herocharge

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

 * */

#include "linked_list.h"
#include <stdio.h>
#include <assert.h>

#define ALLOC_SIZE 4096 * 1024
#define ALLOC_DOUBLE 1

// Function pointers to (potentially) custom malloc() and
// free() functions.
//
static void * (*malloc_fptr)(size_t size) ;
static void   (*free_fptr)(void* addr);

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
    ll->tail = NULL;
    ll->free_stack = NULL;
    ll->size = 0;
    return ll;
}

bool linked_list_create_in_place(struct linked_list* ll){
    if(ll == NULL)
        return false;
    ll->head = NULL;
    ll->tail = NULL;
    ll->free_stack = NULL;
    ll->size = 0;
    return true;
}

// Assuming ll != NULL
bool __linked_list_save_in_free_stack(struct linked_list * ll, struct node* node){
    node->next = ll->free_stack;
    ll->free_stack = node;
    return true;
}

// Deletes a linked_list.
// \param ll : Pointer to linked_list to delete
// POSTCONDITION : An empty linked_list has its head point to NULL.
// Returns TRUE on success, FALSE otherwise.
//
bool linked_list_delete(struct linked_list * ll){
    if(ll == NULL)
        return false;
    
    struct node* curr = ll->head;
    struct node* old_free_stack = ll->free_stack;
    int block_count = 0;
    while(curr != NULL){
        struct node* next = curr->next;
        // free_fptr(curr); // TODO: check for fail conditions
        if(curr->is_block_head){
            block_count += 1;
            __linked_list_save_in_free_stack(ll, curr);
        }
        curr = next;
    }

    curr = old_free_stack;
    while(curr != NULL){
        struct node* next = curr->next;
        // free_fptr(curr);
        if(curr->is_block_head){
            block_count += 1;
            __linked_list_save_in_free_stack(ll, curr);
        }
        curr = next;
    }
    
    curr = ll->free_stack;
    while(block_count--){
        struct node* next = curr->next;
        free_fptr(curr);
        curr = next;
    }

    free_fptr(ll);

    return true;    
}

// Removes all elements from linked list
// Returns TRUE on success, FALSE otherwise
bool linked_list_remove_all(struct linked_list * ll){
    if(ll == NULL)
        return false;
    
    struct node* curr = ll->head;
    struct node* old_free_stack = ll->free_stack;
    int block_count = 0;
    while(curr != NULL){
        struct node* next = curr->next;
        // free_fptr(curr); // TODO: check for fail conditions
        if(curr->is_block_head){
            block_count += 1;
            __linked_list_save_in_free_stack(ll, curr);
        }
        curr = next;
    }

    curr = old_free_stack;
    while(curr != NULL){
        struct node* next = curr->next;
        // free_fptr(curr);
        if(curr->is_block_head){
            block_count += 1;
            __linked_list_save_in_free_stack(ll, curr);
        }
        curr = next;
    }
    curr = ll->free_stack;
    while(block_count--){
        struct node* next = curr->next;
        free_fptr(curr);
        curr = next;
    }

    ll->head = NULL;
    ll->tail = NULL;
    ll->free_stack = NULL;
    ll->size = 0;

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

struct node * __linked_list_allocate_block(struct linked_list* ll, size_t size){
    assert(size >= 1);
    struct node* head = malloc_fptr(sizeof(struct node) * size);
    if(head == NULL)
        return NULL;
    
    head->is_block_head = true;

    struct node* curr = head;
    for(size_t i = 0; i < size - 1; i++){
        curr->next = curr + 1;
        curr = curr->next;
        curr->is_block_head = false;
    }
    curr->next = NULL;
    ll->free_stack = head->next;
    return head;
}

/// @brief If there is a free node in free stack, then give that, otherwise malloc
/// @param ll 
/// @return node pointer for a free noed
struct node* __linked_list_get_new_node(struct linked_list* ll){
    if(ll->free_stack != NULL){
        struct node* tmp = ll->free_stack;
        ll->free_stack = ll->free_stack->next;
        return tmp;
    }
    else {
        #if ALLOC_DOUBLE
        size_t extra_size = 1024 * 16;
        if(ll->size > extra_size)
            extra_size = ll->size;
        return __linked_list_allocate_block(ll, extra_size);
        #else
        return __linked_list_allocate_block(ll, ALLOC_SIZE);
        #endif
    }
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
    struct node* new_node = __linked_list_get_new_node(ll);
    
    if(new_node == NULL)
        return false;

    new_node->data = data;
    new_node->next = NULL;

    if(ll->head == NULL){
        ll->head = new_node;
        ll->tail = new_node;
    }
    else{
        new_node->next = ll->tail->next;
        ll->tail->next = new_node;
        ll->tail = new_node;
    }

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
    struct node* new_node = __linked_list_get_new_node(ll);

    if(new_node == NULL)
        return false;

    new_node->data = data;
    new_node->next = ll->head;
    ll->head = new_node;
    ll->size += 1;
    if(ll->tail == NULL){
        ll->tail = new_node;
    }
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
    
    if(index == ll->size)
        return linked_list_insert_end(ll, data);

    struct node* new_node = __linked_list_get_new_node(ll);

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
    while(curr != NULL && index < ll->size){
        if(curr->data == data){
            return index;
        }
        curr = curr->next;
        index++;
    }
    return SIZE_MAX;
}



// Assuming ll != NULL;; consider making this public
bool __linked_list_remove_top(struct  linked_list * ll){
    struct node* tmp = ll->head;
    if(tmp == NULL) return false;
    
    ll->head = tmp->next;
    // free_fptr(tmp);
    __linked_list_save_in_free_stack(ll, tmp);
    ll->size -= 1;

    if(ll->size <= 1){
        ll->tail = ll->head;
    }
    
    return true;
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
        return __linked_list_remove_top(ll);
    }

    struct node* curr = ll->head;
    
    for(size_t i = 0; i < index - 1; i++){
        curr = curr->next;
    }

    struct node* node_to_remove = curr->next;
    
    // Update tail if needed
    if(index == ll->size - 1){
        ll->tail = curr;
    }
    
    curr->next = node_to_remove->next;
    // free_fptr(node_to_remove);
    __linked_list_save_in_free_stack(ll, node_to_remove);
    
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
    
    if(index >= ll->size)
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
        return false;

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

    if(iter->current_index + 1 >= iter->ll->size){
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
