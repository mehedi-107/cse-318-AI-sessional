#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <set>

// =================================================================================
// Data Structures
// =================================================================================

using DataRow = std::vector<std::string>;
using DataTable = std::vector<DataRow>;

struct Node {
    bool isLeaf = false;
    std::string prediction;
    int attributeIndex = -1;
    std::string attributeName;
    bool isNumericSplit = false;
    double splitValue = 0.0;
    std::map<std::string, Node*> children;

    ~Node() {
        for (auto& pair : children) {
            delete pair.second;
        }
    }
};

// =================================================================================
// Utility Functions
// =================================================================================

// Splits a string by a delimiter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        // Trim leading/trailing whitespace
        size_t first = token.find_first_not_of(" \t\r\n");
        if (std::string::npos == first) {
            tokens.push_back("");
            continue;
        }
        size_t last = token.find_last_not_of(" \t\r\n");
        tokens.push_back(token.substr(first, (last - first + 1)));
    }
    return tokens;
}

// Loads data from a CSV file
DataTable loadData(const std::string& filepath, std::vector<std::string>& headers) {
    DataTable data;
    std::ifstream file(filepath);
    std::string line;
    
    // Create generic headers if none exist
    if (file.good()) {
        std::getline(file, line);
        // Assuming first line might be a header or data
        // For simplicity, we'll create generic headers
        auto first_row = split(line, ',');
        for(size_t i = 0; i < first_row.size() - 1; ++i) {
            headers.push_back("attr_" + std::to_string(i));
        }
        headers.push_back("label");
        
        // Reset file to beginning
        file.clear();
        file.seekg(0, std::ios::beg);
    }

    while (std::getline(file, line)) {
        if (!line.empty()) {
            data.push_back(split(line, ','));
        }
    }
    return data;
}

// Get class label counts
std::map<std::string, int> getClassCounts(const DataTable& data) {
    std::map<std::string, int> counts;
    if (data.empty()) return counts;
    int labelIndex = data[0].size() - 1;
    for (const auto& row : data) {
        counts[row[labelIndex]]++;
    }
    return counts;
}

// Calculate entropy of a dataset
double calculateEntropy(const DataTable& data) {
    if (data.empty()) return 0.0;
    std::map<std::string, int> counts = getClassCounts(data);
    double entropy = 0.0;
    double total_size = data.size();
    for (const auto& pair : counts) {
        double p = pair.second / total_size;
        if (p > 0) {
            entropy -= p * log2(p);
        }
    }
    return entropy;
}

bool isNumeric(const std::string& s) {
    if (s.empty()) return false;
    char* end = nullptr;
    strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0';
}


// =================================================================================
// Decision Tree Class
// =================================================================================

class DecisionTree {
public:
    DecisionTree(std::string criterion, int maxDepth)
        : criterion(criterion), maxDepth(maxDepth), root(nullptr) {}

    ~DecisionTree() {
        delete root;
    }

    void train(const DataTable& trainingData, const std::vector<std::string>& headers) {
        this->headers = headers;
        this->numeric_columns.clear();
        if (!trainingData.empty()) {
            for (size_t i = 0; i < trainingData[0].size() - 1; ++i) {
                if (isNumeric(trainingData[0][i])) {
                    this->numeric_columns.insert(i);
                }
            }
        }
        
        std::vector<int> attributeIndices;
        for (size_t i = 0; i < headers.size() - 1; ++i) {
            attributeIndices.push_back(i);
        }
        root = buildTree(trainingData, attributeIndices, 0);
    }

    std::string predict(const DataRow& row) {
        return predictRecursive(row, root);
    }

    double evaluate(const DataTable& testData) {
        if (testData.empty()) return 0.0;
        int correct = 0;
        int labelIndex = testData[0].size() - 1;
        for (const auto& row : testData) {
            if (predict(row) == row[labelIndex]) {
                correct++;
            }
        }
        return static_cast<double>(correct) / testData.size();
    }

private:
    std::string criterion;
    int maxDepth;
    Node* root;
    std::vector<std::string> headers;
    std::set<int> numeric_columns;

    std::string getMajorityClass(const DataTable& data) {
        auto counts = getClassCounts(data);
        if (counts.empty()) return "";
        return std::max_element(counts.begin(), counts.end(), 
            [](const auto& a, const auto& b) { return a.second < b.second; })->first;
    }

