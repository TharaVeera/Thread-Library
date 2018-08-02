# Thread-Library
A group project with Amber Strange from Duke's CS 310(Operating and Distributed Systems).

## *Part 1: Deli*
This program, deli.cc, uses a thread library (thread.o) to concurrently simulate a deli restaurant. There are two type of threads: a cashier and a sandwich maker. There is one sandwich maker thread, but many cashier threads. 

The deli program is run in this manner: ./deli 3 sw.in0 sw.in1 sw.in2 sw.in3 sw.in4. The second argument represents the maximum number orders that can be on the sandwich maker's order board at a time. The rest of the arguments are files, which represent cashier threads. 

The thread library initializes cashiers according to their file number (0, 1, 2...). The files contain sandwich orders, which are numbers from (0, 999). Then, cashier threads begin in the postOrder thread_create function. 

The thread library creates the sandwich maker in the makeOrder thread_create function. 

### How the program works
The sandwich maker thread runs while there are cashier threads running, aka there are still sandwich orders to be processed. The sandwich orders are posted on the order board by cashier threads. The order board is a doubly linked list of structs containing the sandwich number, and the id of the cashier thread who posted it. To choose a sandwich to make, the sandiwch maker finds the closest sandwich to the previous one it just made, with the closestSandwich function. After the sandwich is chosen, it is removed from the order board, and the order is deemed ready with the output: READY: cashier '#' sandwich '#'. 

After a order is ready, the sandwich maker signals and wakes up the cashiers, so they can repopulate the order board. Then, the sandwich maker will go to sleep. 

In the cashiers' postOrder function, it signals the sandwich maker to wake up if the order board is full or one of its orders remains on the order board. To add an order, the cashier thread inserts its order to the order board linked list. The order is posted with this output: POSTED: cashier # sandwich #. Once the cashier has finished posting all of its orders, it will decrement the maximum number orders for the order board and then exit. 

The sandwich maker will exit once al the cashier threads have exited. 

The program is tested by looking at the ordering of the POSTED and READY outputs. 

## *Part 2: Thread Library*
The program, thread.cc, is a thread library which supports multiple threads, ensures atomicity, and scheduling in FIFO. These aspects of the library can be certified with the 14 test cases, and the deli.cc file. The libinterrupt.a file simulates software interrupt, which is configured in the interrupt.h file. The thread library's only out put is: "Thread library exiting." and returns an exit status of 0.

### How the program works
To ensure atomicity, threads own locks and condition variables (mutexes). The state of ownership is kept track in a lock map, and a (lock, cv) map. These maps are made up of a queue of TCBs, thread control blocks, which are the physical representation of the thread's data. They contain the thread's context (saved registers), stack pointer, a unique name, and a boolean for whether or not it is alive. These TCBs are also used to delete the thread upon exit of the there library. The methods of the library are encased with interrupt/enables to prevent preemption by the kernel and ensure atomicity. 

#### Functions:
*Thread_libinit* - This intializes the thread library. None of the other methods will return 0 if the library has not been initialized. It creates the first thread and saves the context. In making the context, it passes the thread to the 'trampoline' function. The 'tramploline' function passes the newly created thread to a function ('func', with arguments 'arg), where it can start running. For example, the postOrder and makeOrder functions in deli.cc. Once the desired func has been completed, the threads delete themselves. This occurs by popping off TCBs from the ready queue. Each thread is deleted, and then set to the current thread so it will be switched with next one in contextSwitch. At the end, it uses swapcontext to return to the original thread in libinit. Then this thread deletes itself and the thread library exits. 
*Thread_create* - This creates other threads after thread_inity. It also intializes threads' contexts and then saves it. Like thread_libinit, it makes the context by passing the the thread to the 'trampoline' function, where the thread will return and follow the deletion process explained above. 
*ContextSwitch* - This pops off the next thread on ready queue and swaps contexts with the current thread.
*Thread_yield - This puts the current thread on the ready queue and context switches - allowing the next thread on the ready queue to run.
*Thread_signal* - This checks the (lock, cv) map to see if there are TCBs waiting on this mutext. If so,it removes the TCB at the front of the wait queue and places it on the ready queue. 
*Thread_broadcast* - Like thread_signal, this checks the lock, but instead it removes all of the TCBs on the wait queue and pushes them onto the ready queue. 
*Thread_wait* - This pushes the current thread onto the (lock, cv) queue and then context switches.
There two functions for thread_locks/thread_unlock. One with interrupts and one without interrupt. This was to not interfere with the interrupt disable/enable encasing in the thread_wait function.
*Thread unlock* - First, the thread sets no TCB to be owner of lock in map. If threads are waiting on the lock, it removes the next thread on the lock queue and places it on the ready queue.
*Thread lock* - If no one owns the lock, then the current thread becomes the owner. Otherwise if lock is in use, it is placed on the lock queue and undergoes a context switch. 
