# Thread-Library

## *Part 1: Deli*
This program uses a thread library (thread.o) to concurrently simulate a deli restaurant. There are two type of threads: a cashier and a sandwich maker. There is one sandwich maker thread, but many cashier threads. 

The deli program is run in this manner: ./deli 3 sw.in0 sw.in1 sw.in2 sw.in3 sw.in4. The second argument represents the maximum number orders that can be on the sandwich maker's order board at a time. The rest of the arguments are files, which represent cashier threads. 

The thread library initializes cashiers according to their file number (0, 1, 2...). The files contain sandwich orders, which are numbers from (0, 999). Then, cashier threads begin in the postOrder thread_create function. 

The thread library creates the sandwich maker in the makeOrder thread_create function. 

### How the program works
The sandwich maker thread runs while there are cashier threads running, aka there are still sandwich orders to be processed. The sandwich orders are posted on the order board by cashier threads. The order board is a doubly linked list of structs containing the sandwich number, and the id of the cashier thread who posted it. To choose a sandwich to make, the sandiwch maker finds the closest sandwich to the previous one it just made, with the closestSandwich function. After the sandwich is chosen, it is removed from the order board, and the order is deemed ready with the output: READY: cashier '#' sandwich '#'. 

After a order is ready, the sandwich maker signals and wakes up the cashiers, so they can repopulate the order board. Then, the sandwich maker will go to sleep. 

In the cashiers' postOrder function, it signals the sandwich maker to wake up if the order board is full or one of its orders remains on the order board. To add an order, the cashier thread inserts its order to the order board linked list. The order is posted with this output: POSTED: cashier # sandwich #. Once the cashier has finished posting all of its orders, it will decrement the maximum number orders for the order board and then exit. 

The sandwich maker will exit once al the cashier threads have exited. 

The program is tested by looking at the ordering of the POSTED and READY outputs. 
