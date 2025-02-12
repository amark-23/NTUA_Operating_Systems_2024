
---

## **1. Process Creation and Management**
### **Fork System Call (fork_example.c)**
- `fork()` creates a new child process that runs concurrently with the parent.
- The child receives a process ID (PID) and inherits the parent’s memory.
- **`getpid()` and `getppid()`** functions return process ID and parent process ID.
- **`wait()`** ensures that the parent waits for the child to complete before continuing.
- A file is used for logging process execution.

**OS Concept:** Process Creation, Parent-Child Relationship, Synchronization

---

## **2. Inter-Process Communication (IPC)**
### **Pipelines (pipelines_example.c)**
- A pipeline (`pipe()`) establishes a unidirectional communication channel between processes.
- Uses **Parent-to-Child and Child-to-Parent pipes**.
- **`write()` and `read()`** are used for data transfer.
- **Round-Robin & Random Scheduling**: Parent assigns jobs to child processes using different strategies.

**OS Concept:** IPC Mechanisms, Pipes, Scheduling, Process Synchronization

---

### **Sockets (sockets_example.c)**
- **Client-Server Communication using Sockets**
- **`socket()`** creates a communication endpoint.
- **`connect()`** links the client to a remote server.
- **Polling (`poll()`)** monitors multiple file descriptors for events.
- Uses **TCP (SOCK_STREAM)** for reliable communication.
- **Non-blocking I/O**: `poll()` ensures responsiveness.

**OS Concept:** Network Communication, Client-Server Model, Polling, Non-Blocking I/O

---

## **3. Process Synchronization & Signals**
### **Gates System (gates_parent.c & gates_child.c)**
- The **Parent Process** creates child processes (gates) that **toggle states** based on signals.
- **Signals Used:**
  - `SIGUSR1`: Reports gate status.
  - `SIGUSR2`: Toggles gate state.
  - `SIGALRM`: Timer-based automatic status updates.
  - `SIGCHLD`: Used by the parent to detect child process termination.
  - `SIGTERM`: Graceful termination.

**OS Concept:** Signal Handling, Process Synchronization, Event Handling

---

## **4. Process Scheduling & Handling**
- **Round-Robin and Random Job Assignment (pipelines_example.c)** demonstrate different scheduling policies.
- **Process Replacement (`execv()`)** in gates management ensures continuity after a child exits.
- **Process Termination (`kill()`)** in multiple files (`gates_parent.c`, `pipelines_example.c`) controls the lifetime of child processes.

**OS Concept:** Scheduling, Process Replacement, Process Termination, Resource Management

---

These concepts are essential in **OS Design**, covering process control, IPC, networking, and synchronization.
