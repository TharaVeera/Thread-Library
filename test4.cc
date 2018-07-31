#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>

/*Testing if locks are FIFO*/
/*Expected output: 
Thread: 0x1
num: 1
Thread: 0x2
num: 2
Thread: 0x3
num: 3
Thread: 0x4
num: 4
Thread library exiting.
*/
using namespace std;

int num = 0;
void sleepy(void* arg ){
    thread_lock(1);
    if(num == 0){
        thread_wait(1,1); //all threads should go on the wait queue
    }
    cout<<"Thread: "<< (void*) arg<<endl;
    num++; 
    cout<<"num: "<< num <<endl;
    thread_unlock(1);
    return; 
}

void boss(void* arg ){
    thread_lock(1);
    if(num == 0){
        thread_broadcast(1,1);
    }
    thread_unlock(1);
    return; 

}


void makeThreads(void* argv ){
    thread_create((thread_startfunc_t) sleepy, (void*) 1);
    thread_create((thread_startfunc_t) sleepy, (void*) 2);
    thread_create((thread_startfunc_t) sleepy, (void*) 3);
    thread_create((thread_startfunc_t) sleepy, (void*) 4);
    thread_create((thread_startfunc_t) boss, (void*) 4);
    return;
}

int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) argv);
  exit(0);
}