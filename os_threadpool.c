// SPDX-License-Identifier: BSD-3-Clause

#include "os_threadpool.h"
#include "os_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

os_queue_t* queue = NULL;

/* === TASK === */

/* Creates a task that thread must execute */
os_task_t *task_create(void *arg, void (*f)(void *))
{
	os_task_t* task = malloc(sizeof(os_task_t));
	task->argument = arg;
	task->task = f;
	return task;
}

/* Add a new task to threadpool task queue */
void add_task_in_queue(os_threadpool_t *tp, os_task_t *t)
{
	/* Verificam daca coada a fost creata */
	if (queue) {
		queue_add(queue, t);
	}
}

/* Get the head of task queue from threadpool */
os_task_t *get_task(os_threadpool_t *tp)
{	
	/* Extragem task din coada */
	os_list_node_t* front_node = queue_get(queue);

	/* Verificam daca am extras intr-adevar un task */
	if (!front_node) {
		return NULL;
	}

	/* Facem cast de la void* la tipul pointerm catre task */
	os_task_t* to_process_task = (os_task_t*)(front_node->info);

	/* Eliberam memoria alocata pentru nodul cozii care continea datele necesare */
	free(front_node);
	return to_process_task;
}

/* === THREAD POOL === */

/* Initialize the new threadpool */
os_threadpool_t *threadpool_create(unsigned int nTasks, unsigned int nThreads)
{
	os_threadpool_t* tp = malloc(sizeof(os_threadpool_t));

	/* Alocare resurse necesare pentru thread pool */
	queue = queue_create();
	tp->tasks = NULL;
	tp->num_threads = nThreads;
	tp->should_stop = nTasks;
	tp->threads = malloc(nThreads * sizeof(pthread_t));
	
	return tp;
}

/* Loop function for threads */
void *thread_loop_function(void *args)
{
	os_threadpool_t* tp = (os_threadpool_t*)args;
	while (1) {
		/* Mutex pe coada pentru zona critica */
		/* Operatia de extragere din coada trebuie sa fie efectuata atomic */
		pthread_mutex_lock(&queue->lock);
		if (tp->should_stop == 0) {
			pthread_mutex_unlock(&queue->lock);
			return NULL;
		}
		tp->should_stop--;
		os_task_t* task = get_task(NULL);
		pthread_mutex_unlock(&queue->lock);

		/* Daca extragem null nu avem ce executa */
		if (task) {
			task->task(task->argument);
			free(task);
		}
	}
	return NULL;
}

/* Stop the thread pool once a condition is met */
void threadpool_stop(os_threadpool_t *tp, int (*processingIsDone)(os_threadpool_t *))
{
	/* Cat timp pool-ul nu a terminat de procesat toate task-urile */
	while (tp->should_stop > 0);

	/* Se asteapta finalizarea thread-urilor alocate */
	for (int i = 0; i < tp->num_threads; i++) {
		pthread_join(tp->threads[i], NULL);
	}

	/* Eliberam resursele folosite de catre thread pool */
	free(tp->threads);
	free(tp);
	free(queue);
}
