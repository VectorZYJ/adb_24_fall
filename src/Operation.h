//
// Created by Yujia Zhu on 12/4/2024.
//

#ifndef REPCREC_OPERATION_H
#define REPCREC_OPERATION_H

typedef enum {READ, WRITE} op_type;

class Operation {
public:
    int timestamp;
    op_type type;
    int t_id, x_id, value;
    Operation(int timestamp, op_type type, int t_id, int x_id, int value);
};


#endif //REPCREC_OPERATION_H
