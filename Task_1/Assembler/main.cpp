#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>



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

uint16_t sign_extend(int x, int bit_count)
{
    x &= (1 << bit_count) - 1;

    if (x < 0) {
        x *= -1;
        x = 2 << bit_count - x;
    }
    return x;
}

int parse_line(std::ofstream& assembler, std::string& line, std::unordered_map<std::string, uint16_t>& variables_ptr, std::unordered_map<std::string, uint16_t>& labels, std::vector<std::pair<uint16_t, std::string>>& result, uint32_t& stack_size) {
    std::string op = line.substr(0, 2);
    int i = 2;
    uint16_t instruction = 0;

    if (op == "BR") { // BR
        instruction += OP_BR << 12;

        if (line[i] == 'p') {
            instruction += 1 << 9;
            ++i;
        }
        if (line[i] == 'z') {
            instruction += 1 << 10;
            ++i;
        }
        if (line[i] == 'n') {
            instruction += 1 << 11;
            ++i;
        }

        if (i == 2) {
            instruction += 7 << 9;
        }

        ++i;
        std::string label = line.substr(i);
        result.push_back(std::make_pair(instruction, label));
    }
    else if (op == "AN") { // AND
        instruction += OP_AND << 12;

        i += 3;
        instruction += std::stoi(line.substr(i, 1)) << 9;
        i += 4;
        instruction += std::stoi(line.substr(i, 1)) << 6;
        i += 3;
        if (line[i] == 'R') {
            instruction += std::stoi(line.substr(++i, 1));
        }
        else {
            ++i;

            instruction += 1 << 5;
            instruction += sign_extend(std::stoi(line.substr(i)), 5);
        }

        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "NO") { // NOT
        instruction += OP_NOT << 12;

        i += 3;
        instruction += std::stoi(line.substr(i, 1)) << 9;
        i += 4;
        instruction += std::stoi(line.substr(i, 1)) << 6;

        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "AD") { // ADD
        instruction += OP_ADD << 12;

        i += 3;
        instruction += std::stoi(line.substr(i, 1)) << 9;
        i += 4;
        instruction += std::stoi(line.substr(i, 1)) << 6;
        i += 3;
        if (line[i] == 'R') {
            instruction += std::stoi(line.substr(++i, 1));
        }
        else {
            ++i;

            instruction += 1 << 5;
            instruction += sign_extend(std::stoi(line.substr(i)), 5);
        }

        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "LD") { // LD
        instruction += OP_LD << 12;

        i += 2;
        instruction += std::stoi(line.substr(i, 1)) << 9;
        i += 3;
        if (line[i] == 'R') {
            instruction += 1 << 8;
            instruction += std::stoi(line.substr(++i, 1));
        }
        else {
            instruction += variables_ptr[line.substr(i)];
        }

        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "ST") { // ST
        instruction += OP_ST << 12;

        i += 2;
        instruction += std::stoi(line.substr(i, 1)) << 9;
        i += 3;
        if (line[i] == 'R') {
            instruction += 1 << 8;
            instruction += std::stoi(line.substr(++i, 1));
        }
        else {
            instruction += variables_ptr[line.substr(i)];
        }

        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "PR") { // PRINT
        instruction += OP_PRINT << 12;

        i += 4;

        if (line[i] == 'R') {
            instruction += 1 << 11;
            instruction += std::stoi(line.substr(++i, 1)) << 8;
        }
        else {
            instruction += variables_ptr[line.substr(i)];
        }

        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "SC") { // SCAN
        instruction += OP_SCAN << 12;

        i += 3;

        if (line[i] == 'R') {
            instruction += 1 << 11;
            instruction += std::stoi(line.substr(++i, 1)) << 8;
        }
        else {
            instruction += variables_ptr[line.substr(i)];
        }

        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "PU") { // PUSH
        instruction += OP_PUSH << 12;

        i += 3;

        if (line[i] == 'R') {
            instruction += 1 << 11;
            instruction += std::stoi(line.substr(++i, 1)) << 8;
        }
        else {
            instruction += std::stoi(line.substr(++i, 1));
        }

        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "PO") { // POP
        instruction += OP_POP << 12;

        i += 2;

        instruction += std::stoi(line.substr(++i, 1));
        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "CA") { // CALL
        instruction += OP_CALL << 12;

        i += 3;

        std::string label = line.substr(i);
        result.push_back(std::make_pair(instruction, label));
    }
    else if (op == "RE") { // RET
        instruction += OP_RET << 12;
        result.push_back(std::make_pair(instruction, ""));
    }
    else if (op == "in") { // int variable
        i += 2;
        int j = 0;
        while (line[i + j] != ' ') ++j;
        std::string name = line.substr(i, j);

        i += j + 3;
        uint16_t var = sign_extend(std::stoi(line.substr(i)), 16);

        variables_ptr[name] = assembler.tellp();
        variables_ptr[name] -= 6;
        char buf[2] = { var >> 8, var & 255 };
        assembler.write(buf, sizeof(uint16_t));

        return 2;
    }
    else if (op == "st") {
        if (line[i] == 'r') { // string variable
            i += 5;
            int j = 0;
            while (line[i + j] != ' ') ++j;
            std::string name = line.substr(i, j);

            i += j + 3;
            std::string var = line.substr(i);

            variables_ptr[name] = assembler.tellp();
            variables_ptr[name] -= 6;
            assembler.write(var.c_str(), (var.size() + 1) * sizeof(char));

            return var.size() + 1;
        }
        if (line[i] == 'a') { // stack size
            i += 4;
            stack_size = std::stoi(line.substr(i));

            return stack_size;
        }
    }
    else if (op != "") {
        i = 0;
        std::string name = line.substr(i);

        labels[name] = result.size() - 1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    std::ifstream source(argv[1]);
    std::ofstream assembler(argv[2]);

    char buf16[2] = {0, 0};
    char buf32[4] = { 0, 0, 0, 0 };
    assembler.write(buf16, sizeof(uint16_t));
    assembler.write(buf32, sizeof(uint32_t));

    std::unordered_map<std::string, uint16_t> variables_ptr;
    std::unordered_map<std::string, uint16_t> labels;
    std::vector<std::pair<uint16_t, std::string>> result;
    uint16_t R_IP = 0;
    uint32_t stack_size = 0;

    for (std::string line; std::getline(source, line); ) {
        R_IP += parse_line(assembler, line, variables_ptr, labels, result, stack_size);
    }

    source.close();

    for (int i = 0; i < result.size(); ++i) {
        if (result[i].second != "") {
            result[i].first += sign_extend((labels[result[i].second] - i) * 2, 9);
        }

        buf16[0] = result[i].first >> 8;
        buf16[1] = result[i].first & 255;

        assembler.write(buf16, sizeof(uint16_t));
    }

    assembler.seekp(0, std::ios::beg);

    buf16[0] = R_IP >> 8;
    buf16[1] = R_IP & 255;
    assembler.write(buf16, sizeof(uint16_t));

    buf32[0] = stack_size >> 24;
    buf32[1] = stack_size >> 16;
    buf32[2] = stack_size >> 8;
    buf32[3] = stack_size & 255;
    assembler.write(buf32, sizeof(uint32_t));

    assembler.close();
}