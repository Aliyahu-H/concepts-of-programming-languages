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
    R_6,
    R_7,
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
    OP_SCAN    /* scan variable */
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


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "Russian");

    std::ifstream file(argv[1]);

    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        int size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::shared_ptr<char[]> memblock(new char[size]);
        file.read(memblock.get(), size);
        file.close();

        registers[R_IP] = 0;
        registers[R_IP] = (memblock[0] << 8) + memblock[1];

        while(registers[R_IP] < size) {
            uint16_t instruction;
            instruction = uint16_t(memblock[registers[R_IP]] << 8) + uint8_t(memblock[registers[R_IP] + 1]);
            registers[R_IP] += 2;
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
                        size_t pos = 0;
                        if ((instruction >> 8) & 1) {
                            uint16_t source = instruction & 7;
                            pos = registers[source];
                        }
                        else {
                            pos = instruction & 255;
                        }
                        registers[destination] = (memblock[pos] << 8) + memblock[pos + 1];
                        update_flags(destination);
                    }
                    break;
                case OP_ST: {
                        uint16_t source = (instruction >> 9) & 7;
                        size_t pos = 0;
                        if ((instruction >> 8) & 1) {
                            uint16_t source = instruction & 7;
                            pos = registers[source];
                        }
                        else {
                            pos = instruction & 255;
                        }
                        memblock[pos] = registers[source] >> 8;
                        memblock[pos + 1] = registers[source] & 255;
                        update_flags(registers[source]);
                    }
                    break;
                case OP_PRINT: {
                        size_t pos = 0;
                        if ((instruction >> 11) & 1) {
                            uint16_t source = (instruction >> 8) & 7;
                            printf("%hi\n", registers[source]);
                        }
                        else {
                            pos = instruction & 2047;

                            std::string s = "";
                            while (memblock[pos] != '\0') {
                                s += memblock[pos++];
                            }
                            printf(s.c_str());
                        }
                    }
                    break;
                case OP_SCAN: {
                        size_t pos = 0;
                        if ((instruction >> 11) & 1) {
                            uint16_t destination = (instruction >> 8) & 7;
                            scanf_s("%hi", &registers[destination]);
                        }
                        else {
                            pos = instruction & 2047;

                            std::int16_t var = 0;
                            scanf_s("%hi", &var);

                            memblock[pos] = var >> 8;
                            memblock[pos + 1] = var & 255;
                        }
                    }
                    break;
            }
        }
    }
    else std::cout << "Unable to open file\n";
}