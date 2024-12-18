//
// Created by Yujia Zhu on 12/4/2024.
//

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>

#include "Site.h"
#include "Operation.h"
#include "Transaction.h"
#include "TransactionManager.h"
using namespace std;

int TIMESTAMP = 0;
bool interactive_mode = false;
string filename, line;
ifstream file;
Operation* opt;
TransactionManager* TM;


void read_input();
void execution(string op);

int main(int argc, char* argv[]) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Driver of this RepCRec project

    if (argc == 2) {
        filename = argv[1];
        printf("Read input file %s\n", filename.c_str());
    }
    else if (argc == 1) {
        interactive_mode = true;
        printf("No input files provided, using interactive mode, type 'E' to exit.\n");
    }
    else {
        printf("Invalid params.\n");
        printf("Usage: './RepCRec <filename>' to read input file\n");
        printf("or './RepCRec' to start interactive mode\n");
        return 0;
    }
    TM = new TransactionManager();
    read_input();
    return 0;
}

void execution(string op) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Execute an operation
    // Inputs:
    //  - op: string, the operation

    TIMESTAMP = TIMESTAMP + 1;
    int t_id, x_id, site_id, value;
    switch (op[0]) {
        case 'b':
            sscanf(op.c_str(), "begin(T%d)", &t_id);
            TM->begin(t_id, TIMESTAMP);
            break;
        case 'R':
            sscanf(op.c_str(), "R(T%d,x%d)", &t_id, &x_id);
            opt = new Operation(TIMESTAMP, READ, t_id, x_id, -1);
            TM->read(t_id, opt);
            break;
        case 'W':
            sscanf(op.c_str(), "W(T%d,x%d,%d)", &t_id, &x_id, &value);
            opt = new Operation(TIMESTAMP, WRITE, t_id, x_id, value);
            TM->write(t_id, opt);
            break;
        case 'd':
            TM->dump();
            break;
        case 'e':
            sscanf(op.c_str(), "end(T%d)", &t_id);
            TM->validate(t_id, TIMESTAMP);
            break;
        case 'f':
            sscanf(op.c_str(), "fail(%d)", &site_id);
            TM->fail(site_id, TIMESTAMP);
            break;
        case 'r':
            sscanf(op.c_str(), "recover(%d)", &site_id);
            TM->recover(site_id);
            break;
        default:
            printf("Invalid operation, please retry!\n");
    }
}


void read_input() {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Read input from file or standard input

    if (interactive_mode) {
        while (true) {
            printf(">> ");
            getline(cin, line);
            int pos = line.find(' ');
            while (pos >= 0) {
                line.erase(pos, 1);
                pos = line.find(' ');
            }
            if (line[0] == 'E') {
                printf("End!\n");
                break;
            }
            execution(line);
        }
    }
    else {
        file.open(filename.c_str());
        while (getline(file, line)) {
            int pos = line.find(' ');
            while (pos >= 0) {
                line.erase(pos, 1);
                pos = line.find(' ');
            }
            printf(">> %s\n", line.c_str());
            execution(line);
        }
    }
}
