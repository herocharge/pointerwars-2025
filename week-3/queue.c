/**
 * @file queue.c
 * @author herocharge
 * @brief 
 * @version 0.1
 * @date 2025-09-01
 * 
 * MIT License

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

 * 
 */

#include "queue.h"

static void * (*malloc_fptr)(size_t size);
static void   (*free_fptr)(void* addr);



// Creates a new queue.
// PRECONDITION: Register malloc() and free() functions via the
//               queue_register_malloc() and 
//               queue_register_free() functions.
// Returns a new linked_list on success, NULL on failure.
//
struct queue * queue_create(void){

    struct queue * queue = malloc_fptr(sizeof(struct queue));
    if(queue == NULL){
        return NULL;
    }

    bool success = linked_list_create_in_place(&(queue->ll));
    if(!success){
        free_fptr(queue);
        return NULL;
    }
    
    return queue;
}

// Deletes a linked_list.
// \param queue : Pointer to queue to delete
// Returns TRUE on success, FALSE otherwise.
//
bool queue_delete(struct queue * queue){
    if(queue == NULL)
        return false;
    
    bool success = linked_list_remove_all(&(queue->ll));

    free_fptr(queue);
    return success;
}

// Pushes an unsigned int onto the queue.
// \param queue : Pointer to queue.
// \param data  : Data to insert.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_push(struct queue * queue, unsigned int data){
    if(queue == NULL)
        return false;
    
    bool success = linked_list_insert_end(&(queue->ll), data);
    if(!success)
        return false;
    
    return true;
}

// Pops an unsigned int from the queue, if one exists.
// \param queue       : Pointer to queue.
// \param popped_data : Pointer to popped data (provided by caller), if pop occurs.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_pop(struct queue * queue, unsigned int * popped_data){
    // NULL queue
    if(queue == NULL)
        return false;
    
    // Empty queue
    if(queue_size(queue) == 0){
        return false;
    }
    
    *popped_data = ((queue->ll).head)->data;
    bool success = linked_list_remove(&(queue->ll), 0);
    if(!success)
        return false;

    return true;
}

// Returns the size of the queue.
// \param queue : Pointer to queue.
// Returns size on success, SIZE_MAX otherwise.
//
size_t queue_size(struct queue * queue){
    if(queue == NULL)
        return SIZE_MAX;
    
    return (queue->ll).size;
}

// Returns whether an entry exists to be popped.
// \param queue: Pointer to queue.
// Returns TRUE if an entry can be popped, FALSE otherwise.
//
bool queue_has_next(struct queue * queue){
    if(queue == NULL)
        return false;
    
    return queue_size(queue) > 0;
}

// Returns the value at the head of the queue, but does
// not pop it.
// \param queue       : Pointer to queue.
// \param popped_data : Pointer to popped data (provided by caller), if pop occurs.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_next(struct queue * queue, unsigned int * popped_data){
    if(queue == NULL)
        return false;
    
    if(queue_size(queue) == 0){
        return false;
    }
    struct node* head = (queue->ll).head;
    if(head == NULL)
        return false;
    
    *popped_data = head->data;
    return true;
}

// Registers malloc() function.
// \param malloc : Function pointer to malloc()-like function.
// POSTCONDITION: Initializes malloc() function pointer in linked_list.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_register_malloc(void * (*malloc)(size_t)){
    linked_list_register_malloc(malloc);
    malloc_fptr = malloc;
    return true;
}

// Registers free() function.
// \param free : Function pointer to free()-like function.
// POSTCONDITION: Initializes free() functional pointer in linked_list.
// Returns TRUE on success, FALSE otherwise.
//
bool queue_register_free(void (*free)(void*)){
    linked_list_register_free(free);
    free_fptr = free;
    return true;
}
