#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
using namespace std;

//Testing library intializaition
/*Expected output:
Thread library exiting.*/
int num =2; 

void t(void* argv){
    return;
}

void makeThreads(void* argv ){
     thread_create((thread_startfunc_t)t, (void*) 1  );

return;
}

int main(int argc, char** argv){
    if(thread_signal(1, 1)==0){
        cout<< "Signal not allowed\n";
    }
	if(thread_broadcast(1, 1) == 0){
		cout<< "Broadcast not allowed\n";
	}
    if(thread_wait(1, 1) == 0){
		cout<< "Wait not allowed\n";
	}
    if(thread_lock(1) == 0){
		cout<< "Lock not allowed\n";
	}
    if(thread_unlock(1) == 0){
		cout<< "Unlock not allowed\n";
	}
    if(thread_create(makeThreads, &num) == 0){
		cout<< "Threadcreate not allowed\n";
	}
  thread_libinit((thread_startfunc_t) makeThreads, (void*) &num);
  exit(0);
}