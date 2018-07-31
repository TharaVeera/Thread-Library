
#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
using namespace std;

static int i = 100; 
static int num = 2; 

void thread((void*) args){

    cout<<"Thread "<< (char*) args<< " is running."<<endl; 
    //do some shit 
    return;

}

void makeThreads(void* argv ){
    for(int j = 0; j<i; j++ ){
        if( (thread_create((thread_startfunc_t) thread, &j) == -1){
            cout<<"Threadcreate failed";
            return -1; 
        } 
        thread_create((thread_startfunc_t) thread, &j);
    }

} 

int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) &num);
  exit(0);
}
