#define SEGMENT_REG(privilege,in_ldt,index) \
  ((privilege & 3) | ((in_ldt? 1 : 0) << 2) | (index << 3))

static inline void load_code_segment(int privilege, bool in_ldt, int idx){  
  asm volatile ("ljmp %0,$1f\n1:\n"
                :
                : "i"(SEGMENT_REG(privilege, in_ldt, idx))
                : "memory");
}

/* Loads ds,es,fs,gs,and ss with the same data segment. Usually what we want. */
static inline void load_data_segments(int privilege, bool in_ldt, int idx){
  asm volatile ("movw %0, %%ax   \n\
                 movw %%ax,  %%ss\n\
                 movw %%ax,  %%ds\n\
                 movw %%ax,  %%es\n\
                 movw %%ax,  %%fs\n\
                 movw %%ax,  %%gs"
                :
                : "i"(SEGMENT_REG(privilege, in_ldt, idx)) 
                : "memory","eax");
}

static inline void load_es(int privilege, bool in_ldt, int idx){
  asm volatile ("movw %0, %%ax   \n\
                 movw %%ax,  %%es\n"
                :
                : "i"(SEGMENT_REG(privilege, in_ldt, idx)) 
                : "memory","eax");
}


static inline void load_fs(int privilege, bool in_ldt, int idx){
  asm volatile ("movw %0, %%ax   \n\
                 movw %%ax,  %%fs\n"
                :
                : "i"(SEGMENT_REG(privilege, in_ldt, idx)) 
                : "memory","eax");
}

static inline void load_gs(int privilege, bool in_ldt, int idx){
  asm volatile ("movw %0, %%ax   \n\
                 movw %%ax,  %%gs\n"
                :
                : "i"(SEGMENT_REG(privilege, in_ldt, idx)) 
                : "memory","eax");
}



static inline void load_ss(int privilege, bool in_ldt, int idx){
  asm volatile ("movw %0, %%ax   \n\
                 movw %%ax,  %%ss\n"
                :
                : "i"(SEGMENT_REG(privilege, in_ldt, idx)) 
                : "memory","eax");
}


static inline void load_ds(int privilege, bool in_ldt, int idx){
  asm volatile ("movw %0, %%ax   \n\
                 movw %%ax,  %%ds\n"
                :
                : "i"(SEGMENT_REG(privilege, in_ldt, idx)) 
                : "memory","eax");
}


/* Note that task register cannot be in a LDT. */
static inline void load_tr(int privilege, bool in_ldt, int idx){
  asm volatile ("ltr %0" : :"r"((uint16_t) SEGMENT_REG(privilege,in_ldt,idx)) : "memory");
}

