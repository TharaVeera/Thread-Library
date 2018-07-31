#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
using namespace std;

/*Testing bunch of functions
Expected output:
SODA ADDED by: P1. Number of sodas currently: 0
SODA ADDED by: P0. Number of sodas currently: 1
SODA TAKEN by: C0. Number of sodas currently: 1
SODA TAKEN by: C1. Number of sodas currently: 0
Thread library exiting.*/

static int numSodas = 0; 
static int maxSodas = 5; 

void produce(void* argv)
{
    cout<<(char *) argv<<" is running thread_lock(1);"<<endl;
    thread_lock(1);
    cout<<(char *) argv<<" is running  while (numSodas == maxSodas) {"<<endl;
    while (numSodas == maxSodas) {
        cout<<(char *) argv<<" is running  thread_yield();"<<endl;
        thread_yield();
        cout<<(char *) argv<<" is running thread_wait(1, 2);"<<endl;
        thread_wait(1, 2);
        cout<<(char *) argv<<" numsodas"<<endl;
    }
    numSodas++;
    cout<< "SODA ADDED by: "<< (char *) argv<< ". Number of sodas currently: " << numSodas << endl;
    cout<<(char *) argv<<" is running thread_signal(1, 2);"<<endl;
    thread_signal(1, 2);
    cout<<(char *) argv<<" is running thread_yield();"<<endl;
    thread_yield();
    cout<<(char *) argv<<" is running thread_unlock(1);"<<endl;
    thread_unlock(1);
    cout<<(char *) argv<<" ret;"<<endl;
    return; 
} 

void consume(void* argv)
{
    cout<<(char *) argv<<" is running"<<" thread_lock(1)"<<endl;
 thread_lock(1);
 cout<<(char *) argv<<" is running"<<" while (numSodas == 0) {"<<endl;
 while (numSodas == 0) {
     cout<<(char *) argv<<" is running"<<" thread_yield()"<<endl;
    thread_yield();
    cout<<(char *) argv<<" is running"<<" thread_wait(1, 2);"<<endl;
    thread_wait(1, 2);
    cout<<(char *) argv<<" is running"<<" numsodas"<<endl;
 }
 numSodas--;
 cout<< "SODA TAKEN by: "<< (char *) argv<< ". Number of sodas currently: " << numSodas << endl;
 cout<<(char *) argv<<" is running"<<" thread_signal(1, 2);"<<endl;
 thread_signal(1, 2);
 cout<<(char *) argv<<" is running"<<" thread_yield();"<<endl;
 thread_yield();
 cout<<(char *) argv<<" is running"<<" thread_unlock(1);"<<endl;
 thread_unlock(1);
  cout<<(char *) argv<<" is running"<<" ret;"<<endl;
 return; 
} 

void makeThreads(void* argv ){
    thread_create((thread_startfunc_t)consume,(void*)  "C0"  ); /* C0 */
    thread_create((thread_startfunc_t)consume,(void*)  "C1"); /* C1 */
    thread_create((thread_startfunc_t)produce, (void*) "P1"); /* P1 */
    cout<<(char *) argv<<" is running thread yield"<<endl;
    thread_yield();
    cout<<(char *) argv<<" is running producs "<<endl;
    produce((void*) "P0"); /* P0 */
    cout<<(char *) argv<<" ret"<<endl;
    return; 
} 
int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) argv);
  exit(0);
}
