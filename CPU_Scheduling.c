#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#define MAX_NAME_LENGTH 10
#define MAX_PROCESSES 100

// Structure to represent a process
struct Process {
    char name[MAX_NAME_LENGTH];
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    int priority;
    int responseTime;
    int startTime;
};

// Structure to represent a task queue
struct TaskQueue {
    struct Process processes[MAX_PROCESSES];
    int front;
    int rear;
    int size;
};

// Structure to represent a Gantt chart entry
struct GanttEntry {
    char processName[10];
    int executionTime;
};
int numOfAddedProcesses = 0;

struct TaskQueue originalTaskQueue;


// Function prototypes
void enqueue(struct TaskQueue* queue, struct Process process);
void fcfsScheduling(struct TaskQueue* taskQueue);
void sjfPreemptiveScheduling(struct TaskQueue* taskQueue);
void roundRobinScheduling(struct TaskQueue* taskQueue, int timeQuantum);
void prioritySchedulingWithPreemption(struct TaskQueue* taskQueue);
void printProcessTable(struct TaskQueue* taskQueue);
void printMetricsTable(struct TaskQueue* taskQueue);
void removeProcess(struct TaskQueue* queue, char* processName);
void clearInputBuffer();
int min(int a, int b);
void initializeOriginalTaskQueue(struct TaskQueue* taskQueue);

