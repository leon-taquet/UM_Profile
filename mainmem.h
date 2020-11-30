#ifndef MAINMEM_INCLUDED
#define MAINMEM_INCLUDED

#include <seq.h> 
#include <stdint.h>

#define T MainMem

typedef struct MainMem *MainMem;

extern T MainMem_new(Seq_T zero_segment);
extern void MainMem_free(T* mem);

extern void     MainMem_unmap(T mem, uint32_t idx);
extern uint32_t MainMem_map  (T mem, uint32_t num);

extern void MainMem_load_program(T mem, uint32_t segment_idx, 
    uint32_t new_counter);

extern void MainMem_write(T mem, uint32_t segment_idx, 
    uint32_t word_idx, uint32_t val);
extern uint32_t MainMem_read(T mem, uint32_t segment_idx,
    uint32_t word_idx);

extern uint32_t MainMem_next_instruction(T mem);

#undef T
#endif
