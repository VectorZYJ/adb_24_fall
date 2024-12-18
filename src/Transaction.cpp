//
// Created by Yujia Zhu on 12/4/2024.
//

#include <climits>
#include "Transaction.h"

Transaction::Transaction(int id, int timestamp) {
    this->id = id;
    this->start_time = timestamp;
    this->end_time = INT_MAX;
    this->is_committed = false;
    this->is_aborted = false;
    this->waiting = false;
    this->operation_list.clear();
    this->write_set.clear();
    this->read_set.clear();
    this->writes.clear();
}

void Transaction::add_operation(Operation *opt) {
    this->operation_list.push_back(opt);
}