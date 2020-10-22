#define HASHED_PAGE_TABLE_SIZE 10

union semun {
int val;
struct semid_ds *buf;
unsigned short *array;
};

struct mystruct{//gia tin koini mnimi
int number;
char deiktis;
};

struct node{//komvos hashed page table
  unsigned int vpn; //virtual page number
  unsigned int page_frame;
  char deiktis;
  node *next;
};

struct hash_table_item{
  struct node *head;//isws 8elei kai tail
};

void semaphore_up(int,int);
void semaphore_down(int,int);
void semaphore_initialize(int,int,int);
void init_hashed_page_table(struct hash_table_item*,int);
struct node* find(struct hash_table_item*,unsigned int,int,int);
void insert(struct hash_table_item*,unsigned int,int,char);
void update(struct hash_table_item*,unsigned int,int,char);
int flush(struct hash_table_item*,int,int);
void print_hash(struct hash_table_item*);
