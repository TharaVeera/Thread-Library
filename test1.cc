//missed wake up call test
#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>


/*
Each test case for the thread library will be a short C++ program that uses functions in the
 thread library (e.g., the example program above). Each test case should be run without any arguments 
 and should not use any input files. Test cases should exit(0) when run with a correct thread library 
 (normally this will happen when your test case's last runnable thread ends or blocks). If you submit your 
 deli simulation as a test case, remember to specify all inputs (number of requesters, buffers, and the 
 list of requests) statically in the program. This shouldn't be too inconvenient because the list of requests 
 should be short to make a good test case (i.e. one that you can trace through what should happen). 
*/


/*
This is testing for missed wakeup because signaled and didn't broadcast. Also tests if 
signal and broadcast generate an error if the the cv and lock combo have not been seen before
Expected output: 
SODA ADDED by: p1. Number of sodas currently: 1
SODA TAKEN by: c1. Number of sodas currently: 0
Thread library exiting.

*/

using namespace std;

static int numSodas = 0; 
static int maxSodas = 5; 

void consumer(void* thread_args){
    //cout<<"Iamhere"<<endl;
    thread_lock(1); //sodalock

    while(numSodas == 0){
        //cout<<"Iamhere"<<endl;
        thread_wait(1, 1);//CV1 = hasSoda, CV for sonsumers 
    }
    numSodas--; 
    cout<< "SODA TAKEN by: "<< (char *) thread_args<< ". Number of sodas currently: " << numSodas << endl;

    thread_signal(1,2); //CV2 = hasRoom, CV for producer
    if(thread_signal(1,3) == -1){
        cout<< "Should not generate error on a unseen CV"<<endl;
    }
    if(thread_signal(2,3) == -1){
        cout<< "Should not generate error on a unseen lock CV"<<endl;
    }

    thread_unlock(1);
    return;

}

void producer(void* thread_args){
    //cout<<"Iamhere"<<endl;
    //cout<<numSodas<<endl;
    thread_lock(1);
    
    while(numSodas == maxSodas ){
        thread_wait(1,2); 
    }
    
    numSodas++;
    cout<<"SODA ADDED by: "<< (char *) thread_args<< ". Number of sodas currently: "<< numSodas<<endl;

    thread_signal(1,1);
    //**this causes missed wakeup because should be broadcast!!
    //c2 sleeps forever

    thread_unlock(1);
    return;

}


void makeThreads(void* argv ){
    thread_create((thread_startfunc_t) consumer, (void*) "c1");
    thread_create((thread_startfunc_t) consumer, (void*) "c2");
    thread_create((thread_startfunc_t) producer, (void*) "p1");

    return;
}



int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) argv);
  exit(0);
}