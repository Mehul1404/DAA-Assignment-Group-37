#include <iostream>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <string>
#include <functional>
#include <set>

using namespace std;

class Graph {
private:
    int num_vertices;
    vector<vector<int>> adj_list;

public:
    Graph(int n) : num_vertices(n) {
        adj_list.resize(n);
    }

    void addEdge(int u, int v) {
        if (u >= 0 && u < num_vertices && v >= 0 && v < num_vertices) {
            auto& adj_u = adj_list[u];
            auto it = lower_bound(adj_u.begin(), adj_u.end(), v);
            if (it == adj_u.end() || *it != v) {
                adj_u.insert(it, v);
            }

            auto& adj_v = adj_list[v];
            it = lower_bound(adj_v.begin(), adj_v.end(), u);
            if (it == adj_v.end() || *it != u) {
                adj_v.insert(it, u);
            }
        }
    }

    const vector<int>& getNeighbors(int u) const {
        if (u >= 0 && u < num_vertices) {
            return adj_list[u];
        }
        static const vector<int> empty_vector;
        return empty_vector;
    }

    bool hasEdge(int u, int v) const {
        if (u >= 0 && u < num_vertices && v >= 0 && v < num_vertices) {
            const auto& neighbors = adj_list[u];
            return binary_search(neighbors.begin(), neighbors.end(), v);
        }
        return false;
    }

    int getNumVertices() const {
        return num_vertices;
    }
};

void setIntersection(const unordered_set<int>& a, const vector<int>& b, unordered_set<int>& result) {
    result.clear();
    result.reserve(min(a.size(), b.size()));
    for (int elem : b) {
        if (a.count(elem)) {
            result.insert(elem);
        }
    }
}

void setDifference(const unordered_set<int>& a, const vector<int>& b, unordered_set<int>& result) {
    result.clear();
    result.reserve(a.size());
    
    vector<bool> b_lookup(a.empty() ? 0 : *max_element(a.begin(), a.end()) + 1, false);
    for (int elem : b) {
        if (elem < b_lookup.size()) {
            b_lookup[elem] = true;
        }
    }
    
    for (int elem : a) {
        if (elem >= b_lookup.size() || !b_lookup[elem]) {
            result.insert(elem);
        }
    }
}

int selectPivot(const unordered_set<int>& SUBG, const unordered_set<int>& CAND, const Graph& G) {
    int max_size = -1;
    int pivot = -1;

    int max_vertex = 0;
    for (int v : CAND) {
        max_vertex = max(max_vertex, v);
    }
    vector<bool> cand_lookup(max_vertex + 1, false);
    for (int v : CAND) {
        cand_lookup[v] = true;
    }

    for (int u : SUBG) {
        const vector<int>& neighbors = G.getNeighbors(u);
        int intersection_size = 0;
        
        for (int v : neighbors) {
            if (v < cand_lookup.size() && cand_lookup[v]) {
                intersection_size++;
            }
        }
        
        if (intersection_size > max_size) {
            max_size = intersection_size;
            pivot = u;
        }
    }
    return pivot;
}

void EXPAND(vector<int>& Q, unordered_set<int>& SUBG, unordered_set<int>& CAND, const Graph& G, int& count, ofstream& outFile, int& max_clique_size) {
    if (SUBG.empty()) {
        count++;
        if (Q.size() > max_clique_size) {
            max_clique_size = Q.size();
        }
        outFile << "{";
        for (size_t i = 0; i < Q.size(); i++) {
            outFile << Q[i];
            if (i < Q.size() - 1) outFile << ",";
        }
        outFile << "}\n";
        return;
    }

    int u = selectPivot(SUBG, CAND, G);
    const vector<int>& u_neighbors = G.getNeighbors(u);

    unordered_set<int> ext;
    setDifference(CAND, u_neighbors, ext);

    unordered_set<int> FINI;

    while (!ext.empty()) {
        int q = *ext.begin();
        ext.erase(ext.begin());

        Q.push_back(q);

        const vector<int>& q_neighbors = G.getNeighbors(q);
        unordered_set<int> SUBG_q, CAND_q;
        setIntersection(SUBG, q_neighbors, SUBG_q);
        setIntersection(CAND, q_neighbors, CAND_q);

        EXPAND(Q, SUBG_q, CAND_q, G, count, outFile, max_clique_size);

        CAND.erase(q);
        FINI.insert(q);
        Q.pop_back();
    }
}

void CLIQUES(const Graph& G, ofstream& outFile, int& total_cliques, int& max_clique_size) {
    vector<int> Q;
    unordered_set<int> SUBG, CAND;
    
    int n = G.getNumVertices();
    Q.reserve(n);
    SUBG.reserve(n);
    CAND.reserve(n);
    
    for (int i = 0; i < G.getNumVertices(); i++) {
        SUBG.insert(i);
        CAND.insert(i);
    }

    EXPAND(Q, SUBG, CAND, G, total_cliques, outFile, max_clique_size);
}

Graph readGraphFromFile(const string& filename) {
    ifstream infile(filename);
    string line;
    set<int> vertices;
    vector<pair<int, int>> edges;



    while (getline(infile, line)) {
        istringstream iss(line);
        int from, to;
        if (iss >> from >> to) {
            vertices.insert(from);
            vertices.insert(to);
            edges.push_back({from, to});
        }
    }

    unordered_map<int, int> vertex_map;
    int index = 0;
    for (int v : vertices) {
        vertex_map[v] = index++;
    }

    Graph G(vertex_map.size());

    for (const auto& edge : edges) {
        int u = vertex_map[edge.first];
        int v = vertex_map[edge.second];
        G.addEdge(u, v);
    }

    return G;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    string filename = argv[1];
    Graph G = readGraphFromFile(filename);

    ofstream outFile("cliques.txt");
    if (!outFile) {
        cerr << "Error opening output file." << endl;
        return 1;
    }

    int total_cliques = 0;
    int max_clique_size = 0;

    auto start = chrono::high_resolution_clock::now();
    CLIQUES(G, outFile, total_cliques, max_clique_size);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    outFile << "\nLargest Clique Size: " << max_clique_size << "\n";
    outFile << "Total Maximal Cliques: " << total_cliques << "\n";
    outFile << "Execution Time: " << elapsed.count() << " seconds\n";

    outFile.close();

    cout << "Largest Clique Size: " << max_clique_size << endl;
    cout << "Total Maximal Cliques: " << total_cliques << endl;
    cout << "Execution Time: " << elapsed.count() << " seconds" << endl;

    return 0;
}