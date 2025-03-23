#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <stdexcept>
using namespace std;

using Vertex = int;
using VertexSet = unordered_set<Vertex>;
using Graph = vector<VertexSet>;

void BronKerboschPivot(Graph& graph, VertexSet P, VertexSet R, VertexSet X,
                      ofstream& outFile, unordered_map<int, int>& indexToOriginal,
                      size_t& max_size, size_t& clique_count);
vector<Vertex> ComputeDegeneracyOrdering(Graph& graph);
Graph ParseGraphFromEdgeList(const string& filePath, 
                           unordered_map<int, int>& originalToIndex,
                           unordered_map<int, int>& indexToOriginal);

void BronKerboschDegeneracy(Graph& graph, ofstream& outFile, 
                           unordered_map<int, int>& indexToOriginal,
                           size_t& max_size, size_t& clique_count) {
    if (graph.empty()) {
        throw runtime_error("Empty graph - check input file format");
    }

    const int n = graph.size();
    cout << "Starting degeneracy ordering (" << n << " nodes)...\n";
    
    auto start_order = chrono::high_resolution_clock::now();
    vector<Vertex> degeneracyOrdering = ComputeDegeneracyOrdering(graph);
    auto end_order = chrono::high_resolution_clock::now();
    cout << "Degeneracy ordering computed in "
         << chrono::duration_cast<chrono::milliseconds>(end_order - start_order).count()
         << " ms\n";

    vector<int> vertexPosition(n, 0);
    for (int i = 0; i < n; ++i)
        vertexPosition[degeneracyOrdering[i]] = i;

    cout << "Beginning clique enumeration...\n";
    auto start_clique = chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
        if (i % 1000 == 0) {
            cout << "Processed " << i << "/" << n << " nodes ("
                 << clique_count << " cliques found, "
                 << "current max size: " << max_size << ")\n";
        }

        Vertex vi = degeneracyOrdering[i];
        VertexSet P, X;

        for (Vertex neighbor : graph[vi]) {
            if (vertexPosition[neighbor] > i) P.insert(neighbor);
            if (vertexPosition[neighbor] < i) X.insert(neighbor);
        }

        VertexSet R = {vi};
        BronKerboschPivot(graph, P, R, X, outFile, indexToOriginal, max_size, clique_count);
    }
    
    auto end_clique = chrono::high_resolution_clock::now();
    cout << "Clique enumeration completed in "
         << chrono::duration_cast<chrono::milliseconds>(end_clique - start_clique).count()
         << " ms\n";
}

void BronKerboschPivot(Graph& graph, VertexSet P, VertexSet R, VertexSet X,
                      ofstream& outFile, unordered_map<int, int>& indexToOriginal,
                      size_t& max_size, size_t& clique_count) {
    if (P.empty() && X.empty()) {
        clique_count++;
        if (R.size() > max_size) max_size = R.size();

        vector<Vertex> clique(R.begin(), R.end());
        sort(clique.begin(), clique.end());
        
        outFile << "{";
        for (size_t i = 0; i < clique.size(); ++i) {
            outFile << indexToOriginal[clique[i]];
            if (i < clique.size() - 1) outFile << ", ";
        }
        outFile << "}\n";
        return;
    }

    Vertex pivotU = -1;
    size_t max_neighbors = 0;
    auto checkPivot = [&](Vertex u) {
        size_t count = 0;
        for (Vertex v : P) if (graph[u].count(v)) count++;
        if (count > max_neighbors) {
            max_neighbors = count;
            pivotU = u;
        }
    };

    for (Vertex u : P) checkPivot(u);
    for (Vertex u : X) checkPivot(u);

    VertexSet candidates;
    if (pivotU != -1) {
        for (Vertex v : P)
            if (!graph[pivotU].count(v)) 
                candidates.insert(v);
    } else {
        candidates = P;
    }

    for (Vertex v : candidates) {
        VertexSet P_new, X_new;
        for (Vertex p : P) if (graph[v].count(p)) P_new.insert(p);
        for (Vertex x : X) if (graph[v].count(x)) X_new.insert(x);
        
        R.insert(v);
        BronKerboschPivot(graph, P_new, R, X_new, outFile, indexToOriginal, max_size, clique_count);
        R.erase(v);
        
        P.erase(v);
        X.insert(v);
    }
}

