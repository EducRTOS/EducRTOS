/* What we do here is a kind of OCaml functor for C. It takes as argment
   (that must be defined earlier, possibly as static inline)

   - A prefix_elt_id_t type identifying elements in the array (e.g. an
     index, or a pointer to an element. Structs should also work, but
     involve more copies).
   - A prefix_priority_t type (e.g. an int).
   - A function to get the priorities of an element (prefix_get_priority)
   - A function to compare the priorities (prefix_is_gt_priority) */

#define INSTANTIATE_HEAP(prefix,NB_ELTS)                                \
                                                                        \
prefix ## _priority_t prefix ## _get_priority(prefix ## _elt_id_t tid); \
_Bool prefix ## _is_gt_priority(prefix ## _priority_t a, prefix ## _priority_t b); \
                                                                        \
                                                                        \
                                                                        \
static prefix ## _elt_id_t elt_array[NB_ELTS];                          \
static unsigned int elt_array_size = 0;                                 \
                                                                        \
void insert_elt(prefix ## _elt_id_t elt){                               \
  unsigned int i = elt_array_size++;                                    \
  prefix ## _priority_t priority = prefix ## _get_priority(elt);        \
  while(1){                                                             \
    if(i == 0) break;                 /* root */                        \
    unsigned int parent = (i - 1)/2;                                    \
    if(prefix ## _is_gt_priority(priority,prefix ## _get_priority(elt_array[parent]))) { \
      elt_array[i] = elt_array[parent];                                 \
      i = parent;                                                       \
    }                                                                   \
    else break;                                                         \
  }                                                                     \
  elt_array[i] = elt;                                                   \
}                                                                       \
                                                                        \
prefix ## _elt_id_t remove_elt(void) {                                  \
  prefix ## _elt_id_t res = elt_array[0];                               \
  elt_array[0] = elt_array[--elt_array_size];                           \
                                                                        \
  unsigned int i = 0;                                                   \
  prefix ## _priority_t i_priority = prefix ## _get_priority(elt_array[i]); \
                                                                        \
  while(1){                                                             \
    unsigned int left = 2 * i + 1;                                      \
    unsigned int right = 2 * i + 2;                                     \
                                                                        \
    unsigned int largest = i;                                           \
    prefix ## _priority_t largest_priority = i_priority;                \
    /* prefix ## _priority_t largest_priority = prefix ## _get_priority(elt_array[i]); */ \
                                                                        \
    if(right < elt_array_size){                                         \
      prefix ## _priority_t right_priority = prefix ## _get_priority(elt_array[right]); \
      if (prefix ## _is_gt_priority(right_priority,largest_priority)){  \
        largest = right;                                                \
        largest_priority = right_priority;                              \
      }                                                                 \
      /* As right is higher than left, we can skip the test. */         \
      goto skip;                                                        \
    }                                                                   \
    if(left < elt_array_size){                                          \
    skip:;                                                              \
      prefix ## _priority_t left_priority = prefix ## _get_priority(elt_array[left]); \
      if (prefix ## _is_gt_priority(left_priority,largest_priority)){   \
        largest = left;                                                 \
        largest_priority = left_priority;                               \
      }                                                                 \
    }                                                                   \
                                                                        \
    if(largest == i) return res;                                        \
    /* swap i and largest. */                                           \
    prefix ## _elt_id_t tmp = elt_array[i];                             \
    elt_array[i] = elt_array[largest];                                  \
    elt_array[largest] = tmp;                                           \
                                                                        \
    i = largest;                                                        \
  }                                                                     \
}



#if 1
/* Unit test and example of instantiation. */

/* An instance of the heap. */
typedef int test_priority_t;
typedef int test_elt_id_t;
static inline test_priority_t test_get_priority(test_elt_id_t tid) {return tid;}
static inline _Bool test_is_gt_priority(test_priority_t a, test_priority_t b) {
  return a > b;
}

#define NB_ELTS 8
INSTANTIATE_HEAP(test,NB_ELTS)

#include <stdio.h>
#include <assert.h>

void check_is_a_heap(void){
  for(unsigned int i = 0; i < elt_array_size; i++){
    test_priority_t ip = test_get_priority(elt_array[i]);
    unsigned int left = 2 * i + 1;
    
    if(left >= elt_array_size) break;
    test_priority_t lp = test_get_priority(elt_array[left]);
    assert(!test_is_gt_priority(lp,ip));
    
    unsigned int right = 2 * i + 2;
    if(right >= elt_array_size) break;    
    test_priority_t rp = test_get_priority(elt_array[right]);
    assert(!test_is_gt_priority(rp,ip));
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
