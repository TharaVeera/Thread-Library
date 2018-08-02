#include <ucontext.h>
#include <iostream>
#include <map>
#include <iterator>
#include <queue>
//#include <tuple>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "thread.h"
#include "interrupt.h"

using namespace std;

/*Each of these functions returns -1 on failure. Each of these functions returns 0 on success,
except for thread_libinit, which does not return at all on success. */


typedef struct TCB //Thread Context block
{
  int name;
  char* sp;
  ucontext_t* context;
  bool alive;
} TCB;


typedef struct Tup{ //Mutex tuple
    int lock;
    int CV;
}Tup;


static queue<TCB*> readyqueue; //Ready queue of threads' TCBs
static map<int, queue<TCB*> > lockqueues; //Map containing which threads are waiting on certain locks
//typedef tuple<int, int> locktups;
static map< int, map<int, queue<TCB*> > >cvqueues; //Map containing which threads are waiting on certain lock, CV pairs
static map<int, int> locks; //


TCB* curr; //Current thread
TCB* first; //First thread the creates other threads
int aName = 0; //Used as unique ID for each thread
TCB* toDelete; 
ucontext_t* beginning;
ucontext_t* original;
bool intiated = false;

static int context_switch();

/* Delete thread by passing pointer to thread context block*/
void delete_thread(TCB* toDelete){
   //Delete thread's stack and register calues
    toDelete->context->uc_stack.ss_sp = NULL;
    delete(toDelete->sp);
    toDelete->context->uc_stack.ss_size = 0;
    toDelete->context->uc_stack.ss_flags = 0;
    toDelete->context->uc_link = NULL;
    delete(toDelete->context);
    delete(toDelete);
    return;
  }

  //Creates threads by passing them to the thread_startfunc_t func
  int trampoline(thread_startfunc_t func, void* arg){
      interrupt_enable();
      func(arg); //Create the other threads

        while(!readyqueue.empty()){
            TCB *next = readyqueue.front(); //Take the next thread off the ready queue

            ucontext_t* newContext = next->context; 
            ucontext_t* oldContext = curr->context;
            if(toDelete == NULL){ //If no more threads to delete, delete the current thread
                toDelete = curr;
                interrupt_disable();
                context_switch();
                interrupt_enable();
            }
            else{ //Else, delete the thread and set the next thread to be deleted to the current thread
              delete_thread(toDelete);
              toDelete = curr;
              interrupt_disable();
              context_switch();
              interrupt_enable();
            }
        }
        interrupt_disable();
        swapcontext(curr->context, original); //Return to original thread that created other threads

    return 0;
}



int thread_create(thread_startfunc_t func, void* arg){
    interrupt_disable();
    if(!intiated) return -1;
    ucontext_t* ucontext_ptr;
    try{
        ucontext_ptr = new ucontext_t();
    }
    catch(bad_alloc& ba){
        return -1;
    }

    /*The function getcontext() initializes the structure pointed at by ucp to the currently active context.
    When successful, getcontext() returns 0 and setcontext() does not return. On error, both return -1
    */
    char *stack;
    TCB* t;
    try{ //Initialize stack and registers for new thread
        getcontext(ucontext_ptr);
        stack = new char [STACK_SIZE];
        ucontext_ptr->uc_stack.ss_sp = stack;
        ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
        ucontext_ptr->uc_stack.ss_flags = 0;
        ucontext_ptr->uc_link = NULL;
        t = new TCB;

        aName++;
        t->name = aName; //Initialize identifier/name for each thread
        t->sp = stack;
        t->context = ucontext_ptr;
    }
    catch(bad_alloc& ba){
        interrupt_enable();
        return -1;
    }
    makecontext(ucontext_ptr, (void (*)())trampoline, 2, func, arg); //Modifies context for other threads

    readyqueue.push(t); //Put newly created thread on the ready queue.
    interrupt_enable();
    return 0;


  }

