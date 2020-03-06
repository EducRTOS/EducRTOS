/* What we do here is a kind of OCaml functor for C. It takes as argment
   (that must be defined earlier, possibly as static inline)

   - A prefix_elt_id_t type identifying elements in the array (e.g. an
     index, or a pointer to an element. Structs should also work, but
     involve more copies).
   - A prefix_priority_t type (e.g. an int).
   - A function to get the priorities of an element (prefix_get_priority)
   - A function to compare the priorities (prefix_is_gt_priority) */

#define INSTANTIATE_HEAP(prefix)                                        \
                                                                        \
prefix ## _priority_t prefix ## _get_priority(prefix ## _elt_id_t tid); \
_Bool prefix ## _is_gt_priority(prefix ## _priority_t a, prefix ## _priority_t b); \
                                                                        \
struct prefix ## _heap {                                                \
  /* Number of elements currently in the heap. */                       \
  unsigned int size;                                                    \
  prefix ## _elt_id_t * const array;                                    \
};                                                                      \
                                                                        \
                                                                        \
                                                                        \
void insert_elt(struct prefix ## _heap *heap, prefix ## _elt_id_t elt){ \
  unsigned int i = heap->size++;                                        \
  prefix ## _priority_t priority = prefix ## _get_priority(elt);        \
  while(1){                                                             \
    if(i == 0) break;                 /* root */                        \
    unsigned int parent = (i - 1)/2;                                    \
    if(prefix ## _is_gt_priority(priority,prefix ## _get_priority(heap->array[parent]))) { \
      heap->array[i] = heap->array[parent];                             \
      i = parent;                                                       \
    }                                                                   \
    else break;                                                         \
  }                                                                     \
  heap->array[i] = elt;                                                 \
}                                                                       \
                                                                        \
prefix ## _elt_id_t remove_elt(struct prefix ## _heap *heap) {          \
  prefix ## _elt_id_t res = heap->array[0];                             \
  heap->array[0] = heap->array[--heap->size];                           \
                                                                        \
  unsigned int i = 0;                                                   \
  prefix ## _priority_t i_priority = prefix ## _get_priority(heap->array[i]); \
                                                                        \
  while(1){                                                             \
    unsigned int left = 2 * i + 1;                                      \
    unsigned int right = 2 * i + 2;                                     \
                                                                        \
    unsigned int largest = i;                                           \
    prefix ## _priority_t largest_priority = i_priority;                \
    /* prefix ## _priority_t largest_priority = prefix ## _get_priority(heap->array[i]); */ \
                                                                        \
    if(right < heap->size){                                             \
      prefix ## _priority_t right_priority = prefix ## _get_priority(heap->array[right]); \
      if (prefix ## _is_gt_priority(right_priority,largest_priority)){  \
        largest = right;                                                \
        largest_priority = right_priority;                              \
      }                                                                 \
      /* As right is higher than left, we can skip the test. */         \
      goto skip;                                                        \
    }                                                                   \
    if(left < heap->size){                                              \
    skip:;                                                              \
      prefix ## _priority_t left_priority = prefix ## _get_priority(heap->array[left]); \
      if (prefix ## _is_gt_priority(left_priority,largest_priority)){   \
        largest = left;                                                 \
        largest_priority = left_priority;                               \
      }                                                                 \
    }                                                                   \
                                                                        \
    if(largest == i) return res;                                        \
    /* swap i and largest. */                                           \
    prefix ## _elt_id_t tmp = heap->array[i];                           \
    heap->array[i] = heap->array[largest];                              \
    heap->array[largest] = tmp;                                         \
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

INSTANTIATE_HEAP(test)

#include <stdio.h>
#include <assert.h>

#define NB_ELTS 8

int the_heap_array[NB_ELTS];
  
struct test_heap the_heap = {
    .size = 0,
    .array = &the_heap_array[0],
};

int after_heap;
struct test_heap *heap = &the_heap;
  
void check_is_a_heap(void){
  for(unsigned int i = 0; i < heap->size; i++){
    test_priority_t ip = test_get_priority(heap->array[i]);
    unsigned int left = 2 * i + 1;
    
    if(left >= heap->size) break;
    test_priority_t lp = test_get_priority(heap->array[left]);
    assert(!test_is_gt_priority(lp,ip));
    
    unsigned int right = 2 * i + 2;
    if(right >= heap->size) break;    
    test_priority_t rp = test_get_priority(heap->array[right]);
    assert(!test_is_gt_priority(rp,ip));
  }
}

void print_elt_array(void){
  printf("|");
  for(unsigned int i = 0; i < heap->size; i++){
    printf("%d|", heap->array[i]);
  }
  printf("\n");
}

#include <limits.h>
#include <assert.h>

int main(void){
  print_elt_array();  
  insert_elt(heap,88);
  print_elt_array();
  check_is_a_heap();
  insert_elt(heap,66);
  print_elt_array();
  check_is_a_heap();
  insert_elt(heap,44);
  print_elt_array();
  check_is_a_heap();  
  insert_elt(heap,77);
  print_elt_array();
  check_is_a_heap();  
  insert_elt(heap,11);
  print_elt_array();
  check_is_a_heap();  
  insert_elt(heap,55);
  print_elt_array();
  check_is_a_heap();
  insert_elt(heap,22);
  print_elt_array();
  check_is_a_heap();
  insert_elt(heap,33);
  print_elt_array();
  check_is_a_heap();

  int last = INT_MAX;
  for(int i = 0; i < NB_ELTS; i++){
    int new = remove_elt(heap);
    printf("%d\n", new);
    print_elt_array();
    check_is_a_heap();
    assert(new <= last);
    last = new;
  }

  return 0;
}

#endif
