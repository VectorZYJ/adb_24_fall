//
// Created by Yujia Zhu on 12/4/2024.
//

#include "Operation.h"

Operation::Operation(int timestamp, op_type type, int t_id, int x_id, int value) {
    this->timestamp = timestamp;
    this->type = type;
    this->t_id = t_id;
    this->x_id = x_id;
    this->value = value;
}