//remove process
void removeProcess(struct TaskQueue* queue, char* processName) {
    int found = 0;
    for (int i = queue->front; i <= queue->rear; i++) {
        if (strcmp(queue->processes[i].name, processName) == 0) {
            found = 1;
            // Shift the remaining processes to the left
            for (int j = i; j < queue->rear; j++) {
                queue->processes[j] = queue->processes[j + 1];
            }
            queue->rear--;
            queue->size--;
            break;
        }
    }
    if (found) {
        printf("Process '%s' removed successfully.\n", processName);
    } else {
        printf("Process '%s' not found in the task queue.\n", processName);
    }
}
// Enqueues a process into the task queue
void enqueue(struct TaskQueue* queue, struct Process process) {
    if (queue->rear == MAX_PROCESSES - 1) {
        printf("Queue is full. Cannot enqueue more tasks.\n");
        return;
    }
    // Increase the rear index and add the process to the queue
    queue->rear++;
    queue->processes[queue->rear] = process;
    queue->size++;
}
// Finds the minimum of two numbers
int min(int a, int b) {
    return (a < b) ? a : b;
}
//getProcessByName
struct Process* getProcessByName(struct TaskQueue* taskQueue, char* name) {
    for (int i = 0; i < taskQueue->size; i++) {
        if (strcmp(taskQueue->processes[i].name, name) == 0) {
            return &taskQueue->processes[i];
        }
    }
    return NULL;
}
// Performs FCFS scheduling on the task queue
void fcfsScheduling(struct TaskQueue* taskQueue) {
    int currentTime = 0;
    int completedProcesses = 0;

    // Create a Gantt chart array to store process names, start times, and execution times
    struct GanttEntry {
        char processName[10];
        int startTime;
        int executionTime;
    };
    struct GanttEntry ganttChart[MAX_PROCESSES * 100];  // Assuming maximum 100 time slots per process
    int ganttIndex = 0;  // Index to keep track of the Gantt chart entries

    // Loop until all processes are completed
    while (completedProcesses < taskQueue->size) {
        bool processExecuted = false;
        // Loop through all the processes in the task queue
        for (int i = 0; i < taskQueue->size; i++) {
            struct Process* process = &taskQueue->processes[i];
            // If the process is not completed and its arrival time is <= current time
            if (process->remainingTime > 0 && process->arrivalTime <= currentTime) {
                // Execute the process for the remaining time
                int executionTime = process->remainingTime;
                process->remainingTime -= executionTime;
                currentTime += executionTime;  // Update current time
                processExecuted = true;

                // Store the Gantt chart entry for the process
                strcpy(ganttChart[ganttIndex].processName, process->name);
                ganttChart[ganttIndex].startTime = currentTime - executionTime;
                ganttChart[ganttIndex].executionTime = executionTime;
                ganttIndex++;

                // If the process is completed
                if (process->remainingTime == 0) {
                    completedProcesses++;  // Increment the number of completed processes
                    process->completionTime = currentTime;  // Set the completion time
                }
            }
        }
        // If no process could be executed, increment current time
        if (!processExecuted) {
            currentTime++;  // Increment time if no process could execute

            // Store the Gantt chart entry for idle time
            strcpy(ganttChart[ganttIndex].processName, "IDLE");
            ganttChart[ganttIndex].startTime = currentTime - 1;
            ganttChart[ganttIndex].executionTime = 1;
            ganttIndex++;
        }
    }

    // Set the start time for each process based on the first appearance in the Gantt chart
    for (int i = 0; i < taskQueue->size; i++) {
        struct Process* process = &taskQueue->processes[i];
        for (int j = 0; j < ganttIndex; j++) {
            if (strcmp(process->name, ganttChart[j].processName) == 0) {
                process->startTime = ganttChart[j].startTime;
                break;
            }
        }
    }
// Print the execution order
printf("\nThe order in which processes or threads are selected for execution:\n");
for (int i = 0; i < ganttIndex; i++) {
    if (strcmp(ganttChart[i].processName, "IDLE") != 0) {
        struct Process* process = getProcessByName(taskQueue, ganttChart[i].processName);
        if (process->remainingTime == 0) {
            printf("From \"%d\" to \"%d\": \"%s\" (completed)\n", ganttChart[i].startTime, ganttChart[i].startTime + ganttChart[i].executionTime, ganttChart[i].processName);
        } else {
            printf("From \"%d\" to \"%d\": \"%s\" (%d time units left)\n", ganttChart[i].startTime, ganttChart[i].startTime + ganttChart[i].executionTime, ganttChart[i].processName, process->remainingTime);
        }
    }
}


    // Print the Gantt chart
    printf("\nGantt Chart:\n");
    printf("---------------------------------------------------------------------\n");
    printf("|");
    for (int i = 0; i < ganttIndex; i++) {
        printf(" %s ", ganttChart[i].processName);
        if (i < ganttIndex - 1 && strcmp(ganttChart[i].processName, "IDLE") != 0 &&
            ganttChart[i].startTime + ganttChart[i].executionTime < ganttChart[i + 1].startTime) {
            printf("| IDLE ");
        }
        printf("|");
    }
    printf("\n---------------------------------------------------------------------\n");
    printf("%d", ganttChart[0].startTime);
    int cumulativeTime = ganttChart[0].startTime;
    for (int i = 0; i < ganttIndex; i++) {
        cumulativeTime += ganttChart[i].executionTime;
        printf("------%d", cumulativeTime);
    }
    printf("\n");

    // Print the process table and metrics table
    printProcessTable(taskQueue);
    printMetricsTable(taskQueue);
}
// Performs RR scheduling on the task queue
void roundRobinScheduling(struct TaskQueue* taskQueue, int quantum) {
    int currentTime = 0;
    int completedProcesses = 0;

    // Create a Gantt chart array to store process names, start times, and execution times
    struct GanttEntry {
        char processName[10];
        int startTime;
        int executionTime;
    };
    struct GanttEntry ganttChart[MAX_PROCESSES * 100];  // Assuming maximum 100 time slots per process
    int ganttIndex = 0;  // Index to keep track of the Gantt chart entries

    // Loop until all processes are completed
    while (completedProcesses < taskQueue->size) {
        bool processExecuted = false;
        // Loop through all the processes in the task queue
        for (int i = 0; i < taskQueue->size; i++) {
            struct Process* process = &taskQueue->processes[i];
            // If the process is not completed and its arrival time is <= current time
            if (process->remainingTime > 0 && process->arrivalTime <= currentTime) {
                // Execute the process for the remaining time or the quantum, whichever is smaller
                int executionTime = process->remainingTime < quantum ? process->remainingTime : quantum;
                process->remainingTime -= executionTime;
                currentTime += executionTime;  // Update current time
                processExecuted = true;

                // Store the Gantt chart entry for the process
                strcpy(ganttChart[ganttIndex].processName, process->name);
                ganttChart[ganttIndex].startTime = currentTime - executionTime;
                ganttChart[ganttIndex].executionTime = executionTime;
                ganttIndex++;

                // If the process is completed
                if (process->remainingTime == 0) {
                    completedProcesses++;  // Increment the number of completed processes
                    process->completionTime = currentTime;  // Set the completion time
                }
            }
        }
        // If no process could be executed, increment current time
        if (!processExecuted) {
            currentTime++;  // Increment time if no process could execute

            // Store the Gantt chart entry for idle time
            strcpy(ganttChart[ganttIndex].processName, "IDLE");
            ganttChart[ganttIndex].startTime = currentTime - 1;
            ganttChart[ganttIndex].executionTime = 1;
            ganttIndex++;
        }
    }

    // Set the start time for each process based on the first appearance in the Gantt chart
    for (int i = 0; i < taskQueue->size; i++) {
        struct Process* process = &taskQueue->processes[i];
        for (int j = 0; j < ganttIndex; j++) {
            if (strcmp(process->name, ganttChart[j].processName) == 0) {
                process->startTime = ganttChart[j].startTime;
                break;
            }
        }
    }


    // Print the Gantt chart
    printf("\nGantt Chart:\n");
    printf("---------------------------------------------------------------------\n");
    printf("|");
    for (int i = 0; i < ganttIndex; i++) {
        printf(" %s ", ganttChart[i].processName);
        if (i < ganttIndex - 1 && strcmp(ganttChart[i].processName, "IDLE") != 0 &&
            ganttChart[i].startTime + ganttChart[i].executionTime < ganttChart[i + 1].startTime) {
            printf("| IDLE ");
        }
        printf("|");
    }
    printf("\n---------------------------------------------------------------------\n");
    printf("%d", ganttChart[0].startTime);
    int cumulativeTime = ganttChart[0].startTime;
    for (int i = 0; i < ganttIndex; i++) {
        cumulativeTime += ganttChart[i].executionTime;
        printf("------%d", cumulativeTime);
    }
    printf("\n");
// Print the execution order
printf("\nThe order in which processes or threads are selected for execution:\n");
for (int i = 0; i < ganttIndex; i++) {
    if (strcmp(ganttChart[i].processName, "IDLE") != 0) {
        struct Process* process = getProcessByName(taskQueue, ganttChart[i].processName);
        if (process->remainingTime == 0) {
            printf("From \"%d\" to \"%d\": \"%s\" (completed)\n", ganttChart[i].startTime, ganttChart[i].startTime + ganttChart[i].executionTime, ganttChart[i].processName);
        } else {
            printf("From \"%d\" to \"%d\": \"%s\" (%d time units left)\n", ganttChart[i].startTime, ganttChart[i].startTime + ganttChart[i].executionTime, ganttChart[i].processName, process->remainingTime);
        }
    }
}

    // Print the process table and metrics table
    printProcessTable(taskQueue);
    printMetricsTable(taskQueue);
}
// Performs SJF scheduling on the task queue (preemptive)
void sjfPreemptiveScheduling(struct TaskQueue* taskQueue) {
    int currentTime = 0;
    int completedProcesses = 0;

    // Create a Gantt chart array to store process names, start times, and execution times
    struct GanttEntry {
        char processName[10];
        int startTime;
        int executionTime;
    };
    struct GanttEntry ganttChart[MAX_PROCESSES * 100];  // Assuming maximum 100 time slots per process
    int ganttIndex = 0;  // Index to keep track of the Gantt chart entries

    // Loop until all processes are completed
    while (completedProcesses < taskQueue->size) {
        int shortestIndex = -1;
        int shortestBurstTime = INT_MAX;

        // Find the process with the shortest burst time among the arrived processes
        for (int i = 0; i < taskQueue->size; i++) {
            struct Process* process = &taskQueue->processes[i];
            if (process->remainingTime > 0 && process->arrivalTime <= currentTime &&
                process->burstTime < shortestBurstTime) {
                shortestIndex = i;
                shortestBurstTime = process->burstTime;
            }
        }

        if (shortestIndex != -1) {
            // Execute the process with the shortest burst time
            struct Process* process = &taskQueue->processes[shortestIndex];
            int executionTime = 1;
            process->remainingTime -= executionTime;
            currentTime += executionTime;  // Update current time

            // Store the Gantt chart entry for the process
            strcpy(ganttChart[ganttIndex].processName, process->name);
            ganttChart[ganttIndex].startTime = currentTime - executionTime;
            ganttChart[ganttIndex].executionTime = executionTime;
            ganttIndex++;

            // If the process is completed
            if (process->remainingTime == 0) {
                completedProcesses++;  // Increment the number of completed processes
                process->completionTime = currentTime;  // Set the completion time
            }
        } else {
            // If no process could be executed, increment current time
            currentTime++;  // Increment time if no process could execute

            // Store the Gantt chart entry for idle time
            strcpy(ganttChart[ganttIndex].processName, "IDLE");
            ganttChart[ganttIndex].startTime = currentTime - 1;
            ganttChart[ganttIndex].executionTime = 1;
            ganttIndex++;
        }
    }

    // Set the start time for each process based on the first appearance in the Gantt chart
    for (int i = 0; i < taskQueue->size; i++) {
        struct Process* process = &taskQueue->processes[i];
        for (int j = 0; j < ganttIndex; j++) {
            if (strcmp(process->name, ganttChart[j].processName) == 0) {
                process->startTime = ganttChart[j].startTime;
                break;
            }
        }
        process->waitingTime = process->startTime - process->arrivalTime; // Calculate waiting time
    }
    
    // Print the execution order
    printf("\nThe order in which processes or threads are selected for execution:\n");
    for (int i = 0; i < ganttIndex; i++) {
        if (strcmp(ganttChart[i].processName, "IDLE") != 0) {
            struct Process* process = getProcessByName(taskQueue, ganttChart[i].processName);
            if (process->remainingTime == 0) {
                printf("From \"%d\" to \"%d\": \"%s\" (completed)\n", ganttChart[i].startTime, ganttChart[i].startTime + ganttChart[i].executionTime, ganttChart[i].processName);
            } else {
                printf("From \"%d\" to \"%d\": \"%s\" (%d time units left)\n", ganttChart[i].startTime, ganttChart[i].startTime + ganttChart[i].executionTime, ganttChart[i].processName, process->remainingTime);
            }
        }
    }


    // Print the Gantt chart
    printf("\nGantt Chart:\n");
    printf("---------------------------------------------------------------------\n");
    printf("|");
    for (int i = 0; i < ganttIndex; i++) {
        printf(" %s ", ganttChart[i].processName);
        if (i < ganttIndex - 1 && strcmp(ganttChart[i].processName, "IDLE") != 0 &&
            ganttChart[i].startTime + ganttChart[i].executionTime < ganttChart[i + 1].startTime) {
            printf("| IDLE ");
        }
        printf("|");
    }
    printf("\n---------------------------------------------------------------------\n");
    printf("%d", ganttChart[0].startTime);
    int cumulativeTime = ganttChart[0].startTime;
    for (int i = 0; i < ganttIndex; i++) {
        cumulativeTime += ganttChart[i].executionTime;
        printf("------%d", cumulativeTime);
    }
    printf("\n");

    // Print the process table and metrics table
    printProcessTable(taskQueue);
    printMetricsTable(taskQueue);
}


