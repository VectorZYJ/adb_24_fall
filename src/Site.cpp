//
// Created by Yujia Zhu on 12/4/2024.
//

#include <cstdio>
#include "Site.h"

Site::Site(int id) {
    this->id = id;
    this->is_available = true;
    if (id % 2 == 0) {
        this->is_readable[id - 1] = true;
        this->is_readable[id + 9] = true;
        this->variables[id - 1].push_back(Site::init_record(id - 1));
        this->variables[id + 9].push_back(Site::init_record(id + 9));
    }
    for (int i=1; i<=10; i++) {
        this->is_readable[i * 2] = true;
        this->variables[i * 2].push_back(Site::init_record(i * 2));
    }

}

record Site::init_record(int x_id) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Initialize a variable

    record rec;
    rec.val = x_id * 10;
    rec.time = 0;
    return rec;
}

void Site::print() {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Print out all variables in a site
    // Output: site <id> - x<id>: <value> ......

    printf("site %d - ", this->id);
    bool flag = true;
    for (int i=1; i<=20; i++) {
        if (this->variables.count(i) > 0) {
            if (flag) {
                printf("x%d: %d", i, this->variables[i].back().val);
                flag = false;
            }
            else printf(", x%d: %d", i, this->variables[i].back().val);
        }
    }
    printf("\n");
}

int Site::get_latest_record(int x_id, int time) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Get last committed record before a certain time
    // Input:
    //  - x_id: int, variable id
    //  - time: int, time as specified above

    vector<record> variable = this->variables[x_id];
    for (int i=variable.size()-1; i>=0; i--) {
        record rec = variable[i];
        if (rec.time < time)
            return rec.val;
    }
    return -1;
}

void Site::update_record(int x_id, int value, int time) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Update a record
    // Input:
    //  - x_id: int, variable id
    //  - value: int, value of variable
    //  - time: int, update time

    record rec;
    rec.val = value, rec.time = time;
    variables[x_id].push_back(rec);
}

void Site::fail() {
    // Author: Yujia Zhu
    // Date: 12/5/2024
    // Description: Site fails

    is_available = false;
    for (int i=1; i<=20; i++)
        if (variables.count(i) > 0)
            is_readable[i] = false;
}

void Site::recover() {
    // Author: Yujia Zhu
    // Date: 12/5/2024
    // Description: Site recovers

    is_available = true;
    if (id % 2 == 0) {
        is_readable[id - 1] = true;
        is_readable[id + 9] = true;
    }
}