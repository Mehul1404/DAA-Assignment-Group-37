#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <stack>
#include <set>
#include <iomanip>
using namespace std;
using namespace std::chrono;

struct ExecutionFrame {
    int vertexIndex;
    vector<int> activeClique;
    vector<int> preservedCliqueSection;
    int executionPhase;
    bool isStateRestoration;
};

vector<int> orderedVertices;
unordered_map<int, int> vertexLabelMapping;
vector<vector<int>> adjacencyList;
vector<int> maximalitySupport, adjacencyTemporary;
int totalVertices;
vector<size_t> cliqueSizeDistribution;
size_t totalMaximalCliques = 0;

time_point<high_resolution_clock> algorithmStartTime;
size_t lastCliqueReportCount = 0;
size_t processedFramesTotal = 0;

vector<int> computeIntersection(const vector<int>& firstSet, const vector<int>& secondSet) {
    vector<int> intersectionResult;
    int firstIdx = 0, secondIdx = 0;
    while (firstIdx < firstSet.size() && secondIdx < secondSet.size()) {
        if (firstSet[firstIdx] == secondSet[secondIdx]) {
            intersectionResult.push_back(firstSet[firstIdx]);
            firstIdx++;
            secondIdx++;
        } else if (firstSet[firstIdx] < secondSet[secondIdx]) {
            firstIdx++;
        } else {
            secondIdx++;
        }
    }
    return intersectionResult;
}

vector<int> computeSetDifference(const vector<int>& primarySet, const vector<int>& secondarySet) {
    vector<int> differenceResult;
    int primaryIdx = 0, secondaryIdx = 0;
    while (primaryIdx < primarySet.size() && secondaryIdx < secondarySet.size()) {
        if (primarySet[primaryIdx] == secondarySet[secondaryIdx]) {
            primaryIdx++;
            secondaryIdx++;
        } else if (primarySet[primaryIdx] < secondarySet[secondaryIdx]) {
            differenceResult.push_back(primarySet[primaryIdx]);
            primaryIdx++;
        } else {
            secondaryIdx++;
        }
    }
    while (primaryIdx < primarySet.size()) {
        differenceResult.push_back(primarySet[primaryIdx]);
        primaryIdx++;
    }
    return differenceResult;
}

void registerClique(const vector<int>& maximalClique) {
    if (maximalClique.empty()) return;
    size_t cliqueSize = maximalClique.size();
    if (cliqueSize >= cliqueSizeDistribution.size()) {
        cliqueSizeDistribution.resize(cliqueSize + 1, 0);
    }
    cliqueSizeDistribution[cliqueSize]++;
    totalMaximalCliques++;
}

void updateProgressReport(const vector<int>& currentClique, int currentVertex, size_t stackSize) {
    auto currentTime = high_resolution_clock::now();
    auto elapsedSeconds = duration_cast<seconds>(currentTime - algorithmStartTime).count();
    size_t minutes = elapsedSeconds / 60;
    size_t seconds = elapsedSeconds % 60;

    cout << "\n[Progress] Time: " << minutes << "m " << seconds << "s"
         << " | Cliques: " << totalMaximalCliques
         << endl;

    if (totalMaximalCliques - lastCliqueReportCount >= 1000) {
        lastCliqueReportCount = totalMaximalCliques;
    }
}

