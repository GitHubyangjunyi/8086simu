#ifndef DO_INS_H
#define DO_INS_H 1

#include <string>
#include <vector>

using namespace std;

extern const unsigned memory_size; // because 64K is enough for every thing

extern int regs[14];
extern  vector<char> memory;
extern bool runing;

// load and store is 16 bit
unsigned load(unsigned pos) {
    if (pos >= memory_size)
        throw "read memory out";
    int v = 0;
    // printf("loadm (%X) %02X%02X\n", pos, memory[pos*2], memory[pos*2+1]);
    v  = memory[pos*2] << 8;
    v |= memory[pos*2+1] & 0xFF;
    return v;
}
void store(unsigned pos, unsigned w) {
    if (pos >= memory_size)
        throw "write memory out";
    memory[pos*2] = (w >> 8) & 0xFF;
    memory[pos*2+1] = w & 0xFF;
    // printf("store memory%5X in %d\n", w, pos);
}

unsigned get_pos(unsigned seg, unsigned reg)
{
    return (regs[seg] << 4) + regs[reg];
}

// 16bit
int to_signed(unsigned u)
{
    return (u & 0x8000) ? (u - 0xFFFF - 1): u;
}
void do_ins(unsigned ins) // 32 bit
{
    unsigned in    = (ins >> 24) & 0xFF;
    bool is_idata = in & 0x80;
    unsigned inscode    = in & 0x7F;

    unsigned reg   = (ins >> 16) & 0xFF;
    unsigned uidt  = (ins)       & 0xFFFF;

    unsigned reg1  = (ins >>  8) & 0xFF;
    unsigned reg2  = (ins >>  0) & 0xFF;
    unsigned reg3  = (ins >> 16) & 0xFF;
    // printf("uidt %X\n", uidt);

    // printf("instruction: %X , is_idata: %d\n", inscode, is_idata);
    unsigned code;
    int res, rem;
    switch (inscode) {
    case MOV:
        unsigned pos;
        printf("MOV ");
        if (is_idata) {
            // instant data
            regs[reg] = uidt;
            printf("%s,%d\n", reg_repr[reg].c_str(), uidt);
        } else {
            // reg1 = reg2
            regs[reg1] = regs[reg2];
            printf("%s,%s\n", reg_repr[reg1].c_str(), reg_repr[reg2].c_str());
        }
        regs[IP] += 2;
        break;
    case LOAD:
        // load
        pos = get_pos(DS, reg2);
        regs[reg1] = load(pos);
        printf("LOAD %s,[%s] (%d)\n",
            reg_repr[reg1].c_str(), reg_repr[reg2].c_str(), regs[reg1]);
        regs[IP] += 2;
        break;
    case SAVE:
        // store
        pos = get_pos(DS, reg1);
        store(pos, regs[reg2]);
        printf("SAVE [%s],%s\n", reg_repr[reg1].c_str(), reg_repr[reg2].c_str());
        regs[IP] += 2;
        break;
    case ADD:
        cout<<"ADD ";
        if (is_idata) {
            cout<<reg_repr[reg]<<","<<uidt;
            regs[reg] = regs[reg] + uidt;
            cout<<" => "<<regs[reg]<<endl;
        } else {
            cout<<reg_repr[reg1]<<","<<reg_repr[reg2];
            regs[reg1] = regs[reg1] + regs[reg2];
            cout<<" => " <<regs[reg1]<<endl;
        }
        regs[IP] += 2;
        break;
    case SUB:
        cout<<"SUB"<<endl;
        if (is_idata) {
            regs[reg] = regs[reg] - uidt;
        } else {
            regs[reg1] = regs[reg1] - regs[reg2];
        }
        regs[IP] += 2;
        break;
    case MUL:
        cout<<"MUL"<<endl;
        if (is_idata) {
            regs[reg] = regs[reg] * uidt;
        } else {
            regs[reg1] = regs[reg1] * regs[reg2];
        }
        regs[IP] += 2;
        break;
    case DIV:
        cout<<"DIV"<<endl;
        if (is_idata) {
            printf("%d / %d\n", regs[reg], uidt);
            res = regs[reg] / uidt;
            rem = (regs[reg]) % uidt;
            regs[reg] = res; regs[DX] = rem;
        } else {
            printf("%d / %d\n", regs[reg], regs[reg2]);
            res = regs[reg1] / regs[reg2];
            rem = (regs[reg]) % (regs[reg2]);
            regs[reg1] = res; regs[DX] = rem;
        }
        printf("%d %d\n", res, rem);

        regs[IP] += 2;
        break;
    case NOT: // can be short
        cout<<"NOT"<<endl;
        regs[reg1] = ~(regs[reg1]);
        regs[IP] += 2;
        break;
    case AND:
        cout<<"AND"<<endl;
        if (is_idata) {
            regs[reg] = regs[reg] & uidt;
        } else {
            regs[reg1] = regs[reg1] & regs[reg2];
        }
        regs[IP] += 2;
        break;
    case OR:
        cout<<"OR"<<endl;
        if (is_idata) {
            regs[reg] = regs[reg] | uidt;
        } else {
            regs[reg1] = regs[reg1] | regs[reg2];
        }
        regs[IP] += 2;
        break;
    case XOR:
        cout<<"XOR"<<endl;
        if (is_idata) {
            regs[reg] = regs[reg] ^ uidt;
        } else {
            regs[reg1] = regs[reg1] ^ regs[reg2];
        }
        regs[IP] += 2;
        break;
    case JCXZ: // can be short
        printf("JCXZ :%X ? ", regs[CX]);
        if (regs[CX] == 0) {
            regs[IP] += 2;
            break;
        }
        if (is_idata) {
            // printf("%d => %d\n", uidt, to_signed(uidt));
            regs[IP] += to_signed(uidt);
            printf("%d\n", to_signed(uidt));
        } else {
            regs[IP] = regs[reg1]; // long jump
            printf("[%s]\n", reg_repr[reg1].c_str());
        }
        break;
    case JMP: // can be short
        printf("JMP ");
        if (is_idata) {
            regs[IP] += to_signed(uidt);
            printf("%d\n", to_signed(uidt));
        } else {
            regs[IP] = regs[reg1]; // long jump
            printf("[%s](%d)\n", reg_repr[reg1].c_str(), regs[reg1]);
        }
        break;
    case INT: // can be short
        code = is_idata ? uidt : regs[reg1];
        printf("INT %d\n", code);
        switch (code) {
            case 0:
                runing = false;
                break;
#ifdef TEST_MODE
#include "tests/.test.int.h"
#endif
        }
    case NOP: // can be short
        regs[IP] += 2;
        break;
    case PUSH: // can be short
        cout<<"PUSH "<<reg_repr[reg1];
        regs[SP]--;
        store(get_pos(SS, SP), regs[reg1]);
        cout<<"("<<regs[SP]<<")"<<endl;
        regs[IP] += 2;
        break;
    case POP: // can be short
        cout<<"POP "<<reg_repr[reg1]<<endl;
        regs[reg1] = load(get_pos(SS, SP));
        regs[SP]++;
        regs[IP] += 2;
        break;
    }
}
void cpu_run()
{
    unsigned ins, p;
    char c = 'y';
    while (runing && c == 'y') {
        ins = load(get_pos(CS, IP));
        p   = load(get_pos(CS, IP)+1);
        // printf("------------\nI: %04X|%04X\n",
        //     ins&0xFFFF, p&0xFFFF);
        // cin >> c;
        // printf("%d\n", c);
        do_ins(ins<<16|(p&0xFFFF));

        // print regs
        // for (int i = 0; i < reg_repr.size(); ++i) {
        //     printf("%s:%X ", reg_repr[i].c_str(), regs[i]);
        //     if (i && i % 8 == 0) {
        //         printf("\n");
        //     }
        // }
        // printf("\n");
    }
}
#endif
