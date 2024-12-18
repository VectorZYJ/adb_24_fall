//
// Created by Yujia Zhu on 12/4/2024.
//

#include "TransactionManager.h"

TransactionManager::TransactionManager() {
    this->tscn_list.clear();
    this->committed_tscn.clear();
    this->site_list = vector<Site*>(11);
    this->serial_graph.vertex.clear();
    this->serial_graph.edges.clear();
    this->failure_history = vector<vector<int>>(11);

    for (int i=1; i<=10; i++)
        site_list[i] = new Site(i);
}

void TransactionManager::begin(int id, int time) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Start a transaction
    // Inputs:
    //  - id: int, transaction id
    //  - time: int, start time of transaction
    Transaction* tscn = new Transaction(id, time);
    this->tscn_list[id] = tscn;
}

void TransactionManager::read(int t_id, Operation *opt) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Perform read operation for a transaction
    // Inputs:
    //  - t_id: int, transaction id
    //  - opt: Operation*, a read operation

    // Perform read operation for a transaction
    Transaction* tscn = tscn_list[t_id];
    if (tscn->is_aborted) return;
    int x_id = opt->x_id, val = 0;

    // Check if each site containing x failed between last commit and T start
    bool all_failed = true;
    vector<int> sites_id;
    if (x_id % 2 == 1) sites_id.push_back((x_id + 1) % 10);
    else  {
        for (int i=1; i<=10; i++)
            sites_id.push_back(i);
    }
    for (int site_id : sites_id) {
        Site* site = site_list[site_id];
        int t_start_time = tscn->start_time;
        int last_commit_time = 0;
        for (int i=site->variables[x_id].size()-1; i>=0; i--)
            if (site->variables[x_id][i].time < t_start_time) {
                last_commit_time = site->variables[x_id][i].time;
                break;
            }
        bool is_failed_between_time = false;
        for (int fail_time: failure_history[site_id])
            if (fail_time > last_commit_time && fail_time < t_start_time) {
                is_failed_between_time = true;
                break;
            }
        if (! is_failed_between_time) {
            all_failed = false;
            break;
        }
    }
    // If each site containing x failed between last commit and T start, T aborts immediately
    if (all_failed) {
        abort(t_id, opt->timestamp, "each site containing x failed");
        return;
    }

    // If T write to x, read its own writes
    tscn->add_operation(opt);
    if (tscn->write_set.count(x_id) > 0)
        for (int i = tscn->writes.size() - 1; i >= 0; i--) {
            write_record rec = tscn->writes[i];
            if (rec.x_id == x_id) {
                val = rec.val;
                break;
            }
        }
    // Otherwise, read committed version of x from available sites
    else {
        int site_id = 0;
        if (x_id % 2 == 1) {
            site_id = (x_id + 1) % 10;
        }
        else {
            for (int i=1; i<=10; i++)
                if (site_list[i]->is_readable[x_id]){
                    site_id = i;
                    break;
                }
        }
        val = site_list[site_id]->get_latest_record(x_id, tscn->start_time);
    }
    tscn->read_set.insert(x_id);
    printf("x%d: %d\n", x_id, val);
}

void TransactionManager::write(int t_id, Operation *opt) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Perform write operation for a transaction
    // Inputs:
    //  - t_id: int, transaction id
    //  - opt: Operation*, a write operation

    // Perform write operation for a transaction
    Transaction* tscn = tscn_list[t_id];
    if (tscn->is_aborted) return;
    int x_id = opt->x_id, val = opt->value;

    // Write value to available sites of x locally
    tscn->add_operation(opt);
    if (x_id % 2 == 1) {
        int site_id = (x_id + 1) % 10;
        if (site_list[site_id]->is_available) {
            tscn->write_set.insert(x_id);
            write_record rec;
            rec.x_id = x_id, rec.site_id = site_id, rec.val = val, rec.time = opt->timestamp;
            tscn->writes.push_back(rec);
            printf("T%d writes locally to site %d, x%d = %d\n", tscn->id, rec.site_id, rec.x_id, rec.val);
        }
    }
    else {
        for (int i=1; i<=10; i++) {
            if (site_list[i]->is_available) {
                tscn->write_set.insert(x_id);
                write_record rec;
                rec.x_id = x_id, rec.site_id = i, rec.val = val, rec.time = opt->timestamp;
                tscn->writes.push_back(rec);
                printf("T%d writes locally to site %d, x%d = %d\n", tscn->id, rec.site_id, rec.x_id, rec.val);
            }
        }
    }
}

