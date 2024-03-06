# Parallel Graph

## Overview
This project is a comprehensive exploration into parallel programming, emphasizing the use of a thread pool for efficient graph processing. The objective is to improve understanding of synchronization mechanisms, POSIX threading, and the comparison between serial and parallel execution methodologies.

## src/ Directory Contents:

### Graph Handling (`os_graph.c`, `os_graph.h`)
Implements functionalities for creating and managing graphs. Supports node and edge manipulations, facilitating graph traversals and data manipulation.

### Queue Implementation (`os_list.c`, `os_list.h`)
Defines a queue structure, essential for task scheduling in the thread pool. It supports operations such as enqueue and dequeue, underpinning the task management within the thread pool system.

### Thread Pool Framework (`os_threadpool.c`, `os_threadpool.h`)
Skeleton for developing a robust thread pool. Includes placeholders for initializing the pool, managing the lifecycle of threads, and handling task execution. Key for enabling parallel processing of graph data.

### Parallel Graph Processing (`parallel.c`)
Utilizes the thread pool to traverse and process graph nodes in parallel. A showcase of the potential speed-up achievable through concurrent task execution compared to serial approaches.

### Serial Graph Processing (`serial.c`)
Provides a baseline serial approach to graph traversal and processing for performance comparison against the parallel implementation.
