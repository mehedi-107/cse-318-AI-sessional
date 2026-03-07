#include <bits/stdc++.h>
using namespace std;

vector<string> str_split(string &s){
    vector<string> str_arr;
    string str;
    for(int i=0;i<s.size();i++){
        if(s[i]==','){
            str_arr.push_back(str);
            str.clear();
        }
        else str+=s[i];                 
    }
    if(str.size()!=0) str_arr.push_back(str);
    return str_arr;
}

class Node{
    public:
        bool isLeaf;
        string name;
        bool is_num;
        int attribute_index;
        string majority_class;
        double threshold;
        map<string, Node*> child;
        Node *left, *right;

        Node(int attr, bool is_Num, double Threshold, string maj_class = ""){
            isLeaf = false;
            name = "";  
            attribute_index = attr;
            majority_class = maj_class;
            threshold = Threshold;
            is_num = is_Num;
            left = nullptr;
            right = nullptr;
        }
        Node(string Name){
            isLeaf = true;
            name = Name;
            attribute_index = -1;
            majority_class = Name;
            threshold = 0.0;
            is_num = false;
            left = nullptr;
            right = nullptr;
        }
};

double entropy(const vector<string> &arr) {
    if(arr.empty()) return 0.0;
    
    unordered_map<string,int> cnt;
    for(const auto& s : arr){
        cnt[s]++;
    }
    
    double ent = 0.0;
    int total = arr.size();
    for(const auto& p : cnt){
        double pc = (double)p.second / total;
        if(pc > 0) ent -= pc * log2(pc);
    }
    return ent;
}

// Optimized numeric split function
pair<double,double> best_num_split(const vector<vector<string>> &data, int attribute, 
                                   const vector<string> &classes, const string &criterion,
                                   const vector<int> &indices){
    
    vector<pair<double,string>> vals;
    vals.reserve(indices.size());
    
    // Pre-convert and collect values
    for(int idx : indices){
        try {
            double v = stod(data[idx][attribute]);
            vals.push_back({v, classes[idx]});
        } catch(...) {
            continue; // Skip invalid values
        }
    }
    
    if(vals.size() < 2) return {-1e9, 0.0};
    
    // Sort once
    sort(vals.begin(), vals.end());
    
    // Calculate total entropy once
    vector<string> all_classes;
    all_classes.reserve(vals.size());
    for(const auto& p : vals) all_classes.push_back(p.second);
    double total_ent = entropy(all_classes);
    
    double best_gain = -1e9;
    double best_thr = 0.0;
    
    // Try different thresholds
    for(int i = 1; i < vals.size(); i++){
        if(vals[i-1].first == vals[i].first) continue;
        
        double thr = 0.5 * (vals[i-1].first + vals[i].first);
        
        // Split without creating new vectors
        vector<string> left, right;
        left.reserve(i);
        right.reserve(vals.size() - i);
        
        for(int j = 0; j < i; j++) left.push_back(vals[j].second);
        for(int j = i; j < vals.size(); j++) right.push_back(vals[j].second);
        
        if(left.empty() || right.empty()) continue;
        
        double entropy_left = entropy(left);
        double entropy_right = entropy(right);
        double weight_left = (double)left.size() / vals.size();
        double weight_right = (double)right.size() / vals.size();
        double IG = total_ent - (weight_left * entropy_left + weight_right * entropy_right);
        
        double score = IG;
        
        if(criterion == "IGR"){
            double IV = 0.0;
            if(weight_left > 0) IV -= weight_left * log2(weight_left);
            if(weight_right > 0) IV -= weight_right * log2(weight_right);
            if(IV > 0) score = IG / IV;
            else score = 0;
        }
        else if(criterion == "NWIG"){
            score = (IG / log2(3.0)) * (1 - 1.0 / vals.size());
        }
        
        if(score > best_gain){
            best_gain = score;
            best_thr = thr;
        }
    }
    
    return {best_gain, best_thr};
}

// Optimized categorical score calculation
double calc_cat_score(const vector<vector<string>> &data, int attribute, 
                     const vector<string> &classes, const string &criterion,
                     const vector<int> &indices){
    
    double IG = entropy(classes);
    unordered_map<string, vector<string>> mp;
    
    for(int idx : indices){
        mp[data[idx][attribute]].push_back(classes[idx]);
    }
    
    for(const auto& p : mp){
        IG -= (double)p.second.size() / indices.size() * entropy(p.second);
    }
    
    if(criterion == "IG") return IG;
    else if(criterion == "IGR"){
        double IV = 0.0;
        for(const auto& p : mp){
            double w = (double)p.second.size() / indices.size();
            if(w > 0) IV -= w * log2(w);
        }
        if(IV > 0) return IG / IV;
        else return 0.0;
    }
    else if(criterion == "NWIG"){
        int k = mp.size();
        double norm = log2(k + 1.0);
        double penalty = 1.0 - (double)(k - 1) / indices.size();
        if(norm > 0 && penalty > 0) return IG / norm * penalty;
        else return 0.0;
    }
    return 0.0;
}

