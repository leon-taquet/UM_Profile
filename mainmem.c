#include "mainmem.h"

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define T MainMem

#define UNMAPPED_HINT 1000
#define SEGMENTS_HINT 1000

struct MainMem {
    Seq_T unmapped;
    Seq_T segments;

    uint32_t counter;
};

T MainMem_new(Seq_T zero_segment)
{
    T result = malloc(sizeof(*result));
    assert(result != NULL);
    
    result->unmapped = Seq_new(UNMAPPED_HINT);
    result->segments = Seq_new(SEGMENTS_HINT);

    Seq_addlo(result->segments, zero_segment);

    result->counter = 0;

    return result;
}

void MainMem_free(T* mem)
{
    assert(mem != NULL && *mem != NULL);

    Seq_free(&((*mem)->unmapped));

    int length = Seq_length((*mem)->segments);
    for (int i = 0; i < length; ++i) {
        Seq_T segment = (Seq_T)Seq_get((*mem)->segments, i);
        if (segment != NULL) {
            Seq_free(&segment);
        }
    }
    Seq_free(&((*mem)->segments));

    free(*mem);
    *mem = NULL;
}

void MainMem_unmap(T mem, uint32_t idx)
{
    Seq_T segment = Seq_put(mem->segments, idx, (void*)NULL);
    Seq_free(&segment);

    Seq_addlo(mem->unmapped, (void*)(uintptr_t)idx);
}

uint32_t MainMem_map(T mem, uint32_t num)
{
    assert(~(uint32_t)0 > (uint32_t)Seq_length(mem->segments)
        || Seq_length(mem->unmapped) != 0);

    Seq_T addition = Seq_new(num);
    for (int i = 0; (uint32_t)i < num; ++i) {
        Seq_addlo(addition, (void*)(uintptr_t)(uint32_t)0);
    }

    if (Seq_length(mem->unmapped) > 0) {
        int idx = (uint32_t)(uintptr_t)Seq_remlo(mem->unmapped); 
        Seq_put(mem->segments, idx, (void*)addition);
        return idx;
    }

    Seq_addhi(mem->segments, (void*)addition);
    return Seq_length(mem->segments) - 1;
}

Seq_T get_segment(T mem, uint32_t idx)
{
    return (Seq_T)Seq_get(mem->segments, idx);
}

void replace_zero_segment(T mem, Seq_T replacement)
{
    Seq_T old_zero = Seq_put(mem->segments, 0, replacement);
    if (old_zero != NULL) {
        Seq_free(&old_zero);
    }
}

void MainMem_load_program(T mem, uint32_t segment_idx, uint32_t new_counter)
{
    if (segment_idx != (uint32_t)0) {
        Seq_T to_cpy = (Seq_T)Seq_get(mem->segments, segment_idx);
        Seq_T duplicate = Seq_new(Seq_length(to_cpy));
        int length = Seq_length(to_cpy);
        for (int i = 0; i < length; ++i) {
            Seq_addhi(duplicate, Seq_get(to_cpy, i));
        }

        replace_zero_segment(mem, duplicate);
    }

    mem->counter = new_counter;
}

void MainMem_write(T mem, uint32_t segment_idx, uint32_t word_idx, 
    uint32_t val) 
{
    Seq_put((Seq_T)Seq_get(mem->segments, segment_idx), word_idx,
        (void*)(uintptr_t)val);
}

uint32_t MainMem_read(T mem, uint32_t segment_idx, uint32_t word_idx)
{
    return (uint32_t)(uintptr_t)Seq_get(
        (Seq_T)Seq_get(mem->segments, segment_idx), word_idx
    );
}

uint32_t MainMem_next_instruction(T mem) 
{
    return MainMem_read(mem, 0, mem->counter++);
}

