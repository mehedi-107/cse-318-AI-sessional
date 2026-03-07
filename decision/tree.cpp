#include <bits/stdc++.h>
using namespace std;

vector<string>str_split(string &s){
    vector<string>str_arr;
    string str;
    for(int i=0;i<s.size();i++){
        if(s[i]==',' || s[i]==' '){  // Split by comma OR space
            if(!str.empty()){  // Only add non-empty strings
                str_arr.push_back(str);
                str.clear();
            }
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

        Node(int attr, bool is_Num, double Threshold, string maj_class = ""){// for internal nodes
            isLeaf = false;
            name = "";  
            attribute_index = attr;
            majority_class = maj_class;
            threshold = Threshold;
            is_num = is_Num;
            left = nullptr;
            right = nullptr;
        }
        Node(string Name){ // for leaf nodes
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



double entropy(vector<string> &arr) {
    map<string,int>cnt;
    for(int i=0;i<arr.size();i++){
        cnt[arr[i]]++;
    }
    double ent = 0.0;
    for(auto i: cnt){
        double pc = 1.0*i.second/arr.size();
        if(pc>0) ent-= pc*log2(pc);
    }
    return ent;
}


double calc_num_gain(vector<vector<string>> &data, int attribute, vector<string> &classes, double threshold, string &criterion){
    vector<string> left_classes,right_classes;
    for(int i = 0; i < data.size(); i++) {// spliting into left and right based on threshold
        double val = stod(data[i][attribute]);
        if(val <= threshold) {
            left_classes.push_back(classes[i]);
        } else {
            right_classes.push_back(classes[i]);
        }
    }
    if(left_classes.empty() || right_classes.empty()){
        return 0.0;
    }
    double total_entropy = entropy(classes);
    double left_entropy = entropy(left_classes);
    double right_entropy = entropy(right_classes);
    double weight_left = 1.0*left_classes.size()/data.size();
    double weight_right = 1.0*right_classes.size()/data.size();
    double ig = total_entropy - (weight_left * left_entropy + weight_right * right_entropy);
    
    if(criterion == "IG"){
        return ig;
    }
    else if(criterion == "IGR"){
        double temp = 0.0;
        if(weight_left > 0) temp -= weight_left*log2(weight_left);
        if(weight_right > 0) temp -= weight_right*log2(weight_right);
        return (temp> 0) ? ig/temp : 0.0;
    }
    else if(criterion == "NWIG"){
        return (ig/log2(3.0))*(1.0 - 1.0/data.size());
    }
    
    return ig;
}

pair<double,double> best_num_split(vector<vector<string>> &data, int attribute, vector<string> &classes, string &criterion) {
    vector<double> values;
    for(int i = 0; i < data.size(); i++) {
        values.push_back(stod(data[i][attribute]));
    }
    sort(values.begin(), values.end());
    vector<double> thresholds = {
        values[values.size()/4],
        values[values.size()/2], 
        values[3*values.size()/4]
    };
    
    double best_gain = -1e9, best_thr = 0;
    for(double thr : thresholds) {
        double gain = calc_num_gain(data, attribute, classes, thr, criterion);
        if(gain > best_gain) {
            best_gain = gain;
            best_thr = thr;
        }
    }
    return {best_gain, best_thr};
}


double calc_cat_score(vector<vector<string>> &data, int attribute, vector<string> &classes, string &criterion){
    double ig = entropy(classes);
    map<string, vector<string>> mp;
    for(int i=0;i<data.size();i++){
        mp[data[i][attribute]].push_back(classes[i]);
    }
    for(auto i: mp){
        ig -= 1.0*i.second.size()/data.size() * entropy(i.second);
    }
    if(criterion=="IG") return ig;
    else if(criterion=="IGR"){
        double iv = 0.0;
        for(auto i: mp){
            double w = 1.0*i.second.size()/data.size();
            if(w>0) iv -= w*log2(w);
        }
        if(iv>0) return ig/iv;
        else return 0.0;
    }
    else if(criterion=="NWIG"){
        int k = mp.size();
        double norm = log2(k+1.0);
        double penalty = 1.0 - 1.0*(k-1)/data.size();
        if(norm>0 && penalty>0) return (ig/norm)*penalty;
        else return 0.0;
    }
    return 0.0;
}


pair<int,pair<double,double>> best_attribute(vector<vector<string>> &data, vector<string> &classes, vector<int> &attribute, vector<bool> &is_num, string &criterion){
    int best_attr = -1;
    double best_score = -1e9;
    double best_thr = 0.0;
    
    for(int i=0;i<attribute.size();i++){
        int attr = attribute[i];
        if(is_num[attr]){
            pair<double,double> pr = best_num_split(data, attr, classes, criterion);
            if(pr.first > best_score){
                best_score = pr.first;
                best_attr = attr;
                best_thr = pr.second;
            }
        }
        else{
            double score = calc_cat_score(data, attr, classes, criterion);
            if(score > best_score){
                best_score = score;
                best_attr = attr;
                best_thr = 0.0;
            }
        }
    }
    
    return {best_attr, {best_thr, best_score}};
}



Node* build_tree(vector<vector<string>> &data, vector<string> &classes, vector<int> &attribute, vector<bool> &is_num, string &criterion, int depth, int max_depth){
    bool same = true;
    for(int i=1;i<classes.size();i++){
        if(classes[i] != classes[0]) {same = false;break;}
    }
    Node *temp = new Node(classes[0]);
    if(same==true) {
        return temp;// if all classes are same return that common class
    }

    if(attribute.empty() || (max_depth>0 && depth >= max_depth)){
        map<string,int> cnt;
        for(int i=0;i<classes.size();i++) cnt[classes[i]]++;
        string maj = classes[0];
        int mx = 0;
        for(auto i: cnt){
            if(i.second > mx){
                mx = i.second; 
                maj = i.first;
            }
        }
        return new Node(maj);// if no attributes left or max depth reached, return majority class
    }

    int best_attr;
    double best_thr, best_score;
    pair<int,pair<double,double>> temp2 = best_attribute(data, classes, attribute, is_num, criterion);
    best_attr = temp2.first;
    best_thr = temp2.second.first;
    best_score = temp2.second.second;
    map<string,int> cnt;
    for(auto& c : classes) cnt[c]++;
    string majority = classes[0];// majority calculation for backup
    int mx = 0;
    for(auto i: cnt){
        if(i.second > mx){
            mx = i.second; 
            majority = i.first;
        }
    }
    Node *node = new Node(best_attr, is_num[best_attr], best_thr, majority);

    vector<int> remain;
    for(int i=0;i<attribute.size();i++) if(attribute[i] != best_attr) remain.push_back(attribute[i]);

    if(is_num[best_attr]){// spliting based on threshold
        vector<vector<string>> left,right;
        vector<string> lclass, rclass;
        for(int i=0;i<data.size();i++){
            double v = stod(data[i][best_attr]);
            if(v <= best_thr){
                left.push_back(data[i]);
                lclass.push_back(classes[i]);
            }else{
                right.push_back(data[i]);
                rclass.push_back(classes[i]);
            }
        }
        if(left.size()==0){
            node->left = new Node(majority); // if no data in left, return majority class
        }
        else node->left = build_tree(left, lclass, remain, is_num, criterion, depth+1, max_depth);

        if(right.size()==0){
            node->right = new Node(majority);
        }
        else node->right = build_tree(right, rclass, remain, is_num, criterion, depth+1, max_depth);
    }
    else{
        map<string, vector<vector<string>>> grp;
        map<string, vector<string>> grp_class;
        for(int i=0;i<data.size();i++){
            string v = data[i][best_attr];
            grp[v].push_back(data[i]);
            grp_class[v].push_back(classes[i]);
        }
        for(auto i: grp){
            node->child[i.first] = build_tree(i.second, grp_class[i.first], remain, is_num, criterion, depth+1, max_depth);
        }
    }
    return node;
}


string predict(Node *node, vector<string> &ex){
    if(node->isLeaf) return node->name;
    
    int attr = node->attribute_index;
    if(node->is_num){
        double v = stod(ex[attr]);
        if(v <= node->threshold){
            if(node->left) return predict(node->left, ex);
            else return node->majority_class; 
        }
        else{
            if(node->right) return predict(node->right, ex);
            else return node->majority_class;  
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


void tree_cnt(const Node *node, int depth, int &nodeCount, int &maxDepth) {
    if (!node) return;
    nodeCount++;
    maxDepth = max(maxDepth, depth);
    if (!node->isLeaf) {
        if (node->is_num) {
            tree_cnt(node->left, depth+1, nodeCount, maxDepth);
            tree_cnt(node->right, depth+1, nodeCount, maxDepth);
        } else {
            for (auto &kv: node->child) {
                tree_cnt(kv.second, depth+1, nodeCount, maxDepth);
            }
        }
    }
}

bool is_missing(const string& value) {
    if(value == "?" || value.empty()){
        return true;
    }
    return false;
}

bool is_num_val(const string& value) {
    if (is_missing(value)) {return false;}
    try {
        stod(value);
        return true;
    } catch(...) {
        return false;
    }
}

string calc_mode(const vector<string>& values) {
    map<string, int> freq;
    for (const string& val : values) {
        if (!is_missing(val)) {
            freq[val]++;
        }
    }
    
    if (freq.empty()) return "0";
    
    string mode = freq.begin()->first;
    int max_freq = freq.begin()->second;
    
    for (const auto& pair : freq) {
        if (pair.second > max_freq) {
            max_freq = pair.second;
            mode = pair.first;
        }
    }
    
    return mode;
}


double calc_num_mode(const vector<string>& values) {
    vector<double> numeric_values;
    for (const string& val : values) {
        if (!is_missing(val) && is_num_val(val)) {
            numeric_values.push_back(stod(val));
        }
    }
    if (numeric_values.empty()) return 0.0;
    sort(numeric_values.begin(), numeric_values.end());
    if (numeric_values.size() % 2 == 0) {
        return (numeric_values[numeric_values.size()/2 - 1] + numeric_values[numeric_values.size()/2]) / 2.0;
    } 
    else {
        return numeric_values[numeric_values.size()/2];
    }
}

vector<vector<string>>read_data(string file, bool hasHeader, bool dropFirstColumn=false) {
    vector<vector<string>> data;
    ifstream fin(file);
    string line;
    if (hasHeader) getline(fin, line); 
    while(getline(fin, line)){
        if (line.empty()) continue;
        vector<string> fields = str_split(line);
        if (dropFirstColumn && fields.size()>0) {
            fields.erase(fields.begin());
        }
        data.push_back(fields);
    }
    if (!data.empty()) {
        int num_columns = data[0].size();
        for (int col = 0; col < num_columns; col++) {
            vector<string> column_values;
            for (int row = 0; row < data.size(); row++) {
                if (col < data[row].size()) {
                    column_values.push_back(data[row][col]);
                }
            }
            bool is_numeric = false;
            
            for (const string& val : column_values) {
                if (!is_missing(val) && is_num_val(val)) {
                    is_numeric = true;
                    break;
                }
            }
            
            string mode_value;
            if (is_numeric){
                double numeric_mode = calc_num_mode(column_values);
                mode_value = to_string(numeric_mode);
            } 
            else {
                mode_value = calc_mode(column_values);
            }
            for(int row = 0; row < data.size(); row++){
                if (col < data[row].size() && is_missing(data[row][col])) {
                    data[row][col] = mode_value;
                }
            }
            
        }
    }
    
    return data;
}

int main(int argc, char *argv[]) {
    string criterion = argv[1];
    int maxDepth = stoi(argv[2]);
    auto adultData = read_data("Datasets/adult.data", false, false);
    auto irisData = read_data("Datasets/Iris.csv", true, true);

    vector<string> datasets = {"adult", "iris"};

    for (auto &dset : datasets){
        vector<vector<string>> data;
        if (dset == "adult") {
            data = adultData;
        } else {
            data = irisData;
        }
        if (data.empty()) continue;

        int classIdx = data[0].size() - 1;

        vector<bool> is_num(data[0].size(), false);
        for (int j = 0; j < data[0].size(); j++) {
            if (j == classIdx) continue;
            try {
                stod(data[0][j]);
                is_num[j] = true;
            } catch (...) {
                is_num[j] = false;
            }
        }

        vector<int> attribute_index;
        for (int j = 0; j < classIdx; j++) {
            attribute_index.push_back(j);
        }

        int totalCorrect = 0, totalTest = 0;
        for (int trial = 0; trial < 20; trial++) {
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

            Node *root = build_tree(trainData, trainLabels, attribute_index, is_num, criterion, 1, maxDepth);

            int correct = 0;
            for (int i = 0; i < testData.size(); i++) {
                string pred = predict(root, testData[i]);
                if (pred == testLabels[i]) correct++;
            }
            totalCorrect += correct;
            totalTest += testData.size();

            int nodeCount = 0, maxDepth = 0;
            tree_cnt(root, 1, nodeCount, maxDepth);
            cout << "Tree has " << nodeCount << " nodes, max depth: " << maxDepth << "\n";
        }
        double avgAcc = (double)totalCorrect / totalTest;
        cout << "Dataset: " << dset << ", Criterion: " << criterion << ", Avg Accuracy: " << avgAcc * 100 << "%\n";
    }
    return 0;
}
