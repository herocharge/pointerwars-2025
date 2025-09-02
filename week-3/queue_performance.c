#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef COMPILE_ARM_PMU_CODE
#include "arm_pmu.h"
#endif

#include "mmio.h"
#include "queue.h"

// A hacky adjacency matrix. 
//
struct row {
    size_t size;
    unsigned int * adjacent_nodes;
    bool visited;
};

struct row ** rows = NULL; 

// Malloc and free implementations and microbenchmarking.
//
#define GRAB_CLOCK(x) clock_gettime(CLOCK_MONOTONIC, &x);
#define MALLOC_MICRO_ITERATIONS 10000
void * malloc_ptrs[MALLOC_MICRO_ITERATIONS];
long average_malloc_time = 0L;
long average_free_time   = 0L;
size_t malloc_invocations = 0;
size_t free_invocations = 0;

void malloc_microbenchmark(void) {
    for (size_t i = 0; i < MALLOC_MICRO_ITERATIONS; i++) {
        malloc_ptrs[i] = malloc(sizeof(struct node));
    }
}

void free_microbenchmark(void) {
    for (size_t i = 0; i < MALLOC_MICRO_ITERATIONS; i++) {
        free(malloc_ptrs[i]);
    }
}

void * instrumented_malloc(size_t size) {
    ++malloc_invocations;
    return malloc(size);
}

void instrumented_free(void * addr) {
    ++free_invocations;
    free(addr);
}

long compute_timespec_diff(struct timespec start,
                           struct timespec stop) {
    long nanoseconds;
    nanoseconds = (stop.tv_sec - start.tv_sec) * 1000000000L;

    if (start.tv_nsec > stop.tv_nsec) {
        nanoseconds -= 1000000000L;
	nanoseconds += (start.tv_nsec - stop.tv_nsec);
    } else {
        nanoseconds += (stop.tv_nsec - start.tv_nsec);
    }

    return nanoseconds;
}

