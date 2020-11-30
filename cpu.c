#include "cpu.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "parse.h"

#define N_REGISTERS 8

#define T Cpu

struct Cpu {
    uint32_t registers[N_REGISTERS];
    MainMem mem;
};

T Cpu_new(MainMem mem)
{
    T result = malloc(sizeof(*result));
    result->mem = mem;

    for (int i = 0; i < N_REGISTERS; ++i) {
        result->registers[i] = (uint32_t)0;
    }

    return result;
}

void Cpu_free(T* cpu)
{
    MainMem_free(&((*cpu)->mem));

    free(*cpu);
    *cpu = NULL;
}

void op_output(T cpu, three_register params)
{
    printf("%c", (char)cpu->registers[params.rc]);
}

void op_map(T cpu, three_register params)
{
    cpu->registers[params.rb] = MainMem_map(cpu->mem, 
                                            cpu->registers[params.rc]);
}

void op_unmap(T cpu, three_register params)
{
    MainMem_unmap(cpu->mem, cpu->registers[params.rc]);
}

void op_loadvalue(T cpu, load_value params)
{
    cpu->registers[params.r] = params.value;
}

void op_cmov(T cpu, three_register params)
{
    if (cpu->registers[params.rc] != (uint32_t)0) {
        cpu->registers[params.ra] = cpu->registers[params.rb];
    }
}

void op_sload(T cpu, three_register params)
{
    cpu->registers[params.ra] = MainMem_read(cpu->mem, 
                                             cpu->registers[params.rb],
                                             cpu->registers[params.rc]);
}

void op_sstore(T cpu, three_register params)
{
    MainMem_write(cpu->mem, 
                  cpu->registers[params.ra], 
                  cpu->registers[params.rb], 
                  cpu->registers[params.rc]);
}

void op_add(T cpu, three_register params)
{
    cpu->registers[params.ra] = cpu->registers[params.rb] 
                              + cpu->registers[params.rc];
}   


void op_multiply(T cpu, three_register params)
{
    cpu->registers[params.ra] = cpu->registers[params.rb] 
                              * cpu->registers[params.rc];
}   

void op_divide(T cpu, three_register params)
{
    cpu->registers[params.ra] = cpu->registers[params.rb] 
                              / cpu->registers[params.rc];
}   

void op_nand(T cpu, three_register params)
{
    cpu->registers[params.ra] = ~(cpu->registers[params.rb]
                                & cpu->registers[params.rc]);
}

void op_input(T cpu, three_register params)
{
    uint32_t result = ~(uint32_t)0;
    int input = getc(stdin);
    if (input != EOF) {
        result = (uint32_t)(char)input;
    }
    cpu->registers[params.rc] = result;
}

void op_load_program(T cpu, three_register params)
{
    MainMem_load_program(cpu->mem, cpu->registers[params.rb], 
                                   cpu->registers[params.rc]);
}

bool fetch_decode_execute(T cpu)
{
    uint32_t instruction = MainMem_next_instruction(cpu->mem);
    switch (parse_opcode(instruction)) {
        case OUT:
            op_output(cpu, parse_three_register(instruction));
            break; 
        case IN:
            op_input(cpu, parse_three_register(instruction));
            break;
        case ACTIVATE:
            op_map(cpu, parse_three_register(instruction));
            break; 
        case INACTIVATE:
            op_unmap(cpu, parse_three_register(instruction));
            break;
        case LV:
            op_loadvalue(cpu, parse_load_value(instruction));
            break;
        case CMOV:
            op_cmov(cpu, parse_three_register(instruction));
            break;
        case SLOAD:
            op_sload(cpu, parse_three_register(instruction));
            break;
        case SSTORE:
            op_sstore(cpu, parse_three_register(instruction));
            break;
        case ADD:
            op_add(cpu, parse_three_register(instruction));
            break;
        case MUL:
            op_multiply(cpu, parse_three_register(instruction));
            break;
        case DIV:
            op_divide(cpu, parse_three_register(instruction));
            break;
        case NAND:
            op_nand(cpu, parse_three_register(instruction));
            break;
        case LOADP:
            op_load_program(cpu, parse_three_register(instruction));
            break;
        case HALT:
        default:
            return false;
    }

    return true;
}

void test_output()
{
    MainMem mem = MainMem_new(NULL);
    Cpu cpu = Cpu_new(mem);

    cpu->registers[0] = '\n';
    op_output(cpu, (three_register){ 0, 0, 0 });

    for (int i = 0; i < N_REGISTERS; ++i)
    {
        cpu->registers[i] = 'a' + i;
        op_output(cpu, (three_register){ i, 0, 0 });
    }

    Cpu_free(&cpu);
}
