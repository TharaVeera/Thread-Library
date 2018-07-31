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
typedef struct TCB
{
  int name;
  char* sp;
  ucontext_t* context;
  bool alive;
} TCB;


typedef struct Tup{
    int lock;
    int CV;
}Tup;


static queue<TCB*> readyqueue; //new queue<TCB>;
static map<int, queue<TCB*> > lockqueues; //new map<int, queue<TCB>>;
//typedef tuple<int, int> locktups;
static map< int, map<int, queue<TCB*> > >cvqueues; //new map<tuple<int, int>, queue<TCB>>;
static map<int, int> locks; //new map<int, void*>;


TCB* curr; //current thread you are on
TCB* first;
int aName = 0;
TCB* toDelete;
ucontext_t* beginning;
ucontext_t* original;
bool intiated = false;

static int context_switch();

/* Delete thread by passing pointer to thread context block*/
void delete_thread(TCB* toDelete){

    toDelete->context->uc_stack.ss_sp = NULL;
    delete(toDelete->sp);
    toDelete->context->uc_stack.ss_size = 0;
    toDelete->context->uc_stack.ss_flags = 0;
    toDelete->context->uc_link = NULL;
    delete(toDelete->context);
    delete(toDelete);
    return;
  }

  int trampoline(thread_startfunc_t func, void* arg){
      interrupt_enable();
      func(arg);

        while(!readyqueue.empty()){
            TCB *next = readyqueue.front();

            ucontext_t* newContext = next->context;
            ucontext_t* oldContext = curr->context;
            if(toDelete == NULL){
                toDelete = curr;
                interrupt_disable();
                context_switch();
                interrupt_enable();

            }
            else{
            delete_thread(toDelete);
            toDelete = curr;
            interrupt_disable();
            context_switch();
            interrupt_enable();
            }
        }
        interrupt_disable();
        swapcontext(curr->context, original);


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
    try{
        getcontext(ucontext_ptr);
        stack = new char [STACK_SIZE];
        ucontext_ptr->uc_stack.ss_sp = stack;
        ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
        ucontext_ptr->uc_stack.ss_flags = 0;
        ucontext_ptr->uc_link = NULL;
        t = new TCB;

        aName++;
        t->name = aName; //Create identifier/name for each thread
        t->sp = stack;
        t->context = ucontext_ptr;
    }
    catch(bad_alloc& ba){
        interrupt_enable();
        return -1;
    }
    makecontext(ucontext_ptr, (void (*)())trampoline, 2, func, arg);

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


    makecontext(beginning, (void (*)())trampoline, 2, func, arg);
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

  static int context_switch(){

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
    context_switch();
    interrupt_enable();
    return 0;
  }


  int thread_lock(unsigned int lock){
    interrupt_disable();

    if(locks.count(lock)==0||locks[lock]==-1|| locks[lock]==curr->name){
      locks[lock]= curr->name;
    }
    else{ //already locked
      if(lockqueues.count(lock)==0){
      //  cout<<"is it locked"<<endl;
        queue<TCB*> myQ;
        myQ.push(curr);
        lockqueues[lock]= myQ;
        delete(myQ);
        context_switch();
        interrupt_enable();
        return 0;
    }
      else{
        //cout<<"already done been locked"<<endl;
        queue<TCB*> myQ= lockqueues[lock];
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

  //  interrupt_disable();
    if(!intiated) return -1;
    //cout<<"in thread lock with thread:    "<<curr->name<<endl;
    if(locks.count(lock)==0||locks[lock]==-1|| locks[lock]==curr->name){
      locks[lock]= curr->name;
    }
    else{ //already locked
      if(lockqueues.count(lock)==0){
        //cout<<"is it locked"<<endl;
        queue<TCB*> myQ;
        myQ.push(curr);
        lockqueues[lock]= myQ;
        delete(myQ);
        context_switch();
        //interrupt_enable();
        return 0;
    }
      else{
        //cout<<"already done been locked"<<endl;
        queue<TCB*> myQ= lockqueues[lock];
        myQ.push(curr);
        lockqueues[lock] = myQ;
        delete(myQ);
        context_switch();
        //interrupt_enable();
        return 0;
    }
    }
    //interrupt_enable();
    return 0;
  }


  int thread_unlock(unsigned int lock){
    //cout<<"in thread unlock with thread: "<<curr->name<<endl;
    interrupt_disable();
    if(!intiated) return -1;
  //  cout<<"in thread unlock with thread: "<<curr->name<<endl;

    if(locks.count(lock)==0){
      //cout<<"enable error9"<<endl;

      interrupt_enable();
      return -1;
    }
    locks[lock]= -1;
    if(!lockqueues[lock].empty()){
      queue<TCB*> myQ = lockqueues[lock];
      TCB* readyblock = myQ.front();
      myQ.pop(); //pop off TCB from queue to put on readylist
      readyqueue.push(readyblock);
      //%^&*() JUST CHANGED ThIS
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
  //interrupt_disable();
  if(!intiated) return -1;
//  cout<<"in thread unlock with thread: "<<curr->name<<endl;

  if(locks.count(lock)==0){
    //cout<<"enable error9"<<endl;

    interrupt_enable();
    return -1;
  }
  locks[lock]= -1;
  if(!lockqueues[lock].empty()){
    queue<TCB*> myQ = lockqueues[lock];
    TCB* readyblock = myQ.front();
    myQ.pop(); //pop off TCB from queue to put on readylist
    readyqueue.push(readyblock);
    //%^&*() JUST CHANGED ThIS
    locks[lock] = readyblock->name;
    lockqueues[lock] = myQ;
    delete(myQ);
  }
  //cout<<"enable error10"<<endl;


  //interrupt_enable();
  return 0;
}


   int thread_signal(unsigned int lock, unsigned int cond){

    interrupt_disable();
    if(!intiated) return -1;
    //cout<<"in thread signal"<<endl;
    //cout<<"curr thread"<<curr->name<<endl;
    if(!cvqueues[lock][cond].empty()){
      queue<TCB*> myCVq = cvqueues[lock][cond];
      TCB* readyblock = myCVq.front();
      myCVq.pop();
      readyqueue.push(readyblock);
      cvqueues[lock][cond]=myCVq;
        }
    else{
      //cout<<"enable error11"<<endl;

      interrupt_enable();
      return 0;
        }
    //cout<<"enable error12"<<endl;

    interrupt_enable();
    return 0;
   }

    int thread_broadcast(unsigned int lock, unsigned int cond){

        interrupt_disable();
        if(!intiated) return -1;
        //cout<<"in thread broadcast"<<endl;
            if(!cvqueues[lock][cond].empty()){ //if the cvqueue mpa has threads waiting in queue
                queue<TCB*> myCVq = cvqueues[lock][cond]; //get the queue
                while (!myCVq.empty()){ //put all TCB blocks on the ready queue
                    TCB* readyblock = myCVq.front();
                    myCVq.pop();
                    readyqueue.push(readyblock);
                }
                cvqueues[lock][cond]= myCVq;
            }
            else{
              //cout<<"enable error13"<<endl;

              interrupt_enable();
              return 0;
            }
    //cout<<"enable error14"<<endl;

     interrupt_enable();
     return 0;
    }

  int thread_wait(unsigned int lock, unsigned int cond){

    //cout<<"this is the ready queue front "<<readyqueue.front()->name<<endl;
    interrupt_disable();
    if(!intiated) return -1;
  //  cout<<"thread wait with thread: "<<curr->name<<" lock: "<<lock;
  //  cout<<"cond: "<<cond<<endl;
    //interrupt_enable();
    if(locks.count(lock)==0){
    //  cout<<"enable error9"<<endl;

      interrupt_enable();
      return -1;
    }
    locks[lock]= -1;
    if(!lockqueues[lock].empty()){
      queue<TCB*> myQ = lockqueues[lock];
      TCB* readyblock = myQ.front();
      myQ.pop(); //pop off TCB from queue to put on readylist
      //cout<<"curr->name: "<<curr->name<<endl;
      //cout<<"next->name: "<<readyqueue.front()->name<<endl;
      //cout<<"pushing to ready queue"<<readyblock->name<<endl;
      readyqueue.push(readyblock);
      locks[lock] = readyblock->name;
      lockqueues[lock] = myQ;

    }
    //interrupt_disable();
    //cout<<"in htread wait with bitch thread: "<<curr->name<<endl;
    //cout<<"\n"<<endl;
    //cout<<"this is the ready queue front "<<readyqueue.front()->name<<endl;

    if(cvqueues.count(lock)!=0&&cvqueues[lock].count(cond)!=0){ //if lock, cond combo actually exists in this map
        queue<TCB*> myCVq = cvqueues[lock][cond];
        if(!cvqueues[lock][cond].empty()){
            myCVq.push(curr);
            cvqueues[lock][cond] = myCVq;
        //    cout<<"enable error16"<<endl;

            //interrupt_disable()
            context_switch();
            thread_lockwoInterrupts(lock);
            interrupt_enable();
            return 0;
        }
        else{
            myCVq.push(curr);
            cvqueues[lock][cond] = myCVq;
            //cout<<"enable error17"<<endl;

            //interrupt_enable();
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
        //cout<<"enable error18"<<endl;

        //interrupt_enable();
        context_switch();
        thread_lockwoInterrupts(lock);
        interrupt_enable();
        return 0;
    }
    //cout<<"enable error19"<<endl;
    //interrupt_enable();
    if(locks.count(lock)==0||locks[lock]==-1|| locks[lock]==curr->name){
      locks[lock]= curr->name;
    }
    else{ //already locked
      if(lockqueues.count(lock)==0){
        queue<TCB*> myQ;
        myQ.push(curr);
        //cout<<"adding to lock queue 1"<<endl;
        lockqueues[lock]= myQ;
        context_switch();
    }
      else{
        queue<TCB*> myQ= lockqueues[lock];
        //cout<<"adding to lock queue 2"<<endl;
        myQ.push(curr);
        lockqueues[lock] = myQ;
        context_switch();
    }
    }
    interrupt_enable();
    return 0;
    }
