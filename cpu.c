#include "cpu.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "parse.h"

#define N_REGISTERS 8

#define T Cpu

uint32_t registers[N_REGISTERS];


struct Cpu {
        //uint32_t registers[N_REGISTERS];
    MainMem mem;
};


T Cpu_new(MainMem mem)
{
    T result = malloc(sizeof(*result));
    result->mem = mem;

    for (int i = 0; i < N_REGISTERS; ++i) {
        registers[i] = (uint32_t)0;
    }

    return result;
}

void Cpu_free(T* cpu)
{
    MainMem_free(&((*cpu)->mem));

    free(*cpu);
    *cpu = NULL;
}

void op_output(three_register params)
{
    printf("%c", (char)registers[params.rc]);
    
}

void op_map(T cpu, three_register params)
{
    registers[params.rb] = MainMem_map(cpu->mem, 
                                            registers[params.rc]);
}

void op_unmap(T cpu, three_register params)
{
    MainMem_unmap(cpu->mem, registers[params.rc]);
}

void op_loadvalue(load_value params)
{
    registers[params.r] = params.value;
}

void op_cmov(three_register params)
{
    if (registers[params.rc] != (uint32_t)0) {
        registers[params.ra] = registers[params.rb];
    }
}

void op_sload(T cpu, three_register params)
{
    registers[params.ra] = MainMem_read(cpu->mem, 
                                             registers[params.rb],
                                             registers[params.rc]);
}

void op_sstore(T cpu, three_register params)
{
    MainMem_write(cpu->mem, 
                  registers[params.ra], 
                  registers[params.rb], 
                  registers[params.rc]);
}

void op_add(three_register params)
{
    registers[params.ra] = registers[params.rb] 
                              + registers[params.rc];
}   


void op_multiply(three_register params)
{
    registers[params.ra] = registers[params.rb] 
                              * registers[params.rc];
}   

void op_divide(three_register params)
{
    registers[params.ra] = registers[params.rb] 
                              / registers[params.rc];
}   

void op_nand(three_register params)
{
    registers[params.ra] = ~(registers[params.rb]
                                & registers[params.rc]);
}

void op_input(three_register params)
{
    uint32_t result = ~(uint32_t)0;
    int input = getc(stdin);
    if (input != EOF) {
        result = (uint32_t)(char)input;
    }
    registers[params.rc] = result;
}

void op_load_program(T cpu, three_register params)
{
    MainMem_load_program(cpu->mem, registers[params.rb], 
                                   registers[params.rc]);
}

void fetch_decode_execute(T cpu)
{
    bool running = true;

    MainMem mem = cpu->mem;

    while ( running ){
        
        //uint32_t instruction = MainMem_next_instruction(cpu->mem);

        uint32_t instruction = mem->zero_seg->seg[mem->counter++];

        switch ((Um_opcode)(instruction >> 28)) {
        case OUT:
            op_output(parse_three_register(instruction));
            break; 
        case IN:
            op_input(parse_three_register(instruction));
            break;
        case ACTIVATE:
            op_map(cpu, parse_three_register(instruction));
            break; 
        case INACTIVATE:
            op_unmap(cpu, parse_three_register(instruction));
            break;
        case LV:
            op_loadvalue(parse_load_value(instruction));
            break;
        case CMOV:
            op_cmov(parse_three_register(instruction));
            break;
        case SLOAD:
            op_sload(cpu, parse_three_register(instruction));
            break;
        case SSTORE:
            op_sstore(cpu, parse_three_register(instruction));
            break;
        case ADD:
            op_add(parse_three_register(instruction));
            break;
        case MUL:
            op_multiply(parse_three_register(instruction));
            break;
        case DIV:
            op_divide(parse_three_register(instruction));
            break;
        case NAND:
            op_nand(parse_three_register(instruction));
            break;
        case LOADP:
            op_load_program(cpu, parse_three_register(instruction));
            break;
        case HALT:
            running = false;
        }
    };
}

/********************************************************/
/* void test_output()                                   */
/* {                                                    */
/*     MainMem mem = MainMem_new(NULL);                 */
/*     Cpu cpu = Cpu_new(mem);                          */
/*                                                      */
/*     registers[0] = '\n';                             */
/*     op_output(cpu, (three_register){ 0, 0, 0 });     */
/*                                                      */
/*     for (int i = 0; i < N_REGISTERS; ++i)            */
/*     {                                                */
/*         registers[i] = 'a' + i;                      */
/*         op_output(cpu, (three_register){ i, 0, 0 }); */
/*     }                                                */
/*                                                      */
/*     Cpu_free(&cpu);                                  */
/* }                                                    */
/********************************************************/