// Performs preemptive priority scheduling on the task queue
void prioritySchedulingWithPreemption(struct TaskQueue* taskQueue) {
    int currentTime = 0;
    int completedProcesses = 0;

    // Create a Gantt chart array to store process names, start times, and execution times
    struct GanttEntry {
        char processName[10];
        int startTime;
        int executionTime;
    };
    struct GanttEntry ganttChart[MAX_PROCESSES * 100];  // Assuming maximum 100 time slots per process
    int ganttIndex = 0;  // Index to keep track of the Gantt chart entries

    // Loop until all processes are completed
    while (completedProcesses < taskQueue->size) {
        bool processExecuted = false;
        int highestPriority = INT_MAX;  // Initialize with the highest priority possible

        // Find the highest priority among the ready processes
        for (int i = 0; i < taskQueue->size; i++) {
            struct Process* process = &taskQueue->processes[i];
            if (process->remainingTime > 0 && process->arrivalTime <= currentTime && process->priority < highestPriority) {
                highestPriority = process->priority;
            }
        }

        // Execute the process with the highest priority
        for (int i = 0; i < taskQueue->size; i++) {
            struct Process* process = &taskQueue->processes[i];
            if (process->remainingTime > 0 && process->arrivalTime <= currentTime && process->priority == highestPriority) {
                // Execute the process for a time quantum (assuming time quantum = 1)
                int executionTime = 1;
                process->remainingTime -= executionTime;
                currentTime += executionTime;
                processExecuted = true;

                // Store the Gantt chart entry for the process
                strcpy(ganttChart[ganttIndex].processName, process->name);
                ganttChart[ganttIndex].startTime = currentTime - executionTime;
                ganttChart[ganttIndex].executionTime = executionTime;
                ganttIndex++;

                // If the process is completed
                if (process->remainingTime == 0) {
                    completedProcesses++;  // Increment the number of completed processes
                    process->completionTime = currentTime;  // Set the completion time
                }

                break;  // Exit the loop after executing one process
            }
        }

        // If no process could be executed, increment current time
        if (!processExecuted) {
            currentTime++;  // Increment time if no process could execute

            // Store the Gantt chart entry for idle time
            strcpy(ganttChart[ganttIndex].processName, "IDLE");
            ganttChart[ganttIndex].startTime = currentTime - 1;
            ganttChart[ganttIndex].executionTime = 1;
            ganttIndex++;
        }
    }

    // Set the start time for each process based on the first appearance in the Gantt chart and calculate the turnaround time
    for (int i = 0; i < taskQueue->size; i++) {
        struct Process* process = &taskQueue->processes[i];
        for (int j = 0; j < ganttIndex; j++) {
            if (strcmp(process->name, ganttChart[j].processName) == 0) {
                process->startTime = ganttChart[j].startTime;
                process->turnaroundTime = ganttChart[j].startTime + ganttChart[j].executionTime - process->arrivalTime;
                break;
            }
        }
    }

    // Print the Gantt chart
    printf("\nGantt Chart:\n");
    printf("---------------------------------------------------------------------\n");
    printf("|");
    for (int i = 0; i < ganttIndex; i++) {
        printf(" %s ", ganttChart[i].processName);
        if (i < ganttIndex - 1 && strcmp(ganttChart[i].processName, "IDLE") != 0 &&
            ganttChart[i].startTime + ganttChart[i].executionTime < ganttChart[i + 1].startTime) {
            printf("| IDLE ");
        }
        printf("|");
    }
    printf("\n---------------------------------------------------------------------\n");
    printf("%d", ganttChart[0].startTime);
    int cumulativeTime = ganttChart[0].startTime;
    for (int i = 0; i < ganttIndex; i++) {
        cumulativeTime += ganttChart[i].executionTime;
        printf("------%d", cumulativeTime);
    }
    printf("\n");

    // Print the execution order
    printf("\nThe order in which processes or threads are selected for execution:\n");
    for (int i = 0; i < ganttIndex; i++) {
        if (strcmp(ganttChart[i].processName, "IDLE") != 0) {
            struct Process* process = getProcessByName(taskQueue, ganttChart[i].processName);
            if (process->remainingTime == 0) {
                printf("From \"%d\" to \"%d\": \"%s\" (completed)\n", ganttChart[i].startTime, ganttChart[i].startTime + ganttChart[i].executionTime, ganttChart[i].processName);
            } else {
                printf("From \"%d\" to \"%d\": \"%s\" (%d time units left)\n", ganttChart[i].startTime, ganttChart[i].startTime + ganttChart[i].executionTime, ganttChart[i].processName, process->remainingTime);
            }
        }
    }

    // Print the process table and metrics table
    printProcessTable(taskQueue);
    printMetricsTable(taskQueue);
}


