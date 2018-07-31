//ping pong test
#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>


using namespace std;
int num = 0; 


void ping(void* thread_args){
    thread_lock(1); 
    //cout<<"ping"<<endl;
    while(num == 0){
        cout<<"Pingwait"<<endl;
        num++;
        thread_signal(1,1);
        thread_wait(1,1);
    }
    //num++; 
    cout << (char *) thread_args<< "!  num: " << num << endl;
    thread_unlock(1); 
    return;
    
}

void pong(void* thread_args){
    thread_lock(1); 
     //cout<<"pong"<<endl;
    while(num == 1){
        num--;
        cout<<"Pongwait"<<endl;
        thread_signal(1,1);
        thread_wait(1,1);
    }
    //num++; 
    cout<< (char *) thread_args<< "!  num: " << num << endl;
    thread_unlock(1);
    return;

}

void makeThreads(void* argv ){
    thread_create((thread_startfunc_t) ping, (void*) "ping");
    thread_create((thread_startfunc_t) pong, (void*) "pong");

    return;
}

int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) argv);
  exit(0);
}