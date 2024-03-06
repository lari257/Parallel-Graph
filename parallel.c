// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "os_list.h"

#define MAX_TASK 100
#define MAX_THREAD 4

static int sum = 0;
static os_graph_t *graph;

/* Structura folosita pentru un task de parcurgere a grafului */
typedef struct {
	int start, end;
	pthread_mutex_t* lock;
} task_data;

/* Functie de executare a adunarii in mod atomic */
void atomic_add(int* acc, int addition, pthread_mutex_t* lock) {
	pthread_mutex_lock(lock);
	*acc += addition;
	pthread_mutex_unlock(lock);
}

/* processNode si traverse_graph sunt preluate din implementarea seriala */
/* aceste functii sunt adaptate pentru a fi apelate in cadrul unui thread */
void processNode(unsigned int nodeIdx, pthread_mutex_t* lock)
{
    os_node_t *node = graph->nodes[nodeIdx];
    for (int i = 0; i < node->cNeighbours; i++)
        if (graph->visited[node->neighbours[i]] == 0) {
			pthread_mutex_lock(lock);
            graph->visited[node->neighbours[i]] = 1;
			pthread_mutex_unlock(lock);
            processNode(node->neighbours[i], lock);
        }
}

void traverse_graph(void* arg)
{
	int start = ((task_data*)arg)->start;
	int end = ((task_data*)arg)->end;
	pthread_mutex_t* lock = ((task_data*)arg)->lock;

    for (int i = start; i < end; i++)
    {
        if (graph->visited[i] == 0) {
			pthread_mutex_lock(lock);
            graph->visited[i] = 1;
			pthread_mutex_unlock(lock);
            processNode(i, lock);
        }
    }
	for (int i = start; i < end; i++) {
		atomic_add(&sum, graph->nodes[i]->nodeInfo, lock);
	}

	free(arg);
}

int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");
	if (input_file == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	graph = create_graph_from_file(input_file);
	if (graph == NULL) {
		fprintf(stderr, "[Error] Can't read the graph from file\n");
		return -1;
	}

	pthread_mutex_t lock;

	// Initializam un mutex pe care sa-l partajam task-urilor
	pthread_mutex_init(&lock, NULL);

	// Cream thread pool-ul
	os_threadpool_t* tp = threadpool_create(MAX_TASK, MAX_THREAD);

	// Calculam numarul de noduri de care ar trebui sa se ocupe fiecare task submis
    int nodes_for_task = graph->nCount / MAX_TASK;

	// Cream resursele necesare si adaugam in coada de procesare task-urile pt pool
	for (int i = 0; i < MAX_TASK; i++) {
		task_data* args = malloc(sizeof(task_data));
		args->start = i * nodes_for_task;
		args->end = (i == MAX_TASK - 1) ? 
			graph->nCount : (i + 1) * nodes_for_task;
		
		args->lock = &lock;

		add_task_in_queue(tp, task_create(args, traverse_graph));
	}

	// Lansam executarea thread-urilor
	for (int i = 0; i < tp->num_threads; i++) {
		pthread_create(&tp->threads[i], NULL, thread_loop_function, tp);
	}

	// Asteptam finalizarea task-urilor si dealocam resursele folosite pt pool
	threadpool_stop(tp, NULL);
	pthread_mutex_destroy(&lock);

	printf("%d", sum);

	return 0;
}