vector<Vertex> ComputeDegeneracyOrdering(Graph& graph) {
    const int n = graph.size();
    vector<pair<int, int>> degrees(n);
    vector<bool> processed(n, false);

    for (int v = 0; v < n; ++v)
        degrees[v] = {v, static_cast<int>(graph[v].size())};

    vector<Vertex> ordering;
    ordering.reserve(n);

    for (int i = 0; i < n; ++i) {
        auto it = min_element(degrees.begin(), degrees.end(),
            [&](const auto& a, const auto& b) {
                return !processed[a.first] && 
                      (processed[b.first] || a.second < b.second);
            });

        if (it == degrees.end()) break;

        const Vertex u = it->first;
        ordering.push_back(u);
        processed[u] = true;

        for (Vertex v : graph[u])
            if (!processed[v])
                degrees[v].second--;
    }

    return ordering;
}

Graph ParseGraphFromEdgeList(const string& filePath, 
                           unordered_map<int, int>& originalToIndex,
                           unordered_map<int, int>& indexToOriginal) {
    ifstream file(filePath);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filePath);
    }

    unordered_set<int> uniqueVertices;
    string line;
    size_t edge_count = 0;

    cout << "Reading graph data from: " << filePath << endl;
    
    // First pass: collect vertices
    bool isHeader = true;
    while (getline(file, line)) {
        
        
        int from, to;
        if (stringstream(line) >> from >> to) {
            uniqueVertices.insert(from);
            uniqueVertices.insert(to);
            edge_count++;
        }
    }

    cout << "Found " << uniqueVertices.size() << " unique vertices\n";
    cout << "Found " << edge_count << " edges in file\n";

    // Create mappings
    vector<int> vertices(uniqueVertices.begin(), uniqueVertices.end());
    sort(vertices.begin(), vertices.end());

    int index = 0;
    for (int v : vertices) {
        originalToIndex[v] = index;
        indexToOriginal[index] = v;
        index++;
    }

    // Build undirected graph
    Graph graph(vertices.size());
    file.clear();
    file.seekg(0);
    isHeader = true;
    size_t added_edges = 0;
    
    while (getline(file, line)) {
       int from, to;
        if (stringstream(line) >> from >> to) {
            try {
                const int u = originalToIndex.at(from);
                const int v = originalToIndex.at(to);
                if (u != v) {  // Avoid self-loops
                    graph[u].insert(v);
                    graph[v].insert(u);
                    added_edges++;
                }
            } catch (const out_of_range&) {
                cerr << "Warning: Found edge with unknown vertex, skipping\n";
            }
        }
    }



    return graph;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <edge_list_file>\n";
        return 1;
    }

    try {
        auto start_total = chrono::high_resolution_clock::now();

        unordered_map<int, int> originalToIndex, indexToOriginal;
        Graph graph = ParseGraphFromEdgeList(argv[1], originalToIndex, indexToOriginal);
        
        cout << "\nOpening output file cliques.txt...\n";
        ofstream outFile("cliquesskitter.txt");
        if (!outFile) {
            throw runtime_error("Failed to create output file. Check permissions.");
        }

        size_t max_clique_size = 0;
        size_t total_cliques = 0;
        
        BronKerboschDegeneracy(graph, outFile, indexToOriginal, max_clique_size, total_cliques);
        outFile.close();

        auto end_total = chrono::high_resolution_clock::now();
        auto duration_total = chrono::duration_cast<chrono::milliseconds>(end_total - start_total);

        cout << "\n======= Final Results =======\n"
             << "Maximum clique size: " << max_clique_size << "\n"
             << "Total maximal cliques: " << total_cliques << "\n"
             << "Total execution time: " << duration_total.count() << " ms\n"
             << "Output file: cliques.txt\n";

    } catch (const exception& e) {
        cerr << "\nError: " << e.what() << endl;
        return 1;
    }

    return 0;
}