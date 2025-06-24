#include "queue.h"

static void * (*malloc_fptr)(size_t size);
static void   (*free_fptr)(void* addr);

// Returns pointer to the linked list memeber of the queue
struct linked_list * _queue_get_ll(struct queue* queue){
    // When using queue->ll->(member),
    // we do two memory accesses, getting `queue` and then getting `ll`
    // Since I am allocating them beside each other,
    // I directly get `ll` using the address of queue

    // queue + 1 -> would skip the pointer by the size of queue 
    // because queue is of type struct queue

    return (struct linked_list*)(queue + 1);

    // TODO: #define _queue_get_ll LL
}

// Creates a new queue.
// PRECONDITION: Register malloc() and free() functions via the
//               queue_register_malloc() and 
//               queue_register_free() functions.
// Returns a new linked_list on success, NULL on failure.
//
struct queue * queue_create(void){

    // Allocating the queue struct and ll struct next to each other
    // to hopefully improve cache hits
    struct queue * queue = malloc_fptr(sizeof(struct queue) + sizeof(struct linked_list));
    if(queue == NULL){
        return NULL;
    }

    queue->ll = (struct linked_list*)(queue + 1);

    struct linked_list * ll = _queue_get_ll(queue);
    ll->head = NULL;
    ll->tail = NULL;
    ll->size = 0;
    return queue;
}

// Deletes a linked_list.
// \param queue : Pointer to queue to delete
// Returns TRUE on success, FALSE otherwise.
//
bool queue_delete(struct queue * queue){
    if(queue == NULL)
        return false;
    
    bool success = linked_list_remove_all(_queue_get_ll(queue));

    // TIL: malloc and free don't really care about types
    // If you call free on a pointer that isn't returned by
    // a malloc then it isn't valid.
    // If you allocated struct[100] using a single malloc
    // then calling free on the pointer to the first struct
    // will free the entire 100 elements because that's the 
    // block that was allocated, it doesnt care about type
    // TODO: fact check
    // src:  https://stackoverflow.com/a/20297598
    free_fptr(queue); // This call will free both queue and queue->ll;
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
    
    bool success = linked_list_insert_end(_queue_get_ll(queue), data);
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
    
    *popped_data = _queue_get_ll(queue)->head->data;
    bool success = linked_list_remove(_queue_get_ll(queue), 0);
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
    
    return _queue_get_ll(queue)->size;
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
    struct node* head = _queue_get_ll(queue)->head;
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
