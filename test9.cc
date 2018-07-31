#include "thread.h"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
using namespace std;

/*Expected output
First is running
First is running
C1 is running
Absent: 1
C2 is running
Thread library exiting.*/

int absent = 2;

void gate(void* arg)
{
cout<<(char*)arg<<" is running"<<endl;
 thread_lock(1);
 absent = absent - 1;
 cout<<"Absent: "<<absent<<endl;
 if (absent == 0){
    cout<<(char*)arg<<" is running"<<endl;
    thread_signal(1, 1);
    cout<<(char*)arg<<" is running"<<endl;
    thread_wait(1, 2);
    cout<<(char*)arg<<" is running"<<endl;
    thread_yield(); /* a little twirl for fun */
    cout<<(char*)arg<<" is running"<<endl;
    thread_unlock(1);
 }
 return; 
} 

void makeThreads(void* arg) { /* C */
    thread_lock(1);
    cout<<(char*)arg<<" is running"<<endl;
    thread_create((thread_startfunc_t)gate, (void *)"C1"); /* 1 */
    thread_create((thread_startfunc_t)gate, (void *)"C2"); /* 2 */
    cout<<(char*)arg<<" is running"<<endl;
    thread_wait(1, 1);
    cout<<(char*)arg<<" is running"<<endl;
    thread_broadcast(1, 2);
    cout<<(char*)arg<<" is running"<<endl;
    thread_unlock(1);
    return; 
} 

int main(int argc, char** argv){
  
  thread_libinit((thread_startfunc_t) makeThreads, (void*) "First");
  exit(0);
}