pair<int,pair<double,double>> best_attribute(const vector<vector<string>> &data, 
                                           const vector<string> &classes, 
                                           const vector<int> &attribute, 
                                           const vector<bool> &is_num, 
                                           const string &criterion,
                                           const vector<int> &indices){
    int best_attr = -1;
    double best_score = -1e9;
    double best_thr = 0.0;
    
    for(int i = 0; i < attribute.size(); i++){
        int attr = attribute[i];
        if(is_num[attr]){
            pair<double,double> pr = best_num_split(data, attr, classes, criterion, indices);
            if(pr.first > best_score){
                best_score = pr.first;
                best_attr = attr;
                best_thr = pr.second;
            }
        }
        else{
            double score = calc_cat_score(data, attr, classes, criterion, indices);
            if(score > best_score){
                best_score = score;
                best_attr = attr;
                best_thr = 0.0;
            }
        }
    }
    
    return {best_attr, {best_thr, best_score}};
}

string find_majority_class(const vector<string> &classes) {
    if(classes.empty()) return "";
    
    unordered_map<string, int> cnt;
    for(const auto& c : classes) cnt[c]++;
    
    return max_element(cnt.begin(), cnt.end(), 
                      [](const pair<string, int>& a, const pair<string, int>& b) {
                          return a.second < b.second;
                      })->first;
}

// Optimized tree building with indices
Node* build_tree(const vector<vector<string>> &data, const vector<string> &classes, 
                 const vector<int> &attribute, const vector<bool> &is_num, 
                 const string &criterion, int depth, int max_depth, 
                 const vector<int> &indices){
    
    if(indices.empty()) return new Node("unknown");
    
    // Create current classes vector
    vector<string> current_classes;
    current_classes.reserve(indices.size());
    for(int idx : indices) current_classes.push_back(classes[idx]);
    
    // Check if all classes are the same
    bool same = true;
    for(int i = 1; i < current_classes.size(); i++){
        if(current_classes[i] != current_classes[0]) {
            same = false;
            break;
        }
    }
    
    if(same) return new Node(current_classes[0]);
    
    // Early stopping conditions
    if(attribute.empty() || (max_depth > 0 && depth >= max_depth) || 
       indices.size() < 10){ // Minimum samples per leaf
        string majority = find_majority_class(current_classes);
        return new Node(majority);
    }
    
    // Find best attribute
    auto best_result = best_attribute(data, classes, attribute, is_num, criterion, indices);
    int best_attr = best_result.first;
    double best_thr = best_result.second.first;
    double best_score = best_result.second.second;
    
    // Early stopping if no good split found
    if(best_attr < 0 || best_score < 0.001){
        string majority = find_majority_class(current_classes);
        return new Node(majority);
    }
    
    string majority = find_majority_class(current_classes);
    Node *node = new Node(best_attr, is_num[best_attr], best_thr, majority);
    
    // Remove best attribute from remaining attributes
    vector<int> remain;
    for(int i = 0; i < attribute.size(); i++) {
        if(attribute[i] != best_attr) remain.push_back(attribute[i]);
    }
    
    if(is_num[best_attr]){
        vector<int> left_indices, right_indices;
        
        for(int idx : indices){
            try {
                double v = stod(data[idx][best_attr]);
                if(v <= best_thr) left_indices.push_back(idx);
                else right_indices.push_back(idx);
            } catch(...) {
                right_indices.push_back(idx); // Default to right for invalid values
            }
        }
        
        if(left_indices.empty()){
            node->left = new Node(majority);
        } else {
            node->left = build_tree(data, classes, remain, is_num, criterion, 
                                  depth + 1, max_depth, left_indices);
        }
        
        if(right_indices.empty()){
            node->right = new Node(majority);
        } else {
            node->right = build_tree(data, classes, remain, is_num, criterion, 
                                   depth + 1, max_depth, right_indices);
        }
    }
    else{
        unordered_map<string, vector<int>> grp;
        
        for(int idx : indices){
            string v = data[idx][best_attr];
            grp[v].push_back(idx);
        }
        
        for(const auto& p : grp){
            if(!p.second.empty()){
                node->child[p.first] = build_tree(data, classes, remain, is_num, 
                                                criterion, depth + 1, max_depth, p.second);
            }
        }
    }
    
    return node;
}

string predict(Node *node, const vector<string> &ex){
    if(node->isLeaf) return node->name;
    
    int attr = node->attribute_index;
    if(node->is_num){
        try {
            double v = stod(ex[attr]);
            if(v <= node->threshold){
                if(node->left) return predict(node->left, ex);
                else return node->majority_class;
            }
            else{
                if(node->right) return predict(node->right, ex);
                else return node->majority_class;
            }
        } catch(...) {
            return node->majority_class;
        }
    }
    else{
        string v = ex[attr];
        if(node->child.find(v) != node->child.end()) {
            return predict(node->child[v], ex);
        }
        else return node->majority_class;
    }
}

