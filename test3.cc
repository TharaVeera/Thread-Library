#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

//Thread tries to acquire lock when it's already held. Hand-off locks. 

/* Expected output: 
Hi thread
num: 5
Bye thread
num: 0
Thread lock correctly unacquired
Thread library exiting.
*/

using namespace std;
int num = 0;
void hi(void* thread_args){
    thread_lock(1); 
    cout<<"Hi thread"<<endl;
    num = num +5; 
    cout<<"num: "<< num<< endl;
    thread_yield(); //1 goes to readyqueue and still holds lock
    num = 0;
    cout<<"num: "<< num<< endl;
    thread_unlock(1); 
    return; 
}

void bye(void* thread_args){
    cout<<"Bye thread"<<endl;
    //cout<<thread_lock(1)<<endl;
    thread_lock(1); 
    //cout<<"num: "<< num<< endl;
    //cout<<"Thread lock not acquired"<< endl;

    if(num == 0){
        cout<<"Thread lock correctly unacquired"<< endl;
        thread_unlock(1);
        return;
    }
    num = num +5;  //should not occurr because thread cannot acquire lock 
    cout<<"num: "<< num<< endl;
    cout<<"Thread lock incorrectly acquired"<< endl;
    thread_unlock(1); 
    return; 
}


void makeThreads(void* argv ){
    thread_create((thread_startfunc_t) hi, (void*) "1");
    thread_create((thread_startfunc_t) bye, (void*) "2");
    return;
}

int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) argv);
  exit(0);
}