bool breadth_first_search(unsigned int i, unsigned int j) {
    struct queue * queue = queue_create();

    bool found_path = false;
    unsigned int next_node = i;
    size_t node_count = 0;
    struct timespec start, stop;
    int pushes = 0;
    int pops = 0;
    GRAB_CLOCK(start)
    while(!found_path) {
        // Push data onto the queue.
	//
        // printf("next node:%u\n", next_node);
        struct row * row = rows[next_node];

	if (row == NULL || row->visited) {
            bool not_done = queue_pop(queue, &next_node);
	    ++node_count;
	    if (!not_done) break;
	    continue;
	} else {
            row->visited = true;
	}

	if (row != NULL) { // didn't we check for this? will it be relevant in multicore? 
	    
       unsigned int* adjacent_nodes = row->adjacent_nodes;
        for(size_t node = 0; node < row->size; node++) {
            unsigned int data = adjacent_nodes[node];

            // Optimization 1: don't bother with nodes that were already visited
            if(rows[data] == NULL || rows[data]->visited)
                continue;
            // Check if we found the node.
            //
            if (j == data) {
                    found_path = true;
            }
            bool sanity = queue_push(queue, adjacent_nodes[node]);
            // pushes += 1;
            // fprintf(stderr, "PUSH\n");
            if (!sanity) {
                        printf("Error pushing into queue.\n");
                return 1;
            }
	    }

        // unrolling the loop, coz why not?
        // size_t node = 0;
        // for(; node + 15 < row->size; node += 16) {
        //     // TODO: SIMD This thing

        //     unsigned int data1 = adjacent_nodes[node];
        //     unsigned int data2 = adjacent_nodes[node+1];
        //     unsigned int data3 = adjacent_nodes[node+2];
        //     unsigned int data4 = adjacent_nodes[node+3];
        //     unsigned int data5 = adjacent_nodes[node+4];
        //     unsigned int data6 = adjacent_nodes[node+5];
        //     unsigned int data7 = adjacent_nodes[node+6];
        //     unsigned int data8 = adjacent_nodes[node+7];
        //     unsigned int data9 = adjacent_nodes[node+8];
        //     unsigned int data10 = adjacent_nodes[node+9];
        //     unsigned int data11 = adjacent_nodes[node+10];
        //     unsigned int data12 = adjacent_nodes[node+11];
        //     unsigned int data13 = adjacent_nodes[node+12];
        //     unsigned int data14 = adjacent_nodes[node+13];
        //     unsigned int data15 = adjacent_nodes[node+14];
        //     unsigned int data16 = adjacent_nodes[node+15];

        //     if(j == data1 || j == data2 || j == data3 || j == data4 ||
        //        j == data5 || j == data6 || j == data7 || j == data8 ||
        //        j == data9 || j == data10 || j == data11 || j == data12 ||
        //        j == data13 || j == data14 || j == data15 || j == data16) {
        //         found_path = true;
        //         break;
        //     }

        //     // if(rows[data1] != NULL && !rows[data1]->visited){
        //         queue_push(queue, data1);
        //     // }
        //     // if(rows[data2] != NULL && !rows[data2]->visited){
        //         queue_push(queue, data2);
        //     // }
        //     // if(rows[data3] != NULL && !rows[data3]->visited){
        //         queue_push(queue, data3);
        //     // }
        //     // if(rows[data4] != NULL && !rows[data4]->visited){
        //         queue_push(queue, data4);
        //     // }
        //     // if(rows[data5] != NULL && !rows[data5]->visited){
        //         queue_push(queue, data5);
        //     // }
        //     // if(rows[data6] != NULL && !rows[data6]->visited){
        //         queue_push(queue, data6);
        //     // }
        //     // if(rows[data7] != NULL && !rows[data7]->visited){
        //         queue_push(queue, data7);
        //     // }
        //     // if(rows[data8] != NULL && !rows[data8]->visited){
        //         queue_push(queue, data8);
        //     // }
        //     // if(rows[data9] != NULL && !rows[data9]->visited){
        //         queue_push(queue, data9);
        //     // }
        //     // if(rows[data10] != NULL && !rows[data10]->visited){
        //         queue_push(queue, data10);
        //     // }
        //     // if(rows[data11] != NULL && !rows[data11]->visited){
        //         queue_push(queue, data11);
        //     // }
        //     // if(rows[data12] != NULL && !rows[data12]->visited){
        //         queue_push(queue, data12);
        //     // }
        //     // if(rows[data13] != NULL && !rows[data13]->visited){
        //         queue_push(queue, data13);
        //     // }
        //     // if(rows[data14] != NULL && !rows[data14]->visited){
        //         queue_push(queue, data14);
        //     // }
        //     // if(rows[data15] != NULL && !rows[data15]->visited){
        //         queue_push(queue, data15);
        //     // }
        //     // if(rows[data16] != NULL && !rows[data16]->visited){
        //         queue_push(queue, data16);
        //     // }
        // }
        // for(; node < row->size; node++) {
        //     unsigned int data = adjacent_nodes[node];

        //     // Optimization 1: don't bother with nodes that were already visited
        //     // if(rows[data] == NULL || rows[data]->visited)
        //     //     continue;
        //     // Check if we found the node.
        //     //
        //     if (j == data) {
        //             found_path = true;
        //             break;
        //     }
        //     queue_push(queue, adjacent_nodes[node]);
        //     // if (!sanity) {
        //     //             printf("Error pushing into queue.\n");
        //     //     return 1;
        //     // }

        // }
	}

	// Pop the next row off the queue.
	//
	bool full = queue_pop(queue, &next_node);
    // pops += 1;
    // fprintf(stderr, "POP\n");
	if (!full) {
            break;
	}
	++node_count;
    }
    queue_delete(queue);
    GRAB_CLOCK(stop)
    long nanoseconds = compute_timespec_diff(start, stop);
    printf("Nodes visited: %ld\n", node_count);
    printf("Time elapsed [s]: %0.3f\n", (float)nanoseconds / 1000000000.0f);
    fprintf(stderr, "%0.3f\n", (float)nanoseconds / 1000000000.0f);
    fprintf(stderr, "%d %d\n", pushes, pops);
    printf("malloc calls : %ld free calls: %ld\n", malloc_invocations, free_invocations);
    printf("Estimated percentage of time spent in malloc() %0.3f\n", 100.0f * (float)(malloc_invocations * average_malloc_time) / (float)nanoseconds);
    printf("Estimated percentage of time spent in free(): %0.3f\n", 100.0f * (float)(free_invocations * average_free_time) / (float)nanoseconds);
    return found_path;
}

