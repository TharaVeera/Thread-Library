#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
using namespace std;

int num =2;

/*Expected output
Wait failed
Lock failed
Unlock failed
Thread library exiting.*/

void t2(void * args){
  if(thread_wait (1,1) == -1){
    cout<< "Wait failed\n";
  }
  if(thread_lock(1) == -1){
    cout<< "Lock failed\n";
  }

  if(thread_lock(1) == -1){
    cout<< "Lock failed\n";
  }
   if(thread_unlock(1) == -1){
    cout<< "Unlock failed\n";
  }
   if(thread_unlock(2) == -1){
    cout<< "Unlock failed\n";
  }
  if(thread_yield() == -1){
    cout<< "Yield failed\n";
  }
  return; 
}


void makeThreads(void* argv ){
    thread_create((thread_startfunc_t)t2,(void*)  2); /* C1 */

} 
  int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) &num);
  exit(0);
}