void TransactionManager::validate(int t_id, int timestamp) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Validate a transaction to determine whether commit or abort
    // Inputs:
    //  - t_id: int, transaction id
    //  - timestamp: int, end time of a transaction

    bool check_graph = true, check_fcw = true, check_ava_copy = true;
    // Check cycle and RW edges
    graph new_graph = update_graph(t_id);
    check_graph = find_cycle(new_graph);
    // First committer wins rule
    Transaction* tscn = tscn_list[t_id];
    for (int i : committed_tscn) {
        Transaction* t = tscn_list[i];
        for (Operation* opt : t->operation_list) {
            if (opt->type == WRITE && opt->timestamp > tscn->start_time && tscn->write_set.count(opt->x_id) > 0) {
                check_fcw = false;
                break;
            }
        }
        if (! check_fcw) break;
    }
    // Available copy rule
    for (write_record rec : tscn->writes) {
        int site_id = rec.site_id;
        int time = rec.time;
        vector<int> site_fail_history = failure_history[site_id];
        for (int i=site_fail_history.size()-1; i>=0; i--) {
            if (site_fail_history[i] > time) {
                check_ava_copy = false;
                break;
            }
        }
        if (! check_fcw) break;
    }

    if (! check_graph)
        abort(t_id, timestamp, "two consecutive RW edges in cycle");
    else if (! check_fcw)
        abort(t_id, timestamp, "first committer wins rule");
    else if (! check_ava_copy)
        abort(t_id, timestamp, "available copy rule");
    else
        commit(t_id, timestamp);
}

void TransactionManager::commit(int t_id, int timestamp) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Commit a transaction
    // Inputs:
    //  - t_id: int, transaction id
    //  - timestamp: int, commit time

    // Commit all local writes of a transaction
    printf("T%d commits\n", t_id);
    Transaction* tscn = tscn_list[t_id];
    for (int i=0; i<tscn->writes.size(); i++) {
        write_record rec = tscn->writes[i];
        int site_id = rec.site_id, x_id = rec.x_id, value = rec.val;
        Site* site = site_list[site_id];
        if (site->is_available) {
            site->update_record(x_id, value, timestamp);
            if (! site->is_readable[x_id])
                site->is_readable[x_id] = true;
        }
    }
    tscn->end_time = timestamp;
    tscn->is_committed = true;

    // Update serial graph using committed transaction
    serial_graph = update_graph(t_id);
    committed_tscn.push_back(t_id);
}

void TransactionManager::abort(int t_id, int timestamp, const string& message) {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Abort a transaction
    // Inputs:
    //  - t_id: int, transaction id
    //  - timestamp: int, abort time
    //  - message: string, reason for abortion

    // Abort a transaction
    printf("T%d aborts, %s\n", t_id, message.c_str());
    Transaction* tscn = tscn_list[t_id];
    tscn->end_time = timestamp;
    tscn->is_committed = false;
    tscn->is_aborted = true;
}

void TransactionManager::fail(int site_id, int timestamp) {
    // Author: Yujia Zhu
    // Date: 12/5/2024
    // Description: Cause a site to fail
    // Inputs:
    //  - site_id: int, site id
    //  - timestamp: int, site fail time

    // Site fails
    Site* site = site_list[site_id];
    site->fail();
    failure_history[site_id].push_back(timestamp);
}

void TransactionManager::recover(int site_id) {
    // Author: Yujia Zhu
    // Date: 12/5/2024
    // Description: Recover a site
    // Inputs:
    //  - site_id: int, site id
    //  - timestamp: int, site recover time

    // Site recovers
    Site* site = site_list[site_id];
    site->recover();
}

void TransactionManager::dump() {
    // Author: Yujia Zhu
    // Date: 12/4/2024
    // Description: Print out all variable information at all sites

    // Print variables at all sites
    for (int i=1; i<=10; i++)
        site_list[i]->print();
}