    Node* buildTree(const DataTable& data, std::vector<int> attributeIndices, int depth) {
        // --- Base Cases for stopping recursion ---
        auto counts = getClassCounts(data);
        if (counts.size() <= 1) { // Pure node
            Node* leaf = new Node();
            leaf->isLeaf = true;
            leaf->prediction = data.empty() ? "" : data[0].back();
            return leaf;
        }
        if (attributeIndices.empty()) { // No more attributes
            Node* leaf = new Node();
            leaf->isLeaf = true;
            leaf->prediction = getMajorityClass(data);
            return leaf;
        }
        if (maxDepth > 0 && depth >= maxDepth) { // Max depth reached
            Node* leaf = new Node();
            leaf->isLeaf = true;
            leaf->prediction = getMajorityClass(data);
            return leaf;
        }
        
        // --- Find the best attribute to split on ---
        double bestGain = -1.0;
        int bestAttrIndex = -1;
        double bestSplitValue = 0.0;
        
        double parentEntropy = calculateEntropy(data);

        for (int attrIndex : attributeIndices) {
            std::set<std::string> uniqueValues;
            for (const auto& row : data) {
                uniqueValues.insert(row[attrIndex]);
            }
            
            bool is_numeric = numeric_columns.count(attrIndex);

            if (is_numeric) {
                 // Numeric attribute: find best threshold
                std::vector<double> sortedValues;
                for(const auto& val_str : uniqueValues) sortedValues.push_back(stod(val_str));
                std::sort(sortedValues.begin(), sortedValues.end());
                
                for (size_t i = 0; i < sortedValues.size() - 1; ++i) {
                    double threshold = (sortedValues[i] + sortedValues[i+1]) / 2.0;
                    DataTable left, right;
                    for (const auto& row : data) {
                        if (stod(row[attrIndex]) <= threshold) left.push_back(row);
                        else right.push_back(row);
                    }
                    if (left.empty() || right.empty()) continue;

                    double p_left = (double)left.size() / data.size();
                    double p_right = (double)right.size() / data.size();
                    double conditionalEntropy = p_left * calculateEntropy(left) + p_right * calculateEntropy(right);
                    double currentGain = parentEntropy - conditionalEntropy;

                    if (criterion == "IGR" || criterion == "NWIG") {
                        double splitInfo = - (p_left * log2(p_left) + p_right * log2(p_right));
                         if (criterion == "IGR") {
                            if (splitInfo > 1e-9) currentGain /= splitInfo; else currentGain = 0;
                        } else { // NWIG
                           currentGain = (currentGain / log2(2.0 + 1.0)) * (1.0 - (2.0-1.0)/data.size());
                        }
                    }

                    if (currentGain > bestGain) {
                        bestGain = currentGain;
                        bestAttrIndex = attrIndex;
                        bestSplitValue = threshold;
                    }
                }

            } else {
                // Categorical attribute
                double conditionalEntropy = 0.0;
                double splitInfo = 0.0;
                
                for (const auto& value : uniqueValues) {
                    DataTable subset;
                    for (const auto& row : data) {
                        if (row[attrIndex] == value) {
                            subset.push_back(row);
                        }
                    }
                    double p = (double)subset.size() / data.size();
                    if (p > 0) {
                        conditionalEntropy += p * calculateEntropy(subset);
                        splitInfo -= p * log2(p);
                    }
                }
                double currentGain = parentEntropy - conditionalEntropy;
                
                if (criterion == "IGR") {
                    if (splitInfo > 1e-9) currentGain /= splitInfo; else currentGain = 0;
                } else if (criterion == "NWIG") {
                    double k = uniqueValues.size();
                    if (k > 1) {
                         currentGain = (currentGain / log2(k + 1.0)) * (1.0 - (k-1.0)/data.size());
                    } else {
                        currentGain = 0;
                    }
                }

                if (currentGain > bestGain) {
                    bestGain = currentGain;
                    bestAttrIndex = attrIndex;
                }
            }
        }

        if (bestGain <= 1e-9) { // If no split improves information, create leaf
            Node* leaf = new Node();
            leaf->isLeaf = true;
            leaf->prediction = getMajorityClass(data);
            return leaf;
        }
        
        // --- Create a split node ---
        Node* node = new Node();
        node->attributeIndex = bestAttrIndex;
        node->attributeName = headers[bestAttrIndex];
        node->isNumericSplit = numeric_columns.count(bestAttrIndex);
        node->splitValue = bestSplitValue;
        
        std::vector<int> nextAttributeIndices;
        for (int idx : attributeIndices) {
            if (idx != bestAttrIndex) {
                nextAttributeIndices.push_back(idx);
            }
        }

        // --- Partition data and recurse ---
        if (node->isNumericSplit) {
            DataTable left, right;
            for (const auto& row : data) {
                if (stod(row[bestAttrIndex]) <= node->splitValue) left.push_back(row);
                else right.push_back(row);
            }
            node->children["<="] = buildTree(left, nextAttributeIndices, depth + 1);
            node->children[">"] = buildTree(right, nextAttributeIndices, depth + 1);
        } else {
            std::map<std::string, DataTable> subsets;
            for (const auto& row : data) {
                subsets[row[bestAttrIndex]].push_back(row);
            }
            for (auto const& [value, subset] : subsets) {
                node->children[value] = buildTree(subset, nextAttributeIndices, depth + 1);
            }
        }
        
        return node;
    }

