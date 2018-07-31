#include <iostream>
#include <cmath>
#include <fstream>
#include "thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
using namespace std;


 typedef struct Order{
  int cashier;
  int sandwich;
  Order* next;
} Order;

int numOrders = 0;
int cashiersAlive;
int maxOrders;
Order* orderboard = NULL;

typedef struct file_thread{
  //char* filename;
  int cashier;
}file_thread;

void print_orderboard(){
  Order* head = orderboard;
  if(head){
    while(head->next){
      cout<<" head: "<<head;
      cout<<" sandwich: "<< head->sandwich;
      cout<<" cashier: "<<head->cashier;
      cout<<" next: "<< head->next<<endl;
      head = head->next;
  }
  cout<<"sandwich: "<< head->sandwich;
  cout<<"cashier: "<<head->cashier<<endl;
  cout<<"next: " <<head->next<<endl;
}
  else{
    cout<<"null"<<endl;
  }
  return;
}

Order* closestSandwich(int sand_num){
  //print_orderboard();
  Order* head = orderboard;
  Order* closest;
  Order* temp = new Order;

  int mindiff = 1000;

  while(head->next){
    int diff = abs(head->sandwich - sand_num);
    if(diff <= mindiff){
      mindiff= diff;
      closest = head;
      //cout<<"mindiff"<<mindiff<<"sandwich/sandnum"<<head->sandwich<<"/"<<sand_num<<endl;
    }
    head = head->next;

  }
  //to check last element
  int diff = abs(head->sandwich - sand_num);
  if(diff <= mindiff){
    mindiff = diff;
    closest = head;
  }
  head = orderboard;
  if(head->next){
    while(head->next->next){
      Order* todelete;
      if(head==closest){
        temp->cashier = head->cashier;
        temp->sandwich = head->sandwich;
        temp->next = NULL;
        head->cashier = head->next->cashier;
        head->sandwich = head->next->sandwich;
        todelete = head->next;

        head->next = head->next->next;

        delete(todelete);
        return temp;


    }
    head = head->next;
  }
}
  if(head==closest){
    Order* todelete;
    temp->cashier = head->cashier;
    temp->sandwich = head->sandwich;
    temp->next = NULL;
    if(!head->next){
      todelete = head;
      orderboard = NULL;
      delete(todelete);

      return temp;}
    head->sandwich = head->next->sandwich;
    head->cashier = head->next->cashier;
    todelete = head->next;
    head->next = NULL;
    delete(todelete);
    return temp;
  }
  else{
    Order* todelete;

    temp->cashier = head->next->cashier;
    temp->sandwich = head->next->sandwich;
    temp->next = NULL;
    todelete = head->next;
    head->next = NULL;
    delete(todelete);
    return temp;

  }

  }


bool hasOrder(int num){

  Order* temp = orderboard;

  if(temp!=NULL){
    while(temp->next){

      if(temp->cashier==num){
        return true;
      }
      temp = temp->next;
    }
    if(temp->cashier==num){

      return true;
  }
}

  return false;
}


void postOrder(void* thread_args){
  //
  //print_orderboard();

  file_thread* input = (file_thread*) thread_args;
  //char* filename = input->filename;
  int i = input->cashier;

  thread_lock(1);

  //ifstream inFile;
  //inFile.open(filename);


  int x =0; 

  while(x<4){//*****

    while(numOrders == maxOrders|| numOrders == cashiersAlive||hasOrder(i)){//define This

      thread_signal(1, 2);
      thread_wait(1, 1);

    };
    /*while(hasOrder(i)&& numOrders!=maxOrders &&numOrders!= cashiersAlive){
      thread_wait(1, 1);
      //thread_signal(1,1); ******
    }*/

    Order* bitch2add = new Order;
    bitch2add->cashier = i;//****
    bitch2add->sandwich = rand() % 50 + 1;
    bitch2add->next = NULL;
    numOrders++;
    std::cout << "POSTED: cashier " << i << " sandwich " << x <<endl;
    x++;
    if(orderboard==NULL){
      bitch2add->next = NULL;
      orderboard = bitch2add;
      }
    else{
      bitch2add->next = orderboard->next;
      orderboard->next = bitch2add;

    }
    if(numOrders == maxOrders || numOrders==cashiersAlive){
      thread_signal(1,2);
    }
    thread_wait(1,1);

//figure out how to exit, what to do, update cashiersAlive when etc.
  }
  while(hasOrder(i)){
  thread_signal(1,2);
  thread_wait(1,1);
}

  cashiersAlive--;
  if(numOrders==maxOrders || numOrders ==cashiersAlive){
    thread_signal(1,2);
  }
  thread_unlock(1);
  delete(input);
  return;



}

//
void makeOrder(){
  //thread_wait(1,2);
  thread_lock(1);

  int sand_num = -1;
  while(cashiersAlive>0){
    if(numOrders== maxOrders || numOrders==cashiersAlive){

      Order* toMake = closestSandwich(sand_num);
      int cashier = toMake->cashier;
      sand_num = toMake->sandwich;
      delete(toMake);
      std::cout << "READY: cashier " << cashier << " sandwich " << sand_num <<endl;
      numOrders--;
    }
    thread_broadcast(1, 1);
    thread_wait(1, 2);
  }
  thread_unlock(1);
  delete(orderboard);
  return;
}


void deli_sim(void* argv ){
  char** args = (char**) argv;
  maxOrders = atoi(args[1]);
  void* a;
  //figure out how to get i from argv
  for(int i = 2; i < cashiersAlive+2; i++){
    file_thread* thread_args = new file_thread;
    //thread_args->filename = args[i];
    thread_args->cashier = i-2;
    thread_create((thread_startfunc_t) postOrder, (void*) thread_args);


}
  thread_create((thread_startfunc_t) makeOrder, a);
  start_preemptions(0, 1, 123);
  return;

}

int main(int argc, char** argv){
  cashiersAlive = 5; //argc -2
  thread_libinit((thread_startfunc_t) deli_sim, (void*) argv);
  return(0);
}
