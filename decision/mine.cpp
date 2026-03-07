#include <bits/stdc++.h>
using namespace std;

// Utility to split a string by delimiter
static vector<string> split(const string &s, char delim) {
    vector<string> elems;
    string item;
    for (char c : s) {
        if (c == delim) {
            elems.push_back(item);
            item.clear();
        } else {
            item.push_back(c);
        }
    }
    elems.push_back(item);
    return elems;
}

// Structure for a decision tree node
struct Node {
    bool isLeaf;
    string label;               // For leaf: predicted class
    int attrIndex = -1;         // attribute index for split (if internal)
    double threshold = 0.0;     // threshold if numeric
    bool isNumeric = false;     // is the split attribute numeric
    map<string, Node*> children; // for categorical splits
    Node *left = nullptr, *right = nullptr; // for numeric splits

    // Constructor for leaf
    Node(const string &lab) : isLeaf(true), label(lab) {}
    // Constructor for internal
    Node(int attr, bool numeric = false, double thresh = 0.0)
        : isLeaf(false), attrIndex(attr), isNumeric(numeric), threshold(thresh) {}
};

// Compute the entropy of a set of examples (vector of class labels)
double entropy(const vector<string> &labels) {
    map<string,int> count;
    for (auto &lab: labels) count[lab]++;
    double H = 0.0;
    double n = labels.size();
    for (auto &p: count) {
        double p_i = p.second / n;
        if (p_i > 0)
            H -= p_i * log2(p_i);
    }
    return H;
}

// Compute Gini impurity of labels (for reference, not used as main criterion here)
double giniImpurity(const vector<string> &labels) {
    map<string,int> count;
    for (auto &lab: labels) count[lab]++;
    double g = 1.0;
    double n = labels.size();
    for (auto &p: count) {
        double p_i = p.second / n;
        g -= p_i * p_i;  // Gini = 1 - sum(p_i^2)
    }
    return g;
}

// Compute majority error (1 - max class fraction)
double majorityError(vector<string> &arr) {
    map<string,int> cnt;
    for(int i=0;i<arr.size();i++){
        cnt[arr[i]]++;
    }
    int mx = 0;
    for(auto i: cnt){
        mx = max(mx, i.second);
    }
    double p = 1.0*mx/arr.size();
    p = 1 - p;
    return p;
}

// Find the best threshold split for a numeric attribute at attrIndex.
// Returns (bestGain, bestThreshold).
// The function splits data into two parts for each candidate midpoint.
pair<double,double> findBestNumericSplit(
    const vector<vector<string>> &data, int attrIndex, 
    const vector<string> &classes, const string &criterion)
{
    int N = data.size();
    vector<pair<double,string>> vals; 
    vals.reserve(N);
    for(int i=0;i<N;i++){
        double v = stod(data[i][attrIndex]);
        vals.push_back({v, classes[i]});
    }
    sort(vals.begin(), vals.end(),
         [](auto &a, auto &b){ return a.first < b.first; });

    double totalEntropy = entropy(classes);
    double bestGain = -1e9, bestThresh = 0.0;
    // Evaluate splits between distinct values
    for(int i=1; i<N; i++){
        if (vals[i-1].first == vals[i].first) continue;
        // Potential threshold is midpoint
        double thr = 0.5*(vals[i-1].first + vals[i].first);
        vector<string> leftLabs, rightLabs;
        for(auto &p: vals){
            if (p.first <= thr) leftLabs.push_back(p.second);
            else rightLabs.push_back(p.second);
        }
        // Compute weighted entropy
        double HL = entropy(leftLabs), HR = entropy(rightLabs);
        double wL = leftLabs.size()/(double)N, wR = rightLabs.size()/(double)N;
        double IG = totalEntropy - (wL*HL + wR*HR);

        double score = IG;
        if (criterion == "IGR") {
            // Gain Ratio = IG / IV
            double IV = 0.0;
            if (wL>0) IV -= wL * log2(wL);
            if (wR>0) IV -= wR * log2(wR);
            if (IV > 0) score = IG / IV;
            else score = 0;
        }
        else if (criterion == "NWIG") {
            // Normalize by log2(2) = 1, so NWIG = IG for numeric splits (no effect)
            score = IG / log2(2.0);
        }
        // Choose best
        if (score > bestGain) {
            bestGain = score;
            bestThresh = thr;
        }
    }
    return {bestGain, bestThresh};
}

// Compute information gain (or variant) for a categorical attribute.
double computeCategoricalScore(
    const vector<vector<string>> &data, int attrIndex, 
    const vector<string> &classes, const string &criterion)
{
    int N = data.size();
    double totalEntropy = entropy(classes);
    // Partition data by attribute value
    map<string, vector<string>> subsets;
    for(int i=0;i<N;i++){
        subsets[data[i][attrIndex]].push_back(classes[i]);
    }
    // Compute IG = H(S) - sum(|Sv|/|S| * H(Sv))
    double IG = totalEntropy;
    for(auto &entry: subsets){
        auto &labs = entry.second;
        double w = labs.size()/(double)N;
        IG -= w * entropy(labs);
    }
    if (criterion == "IG")
        return IG;
    // Gain Ratio
    if (criterion == "IGR") {
        // Compute intrinsic value
        double IV = 0.0;
        for(auto &entry: subsets){
            double w = entry.second.size()/(double)N;
            if (w>0) IV -= w * log2(w);
        }
        if (IV > 0) return IG / IV;
        else return 0.0;
    }
    // Normalized Weighted IG (penalize many splits)
    if (criterion == "NWIG") {
        int k = subsets.size();
        double norm = (k>0 ? log2(k+1.0) : 1.0);
        return IG / norm;
    }
    return 0.0;
}