/* Creates the first thread and saves the context. This thread is switched into at the end used to destroy to all other threads */
  int thread_libinit(thread_startfunc_t func, void* arg){
    if (intiated) return -1;
    intiated = true;

    try{
      first = new TCB;
    }
    catch (bad_alloc& ba){
        return -1;
    }

    try{
        beginning = new ucontext_t;
    }
    catch(bad_alloc& ba){
        return -1;
    }

    first->name = aName++;
    first->context = beginning;
    getcontext(beginning);

    char *stack;
    try{
      stack = new char [STACK_SIZE];
      beginning->uc_stack.ss_sp = stack;
      beginning->uc_stack.ss_size = STACK_SIZE;
      beginning->uc_stack.ss_flags = 0;
      beginning->uc_link = NULL;
      first->sp = stack;
      first->alive = true;
      curr = first;
    }
    catch(bad_alloc& ba){
        beginning->uc_stack.ss_sp = NULL;
        beginning->uc_stack.ss_size = 0;
        delete(stack);
        return -1;
    }

    
    makecontext(beginning, (void (*)())trampoline, 2, func, arg); //Modifies contexts for other threads
    try{
      original = new ucontext_t();
    }
    catch(bad_alloc& ba){
        return -1;
    }

    interrupt_disable();
    swapcontext(original, beginning); //Save registers to beginning thread
    delete(first->alive);
    delete(first->sp);
    delete(first->name);
    delete(first->context);
    delete(first);
    interrupt_enable();
    cout << "Thread library exiting.\n";
    exit(0);
  }

  static int context_switch(){ //Change threads
    if (!readyqueue.empty()){
        TCB* toSwitch = readyqueue.front(); //Get TCB at front of ready queue
        readyqueue.pop(); //Removes element at front of queue
        ucontext_t* oldContext = curr->context;
        ucontext_t* newContext = toSwitch->context;
        curr = toSwitch;
        swapcontext( oldContext, newContext);
        return 0;
    }

    cout<<"Thread library exiting.\n";
    exit(0);
  }



