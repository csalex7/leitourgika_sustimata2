#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include "utilityFunctions.h"

void semaphore_up(int sem_id,int sem_number){//function gia na kanoume up ton simaforo me id=sem_id
  struct sembuf sem_b;
  sem_b.sem_num =sem_number;
  sem_b.sem_op = 1;
  sem_b.sem_flg =0;
  if(semop(sem_id,&sem_b,1) == -1){
    perror("Semaphore up operation failed");
    exit(-1);
  }
}

void semaphore_down(int sem_id,int sem_number){//function gia na kanoume down ton simaforo me id=sem_id
  struct sembuf sem_b;
  sem_b.sem_num =sem_number;
  sem_b.sem_op = -1;
  sem_b.sem_flg = 0;
  if(semop(sem_id,&sem_b,1) == -1){
    perror("Semaphore down operation failed");
  }
}

void semaphore_initialize(int s_id,int sem_number,int value){
  union semun sem_union;
  sem_union.val=value;
  if(semctl(s_id,sem_number,SETVAL,sem_union)== -1) {
    perror("Semaphore initialization");
  }
}

void init_hashed_page_table(struct hash_table_item* table,int size){//arxikopoiisi tou page table,ka8e index se NULL
  for(int i=0;i<size;i++){
    table[i].head=NULL;
  }
}

struct node* find(struct hash_table_item* hash,unsigned int vpn,int process_number,int table_index){//sunartisi gia euresi eggrafis ston pinaka katakermastismou
  struct node* temp;
  if(process_number==1){
    temp=hash[table_index].head; //pairnoume tin kefali tis listas sto antistoixo index tou pinaka
  }
  else{
    temp=hash[table_index+(HASHED_PAGE_TABLE_SIZE/2)].head;
  }
  while(temp!=NULL){
    if(temp->vpn==vpn){//vrikame tin eggrafi ston pinaka
      //printf("VRETHIKE\n");
      return temp;
    }
    temp=temp->next;
  }
  //printf("DEN VRETHIKE\n");
  return NULL;
}

void insert(struct hash_table_item* hash,unsigned int vpn,int table_index,char d){
    struct node *list;
    list = hash[table_index].head;
    struct node *new_entry = (struct node*) malloc(sizeof(struct node));
    new_entry->vpn=vpn;
    new_entry->page_frame= vpn/10; //tuxaio
    new_entry->deiktis= d;
    new_entry->next= NULL;

    if(list==NULL){//an i lista  se auto to index einai null
      hash[table_index].head=new_entry;
    }
    else{//an i lista exei stoixeia,pros8etoume sto telos tis
      while(list->next!=NULL){
        //printf("EDWWWWWW\n");
        list=list->next;
      }
      list->next=new_entry;
    }

}


void update(struct hash_table_item* hash,unsigned int vpn,int table_index,char d){
  struct node *list;
  list = hash[table_index].head;
  while(list!=NULL){
    if(list->vpn==vpn){
      list->deiktis=d;
      return;
    }
    list=list->next;
  }
}

int flush(struct hash_table_item* hash,int starting_index,int stop_index){
  struct node *current;
  struct node *temp;
  int num_of_writes=0;
  for(int i=starting_index;i<stop_index;i++){
    current=hash[i].head;
    while(current!=NULL){
      if(current->deiktis=='W'){//an prepei na sw8ei ston disko i selida epeidi tropopoii8ike
        num_of_writes++;
      }
      temp=current->next;
      free(current);
      current=temp;
    }
    hash[i].head=NULL;
  }
  return num_of_writes;
}

void print_hash(struct hash_table_item* hash){
  int i=0;
  struct node *temp;
  for(i=0;i<HASHED_PAGE_TABLE_SIZE;i++){
    printf("i=%d\n",i);
    temp=hash[i].head;
    while(temp!=NULL){
      printf("temp_vpn==%u\n",temp->vpn);
      //printf("dektis=%c\n",temp->deiktis);
      temp=temp->next;
    }
  }
}
