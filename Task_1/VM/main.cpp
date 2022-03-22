#include <iostream>
#include <fstream>
#include <string>
#include <memory>


enum
{
    R_0 = 0,
    R_1,
    R_2,
    R_3,
    R_4,
    R_5,
    R_SP,
    R_BP,
    R_IP,
    R_COND,
    R_COUNT
};

enum
{
    OP_BR = 0, /* branch */
    OP_AND,    /* and */
    OP_NOT,    /* not */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_PRINT,  /* print string */
    OP_SCAN,   /* scan variable */
    OP_PUSH,   /* push variable on stack */
    OP_POP,    /* pop variable from stack */
    OP_CALL,   /* call function */
    OP_RET     /* return from function */
};

enum
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

uint16_t registers[R_COUNT];

void update_flags(uint16_t r)
{
    if (registers[r] == 0) {
        registers[R_COND] = FL_ZRO;
    }
    else if (registers[r] >> 15) {
        registers[R_COND] = FL_NEG;
    }
    else {
        registers[R_COND] = FL_POS;
    }
}

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void push(std::shared_ptr<char[]>& memblock, uint16_t value) {
    registers[R_SP] += 2;
    memblock[registers[R_SP] - 1] = value >> 8;
    memblock[registers[R_SP]] = value & 255;
}

void pop(std::shared_ptr<char[]>& memblock, uint16_t value) {
    registers[value] = (memblock[registers[R_SP] - 1] << 8) + memblock[registers[R_SP]];
    registers[R_SP] -= 2;
}


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "Russian");

    std::ifstream file(argv[1]);

    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        int size = file.tellg();
        file.seekg(0, std::ios::beg);

        char buf16[2] = { 0, 0 };
        file.read(buf16, sizeof(uint16_t));
        registers[R_IP] = (buf16[0] << 8) + buf16[1];
        registers[R_COND] = FL_ZRO;

        char buf32[4] = { 0, 0, 0, 0 };
        file.read(buf32, sizeof(uint32_t));
        uint32_t stack_size = (buf32[0] << 24) + (buf32[1] << 16) + (buf32[2] << 8) + buf32[3];
        size += stack_size - 4;

        std::shared_ptr<char[]> memblock(new char[size]);
        file.read((memblock.get() + stack_size), size);
        file.close();

        while(registers[R_IP] < size) {
            uint16_t instruction;
            instruction = uint16_t(memblock[registers[R_IP]] << 8) + uint8_t(memblock[registers[R_IP] + 1]);
            uint16_t op = instruction >> 12;

            switch(op) {
                case OP_BR: {
                        uint16_t condition = (instruction >> 9) & 7;

                        if (condition & registers[R_COND]) {
                            registers[R_IP] += sign_extend(instruction & 511, 9);
                        }
                    }
                    break;
                case OP_AND: {
                        uint16_t destination = (instruction >> 9) & 7;
                        uint16_t first = (instruction >> 6) & 7;
                        if ((instruction >> 5) & 1) {
                            registers[destination] = registers[first] & sign_extend(instruction & 31, 5);
                        }
                        else {
                            uint16_t second = instruction & 7;
                            registers[destination] = registers[first] & registers[second];
                        }
                        update_flags(destination);
                    }
                    break;
                case OP_NOT: {
                        uint16_t destination = (instruction >> 9) & 7;
                        uint16_t source = (instruction >> 6) & 7;

                        registers[destination] = ~registers[source];
                        update_flags(destination);
                    }
                    break;
                case OP_ADD: {
                        uint16_t destination = (instruction >> 9) & 7;
                        uint16_t first = (instruction >> 6) & 7;
                        if ((instruction >> 5) & 1) {
                            registers[destination] = registers[first] + sign_extend(instruction & 31, 5);
                        }
                        else {
                            uint16_t second = instruction & 7;
                            registers[destination] = registers[first] + registers[second];
                        }
                        update_flags(destination);
                    }
                    break;
                case OP_LD: {
                        uint16_t destination = (instruction >> 9) & 7;
                        size_t pos = stack_size;
                        if ((instruction >> 8) & 1) {
                            uint16_t source = instruction & 7;
                            pos += registers[source];
                        }
                        else {
                            pos += instruction & 255;
                        }
                        registers[destination] = (memblock[pos] << 8) + memblock[pos + 1];
                        update_flags(destination);
                    }
                    break;
                case OP_ST: {
                        uint16_t source = (instruction >> 9) & 7;
                        size_t pos = stack_size;
                        if ((instruction >> 8) & 1) {
                            uint16_t source = instruction & 7;
                            pos += registers[source];
                        }
                        else {
                            pos += instruction & 255;
                        }
                        memblock[pos] = registers[source] >> 8;
                        memblock[pos + 1] = registers[source] & 255;
                        update_flags(registers[source]);
                    }
                    break;
                case OP_PRINT: {
                        size_t pos = stack_size;
                        if ((instruction >> 11) & 1) {
                            uint16_t source = (instruction >> 8) & 7;
                            printf("%hi\n", registers[source]);
                        }
                        else {
                            pos += instruction & 2047;

                            std::string s = "";
                            while (memblock[pos] != '\0') {
                                s += memblock[pos++];
                            }
                            printf(s.c_str());
                        }
                    }
                    break;
                case OP_SCAN: {
                        size_t pos = stack_size;
                        if ((instruction >> 11) & 1) {
                            uint16_t destination = (instruction >> 8) & 7;
                            scanf_s("%hi", &registers[destination]);
                        }
                        else {
                            pos += instruction & 2047;

                            std::int16_t var = 0;
                            scanf_s("%hi", &var);

                            memblock[pos] = var >> 8;
                            memblock[pos + 1] = var & 255;
                        }
                    }
                    break;
                case OP_PUSH: {
                        uint16_t value = 0;
                        if ((instruction >> 11) & 1) {
                            value = registers[(instruction >> 8) & 7];
                        }
                        else {
                            value = sign_extend(instruction & 2047, 11);
                        }

                        push(memblock, value);
                    }
                    break;
                case OP_POP: {
                        uint16_t value = 0;
                        value = instruction & 7;

                        pop(memblock, value);
                    }
                    break;
                case OP_CALL: {
                        push(memblock, registers[R_BP]);
                        push(memblock, registers[R_IP]);

                        registers[R_BP] = registers[R_SP];
                        registers[R_IP] += sign_extend(instruction & 511, 9);
                    }
                    break;
                case OP_RET: {
                        pop(memblock, R_IP);
                        pop(memblock, R_BP);
                    }
                    break;
            }

            registers[R_IP] += 2;
        }
    }
    else std::cout << "Unable to open file\n";
}