void test_unmap()
{
    T totest = MainMem_new(NULL);
    Seq_remlo(totest->segments);
    for (int i = 0; i < 5; ++i) {
        Seq_T addition = Seq_new(5);
        for (int j = 0; j < 5; ++j) {
            Seq_addlo(addition, (void*)(uintptr_t)j);
        }
        Seq_addlo(totest->segments, addition);
    }
    for (int i = 0; i < Seq_length(totest->segments); ++i) {
        MainMem_unmap(totest, i);

        assert((Seq_T)Seq_get(totest->segments, i) == NULL);
        assert((uint32_t)(uintptr_t)Seq_get(totest->unmapped, 0) 
            == (uint32_t)i);
    }

    assert(Seq_length(totest->unmapped) == 5);

    MainMem_free(&totest);
}

void test_map()
{
    T totest = MainMem_new(NULL);
    Seq_remlo(totest->segments);

    for (int i = 0; i < 5; ++i) {
        assert(MainMem_map(totest, 5) == (uint32_t)i);
        Seq_T addition = (Seq_T)Seq_get(totest->segments, i);
        assert(Seq_length(addition) == 5);

        for (int j = 0; j < Seq_length(addition); ++j) {
            assert((uint32_t)(uintptr_t)Seq_get(addition, j) == (uint32_t)0);
        }
    }

    for (int i = 0; i < 5; ++i) {
        MainMem_unmap(totest, i);
    }

    for (int i = 0; i < 5; ++i) {
        assert(MainMem_map(totest, 5) == (uint32_t)(4 - i));

        assert(Seq_length(totest->unmapped) == 4 - i);

        for (int j = 0; j < 5 - i - 1; ++j) {
            assert((Seq_T)Seq_get(totest->segments, j) == NULL);
        }
    }

    MainMem_free(&totest);
}

bool compare_seq(Seq_T seq1, Seq_T seq2)
{
    if (Seq_length(seq1) != Seq_length(seq2)) {
        return false;
    }

    for (int i = 0; i < Seq_length(seq1); ++i) {
        if (Seq_get(seq1, i) != Seq_get(seq2, i)) {
            return false;
        }
    }

    return true;
}

void test_load_program()
{
    MainMem totest = MainMem_new(NULL);
    Seq_remlo(totest->segments);
    for (int i = 0; i < 5; ++i) {
        Seq_T addition = Seq_new(5);
        for (int j = 0; j < 5; ++j) {
            Seq_addlo(addition, (void*)(uintptr_t)(uint32_t)(j + i));
        }
        Seq_addlo(totest->segments, addition);
    }

    for (int i = 0; i < 5; ++i) {
        MainMem_load_program(totest, i, i);
        assert(compare_seq((Seq_T)Seq_get(totest->segments, 0), 
                           (Seq_T)Seq_get(totest->segments, i)));
        assert(totest->counter == (uint32_t)i);
    }
    
    MainMem_free(&totest);
}

void test_readwrite()
{
    MainMem totest = MainMem_new(NULL);
    Seq_remlo(totest->segments);
    for (int i = 0; i < 5; ++i) {
        Seq_T addition = Seq_new(5);
        for (int j = 0; j < 5; ++j) {
            Seq_addlo(addition, (void*)(uintptr_t)(uint32_t)0);
        }
        Seq_addlo(totest->segments, addition);
    }

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            MainMem_write(totest, i, j, i + j); 
        }
        
        Seq_T seg = (Seq_T)Seq_get(totest->segments, i);
        for (int j = 0; j < 5; ++j) {
            assert((uint32_t)(i + j) == (uint32_t)(uintptr_t)Seq_get(seg, j)); 
        }
    }

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            assert((uint32_t)(i + j) == MainMem_read(totest, i, j)); 
        }
    }

    MainMem_free(&totest);
}

void test_next_instruction()
{
    Seq_T zero_seg = Seq_new(5);
    for (int i = 0; i < 5; ++i) {
        Seq_addhi(zero_seg, (void*)(uintptr_t)(uint32_t)i);
    }

    MainMem totest = MainMem_new(zero_seg);

    for (int i = 0; i < 5; ++i) {
        assert(totest->counter == (uint32_t)i);
        assert(MainMem_read(totest, 0, i) == MainMem_next_instruction(totest));
    }

    MainMem_free(&totest);
}