// Choose the best attribute (and threshold if numeric) to split the data.
void chooseBestAttribute(const vector<vector<string>> &data, 
                         const vector<string> &classes, 
                         const vector<int> &attribute_index,
                         const vector<bool> &isNumeric, 
                         const string &criterion,
                         int &bestAttr, double &bestThresh, double &bestScore) 
{
    bestScore = -1e9;
    bestAttr = -1; bestThresh = 0.0;
    // Try each attribute
    for(int attr: attribute_index){
        if (isNumeric[attr]) {
            // Numeric attribute: find best threshold
            pair<double,double> pr = findBestNumericSplit(data, attr, classes, criterion);
            double gain = pr.first;
            double thr = pr.second;
            if (gain > bestScore) {
                bestScore = gain;
                bestAttr = attr;
                bestThresh = thr;
            }
        } else {
            // Categorical attribute
            double score = computeCategoricalScore(data, attr, classes, criterion);
            if (score > bestScore) {
                bestScore = score;
                bestAttr = attr;
                bestThresh = 0.0;
            }
        }
    }
}


Node* buildTree(const vector<vector<string>> &data, const vector<string> &classes, vector<int> attribute_index,const vector<bool> &isNumeric, const string &criterion, int depth, int maxDepth){
    // Check termination
    // 1. If all examples have same class => make leaf
    bool allSame = true;
    for(int i=1; i<classes.size(); i++){
        if (classes[i] != classes[0]) { allSame = false; break; }
    }
    if (allSame) {
        return new Node(classes[0]);
    }
    // 2. If no attributes left or depth limit reached => make leaf (majority class)
    if (attribute_index.empty() || (maxDepth>0 && depth >= maxDepth)) {
        // Find majority class
        map<string,int> count;
        for(auto &lab: classes) count[lab]++;
        string maj = classes[0];
        int maxc = 0;
        for(auto &p: count){
            if (p.second > maxc) { maxc = p.second; maj = p.first; }
        }
        return new Node(maj);
    }
    // Choose best attribute to split
    int bestAttr; double bestThresh, bestScore;
    chooseBestAttribute(data, classes, attribute_index, isNumeric, 
                        criterion, bestAttr, bestThresh, bestScore);
    if (bestAttr < 0) {
        // No good split found (should not happen), do majority
        map<string,int> count;
        for(auto &lab: classes) count[lab]++;
        string maj = classes[0]; int maxc=0;
        for(auto &p: count) if (p.second>maxc) { maxc=p.second; maj=p.first; }
        return new Node(maj);
    }

    // Create internal node
    Node *node = new Node(bestAttr, isNumeric[bestAttr], bestThresh);
    // Remove this attribute from available list (for further splits)
    vector<int> remaining;
    for(int a: attribute_index) if (a != bestAttr) remaining.push_back(a);

    // Split data accordingly
    if (isNumeric[bestAttr]) {
        // Numeric: two branches
        vector<vector<string>> leftData, rightData;
        vector<string> leftClasses, rightClasses;
        for(int i=0;i<data.size();i++){
            double val = stod(data[i][bestAttr]);
            if (val <= bestThresh) {
                leftData.push_back(data[i]);
                leftClasses.push_back(classes[i]);
            } else {
                rightData.push_back(data[i]);
                rightClasses.push_back(classes[i]);
            }
        }
        // If a side is empty, make it a leaf with majority class
        if (leftData.empty()) {
            node->left = new Node("");
        } else {
            node->left = buildTree(leftData, leftClasses, remaining, isNumeric, criterion, depth+1, maxDepth);
        }
        if (rightData.empty()) {
            node->right = new Node("");
        } else {
            node->right = buildTree(rightData, rightClasses, remaining, isNumeric, criterion, depth+1, maxDepth);
        }
    } else {
        // Categorical: multi-way split
        // Group examples by value
        map<string, vector<vector<string>>> subsets;
        map<string, vector<string>> subsetClasses;
        for(int i=0;i<data.size();i++){
            string v = data[i][bestAttr];
            subsets[v].push_back(data[i]);
            subsetClasses[v].push_back(classes[i]);
        }
        for (auto &entry : subsets) {
            string val = entry.first;
            auto subsetData = entry.second;
            auto subsetLabs = subsetClasses[val];
            // Build subtree
            Node *child = buildTree(subsetData, subsetLabs, remaining, isNumeric, criterion, depth+1, maxDepth);
            node->children[val] = child;
        }
    }
    return node;
}

