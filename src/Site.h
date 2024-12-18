//
// Created by Yujia Zhu on 12/4/2024.
//

#include <map>
#include <vector>
using namespace std;

#ifndef REPCREC_SITE_H
#define REPCREC_SITE_H

typedef struct {
    int val, time;
} record;

class Site {
public:
    int id;
    bool is_available;
    map<int, bool> is_readable;
    map<int, vector<record> > variables;
    Site(int id);
    static record init_record(int x_id);
    int get_latest_record(int x_id, int time);
    void update_record(int x_id, int value, int time);
    void fail();
    void recover();
    void print();
};


#endif //REPCREC_SITE_H