void initializeGraphData(const string& filename) {
    ifstream inputFile(filename);
    vector<pair<int, int>> edgeList;
    unordered_map<int, int> vertexDegrees;

    int firstVertex, secondVertex;
    while (inputFile >> firstVertex >> secondVertex) {
        edgeList.emplace_back(firstVertex, secondVertex);
        vertexDegrees[firstVertex]++;
        vertexDegrees[secondVertex]++;
    }

    vector<int> originalVertexOrder;
    for (auto& vertexDegree : vertexDegrees) {
        originalVertexOrder.push_back(vertexDegree.first);
    }
    sort(originalVertexOrder.begin(), originalVertexOrder.end(), 
        [&vertexDegrees](int a, int b) {
            return vertexDegrees[a] != vertexDegrees[b] ? vertexDegrees[a] < vertexDegrees[b] : a < b;
        });

    totalVertices = originalVertexOrder.size();
    vertexLabelMapping.reserve(totalVertices);
    for (int idx = 0; idx < totalVertices; ++idx) {
        vertexLabelMapping[originalVertexOrder[idx]] = idx + 1;
    }

    adjacencyList.resize(totalVertices + 1);
    for (auto& edge : edgeList) {
        int relabeledFirst = vertexLabelMapping[edge.first];
        int relabeledSecond = vertexLabelMapping[edge.second];
        adjacencyList[relabeledFirst].push_back(relabeledSecond);
        adjacencyList[relabeledSecond].push_back(relabeledFirst);
    }

    for (int vertex = 1; vertex <= totalVertices; ++vertex) {
        sort(adjacencyList[vertex].begin(), adjacencyList[vertex].end());
    }
}

