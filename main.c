#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include "utilityFunctions.h"

int main(int argc, char *argv[]){
  int i,j,pid,parent_id,child_id,hex_page_number,loops,cc,table_index;
  unsigned int page_number,virtual_page_number,offset,page_number_mask,offset_mask;
  int k,q,frames,max=10000;
  if(argc<4 || argc>5){//an dw8oun la8os orismata
    printf("wrong arguments given!\n");
    return -1;
  }
  if(atoi(argv[1])<1 || atoi(argv[2])<1 || atoi(argv[3])<1){
    printf("arguments should be positive\n");
    return -1;
  }
  k=atoi(argv[1]);
  frames=atoi(argv[2]);
  q=atoi(argv[3]);
  if(argc==5){
    if(atoi(argv[4])<1){
      printf("arguments should be positive\n");
      return -1;
    }
    max=atoi(argv[4]);
  }
  int read_from_disk=0,write_on_disk=0,total_pf=0,total_frames=0,PM1_k=0,PM2_k=0,max_frames_per_process=frames/2,PM1_frames=0,PM2_frames=0;

  FILE* fp;
  parent_id=getpid();
  char hexnumber[10],c,d;
  struct node* temp= (struct node*) malloc(sizeof(struct node));

  int sem_id =semget((key_t)1231,3,IPC_CREAT | 0666); //dimiourgoume 3 simaforous
  if (sem_id==-1){
    perror("semaphore creation failed");
    exit(-1);
  }
  semaphore_initialize(sem_id,0,1);//mutex
  semaphore_initialize(sem_id,1,1);//empty
  semaphore_initialize(sem_id,2,0);//full

  int shm_id1=shmget((key_t)7111,sizeof(mystruct)*q,IPC_CREAT | 0666); //dimiourgia shared memory gia na diavivazontai ta aitimata ton MM
  int shm_id2=shmget((key_t)7770,sizeof(int),IPC_CREAT | 0666); //dimiourgia shared memory gia na 3eroume an oi anafores einai apo tin PM1 i tin PM2
  int shm_id3=shmget((key_t)8888,sizeof(int),IPC_CREAT | 0666);

  if (shm_id1==-1 || shm_id2==-1){
    perror("shared memory creation failed");
    exit(-1);
  }

  for(i=0;i<3;i++){
    pid=fork();
    if(pid<0){
      perror("fork failed");
      exit(-1);
    }
    else if(pid==0){
      mystruct* my_struct=(mystruct*)shmat(shm_id1,0,0);//attach shared memory1
      int* process_number=(int*)shmat(shm_id2,0,0);//attach shared memory2

      page_number_mask=0xfffff000; //maska gia na paroume ta 20 prwta bits tis eikonikis selidas
      offset_mask=0x00000fff; //maska gia to offset
      child_id=getpid();
      if(child_id==parent_id+1){
        if ((fp = fopen("bzip.trace", "r")) == NULL){//anoigma arxeiou 1is diergasias
          perror("fopen bzip.trace");
          return 1;
        }
      }
      else if(child_id==parent_id+2){
        if ((fp = fopen("gcc.trace", "r")) == NULL){//anoigma arxeiou 2is diergasias
          perror("fopen gcc.trace");
          return 1;
        }
      }
      struct hash_table_item *hashed_page_table;
      hashed_page_table= (struct hash_table_item*) malloc(HASHED_PAGE_TABLE_SIZE * sizeof(struct hash_table_item));
      init_hashed_page_table(hashed_page_table,HASHED_PAGE_TABLE_SIZE);//dimiourgia pinaka katakermastismou

    for(loops=0;loops<(max/(2*q));loops++){
      if(child_id==parent_id+1){ //i 1i diergasia einai i PM1
        semaphore_down(sem_id,1);
        semaphore_down(sem_id,0);
        //printf("PM1\n");
        *process_number=1;
        for(j=0;j<q;j++){
          fscanf(fp,"%s",&hexnumber[0]);
          page_number=(unsigned int)strtol(hexnumber, NULL, 16);
          fscanf(fp,"%c",&c); //keno
          fscanf(fp,"%c",&c);
          my_struct[j].number=page_number;
          my_struct[j].deiktis=c;
        }
        semaphore_up(sem_id,0);
        semaphore_up(sem_id,2);
      }
      else if(child_id==parent_id+2){ //i 2i diergasia einai i PM2
        semaphore_down(sem_id,1);
        semaphore_down(sem_id,0);
        usleep(100);
        //printf("PM2\n");
        *process_number=2;
        for(j=0;j<q;j++){
          fscanf(fp,"%s",&hexnumber[0]);
          page_number=(unsigned int)strtol(hexnumber, NULL, 16);
          fscanf(fp,"%c",&c); //keno
          fscanf(fp,"%c",&c);
          my_struct[j].number=page_number;
          my_struct[j].deiktis=c;
        }
        semaphore_up(sem_id,0);
        semaphore_up(sem_id,2);
      }
      else if(child_id==parent_id+3){ //i 3i diergasia einai o MM
        for(cc=0;cc<2;cc++){
          semaphore_down(sem_id,2);
          semaphore_down(sem_id,0);
          //printf("MM\n");
          for(j=0;j<q;j++){//eksetazoume q anafores apo ka8e diergasia
            virtual_page_number= page_number_mask & my_struct[j].number;
            offset= offset_mask & my_struct[j].number;
            d=my_struct[j].deiktis;
            //printf("vpn=%d\n",virtual_page_number);
            //printf("offset=%d\n",offset);
            table_index=virtual_page_number % (HASHED_PAGE_TABLE_SIZE/2);//efarmozoume sunartisi katakermastismou(mod)gia na vroume to key
            temp=find(hashed_page_table,virtual_page_number,*process_number,table_index);//psaxnoume to virtual_page_number ston pinaka
            if(temp==NULL){ //an den vre8ike i selida,page fault kai tin vazoume sto hash table
              if(*process_number==1){//anafores apo PM1 eisagontai sto 1o miso
                PM1_k++;//auksanoume ton ari8mo twn page_fault gia tin diergasia PM1
                PM1_frames++;
                read_from_disk++;
                total_pf++;
                if(PM1_k==(k+1) || PM1_frames==(max_frames_per_process+1)){//an ta pf eginan k+1 i gemisan ta dia8esima frames kanoume flush
                  write_on_disk+=flush(hashed_page_table,0,HASHED_PAGE_TABLE_SIZE/2); //kanoume flush to 1o miso tou hash table
                  //printf("FLUSHED PM1 HASH\n");
                  PM1_k=0;
                  PM1_frames=0;
                }
                insert(hashed_page_table,virtual_page_number,table_index,d);
              }
              else if(*process_number==2){//anafores apo PM2 eisagontai sto 2o miso
                table_index=table_index+(HASHED_PAGE_TABLE_SIZE/2);
                PM2_k++;//auksanoume ton ari8mo twn page_fault gia tin diergasia PM2
                PM2_frames++;
                read_from_disk++;
                total_pf++;
                if(PM2_k==(k+1) || PM2_frames==(max_frames_per_process+1)){//an ta pf eginan k+1 i gemisan ta dia8esima frames kanoume flush
                  write_on_disk+=flush(hashed_page_table,HASHED_PAGE_TABLE_SIZE/2,HASHED_PAGE_TABLE_SIZE); //kanoume flush to 2o miso tou hash table
                  //printf("FLUSHED PM2 HASH\n");
                  PM2_k=0;
                  PM2_frames=0;
                }
                insert(hashed_page_table,virtual_page_number,table_index,d);
              }
            }
            else if(temp!=NULL && temp->deiktis=='R' && d=='W'){//an uparxei i selida alla i anafora einai W
              update(hashed_page_table,virtual_page_number,table_index,d); //allazoume tin anafora se W
            }
          }
          semaphore_up(sem_id,0);
          semaphore_up(sem_id,1);
       }
      }

    }
    if(child_id==parent_id+3){
      total_frames=PM1_frames+PM2_frames;
      print_hash(hashed_page_table);
      flush(hashed_page_table,0,HASHED_PAGE_TABLE_SIZE);//free to hast table
      printf("-------STATISTICS-------\n");
      printf("number of reads from disk:%d\n",read_from_disk);
      printf("number of writes on disk:%d\n",write_on_disk);
      printf("total page faults:%d\n",total_pf);
      printf("number of entries tested:%d\n",max);
      printf("number of busy frames:%d\n",total_frames);
    }
    if(child_id==parent_id+1 || child_id==parent_id+2)
      fclose(fp);
    }
  }

  for(j=0;j<3;j++) //perimenoume na termatisoun ola ta paidia
    wait(NULL);

    semctl(sem_id, 0, IPC_RMID);//remove semaphore set
    shmctl(shm_id1, IPC_RMID, 0);//remove shared memory

  return 1;

}
