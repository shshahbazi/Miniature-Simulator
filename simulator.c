#include "simulator.h"

const char* Signals[15][11] = { {"0", "0", "0", "0", "0", "0", "1", "1", "0", "0", "0010"},
                                {"0", "0", "0", "0", "0", "0", "1", "1", "0", "0", "0110"},
                                {"0", "0", "0", "0", "0", "0", "1", "1", "0", "0", "0111"},
                                {"0", "0", "0", "0", "0", "0", "1", "1", "0", "0", "0001"},
                                {"0", "0", "0", "0", "0", "0", "1", "1", "0", "0", "0000"},
                                {"0", "0", "0", "0", "0", "1", "0", "1", "0", "0", "0010"},
                                {"0", "0", "0", "0", "0", "1", "0", "1", "0", "0", "0111"},
                                {"0", "0", "0", "0", "0", "1", "0", "1", "0", "0", "0001"},
                                {"0", "0", "0", "0", "0", "0", "0", "1", "0", "1", "0000"},
                                {"0", "0", "1", "0", "1", "1", "0", "1", "0", "0", "0010"},
                                {"0", "0", "0", "1", "0", "1", "0", "0", "0", "0", "0010"},
                                {"0", "1", "0", "0", "0", "0", "0", "0", "0", "0", "0110"},
                                {"0", "0", "0", "0", "0", "0", "0", "1", "1", "0", "0000"},
                                {"1", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0000"},
                                {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0000"} };
int registers[16] = { 0 };
int memory[16001] = { 0 };
int regUsage[16] = { 0 };
int totalusage = 0;
int totalwords = 0;
void main(int argc, char** argv)
{
    FILE* machp = fopen(argv[1], "r");
    FILE* logFile = fopen(argv[2], "w+");
    int finish = 0;
    struct instruction* currInst;
    int line, instCnt = 0, pc = 0;
    currInst = (struct instruction*)malloc(sizeof(struct instruction));
    loader(machp);
    if (argc < 3) 
    {
        printf("***** Please run this program as follows:\n");
        printf("***** %s assprog.as machprog.m\n", argv[0]);
        printf("***** where machprog.m is your machine code\n");
        printf("***** and log.txt will be your log file.\n");
        exit(1);
    }
    if (!machp)
    {
        printf("%s cannot be openned\n", argv[1]);
        exit(1);
    }
    if (!logFile) 
    {
        printf("%s cannot be openned\n", argv[2]);
        exit(1);
    }
    while (fscanf(machp, "%d", &line) != EOF)
    {
        instCnt++;
        strcpy(currInst->binInst, toStr(line, 32));
        currInst->pc = pc;
        currInst->opcode = toInt(&currInst->binInst[4], 4);
        if (currInst->opcode == 14)
        {
            finish = 1;
        }
        if (finish)
        {
            fprintf(logFile, "Instructions: %d lines\n", instCnt);
            fprintf(logFile, "Memmory : %d words\n", totalwords);
            for (int i = 0; i < 16; i++)
                fprintf(logFile, "R%d usage : %%%d \n", i, (100 * regUsage[i] / totalusage));
            exit(0);
        }
        pc = exe(currInst);
        fprintf(logFile, "PC: %d => %s\n", pc, currInst->binInst);
        for (int i = 0; i < 16; i++)
            fprintf(logFile, "R%d = %d\n", i, registers[i]);
    }
}
void loader(FILE* m)
{
    int line, i = 0;
    while (fscanf(m, "%d", &line) != EOF)
    {
        memory[i] = line;
        i++;
        totalwords++;
    }
    rewind(m);
}
int toInt(char* str, int bitnum)
{
    int res = 0, r = 0;
    for (int i = bitnum - 1; i >= 0; i--)
    {
        res += (str[i] - '0') * pow(2, r);
        r++;
    }
    if (str[0] == '1' && bitnum == 32)
        return pow(2, bitnum) - res;
    return res;
}
char* toStr(int n, int bitnum)
{
    if (n < 0)
        n += pow(2, bitnum);
    char* res = (char*)malloc(bitnum + 1);
    for (int i = 0; i < bitnum; i++)
        res[i] = '0';
    res[bitnum] = '\0';
    int j = bitnum - 1;
    while (n > 0)
    {
        res[j] = (n % 2) + '0';
        n /= 2;
        j--;
    }
    return res;
}
struct Mux* MuxConstruct(char* f, char* s, int b)
{
    struct Mux* newmux = (struct Mux*)malloc(sizeof(struct Mux));
    newmux->first = (char*)malloc(b + 1);
    newmux->second = (char*)malloc(b + 1);
    memcpy(newmux->first, &f[0], b);
    newmux->first[b] = '\0';
    memcpy(newmux->second, &s[0], b);
    newmux->second[b] = '\0';
    return newmux;
}
char* Out(const char* signal, struct Mux* t)
{
    if (!strcmp(signal, "0"))
        return t->first;
    else return t->second;
}
void initSignals(int opcode, struct ControlUnit* t)
{
    t->Jump = Signals[opcode][0];
    t->Branch = Signals[opcode][1];
    t->MemRead = Signals[opcode][2];
    t->MemWrite = Signals[opcode][3];
    t->MemToReg = Signals[opcode][4];
    t->ALUSrc = Signals[opcode][5];
    t->RegDst = Signals[opcode][6];
    t->RegWrite = Signals[opcode][7];
    t->jalr = Signals[opcode][8];
    t->lui = Signals[opcode][9];
    t->ALUControl = Signals[opcode][10];
}
int Result(struct ALU* t)
{
    if (!strcmp(t->opcode, "0000"))
        return (t->source1) & (t->source2);
    if (!strcmp(t->opcode, "0001"))
        return (t->source1) | (t->source2);
    if (!strcmp(t->opcode, "0010"))
        return (t->source1) + (t->source2);
    if (!strcmp(t->opcode, "0110"))
        return (t->source1) - (t->source2);
    if (!strcmp(t->opcode, "0111"))
        return (t->source1 < t->source2);
    return -1;
}
const char* Zero(struct ALU* t)
{
    if (Result(t) == 0)
        return "1";
    return "0";
}
int ReadR1(struct RegisterFile* t)
{
    totalusage++;
    regUsage[t->rs]++;
    return (registers[t->rs]);
}
int ReadR2(struct RegisterFile* t)
{
    totalusage++;
    regUsage[t->rt]++;
    return (registers[t->rt]);
}
void regWrite(int d, struct RegisterFile* t)
{
    if (!strcmp(t->regWrite, "1"))
    {
        totalusage++;
        regUsage[t->rd]++;
        registers[t->rd] = d;
    }
}
char* memoryCont()
{
    char* res = NULL;
    for (int i = 0; i <= 16000; i++)
    {
        strcat(res, (char*)i);
        strcat(res, " : ");
        strcat(res, (char*)(memory[i]));
        strcat(res, "\n");
    }
    return res;
}
int Read(struct MEM* t)
{
    if (!strcmp(t->memRead, "1"))
        if (t->address >= 0 && t->address <= 16000)
            return memory[t->address];
}
void memWrite(struct RegisterFile* rg, struct MEM* t)
{
    if (!strcmp(t->memWrite, "1"))
    {
        memory[t->address] = ReadR2(rg);
        totalwords++;
    }
}
int exe(struct instruction* t)
{
    struct ControlUnit* cu = (struct ControlUnit*)malloc(sizeof(struct ControlUnit));
    initSignals(t->opcode, cu);
    struct Mux* RegDstMux = MuxConstruct(&(t->binInst[12]), &(t->binInst[16]), 4);
    struct RegisterFile* rg = (struct RegisterFile*)malloc(sizeof(struct RegisterFile));
    rg->rs = toInt(&(t->binInst[8]), 4);
    rg->rt = toInt(&(t->binInst[12]), 4);
    rg->rd = toInt(Out(cu->RegDst, RegDstMux), 4);
    strcpy(rg->regWrite, cu->RegWrite);
    char seunit[33];
    for (int i = 0; i < 16; i++)
    {
        if (t->binInst[16] != '0')
            seunit[i] = '1';
        else seunit[i] = '0';
    }
    for (int i = 16; i < 33; i++)
        seunit[i] = t->binInst[i];
    struct Mux* AluSrcMux = MuxConstruct(toStr(ReadR2(rg), 32), seunit, 32);
    struct ALU* alu = (struct ALU*)malloc(sizeof(struct ALU));
    strcpy(alu->opcode, cu->ALUControl);
    alu->source1 = ReadR1(rg);
    alu->source2 = toInt(Out(cu->ALUSrc, AluSrcMux), 32);
    struct MEM* mem = (struct MEM*)malloc(sizeof(struct MEM));
    mem->address = Result(alu);
    strcpy(mem->memRead, cu->MemRead);
    strcpy(mem->memWrite, cu->MemWrite);
    memWrite(rg, mem);
    struct Mux* MemRegMux = MuxConstruct(toStr(Result(alu), 32), toStr(Read(mem), 32), 32);
    char loadupper[33];
    for (int i = 16; i < 33; i++)
        loadupper[i - 16] = t->binInst[i];
    for (int i = 16; i < 33; i++)
        loadupper[i] = '0';
    struct Mux* luMux = MuxConstruct(Out(cu->MemToReg, MemRegMux), loadupper, 32);
    struct Mux* jalrMux = MuxConstruct(Out(cu->lui, luMux), toStr((t->pc + 1), 32), 32);
    regWrite(toInt(Out(cu->jalr, jalrMux), 32), rg);
    char BranchControl[2] = "0";
    struct Mux* branchMux = MuxConstruct(toStr((t->pc + 1), 32), toStr((t->pc + 1) + toInt(seunit, 32), 32), 32);
    if (!strcmp(cu->Branch, "1") && !strcmp(Zero(alu), "1"))
        strcpy(BranchControl, "1");
    else strcpy(BranchControl, "0");
    struct Mux* jumpMux = MuxConstruct(Out(BranchControl, branchMux), seunit, 32);
    struct Mux* jalrMux2 = MuxConstruct(Out(cu->Jump, jumpMux), toStr(ReadR1(rg), 32), 32);
    t->pc = toInt(Out(cu->jalr, jalrMux2), 32);
    return t->pc;
}