// Traverse the tree to predict class for one example
string predict(const Node *node, const vector<string> &example) {
    if (node->isLeaf) return node->label;
    int attr = node->attrIndex;
    if (node->isNumeric) {
        double val = stod(example[attr]);
        if (val <= node->threshold) {
            if (node->left) return predict(node->left, example);
            else return node->label; 
        } else {
            if (node->right) return predict(node->right, example);
            else return node->label;
        }
    } else {
        string val = example[attr];
        auto it = node->children.find(val);
        if (it != node->children.end()) {
            return predict(it->second, example);
        } else {
            // Unseen attribute value: fallback (could use majority class)
            return node->label;
        }
    }
}

// Count nodes and depth in the tree
void countTree(const Node *node, int depth, int &nodeCount, int &maxDepth) {
    if (!node) return;
    nodeCount++;
    maxDepth = max(maxDepth, depth);
    if (!node->isLeaf) {
        if (node->isNumeric) {
            countTree(node->left, depth+1, nodeCount, maxDepth);
            countTree(node->right, depth+1, nodeCount, maxDepth);
        } else {
            for (auto &kv: node->children) {
                countTree(kv.second, depth+1, nodeCount, maxDepth);
            }
        }
    }
}


vector<vector<string>>read_data(const string file, bool hasHeader, bool dropFirstColumn=false) {
    vector<vector<string>> data;
    ifstream fin(file);
    string line;
    if (hasHeader) getline(fin, line);  // skip header
    while (getline(fin, line)) {
        if (line.empty()) continue;
        vector<string> fields = split(line, ',');
        if (dropFirstColumn && fields.size()>0) {
            fields.erase(fields.begin());
        }
        // Trim whitespace around values
        for (auto &s: fields) {
            while (!s.empty() && isspace(s.back())) s.pop_back();
            while (!s.empty() && isspace(s.front())) s.erase(0,1);
        }
        data.push_back(fields);
    }
    return data;
}

int main(int argc, char *argv[]) {
    string criterion = argv[1];
    int maxDepth = stoi(argv[2]);
    auto adultData = read_data("Datasets/adult.data", false, false);
    // Iris dataset (CSV with header, drop ID column)
    auto irisData = read_data("Datasets/Iris.csv", true, true);

    // We will run on each dataset separately
    vector<string> datasets = {"iris"};
    for (auto &dset: datasets) {
        vector<vector<string>> data;
        if (dset == "adult") {
            data = adultData;
            // taking only top 2000 rows for performance
            if (data.size() > 2000) {
                data.resize(2000);
            }
        } else {
            data = irisData;
        }
        if (data.empty()) continue;
        // Determine class index (last column)
        int classIdx = data[0].size() - 1;

        // Identify numeric vs categorical attributes (we treat everything non-numeric as categorical)
        vector<bool> isNumeric(data[0].size(), false);
        for (int j = 0; j < data[0].size(); j++) {
            if (j == classIdx) continue;
            // Try to parse first data row to see if numeric
            try {
                stod(data[0][j]);
                isNumeric[j] = true;
            } catch(...) {
                isNumeric[j] = false;
            }
        }
        // Numeric and categorical attribute indices (excluding class)
        vector<int> attribute_index;
        for (int j = 0; j < classIdx; j++) {
            cout<<"Attribute "<<j<<": "<<(isNumeric[j] ? "Numeric" : "Categorical")<<endl;
            attribute_index.push_back(j);
        }

        // Repeat 20 trials with random 80/20 splits
        int totalCorrect = 0, totalTest = 0;
        for (int trial = 0; trial < 20; trial++) {
            // Shuffle data indices
            vector<int> idx(data.size());
            iota(idx.begin(), idx.end(), 0);
            random_shuffle(idx.begin(), idx.end());

            int trainSize = data.size() * 80 / 100;
            vector<vector<string>> trainData, testData;
            vector<string> trainLabels, testLabels;
            for (int i = 0; i < data.size(); i++) {
                auto row = data[idx[i]];
                if (i < trainSize) {
                    trainLabels.push_back(row[classIdx]);
                    row.pop_back();
                    trainData.push_back(row);
                } else {
                    testLabels.push_back(row[classIdx]);
                    row.pop_back();
                    testData.push_back(row);
                }
            }
            // Build tree on training set
            Node *root = buildTree(trainData, trainLabels, attribute_index, isNumeric, criterion, 1, maxDepth);
            // Evaluate on test set
            int correct = 0;
            for (int i = 0; i < testData.size(); i++) {
                string pred = predict(root, testData[i]);
                if (pred == testLabels[i]) correct++;
            }
            totalCorrect += correct;
            totalTest += testData.size();

            // Clean up tree
            // (A recursive delete function could be written; omitted for brevity)
            // In a real implementation, free all nodes to avoid memory leak.
        }
        double avgAcc = (double)totalCorrect / totalTest;
        cout << "Dataset: " << dset 
             << ", Criterion: " << criterion 
             << ", Avg Accuracy: " << avgAcc * 100 << "%\n";
    }
    return 0;
}
