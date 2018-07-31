
#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
using namespace std;

/*Testing if libinit is called twice
Expected output:
Thread library exiting.
*/
/* If thread libinit worked then it would print num
*/
static int num = 0; 

void t(void *argv){
    num++; 
    cout<<"Num: "<< num<<endl;
    return; 
}

void makeThreads(void* argv ){
    thread_libinit((thread_startfunc_t) t, (void*) argv);
    return;
}

int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) argv);
  exit(0);
}

