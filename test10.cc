
#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
using namespace std;


/*
Expected outcome: 
Unlock fails
Thread wait fails
Thread library exiting.*/
int num = 2;

void t(void * args){
  thread_lock(1);
   if(thread_unlock(0) == -1){
    cout<<"Unlock fails"<<endl;
  };
  
  if(thread_signal(0,2) == -1){
    cout<<"Thread signal fails"<<endl;
  };
 
   if(thread_wait(0,5) == -1){
    cout<<"Thread wait fails\n";
  }
}

void makeThreads(void* argv ){
    thread_create((thread_startfunc_t)t, (void*) 1  ); /* C0 */
    thread_create((thread_startfunc_t)t,(void*)  2); /* C1 */
    thread_create((thread_startfunc_t)t, (void*) 3); /* P1 */
} 

int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) &num);
  exit(0);
}