int thread_yield(void){
    interrupt_disable();
    readyqueue.push(curr); //Put current running thread on ready queue
    context_switch(); //Start running thread by popping from ready queue
    interrupt_enable();
    return 0;
  }


  int thread_lock(unsigned int lock){
    interrupt_disable();
    //If lock not held by a thread, set current thread as owner
    if(locks.count(lock) == 0 || locks[lock]== -1 || locks[lock] == curr->name){
      locks[lock]= curr->name;
    }
    else{ //Lock already in use
      if(lockqueues.count(lock) == 0){
        queue<TCB*> myQ;
        myQ.push(curr);
        lockqueues[lock] = myQ;
        delete(myQ);
        context_switch();
        interrupt_enable();
        return 0;
    }
      else{
        queue<TCB*> myQ = lockqueues[lock];
        myQ.push(curr);
        lockqueues[lock] = myQ;
        delete(myQ);
        context_switch();
        interrupt_enable();
        return 0;
      }
    }
    interrupt_enable();
    return 0;
  }

  int thread_lockwoInterrupts(unsigned int lock){
    if(!intiated) return -1;
    //cout<<"in thread lock with thread:    "<<curr->name<<endl;
    //If the current thread already owns the lock, or no one owns the lock
    if(locks.count(lock)==0 || locks[lock] == -1 || locks[lock] == curr->name){ 
      locks[lock]= curr->name; //Set this thread to be the owner of the lock
    }
    else{ //Already locked
      if(lockqueues.count(lock)==0){
        queue<TCB*> myQ;
        myQ.push(curr); //Push the current thread to wait on this lock
        lockqueues[lock]= myQ; 
        delete(myQ);
        context_switch();
        return 0;
    }
      else{
        queue<TCB*> myQ= lockqueues[lock];
        myQ.push(curr);
        lockqueues[lock] = myQ;
        delete(myQ);
        context_switch();
        return 0;
      }
    }

    return 0;
  }


  int thread_unlock(unsigned int lock){
    //cout<<"in thread unlock with thread: "<<curr->name<<endl;
    interrupt_disable();
    if(!intiated) return -1;
    //cout<<"in thread unlock with thread: "<<curr->name<<endl;
    if(locks.count(lock)==0){
      interrupt_enable();
      return -1;
    }
    locks[lock]= -1; //Unlock - set no one to be the owner of this lock
    if(!lockqueues[lock].empty()){
      queue<TCB*> myQ = lockqueues[lock];
      TCB* readyblock = myQ.front();
      myQ.pop(); //pop off TCB from queue to put on readylist
      readyqueue.push(readyblock);
      locks[lock] = readyblock->name;
      lockqueues[lock] = myQ;
      delete(myQ);
    }
    //cout<<"enable error10"<<endl;
    interrupt_enable();
    return 0;
}
int thread_unlockwoInterrupts(unsigned int lock){
  //cout<<"in thread unlock with thread: "<<curr->name<<endl;
  if(!intiated) return -1;
  //cout<<"in thread unlock with thread: "<<curr->name<<endl;
  if(locks.count(lock)==0){
    //cout<<"enable error9"<<endl;
    interrupt_enable();
    return -1;
  }
  locks[lock]= -1;  
  if(!lockqueues[lock].empty()){ //If threads are waiting on this lock
    queue<TCB*> myQ = lockqueues[lock];
    TCB* readyblock = myQ.front();
    myQ.pop(); //Pop off TCB from queue to put on readylist
    readyqueue.push(readyblock);
    locks[lock] = readyblock->name; //Set this thread to be owner of the lock
    lockqueues[lock] = myQ; 
    delete(myQ);
  }
  //cout<<"enable error10"<<endl;
  return 0;
}


   int thread_signal(unsigned int lock, unsigned int cond){
    interrupt_disable();
    if(!intiated) return -1;
    if(!cvqueues[lock][cond].empty()){ //If there are threads waiting on this lock, condition
      queue<TCB*> myCVq = cvqueues[lock][cond]; //Get the queue
      TCB* readyblock = myCVq.front(); 
      myCVq.pop(); //Pop off the next TCB ready to run
      readyqueue.push(readyblock); //Push that thread to the ready queue
      cvqueues[lock][cond]=myCVq; //Reset the cvqueue
    }
    else{
      interrupt_enable();
      return 0;
    }
    interrupt_enable();
    return 0;
   }

    int thread_broadcast(unsigned int lock, unsigned int cond){
        interrupt_disable();
        if(!intiated) return -1;
            if(!cvqueues[lock][cond].empty()){ //If the cvqueue map has threads waiting in queue
                queue<TCB*> myCVq = cvqueues[lock][cond]; //get the queue
                while (!myCVq.empty()){ //put all TCB blocks on the ready queue
                    TCB* readyblock = myCVq.front();
                    myCVq.pop(); //Pop off the next TCB and push it onto the readyqueue
                    readyqueue.push(readyblock);
                }
                cvqueues[lock][cond]= myCVq; //Reset the cvqueue
            }
            else{ //No thread is waiting
              interrupt_enable();
              return 0;
            }
     interrupt_enable();
     return 0;
    }

  int thread_wait(unsigned int lock, unsigned int cond){
    interrupt_disable();
    if(!intiated) return -1;
  //  cout<<"thread wait with thread: "<<curr->name<<" lock: "<<lock;
  //  cout<<"cond: "<<cond<<endl;
    //interrupt_enable();
    if(locks.count(lock)==0){ //If the lock is not a member of the map, return unsuccessfully
      interrupt_enable();
      return -1;
    }
    locks[lock]= -1; 
    if(!lockqueues[lock].empty()){ //If no thread is waiting on the lock
      queue<TCB*> myQ = lockqueues[lock]; 
      TCB* readyblock = myQ.front(); 
      myQ.pop(); //Pop off TCB from queue to put on readylist
      //cout<<"pushing to ready queue"<<readyblock->name<<endl;
      readyqueue.push(readyblock);
      locks[lock] = readyblock->name; //Set this thread to be the owner of the lock
      lockqueues[lock] = myQ;
    }
    if(cvqueues.count(lock)!=0&&cvqueues[lock].count(cond)!=0){ //If lock, condition combo actually exists in this map
        queue<TCB*> myCVq = cvqueues[lock][cond];
        if(!cvqueues[lock][cond].empty()){ //If there are threads waiting on this lock, condition
            myCVq.push(curr); //Push the thread onto the cvqueue
            cvqueues[lock][cond] = myCVq;
            context_switch(); //Swap threads with the next one on the ready queue
            thread_lockwoInterrupts(lock); //Take the lock
            interrupt_enable();
            return 0;
        }
        else{
            myCVq.push(curr);
            cvqueues[lock][cond] = myCVq;
            context_switch();
            thread_lockwoInterrupts(lock);
            interrupt_enable();
            return 0;

        }
    }
    else{
        queue<TCB*> myCVq;
        myCVq.push(curr);
        cvqueues[lock][cond]= myCVq;
        context_switch();
        thread_lockwoInterrupts(lock);
        interrupt_enable();
        return 0;
    }
    //cout<<"enable error19"<<endl;
    //interrupt_enable();
    if(locks.count(lock)==0 || locks[lock] == -1 || locks[lock] == curr->name){ //If no one owns the lock
      locks[lock]= curr->name; //Set this thread as the owner of the lock
    }
    else{ //already locked
      if(lockqueues.count(lock)==0){ 
        queue<TCB*> myQ;
        myQ.push(curr);
        lockqueues[lock]= myQ;
        context_switch();
    }
      else{
        queue<TCB*> myQ= lockqueues[lock];
        myQ.push(curr);
        lockqueues[lock] = myQ;
        context_switch();
      }
    }
    interrupt_enable();
    return 0;
  }
