#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 100

// structure to store process details
typedef struct {
    int pid;             // process id
    int burst_time;      // cpu burst time
    int priority;        // process priority
    int arrival_time;    // arrival time
    int waiting_time;    // waiting time
    int turnaround_time; // turnaround time
    int remaining_time;  // remaining time for preemptive scheduling
} Process;

// reads process information from input.txt (skips the first two header lines)
int read_processes(Process process_list[]) {
    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        perror("Error opening input.txt");
        exit(1);
    }
    
    char buffer[256];
    // skip the first two header lines
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file);
    
    int count = 0;
    // format string to read "P" followed by the pid
    while (fscanf(file, " P%d %d %d %d", 
                  &process_list[count].pid, 
                  &process_list[count].burst_time, 
                  &process_list[count].priority, 
                  &process_list[count].arrival_time) == 4) {
        process_list[count].waiting_time = 0;
        process_list[count].turnaround_time = 0;
        process_list[count].remaining_time = process_list[count].burst_time;
        count++;
        if (count >= MAX_PROCESSES) break;
    }
    fclose(file);
    return count;
}

// ---------------------------------------------------------------------------
// 1. FCFS SCHEDULING
// ---------------------------------------------------------------------------
void fcfs(Process process_list[], int num_processes) {
    Process proc[MAX_PROCESSES];
    // copy original data
    for (int i = 0; i < num_processes; i++) {
        proc[i] = process_list[i];
    }
    
    // sort by arrival time (simple bubble sort)
    for (int i = 0; i < num_processes - 1; i++) {
        for (int j = 0; j < num_processes - i - 1; j++) {
            if (proc[j].arrival_time > proc[j + 1].arrival_time) {
                Process temp = proc[j];
                proc[j] = proc[j + 1];
                proc[j + 1] = temp;
            }
        }
    }
    
    int current_time = 0;
    float total_waiting = 0, total_turnaround = 0;
    
    // compute waiting and turnaround times
    for (int i = 0; i < num_processes; i++) {
        if (current_time < proc[i].arrival_time) {
            current_time = proc[i].arrival_time;
        }
        proc[i].waiting_time = current_time - proc[i].arrival_time;
        current_time += proc[i].burst_time;
        proc[i].turnaround_time = proc[i].waiting_time + proc[i].burst_time;
        
        total_waiting += proc[i].waiting_time;
        total_turnaround += proc[i].turnaround_time;
    }
    
    // print results
    printf("\nFCFS Statistics...\n\n");
    printf("Process    Waiting Time    Turnaround Time\n");
    for (int i = 0; i < num_processes; i++) {
        printf("P%-1d%14d%18d\n",
               proc[i].pid,
               proc[i].waiting_time,
               proc[i].turnaround_time);
    }
    printf("\nAverage Waiting Time: %.2f\n", total_waiting / num_processes);
    printf("\nAverage Turnaround Time: %.2f\n", total_turnaround / num_processes);
}

// ---------------------------------------------------------------------------
// 2. SJF (NONPREEMPTIVE) SCHEDULING
// ---------------------------------------------------------------------------
void sjf(Process process_list[], int num_processes) {
    Process proc[MAX_PROCESSES];
    for (int i = 0; i < num_processes; i++) {
        proc[i] = process_list[i];
    }
    
    int completed = 0;
    int current_time = 0;
    float total_waiting = 0, total_turnaround = 0;
    int visited[MAX_PROCESSES] = {0};
    
    while (completed < num_processes) {
        int selected_index = -1;
        int min_burst = 99999;
        
        // pick the process with the smallest burst among those that have arrived
        for (int i = 0; i < num_processes; i++) {
            if (!visited[i] &&
                proc[i].arrival_time <= current_time &&
                proc[i].burst_time < min_burst) {
                min_burst = proc[i].burst_time;
                selected_index = i;
            }
        }
        
        if (selected_index == -1) {
            current_time++; // no process has arrived yet
            continue;
        }
        
        visited[selected_index] = 1;
        proc[selected_index].waiting_time = current_time - proc[selected_index].arrival_time;
        current_time += proc[selected_index].burst_time;
        proc[selected_index].turnaround_time =
            proc[selected_index].waiting_time + proc[selected_index].burst_time;
        
        total_waiting += proc[selected_index].waiting_time;
        total_turnaround += proc[selected_index].turnaround_time;
        completed++;
    }
    
    // print results
    printf("\nSJF (Nonpreemptive) Statistics...\n\n");
    printf("Process    Waiting Time    Turnaround Time\n");
    for (int i = 0; i < num_processes; i++) {
        printf("P%-1d%14d%18d\n",
               proc[i].pid,
               proc[i].waiting_time,
               proc[i].turnaround_time);
    }
    printf("\nAverage Waiting Time: %.2f\n", total_waiting / num_processes);
    printf("\nAverage Turnaround Time: %.2f\n", total_turnaround / num_processes);
}