// Prints the process table
void printProcessTable(struct TaskQueue* taskQueue) {
    printf("\nProcess Table:\n");
    printf("+-------+-----------------+------------------+--------------+\n");
    printf("| Name  | Arrival Time    | Burst Time       | Priority     |\n");
    printf("|       |                 |                  |              |\n");
    printf("+-------+-----------------+------------------+--------------+\n");
    for (int i = taskQueue->front; i <= taskQueue->rear; i++) {
        printf("| %-5s | %-15d | %-16d | %-12d |\n",
               taskQueue->processes[i].name, taskQueue->processes[i].arrivalTime,
               taskQueue->processes[i].burstTime, taskQueue->processes[i].priority);
    }
    printf("+-------+-----------------+------------------+--------------+\n");
}
// Helper function to calculate and print the metrics table
void printMetricsTable(struct TaskQueue* taskQueue) {
    printf("\nMetrics Table:\n");
    printf("---------------------------------------------------------------------------------\n");
    printf("| Process Id | Completion Time | Turnaround Time | Waiting Time | Response Time |\n");
    printf("---------------------------------------------------------------------------------\n");
    for (int i = 0; i < taskQueue->size; i++) {
        struct Process* process = &taskQueue->processes[i];
        int turnaroundTime = process->completionTime - process->arrivalTime;
        int responseTime = process->startTime - process->arrivalTime;
        int waitingTime = turnaroundTime - process->burstTime;

        printf("| %11s | %15d | %15d | %12d | %13d |\n", process->name, process->completionTime, turnaroundTime, waitingTime, responseTime);
    }
    printf("---------------------------------------------------------------------------------\n");
}