void prepareAlgorithmExecution() {
    maximalitySupport.resize(totalVertices + 1, 0);
    adjacencyTemporary.resize(totalVertices + 1, 0);
    stack<ExecutionFrame> frameStack;
    frameStack.push({2, {1}, {}, 0, false});

    algorithmStartTime = high_resolution_clock::now();
    auto lastReportTime = algorithmStartTime;

    while (!frameStack.empty()) {
        processedFramesTotal++;
        ExecutionFrame currentFrame = frameStack.top();
        frameStack.pop();

        auto currentTime = high_resolution_clock::now();
        if (duration_cast<seconds>(currentTime - lastReportTime).count() >= 30) {
            updateProgressReport(currentFrame.activeClique, currentFrame.vertexIndex, frameStack.size());
            lastReportTime = currentTime;
        }

        if (currentFrame.isStateRestoration) {
            vector<int> updatedClique = computeSetDifference(currentFrame.activeClique, {currentFrame.vertexIndex});
            vector<int> mergedClique;
            merge(updatedClique.begin(), updatedClique.end(),
                  currentFrame.preservedCliqueSection.begin(), currentFrame.preservedCliqueSection.end(),
                  back_inserter(mergedClique));
            mergedClique.erase(unique(mergedClique.begin(), mergedClique.end()), mergedClique.end());
            sort(mergedClique.begin(), mergedClique.end());
            if (!frameStack.empty() && !frameStack.top().isStateRestoration) {
                frameStack.top().activeClique = mergedClique;
            }
            continue;
        }

        int currentVertex = currentFrame.vertexIndex;
        vector<int>& currentClique = currentFrame.activeClique;
        int phase = currentFrame.executionPhase;

        if (currentVertex > totalVertices) {
            registerClique(currentClique);
            if (totalMaximalCliques % 1000 == 0) {
                updateProgressReport(currentClique, currentVertex, frameStack.size());
            }
            continue;
        }

        if (phase == 0) {
            vector<int> neighbors = adjacencyList[currentVertex];
            vector<int> cliqueDifference = computeSetDifference(currentClique, neighbors);
            if (!cliqueDifference.empty()) {
                frameStack.push({currentVertex, currentClique, {}, 1, false});
                frameStack.push({currentVertex + 1, currentClique, {}, 0, false});
                continue;
            }
            phase = 1;
        }

        if (phase == 1) {
            vector<int> neighbors = adjacencyList[currentVertex];
            vector<int> cliqueIntersection = computeIntersection(currentClique, neighbors);
            int intersectionSize = cliqueIntersection.size();

            fill(maximalitySupport.begin(), maximalitySupport.end(), 0);
            fill(adjacencyTemporary.begin(), adjacencyTemporary.end(), 0);

            for (int vertex : cliqueIntersection) {
                for (int adjacent : adjacencyList[vertex]) {
                    if (adjacent == currentVertex) continue;
                    if (!binary_search(currentClique.begin(), currentClique.end(), adjacent)) {
                        adjacencyTemporary[adjacent]++;
                    }
                }
            }

            vector<int> differenceSet = computeSetDifference(currentClique, neighbors);
            for (int vertex : differenceSet) {
                for (int adjacent : adjacencyList[vertex]) {
                    if (!binary_search(currentClique.begin(), currentClique.end(), adjacent)) {
                        maximalitySupport[adjacent]++;
                    }
                }
            }

            bool isMaximal = true;
            for (int adjacent : neighbors) {
                if (adjacent >= currentVertex) continue;
                if (binary_search(currentClique.begin(), currentClique.end(), adjacent)) continue;
                if (adjacencyTemporary[adjacent] == intersectionSize) {
                    isMaximal = false;
                    break;
                }
            }

            if (isMaximal) {
                vector<int> sortedVertices = differenceSet;
                sort(sortedVertices.begin(), sortedVertices.end());
                int setSize = sortedVertices.size();

                for (int position = 0; position < sortedVertices.size(); ++position) {
                    int current = sortedVertices[position];
                    for (int adjacent : adjacencyList[current]) {
                        if (adjacent >= currentVertex) continue;
                        if (binary_search(currentClique.begin(), currentClique.end(), adjacent)) continue;
                        if (adjacencyTemporary[adjacent] == intersectionSize) {
                            if (adjacent >= current) {
                                maximalitySupport[adjacent]--;
                            } else {
                                bool isFirstCandidate = true;
                                for (int prevPos = 0; prevPos < position; ++prevPos) {
                                    if (sortedVertices[prevPos] > adjacent) {
                                        isFirstCandidate = false;
                                        break;
                                    }
                                }
                                if (isFirstCandidate) {
                                    if (maximalitySupport[adjacent] + position == setSize && 
                                        adjacent >= (position == 0 ? 0 : sortedVertices[position - 1])) {
                                        isMaximal = false;
                                    }
                                }
                            }
                        }
                    }
                }

                if (!cliqueIntersection.empty()) {
                    for (int vertex : cliqueIntersection) {
                        for (int adjacent : adjacencyList[vertex]) {
                            if (adjacent >= currentVertex) continue;
                            if (binary_search(currentClique.begin(), currentClique.end(), adjacent)) continue;
                            if (adjacencyTemporary[adjacent] == intersectionSize && maximalitySupport[adjacent] == 0) {
                                if (sortedVertices.empty() || sortedVertices.back() < adjacent) {
                                    isMaximal = false;
                                }
                            }
                        }
                    }
                } else {
                    if (!sortedVertices.empty() && sortedVertices.back() < currentVertex - 1) {
                        isMaximal = false;
                    }
                }
            }

            for (int vertex : cliqueIntersection) {
                for (int adjacent : adjacencyList[vertex]) {
                    if (adjacent == currentVertex) continue;
                    adjacencyTemporary[adjacent] = 0;
                }
            }

            for (int vertex : differenceSet) {
                for (int adjacent : adjacencyList[vertex]) {
                    maximalitySupport[adjacent] = 0;
                }
            }

            if (isMaximal) {
                vector<int> newClique = computeIntersection(currentClique, adjacencyList[currentVertex]);
                newClique.push_back(currentVertex);
                sort(newClique.begin(), newClique.end());
                newClique.erase(unique(newClique.begin(), newClique.end()), newClique.end());

                vector<int> preservedSection = computeSetDifference(currentClique, adjacencyList[currentVertex]);
                frameStack.push({currentVertex, currentClique, preservedSection, 0, true});
                frameStack.push({currentVertex + 1, newClique, {}, 0, false});
            }
        }
    }

    auto endTime = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(endTime - algorithmStartTime).count();

    cout << "\n\nFinal Results:\n";
    cout << "Total maximal cliques: " << totalMaximalCliques << endl;
    for (size_t size = 1; size < cliqueSizeDistribution.size(); ++size) {
        if (cliqueSizeDistribution[size] > 0) {
            cout << "Size " << size << ": " << cliqueSizeDistribution[size] << endl;
        }
    }
    cout << "Execution Time: " << duration << " ms" << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    initializeGraphData(argv[1]);
    prepareAlgorithmExecution();

    return 0;
}