// ---------------------------------------------------------------------------
// 3. SRT (PREEMPTIVE) SCHEDULING
// ---------------------------------------------------------------------------
void srt(Process process_list[], int num_processes) {
    Process proc[MAX_PROCESSES];
    for (int i = 0; i < num_processes; i++) {
        proc[i] = process_list[i];
        proc[i].remaining_time = proc[i].burst_time;
    }
    
    int current_time = 0, completed = 0;
    float total_waiting = 0, total_turnaround = 0;
    
    while (completed < num_processes) {
        int selected_index = -1;
        int min_remaining = 99999;
        
        // pick the process with the smallest remaining time among those that have arrived
        for (int i = 0; i < num_processes; i++) {
            if (proc[i].arrival_time <= current_time &&
                proc[i].remaining_time > 0 &&
                proc[i].remaining_time < min_remaining) {
                min_remaining = proc[i].remaining_time;
                selected_index = i;
            }
        }
        
        // if no process is ready, increment time
        if (selected_index == -1) {
            current_time++;
            continue;
        }
        
        // run for 1 time unit
        proc[selected_index].remaining_time--;
        current_time++;
        
        // if the process finishes
        if (proc[selected_index].remaining_time == 0) {
            completed++;
            int finish_time = current_time;
            proc[selected_index].waiting_time =
                finish_time - proc[selected_index].burst_time - proc[selected_index].arrival_time;
            proc[selected_index].turnaround_time =
                finish_time - proc[selected_index].arrival_time;
            
            total_waiting += proc[selected_index].waiting_time;
            total_turnaround += proc[selected_index].turnaround_time;
        }
    }
    
    // print results
    printf("\nSRT (Preemptive) Statistics...\n\n");
    printf("Process    Waiting Time    Turnaround Time\n");
    for (int i = 0; i < num_processes; i++) {
        printf("P%-1d%14d%18d\n",
               proc[i].pid,
               proc[i].waiting_time,
               proc[i].turnaround_time);
    }
    printf("\nAverage Waiting Time: %.2f\n", total_waiting / num_processes);
    printf("\nAverage Turnaround Time: %.2f\n", total_turnaround / num_processes);
}

// ---------------------------------------------------------------------------
// 4. ROUND ROBIN (RR) SCHEDULING
// ---------------------------------------------------------------------------
void round_robin(Process process_list[], int num_processes) {
    int time_quantum;
    printf("Enter Time Quantum: ");
    scanf("%d", &time_quantum);
    
    Process proc[MAX_PROCESSES];
    for (int i = 0; i < num_processes; i++) {
        proc[i] = process_list[i];
        proc[i].remaining_time = proc[i].burst_time;
    }
    
    int current_time = 0, completed = 0;
    float total_waiting = 0, total_turnaround = 0;
    
    while (completed < num_processes) {
        int all_done = 1;
        for (int i = 0; i < num_processes; i++) {
            // if this process can run
            if (proc[i].remaining_time > 0 &&
                proc[i].arrival_time <= current_time) {
                all_done = 0;
                if (proc[i].remaining_time > time_quantum) {
                    proc[i].remaining_time -= time_quantum;
                    current_time += time_quantum;
                } else {
                    current_time += proc[i].remaining_time;
                    proc[i].remaining_time = 0;
                    completed++;
                    proc[i].waiting_time =
                        current_time - proc[i].burst_time - proc[i].arrival_time;
                    proc[i].turnaround_time =
                        current_time - proc[i].arrival_time;
                    
                    total_waiting += proc[i].waiting_time;
                    total_turnaround += proc[i].turnaround_time;
                }
            }
        }
        if (all_done) {
            current_time++; // move time forward if no process is ready
        }
    }
    
    // print results
    printf("\nRound Robin (RR) Statistics...\n\n");
    printf("Process    Waiting Time    Turnaround Time\n");
    for (int i = 0; i < num_processes; i++) {
        printf("P%-1d%14d%18d\n",
               proc[i].pid,
               proc[i].waiting_time,
               proc[i].turnaround_time);
    }
    printf("\nAverage Waiting Time: %.2f\n", total_waiting / num_processes);
    printf("\nAverage Turnaround Time: %.2f\n", total_turnaround / num_processes);
}

