//
// Created by Yujia Zhu on 12/4/2024.
//
#include <vector>
#include <set>
#include "Operation.h"
#include "Site.h"
using namespace std;

#ifndef REPCREC_TRANSACTION_H
#define REPCREC_TRANSACTION_H

typedef struct {
    int site_id, x_id, val, time;
} write_record;

class Transaction {
public:
    int id;
    int start_time;
    int end_time;
    bool waiting;
    bool is_committed;
    bool is_aborted;
    vector<Operation*> operation_list;
    vector<write_record> writes;
    set<int> read_set, write_set;

    Transaction(int id, int timestamp);
    void add_operation(Operation* opt);
};


#endif //REPCREC_TRANSACTION_H