void add_edge(unsigned int i, unsigned int j) {
    // Check whether row i exists, if not allocate.
    //
    if (rows[i] == NULL) {
        rows[i] = (struct row*)malloc(sizeof(struct row));

        if (rows[i] == NULL) {
            printf("Failed to allocate edge, exiting.\n");
	    exit(1);
	}

	rows[i]->size              = 1;
	rows[i]->adjacent_nodes    = malloc(16 * sizeof(unsigned int));
	rows[i]->visited           = false;
	if (rows[i]->adjacent_nodes == NULL) {
            printf("Unable to malloc adjacent_nodes.\n");
	    exit(1);
	}
	rows[i]->adjacent_nodes[0] = j;
    } else {
        // Check whether to perform realloc.
	// Every 16 nodes we allocate another 16.
	//
	size_t size = rows[i]->size;
	if (size % 16 == 15) {
             rows[i]->adjacent_nodes = realloc(rows[i]->adjacent_nodes, (size + 1 + 16) * sizeof(unsigned int));

	     if (rows[i] == NULL) {
                 printf("Failed to realloc adjacent nodes.\n");
		 exit(1);
	     }
	}

	rows[i]->adjacent_nodes[size] = j;
	++rows[i]->size;
    }
}

int main(void) {

    // Initialize malloc() and free()
    //
    queue_register_malloc(instrumented_malloc);
    queue_register_free(instrumented_free);

#ifdef COMPILE_ARM_PMU_CODE
    // Register ARM PMUs
    //
    setup_pmu_events();
#endif

    // Microbenchmark malloc() and free().
    // These function calls are too short to wrap a high
    // precision timer around them, so run them 10,000 times
    // and take the arithmetic mean.
    //
    for(size_t i = 0; i < 4; i++) {
        // Warm them up a few times.
	//
        malloc_microbenchmark();
	free_microbenchmark();
    }

    // Measure.
    //
    struct timespec malloc_start, malloc_end;
    struct timespec free_start, free_end;

    GRAB_CLOCK(malloc_start)
    malloc_microbenchmark();
    GRAB_CLOCK(malloc_end)
    GRAB_CLOCK(free_start)
    free_microbenchmark();
    GRAB_CLOCK(free_end)
    average_malloc_time = compute_timespec_diff(malloc_start, malloc_end) / MALLOC_MICRO_ITERATIONS;
    average_free_time   = compute_timespec_diff(free_start, free_end) / MALLOC_MICRO_ITERATIONS;

    printf("Average time [ns] per malloc() call: %ld\n", average_malloc_time);
    printf("Average time [ns] per free() call: %ld\n", average_free_time);

    // Parse the file.
    //
    FILE* fptr      = fopen("wikipedia-20070206/wikipedia-20070206.mtx", "r");
    FILE* node_fptr = fopen("nodes", "r"); 

    if (fptr == NULL) {
        printf("Error opening matrix.\n");
	printf("Did you run 'make download_and_decompress_test_data'?\n");
        return 1;
    }

    if (node_fptr == NULL) {
        printf("Error opening node list.\n");
	return 1;
    }

    MM_typecode matrix_code;

    if (mm_read_banner(fptr, &matrix_code) != 0) {
        printf("Malformed Matrix Market file.\n");
	return 1;
    }

    // Determine size of MxN matrix with total non-zero size nz.
    //
    int m, n, nz;
    if (mm_read_mtx_crd_size(fptr, &m, &n, &nz)) {
        printf("Unable to read size of matrix.\n");
	return 1;
    }

    if (m != n) {
        printf("Matrix row and column size not equal. m: %d n: %d\n",
               m, n);
        return 1;
    }

    printf("Wikipedia matrix size m: %d n: %d nz: %d\n", m, n, nz);

    // Start reading in the data.
    //
    rows = (struct row**)malloc(sizeof(struct row*) * (m + 1));
    if (rows == NULL) {
        printf("Failed to allocate row array.\n");
	return 1;
    }

    printf("Allocated %ld bytes for row array.\n",
           sizeof(struct row*) * m + 1);

    // Zero out the row array.
    // A NULL means that a particular node in the graph
    // has no directed edges to other nodes.
    //
    for (int i = 0; i < m + 1; i++) {
        rows[i] = NULL;
    }

    // Parse.
    //
    size_t line_count = 0;
    while(!feof(fptr)) {
	// Grab next directed edge.
	// A pair (i, j) means that node i links to node j.
	//
	unsigned int i, j;
        int retval = fscanf(fptr, "%d %d", &i, &j);
	if (!(retval == 2 || retval == -1)) {
            printf("File parsing error with fscanf() return value of: %d.\n", retval);
	    return 1;
	}

	add_edge(i, j);
	++line_count;
    }
    printf("Read %ld lines of matrix data.\n", line_count);

    // Start the BFS.
    //
    for (size_t i = 0; i < 100; i++) {
        unsigned int node_i = 0; 
        unsigned int node_j = 0;
        int retval = fscanf(node_fptr, "%d %d\n", &node_i, &node_j);	
	if (!(retval == 2 || retval == -1)) {
            printf("Parsing error.\n");
	    return 1;
	}
        printf("(%ld / %ld) Searching for a connection between node %d -> %d\n", 
               i + 1, 100L, node_i, node_j);
#ifdef COMPILE_ARM_PMU_CODE
	reset_and_start_pmu_counters();
#endif
        bool success = breadth_first_search(node_i, node_j);
#ifdef COMPILE_ARM_PMU_CODE
	stop_pmu_counters();
#endif
        if (success) {
            printf("Path found.\n");
        } else {
            printf("No path found.\n");
        }

	// Clear visited fields for next run.
	//
        for (int j = 0; j < m + 1; j++) {
            if (rows[j]) {
                rows[j]->visited = false;
	    }
	}

	// Grab PMU data.
	//
#ifdef COMPILE_ARM_PMU_CODE
	uint64_t pmu_counters[PERF_EVENT_COUNT];
	read_pmu_data(pmu_counters);
	printf("L1D_CACHE_LD: %ld\n", pmu_counters[0]);
	printf("L1D_CACHE_REFILL_LD: %ld\n", pmu_counters[1]);
	printf("L2D_CACHE_REFILL_LD: %ld\n", pmu_counters[2]);
	printf("L1D_TLB_REFILL_LD: %ld\n", pmu_counters[3]);
	printf("BR_PRED: %ld\n", pmu_counters[4]);
	printf("BR_MIS_PRED: %ld\n", pmu_counters[5]);

	printf("L1D load hit rate: %0.3f\n", 1.0f - ((float)pmu_counters[1] / (float)pmu_counters[0]));
	printf("DTLB load hit rate: %0.3f\n", 1.0f - ((float)pmu_counters[3] / (float)pmu_counters[0]));
	printf("L2D load hit rate %0.3f\n", 1.0f - ((float)pmu_counters[1] / (float)pmu_counters[2]));
	printf("Branch prediction accuracy: %0.3f\n", 1.0f - ((float)pmu_counters[5] / (float)pmu_counters[4]));
#endif

	// Clear malloc and free invocation counts.
	//
	malloc_invocations = 0;
	free_invocations   = 0;
    // break;
    }

    printf("All work complete, exit.\n");
    fflush(stdout);

    // Free
    //
    for (int i = 0; i < m + 1; i++) {
        if (rows[i] == NULL) continue;
	free(rows[i]->adjacent_nodes);
	free(rows[i]);
    }

    free(rows);
    fclose(fptr);

    return 0;
}