// ---------------------------------------------------------------------------
// 5. PRIORITY (PR) - NONPREEMPTIVE SCHEDULING
// ---------------------------------------------------------------------------
void priority_scheduling(Process process_list[], int num_processes) {
    Process proc[MAX_PROCESSES];
    for (int i = 0; i < num_processes; i++) {
        proc[i] = process_list[i];
    }
    
    int completed = 0;
    int current_time = 0;
    float total_waiting = 0, total_turnaround = 0;
    int visited[MAX_PROCESSES] = {0};
    
    while (completed < num_processes) {
        int selected_index = -1;
        int best_priority = 99999;
        
        // pick the process with the highest priority (lowest numeric value)
        for (int i = 0; i < num_processes; i++) {
            if (!visited[i] &&
                proc[i].arrival_time <= current_time &&
                proc[i].priority < best_priority) {
                best_priority = proc[i].priority;
                selected_index = i;
            }
        }
        if (selected_index == -1) {
            current_time++;
            continue;
        }
        visited[selected_index] = 1;
        
        proc[selected_index].waiting_time =
            current_time - proc[selected_index].arrival_time;
        current_time += proc[selected_index].burst_time;
        proc[selected_index].turnaround_time =
            proc[selected_index].waiting_time + proc[selected_index].burst_time;
        
        total_waiting += proc[selected_index].waiting_time;
        total_turnaround += proc[selected_index].turnaround_time;
        completed++;
    }
    
    // print results
    printf("\nPriority (PR) - Nonpreemptive Statistics...\n\n");
    printf("Process    Waiting Time    Turnaround Time\n");
    for (int i = 0; i < num_processes; i++) {
        printf("P%-1d%14d%18d\n",
               proc[i].pid,
               proc[i].waiting_time,
               proc[i].turnaround_time);
    }
    printf("\nAverage Waiting Time: %.2f\n", total_waiting / num_processes);
    printf("\nAverage Turnaround Time: %.2f\n", total_turnaround / num_processes);
}

// ---------------------------------------------------------------------------
// MAIN
// ---------------------------------------------------------------------------
int main() {
    Process process_list[MAX_PROCESSES];
    int num_processes = read_processes(process_list);
    
    if (num_processes == 0) {
        printf("No process data found in input.txt. Please check the file format.\n");
        return 1;
    }
    
    int choice;
    do {
        printf("\n============================\n");
        printf("CPU Scheduling Algorithms Menu\n");
        printf("1. First-Come, First-Served (FCFS)\n");
        printf("2. Shortest Job First (SJF) - Nonpreemptive\n");
        printf("3. Shortest Remaining Time (SRT) - Preemptive\n");
        printf("4. Round Robin (RR)\n");
        printf("5. Priority Scheduling (Nonpreemptive)\n");
        printf("0. Exit\n");
        printf("============================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                fcfs(process_list, num_processes);
                break;
            case 2:
                sjf(process_list, num_processes);
                break;
            case 3:
                srt(process_list, num_processes);
                break;
            case 4:
                round_robin(process_list, num_processes);
                break;
            case 5:
                priority_scheduling(process_list, num_processes);
                break;
            case 0:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);
    
    return 0;
}