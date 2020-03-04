#define NB_ELTS 8

/* What we want to do is a kind of functor for C. It takes as argment:

   - An elt identifier type (e.g. an index, or a pointer to an element)
   - A priority type
   - Something to get the priority from the element identifier
   - Something to compare the priorities. */


/* An instance of the heap. */
typedef int priority_t;
typedef int elt_id_t;
priority_t get_priority(elt_id_t tid) {return tid;}
_Bool is_gt_priority(priority_t a, priority_t b) {
  return a > b;
}

extern priority_t get_priority(elt_id_t tid);
extern _Bool is_gt_priority(priority_t a, priority_t b);



static elt_id_t elt_array[NB_ELTS];
static unsigned int elt_array_size = 0;

void insert_elt(elt_id_t elt){
  unsigned int i = elt_array_size++;
  priority_t priority = get_priority(elt);
  while(1){
    if(i == 0) break;                 /* root */
    unsigned int parent = (i - 1)/2;
    if(is_gt_priority(priority,get_priority(elt_array[parent]))) {
      elt_array[i] = elt_array[parent];
      i = parent;
    }
    else break;
  }
  elt_array[i] = elt;
}

elt_id_t remove_elt(void) {
  elt_id_t res = elt_array[0];
  elt_array[0] = elt_array[--elt_array_size];

  unsigned int i = 0;
  priority_t i_priority = get_priority(elt_array[i]);
  
  while(1){
    unsigned int left = 2 * i + 1;
    unsigned int right = 2 * i + 2;

    unsigned int largest = i;
    priority_t largest_priority = i_priority;
    /* priority_t largest_priority = get_priority(elt_array[i]); */

    if(right < elt_array_size){
      priority_t right_priority = get_priority(elt_array[right]);
      if (is_gt_priority(right_priority,largest_priority)){
        largest = right;
        largest_priority = right_priority;
      }
      /* As right is higher than left, we can skip the test. */
      goto skip;
    }
    if(left < elt_array_size){
    skip:;
      priority_t left_priority = get_priority(elt_array[left]);
      if (is_gt_priority(left_priority,largest_priority)){
        largest = left;
        largest_priority = left_priority;
      }
    }
    
    if(largest == i) return res;
    /* swap i and largest. */
    elt_id_t tmp = elt_array[i];
    elt_array[i] = elt_array[largest];
    elt_array[largest] = tmp;

    i = largest;
  }
}

#if 1
/* Unit test. */

#include <stdio.h>
#include <assert.h>

void check_is_a_heap(void){
  for(unsigned int i = 0; i < elt_array_size; i++){
    priority_t ip = get_priority(elt_array[i]);
    unsigned int left = 2 * i + 1;
    
    if(left >= elt_array_size) break;
    priority_t lp = get_priority(elt_array[left]);
    assert(!is_gt_priority(lp,ip));
    
    unsigned int right = 2 * i + 2;
    if(right >= elt_array_size) break;    
    priority_t rp = get_priority(elt_array[right]);
    assert(!is_gt_priority(rp,ip));
  }
}

void print_elt_array(void){
  printf("|");
  for(unsigned int i = 0; i < elt_array_size; i++){
    printf("%d|", elt_array[i]);
  }
  printf("\n");
}

#include <limits.h>
#include <assert.h>

int main(void){
  insert_elt(88);
  print_elt_array();
  check_is_a_heap();
  insert_elt(66);
  print_elt_array();
  check_is_a_heap();
  insert_elt(44);
  print_elt_array();
  check_is_a_heap();  
  insert_elt(77);
  print_elt_array();
  check_is_a_heap();  
  insert_elt(11);
  print_elt_array();
  check_is_a_heap();  
  insert_elt(55);
  print_elt_array();
  check_is_a_heap();
  insert_elt(22);
  print_elt_array();
  check_is_a_heap();
  insert_elt(33);
  print_elt_array();
  check_is_a_heap();

  int last = INT_MAX;
  for(int i = 0; i < NB_ELTS; i++){
    int new = remove_elt();
    printf("%d\n", new);
    print_elt_array();
    check_is_a_heap();
    assert(new <= last);
    last = new;
  }

  return 0;
}

#endif