//clears the input buffer
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Discard characters until a newline or EOF is encountered
    }
}
//initializes the original task queue
void initializeOriginalTaskQueue(struct TaskQueue* taskQueue) {
    originalTaskQueue.front = taskQueue->front;
    originalTaskQueue.rear = taskQueue->rear;
    originalTaskQueue.size = taskQueue->size;
    for (int i = taskQueue->front; i <= taskQueue->rear; i++) {
        originalTaskQueue.processes[i] = taskQueue->processes[i];
    }
}
int main() {
    struct TaskQueue taskQueue;
    taskQueue.front = 0;
    taskQueue.rear = -1;
    taskQueue.size = 0;
    int option, timeQuantum;
    struct Process process;

    do {
        printf("\nDynamic Scheduling Menu\n");
        printf("1. Add Process\n");
        printf("2. Remove Process\n");
        printf("3. Run SJF Scheduling\n");
        printf("4. Run Round Robin Scheduling\n");
        printf("5. Run Priority Scheduling (lower number represents higher priority)\n");
        printf("6. Run FCFS Scheduling\n");
        printf("7. Exit\n");
        printf("Enter your option: ");
        scanf("%d", &option);
        char processName[20];

        switch (option) {
case 1:
    // Add a process to the task queue
    printf("Enter process name: ");
    scanf("%s", processName);
    strcpy(process.name, processName);
    // Check for duplicate process names before completing the input
    for (int i = taskQueue.front; i <= taskQueue.rear; i++) {
        if (strcmp(taskQueue.processes[i].name, process.name) == 0) {
            printf("Error: Process with name '%s' already exists. Cannot add duplicate processes.\n", process.name);
            process.name[0] = '\0';  // Clear the process name
            break;
        }
    }
    if (process.name[0] > '\0') {
        int validInput = 0; // Flag to track valid input
        while (!validInput) {
            printf("Enter arrival time: ");
            if (scanf("%d", &process.arrivalTime) != 1) {
                printf("Invalid input for arrival time. Please enter an integer.\n");
                clearInputBuffer();
                continue;
            } else if (process.arrivalTime < 0) {
                printf("Invalid arrival time. Arrival time should be greater than or equal to 0.\n");
                clearInputBuffer();
                continue;
            }
            validInput = 1;
        }

        int burstTimeValid = 0; // Flag to track valid burst time input
        while (!burstTimeValid) {
            printf("Enter burst time: ");
            if (scanf("%d", &process.burstTime) != 1) {
                printf("Invalid input for burst time. Please enter an integer.\n");
                clearInputBuffer();
                continue;
            }
            if (process.burstTime <= 0) {
                printf("Invalid burst time. Burst time should be greater than 0.\n");
                continue;
            }
            burstTimeValid = 1;
        }

        printf("Enter priority: ");
        scanf("%d", &process.priority);

        process.remainingTime = process.burstTime;
        process.completionTime = 0;
        process.turnaroundTime = 0;
        process.waitingTime = 0;
        enqueue(&taskQueue, process);
        numOfAddedProcesses += 1;
    }
    break;

            case 2:
                if (numOfAddedProcesses > 0){
                    // Remove a process from the task queue
                    printf("\nEnter the name of the process to remove: ");
                    scanf("%s", process.name);
                    removeProcess(&taskQueue, process.name);
                    numOfAddedProcesses -= 1;
                }
                else {
                    printf("No processes added. Please add processes first.\n");
                    clearInputBuffer();
                    continue;
                }
                break;
            case 3:
                if (numOfAddedProcesses > 0) {
                    initializeOriginalTaskQueue(&taskQueue);  // Initialize the original task queue
                    sjfPreemptiveScheduling(&taskQueue);
                    printf("SJF Scheduling completed.\n");
                    // Reset the task queue to the original state
                    taskQueue.front = originalTaskQueue.front;
                    taskQueue.rear = originalTaskQueue.rear;
                    taskQueue.size = originalTaskQueue.size;
                    for (int i = taskQueue.front; i <= taskQueue.rear; i++) {
                        taskQueue.processes[i] = originalTaskQueue.processes[i];
                    }
                } else {
                    printf("No processes added. Please add processes first.\n");
                    clearInputBuffer();
                    continue;
                }
                break;


            case 4:
                if (numOfAddedProcesses > 0) {
                    printf("Enter time quantum: ");
                    scanf("%d", &timeQuantum);
                    initializeOriginalTaskQueue(&taskQueue);  // Initialize the original task queue
                    roundRobinScheduling(&taskQueue, timeQuantum);
                    printf("Round Robin Scheduling completed.\n");
                    // Reset the task queue to the original state
                    taskQueue.front = originalTaskQueue.front;
                    taskQueue.rear = originalTaskQueue.rear;
                    taskQueue.size = originalTaskQueue.size;
                    for (int i = taskQueue.front; i <= taskQueue.rear; i++) {
                        taskQueue.processes[i] = originalTaskQueue.processes[i];
                    }
                } else {
                    printf("No processes added. Please add processes first.\n");
                    clearInputBuffer();
                    continue;
                }
                break;

            case 5:
                if (numOfAddedProcesses > 0) {
                    initializeOriginalTaskQueue(&taskQueue);  // Initialize the original task queue
                    prioritySchedulingWithPreemption(&taskQueue);
                    printf("Priority Scheduling completed.\n");
                    // Reset the task queue to the original state
                    taskQueue.front = originalTaskQueue.front;
                    taskQueue.rear = originalTaskQueue.rear;
                    taskQueue.size = originalTaskQueue.size;
                    for (int i = taskQueue.front; i <= taskQueue.rear; i++) {
                        taskQueue.processes[i] = originalTaskQueue.processes[i];
                    }
                } else {
                    printf("No processes added. Please add processes first.\n");
                    clearInputBuffer();
                    continue;
                }
                break;



            case 6:
                if (numOfAddedProcesses > 0) {
                    initializeOriginalTaskQueue(&taskQueue);  // Initialize the original task queue
                    fcfsScheduling(&taskQueue);
                    printf("FCFS Scheduling completed.\n");
                    // Reset the task queue to the original state
                    taskQueue.front = originalTaskQueue.front;
                    taskQueue.rear = originalTaskQueue.rear;
                    taskQueue.size = originalTaskQueue.size;
                    for (int i = taskQueue.front; i <= taskQueue.rear; i++) {
                        taskQueue.processes[i] = originalTaskQueue.processes[i];
                    }
                } else {
                    printf("No processes added. Please add processes first.\n");
                    clearInputBuffer();
                    continue;
                }
                break;

            case 7:
                printf("Exiting the program.\n");
                break;

            default:
                printf("Invalid option. Please try again.\n");
        }
    } while (option != 7);

    return 0;
}