graph TransactionManager::update_graph(int t_id) {
    // Author: Yujia Zhu
    // Date: 12/5/2024
    // Description: Generate a new serialization graph using newly added transaction
    // Input:
    //  - t_id: int, id for transaction to be added for new graph
    // Output:
    //  - graph: New serialization graph after adding transaction

    // Create a new graph using newly added transaction
    graph new_graph = serial_graph;
    Transaction* tscn = tscn_list[t_id];
    new_graph.vertex.insert(tscn->id);

    for (int id : committed_tscn) {
        Transaction* t = tscn_list[id];
        // T -> T`
        edge e;
        e.to = tscn->id;
        // For WW edge
        if (t->end_time < tscn->start_time) {
            for (int x: tscn->write_set) {
                if (t->write_set.count(x) > 0) {
                    e.type = WW;
                    new_graph.edges[t->id].push_back(e);
                    break;
                }
            }
        }
        // For WR edge
        if (t->end_time < tscn->start_time) {
            for (int x: tscn->read_set) {
                if (t->write_set.count(x) > 0) {
                    e.type = WR;
                    new_graph.edges[t->id].push_back(e);
                    break;
                }
            }
        }
        // For RW edge
        if (t->start_time < tscn->end_time) {
            for (int x: tscn->write_set) {
                if (t->read_set.count(x) > 0) {
                    e.type = RW;
                    new_graph.edges[t->id].push_back(e);
                    break;
                }
            }
        }
        // T` -> T
        e.to = t->id;
        // For WW edge
        if (tscn->end_time < t->start_time) {
            for (int x: t->write_set) {
                if (tscn->write_set.count(x) > 0) {
                    e.type = WW;
                    new_graph.edges[tscn->id].push_back(e);
                    break;
                }
            }
        }
        // For WR edge
        if (tscn->end_time < t->start_time) {
            for (int x: t->read_set) {
                if (tscn->write_set.count(x) > 0) {
                    e.type = WR;
                    new_graph.edges[tscn->id].push_back(e);
                    break;
                }
            }
        }
        // For RW edge
        if (tscn->start_time < t->end_time) {
            for (int x: t->write_set) {
                if (tscn->read_set.count(x) > 0) {
                    e.type = RW;
                    new_graph.edges[tscn->id].push_back(e);
                    break;
                }
            }
        }
    }
    return new_graph;
}

void dfs(int depth, int u, vector<edge_type> &edges, bool &result, graph &g, map<int, int> &visited) {
    // Author: Yujia Zhu
    // Date: 12/5/2024
    // Description: Use DFS to find cycle in graph
    // Input:
    //  - depth: int, recurrence depth
    //  - u: int, current vertex
    //  - edges: edge information
    //  - result: bool, if this graph contains a cycle with two consecutive RW edges
    //  - g: graph, the serialization graph to be checked
    //  - visited: visit information for each vertex

    visited[u] = 1;
    for (edge e: g.edges[u]) {
        int v = e.to;
        if (visited[v] == 0) {
            edges[depth] = e.type;
            dfs(depth+1, v, edges, result, g, visited);
        }
        // Find a cycle
        if (visited[v] == 1) {
            edges[depth] = e.type;
            bool rw_check = true;
            // Find consecutive RW edges
            for (int i=0; i<depth; i++) {
                if (edges[i] == RW && edges[i+1] == RW) {
                    rw_check = false;
                    break;
                }
            }
            if (edges[depth] == RW && edges[0] == RW)
                rw_check = false;
            if (!rw_check)
                result = false;
        }
    }
    visited[u] = 2;
}

bool TransactionManager::find_cycle(graph g) {
    // Author: Yujia Zhu
    // Date: 12/5/2024
    // Description: Check whether the graph g contains a cycle with two consecutive RW edges
    // Input:
    //  - g: graph, the serialization graph to be checked
    // Output:
    //  - bool: If the graph g contains a cycle with two consecutive RW edges

    // Find cycle and RW edges in graph
    map<int, int> visited;
    bool result = true;
    int n = g.vertex.size();
    for (int vertex: g.vertex)
        visited[vertex] = 0;
    for (int vertex: g.vertex)
        if (visited[vertex] == 0) {
            vector<edge_type> edges(n * n);
            dfs(0, vertex, edges, result, g, visited);
            if (! result) break;
        }
    return result;
}