void countTree(const Node *node, int depth, int &nodeCount, int &maxDepth) {
    if (!node) return;
    nodeCount++;
    maxDepth = max(maxDepth, depth);
    if (!node->isLeaf) {
        if (node->is_num) {
            countTree(node->left, depth+1, nodeCount, maxDepth);
            countTree(node->right, depth+1, nodeCount, maxDepth);
        } else {
            for (const auto &kv: node->child) {
                countTree(kv.second, depth+1, nodeCount, maxDepth);
            }
        }
    }
}

vector<vector<string>> read_data(const string &file, bool hasHeader, bool dropFirstColumn = false) {
    vector<vector<string>> data;
    ifstream fin(file);
    
    if(!fin.is_open()) {
        cout << "Error opening file: " << file << endl;
        return data;
    }
    
    string line;
    if (hasHeader) getline(fin, line);
    
    while(getline(fin, line)){
        if (line.empty()) continue;
        vector<string> fields = str_split(line);
        if (dropFirstColumn && fields.size() > 0) {
            fields.erase(fields.begin());
        }
        data.push_back(fields);
    }
    
    fin.close();
    return data;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        cout << "Usage: " << argv[0] << " <criterion> <max_depth>" << endl;
        return 1;
    }
    
    string criterion = argv[1];
    int maxDepth = stoi(argv[2]);
    
    auto start_time = chrono::high_resolution_clock::now();
    
    auto adultData = read_data("Datasets/adult.data", false, false);
    auto irisData = read_data("Datasets/Iris.csv", true, true);
    
    vector<string> datasets = {"adult"}; // Change to test adult dataset
    
    for (const auto &dset: datasets) {
        vector<vector<string>> data;
        if (dset == "adult") {
            data = adultData;
        } else {
            data = irisData;
        }
        
        if (data.empty()) continue;
        
        cout << "Dataset size: " << data.size() << " rows" << endl;
        
        int classIdx = data[0].size() - 1;
        
        // Identify numeric vs categorical attributes
        vector<bool> is_num(data[0].size(), false);
        for (int j = 0; j < data[0].size(); j++){
            if (j == classIdx) continue;
            try {
                stod(data[0][j]);
                is_num[j] = true;
            } catch(...) {
                is_num[j] = false;
            }
        }
        
        vector<int> attribute_index;
        for (int j = 0; j < classIdx; j++) {
            attribute_index.push_back(j);
        }
        
        // Extract all classes
        vector<string> all_classes;
        all_classes.reserve(data.size());
        for(const auto& row : data) {
            all_classes.push_back(row[classIdx]);
        }
        
        int totalCorrect = 0, totalTest = 0;
        
        for (int trial = 0; trial < 5; trial++) { // Reduced trials for testing
            vector<int> idx(data.size());
            iota(idx.begin(), idx.end(), 0);
            random_shuffle(idx.begin(), idx.end());
            
            int trainSize = data.size() * 80 / 100;
            vector<int> train_indices, test_indices;
            
            for (int i = 0; i < data.size(); i++) {
                if (i < trainSize) {
                    train_indices.push_back(idx[i]);
                } else {
                    test_indices.push_back(idx[i]);
                }
            }
            
            cout << "Trial " << trial + 1 << ": Training on " << train_indices.size() 
                 << " examples, testing on " << test_indices.size() << " examples." << endl;
            
            auto trial_start = chrono::high_resolution_clock::now();
            
            Node *root = build_tree(data, all_classes, attribute_index, is_num, 
                                  criterion, 1, maxDepth, train_indices);
            
            auto trial_end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(trial_end - trial_start);
            cout << "Tree building took: " << duration.count() << " ms" << endl;
            
            int correct = 0;
            for (int idx : test_indices){
                vector<string> test_row = data[idx];
                test_row.pop_back(); // Remove class label
                string pred = predict(root, test_row);
                if (pred == all_classes[idx]) correct++;
            }
            
            totalCorrect += correct;
            totalTest += test_indices.size();
            
            int nodeCount = 0, treeDepth = 0;
            countTree(root, 1, nodeCount, treeDepth);
            cout << "Tree has " << nodeCount << " nodes, max depth: " << treeDepth << endl;
            
            // Clean up memory (you should implement proper cleanup)
            // delete root;
        }
        
        double avgAcc = (double)totalCorrect / totalTest;
        cout << "Dataset: " << dset 
             << ", Criterion: " << criterion 
             << ", Max Depth: " << maxDepth
             << ", Avg Accuracy: " << avgAcc * 100 << "%" << endl;
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::seconds>(end_time - start_time);
    cout << "Total execution time: " << total_duration.count() << " seconds" << endl;
    
    return 0;
}