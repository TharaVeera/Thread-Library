
#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
using namespace std;

/*Tests Parent/Child. Does parent thread reap result of child?
Expected output:
Result: 10
Thread library exiting.
*/

static int result= 0;

void Child(void* arg ){
    
    for (int i =0; i<10; i++){
        //cout<<"Result: "<<result<<endl;
        result++; 
    }
    thread_signal(1,1);
    return;
}

void Parent(void* arg ){
    thread_lock(1);
    thread_create((thread_startfunc_t) Child, (void*) "I am child");
    if(result<9){
        //cout<<"p sleep"<<endl;
        thread_wait(1,1);
        //cout<<"p woke"<<endl;
    }
    cout<<"Result: "<<result<<endl;
    thread_unlock(1);
    return; 
}

void makeThreads(void* argv ){
    thread_create((thread_startfunc_t) Parent, (void*) 1);
    return;
}

int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) argv);
  exit(0);
}