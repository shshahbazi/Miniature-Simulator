#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

struct Mux
{
    char* first;
    char* second;
};
struct ControlUnit
{
    const char* Jump, *Branch, *MemRead, *MemWrite, *MemToReg, *ALUSrc, *RegDst, *RegWrite, *jalr, *lui, *ALUControl;
};
struct RegisterFile
{
    char regWrite[2];
    int rs;
    int rt;
    int rd;
};
struct ALU
{
    char opcode[5];
    int source1;
    int source2;
};
struct MEM
{
    int address;
    char memRead[2];
    char memWrite[2];
};
struct instruction
{
    size_t instType; /*0 -> R-format  1 -> I-format  2 -> J-format*/
    size_t intInst;
    char binInst[33];
    int opcode;
    int pc;
};
struct Mux* MuxConstruct(char*, char*, int);
int toInt(char*, int);
char* toStr(int, int);
char* Out(const char*, struct Mux*);
void initSignals(int, struct ControlUnit*);
int ReadR1(struct RegisterFile*);
int ReadR2(struct RegisterFile*);
void regWrite(int, struct RegisterFile*);
int Result(struct ALU*);
const char* Zero(struct ALU*);
char* memoryCont();
int Read(struct MEM*);
void memWrite(struct RegisterFile*, struct MEM*);
int exe(struct instruction*);
void loader(FILE*);