    std::string predictRecursive(const DataRow& row, Node* node) {
        if (node->isLeaf) {
            return node->prediction;
        }

        const std::string& value = row[node->attributeIndex];
        if (node->isNumericSplit) {
            if (stod(value) <= node->splitValue) {
                return predictRecursive(row, node->children["<="]);
            } else {
                return predictRecursive(row, node->children[">"]);
            }
        } else {
            if (node->children.count(value)) {
                return predictRecursive(row, node->children[value]);
            } else {
                // If we see a value we didn't see in training, return a guess
                // A better approach would be to return the majority class of the current node's data
                // For simplicity, we just pick the first child's branch.
                return predictRecursive(row, node->children.begin()->second);
            }
        }
    }
};

// =================================================================================
// Main Experimentation Loop
// =================================================================================
void run_experiments(const std::string& dataset_path) {
    std::vector<std::string> headers;
    DataTable all_data = loadData(dataset_path, headers);

    std::vector<std::string> criteria = {"IG", "IGR", "NWIG"};
    std::vector<int> depths;
    if (dataset_path.find("iris") != std::string::npos) {
        depths = {0}; // 0 means no limit
    } else { // Adult dataset is larger, test more depths
        depths = {0};
    }

    int n_runs = 20;
    
    std::cout << "criterion,max_depth,run_id,accuracy" << std::endl;

    for (const auto& crit : criteria) {
        for (int depth : depths) {
            for (int run = 0; run < n_runs; ++run) {
                // Shuffle and split data
                std::shuffle(all_data.begin(), all_data.end(), std::mt19937{std::random_device{}()});
                
                size_t train_size = all_data.size() * 0.8;
                DataTable train_data(all_data.begin(), all_data.begin() + train_size);
                DataTable test_data(all_data.begin() + train_size, all_data.end());

                // Handle missing data ('?') by imputation on training data
                std::vector<std::string> majority_values(headers.size());
                for(size_t j = 0; j < headers.size() - 1; ++j) {
                    std::map<std::string, int> val_counts;
                    for(const auto& r : train_data) {
                        if (r[j] != "?") val_counts[r[j]]++;
                    }
                    if (!val_counts.empty()) {
                        majority_values[j] = std::max_element(val_counts.begin(), val_counts.end(),
                           [](const auto& a, const auto&b){ return a.second < b.second; })->first;
                    }
                }
                
                for(auto& r : train_data) for(size_t j=0; j<r.size()-1; ++j) if(r[j] == "?") r[j] = majority_values[j];
                for(auto& r : test_data) for(size_t j=0; j<r.size()-1; ++j) if(r[j] == "?") r[j] = majority_values[j];

                DecisionTree tree(crit, depth);
                tree.train(train_data, headers);
                double accuracy = tree.evaluate(test_data);
                
                int depth_label = (depth == 0) ? 99 : depth; // use 99 for 'unlimited' for easier plotting
                std::cout << crit << "," << depth_label << "," << run << "," << accuracy << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <dataset_path>" << std::endl;
        return 1;
    }
    
    std::string dataset_path = argv[1];
    run_experiments(dataset_path);
    
    return 0;
}