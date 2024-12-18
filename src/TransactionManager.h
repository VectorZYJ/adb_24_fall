//
// Created by Yujia Zhu on 12/4/2024.
//

#include <map>
#include <vector>
#include <string>
#include <deque>
#include "Transaction.h"
#include "Site.h"

#ifndef REPCREC_TRANSACTIONMANAGER_H
#define REPCREC_TRANSACTIONMANAGER_H

typedef enum {RW, WR, WW} edge_type;

typedef struct {
    int to;
    edge_type type;
} edge;

typedef struct {
    set<int> vertex;
    map<int, vector<edge> > edges;
} graph;

class TransactionManager {
public:
    map<int, Transaction*> tscn_list;
    vector<int> committed_tscn;
    deque<Operation*> wait_list;
    vector<Site*> site_list;
    vector<vector<int> > failure_history;
    graph serial_graph;
    TransactionManager();
    void begin(int id, int time);
    void read(int t_id, Operation* opt);
    void write(int t_id, Operation* opt);
    void validate(int t_id, int timestamp);
    void commit(int t_id, int timestamp);
    void abort(int t_id, int timestamp, const string& message);
    void fail(int site_id, int timestamp);
    void recover(int site_id);
    void dump();
    graph update_graph(int t_id);
    static bool find_cycle(graph g);
};


#endif //REPCREC_TRANSACTIONMANAGER_H
