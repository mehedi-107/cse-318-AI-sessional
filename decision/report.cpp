#include <bits/stdc++.h>
using namespace std;

// Include the tree implementation
#include "tree.cpp"

class ExperimentReporter {
private:
    struct ExperimentResult {
        string dataset;
        string criterion;
        int maxDepth;
        double avgAccuracy;
        double stdDeviation;
        int avgNodes;
        int avgDepth;
        double avgTrainingTime;
        vector<double> accuracies;
        vector<int> nodeCounts;
        vector<int> depths;
        vector<double> trainingTimes;
    };
    
    vector<ExperimentResult> results;
    
public:
    void runExperiment(const string& dataset, const string& criterion, int maxDepth, 
                      const vector<vector<string>>& data, int trials = 20) {
        ExperimentResult result;
        result.dataset = dataset;
        result.criterion = criterion;
        result.maxDepth = maxDepth;
        result.accuracies.clear();
        result.nodeCounts.clear();
        result.depths.clear();
        result.trainingTimes.clear();
        
        if (data.empty()) return;
        
        int classIdx = data[0].size() - 1;
        
        // Identify numeric vs categorical attributes
        vector<bool> is_num(data[0].size(), false);
        for (int j = 0; j < data[0].size(); j++) {
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
        
        int totalCorrect = 0, totalTest = 0;
        
        for (int trial = 0; trial < trials; trial++) {
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
            
            // Measure training time
            auto start = chrono::high_resolution_clock::now();
            string criterionCopy = criterion;
            Node* root = build_tree(trainData, trainLabels, attribute_index, is_num, criterionCopy, 1, maxDepth);
            auto end = chrono::high_resolution_clock::now();
            auto trainingTime = chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0; // ms
            
            // Evaluate on test set
            int correct = 0;
            for (int i = 0; i < testData.size(); i++) {
                string pred = predict(root, testData[i]);
                if (pred == testLabels[i]) correct++;
            }
            
            // Count tree statistics
            int nodeCount = 0, maxDepthActual = 0;
            tree_cnt(root, 1, nodeCount, maxDepthActual);
            
            // Store results
            double accuracy = (double)correct / testData.size();
            result.accuracies.push_back(accuracy);
            result.nodeCounts.push_back(nodeCount);
            result.depths.push_back(maxDepthActual);
            result.trainingTimes.push_back(trainingTime);
            
            totalCorrect += correct;
            totalTest += testData.size();
        }
        
        // Calculate statistics
        result.avgAccuracy = (double)totalCorrect / totalTest;
        result.avgNodes = accumulate(result.nodeCounts.begin(), result.nodeCounts.end(), 0.0) / trials;
        result.avgDepth = accumulate(result.depths.begin(), result.depths.end(), 0.0) / trials;
        result.avgTrainingTime = accumulate(result.trainingTimes.begin(), result.trainingTimes.end(), 0.0) / trials;
        
        // Calculate standard deviation
        double sum_sq = 0;
        for (double acc : result.accuracies) {
            sum_sq += (acc - result.avgAccuracy) * (acc - result.avgAccuracy);
        }
        result.stdDeviation = sqrt(sum_sq / trials);
        
        results.push_back(result);
        
        cout << "Completed: " << dataset << " - " << criterion << " - Depth " << maxDepth 
             << " - Accuracy: " << result.avgAccuracy * 100 << "%" << endl;
    }
    
    void saveResultsToCSV(const string& filename) {
        ofstream file(filename);
        file << "Dataset,Criterion,MaxDepth,AvgAccuracy,StdDeviation,AvgNodes,AvgDepth,AvgTrainingTime(ms)" << endl;
        
        for (const auto& result : results) {
            file << result.dataset << "," << result.criterion << "," << result.maxDepth << ","
                 << result.avgAccuracy << "," << result.stdDeviation << "," << result.avgNodes << ","
                 << result.avgDepth << "," << result.avgTrainingTime << endl;
        }
        file.close();
        cout << "Results saved to " << filename << endl;
    }
    
    void saveDetailedResults(const string& filename) {
        ofstream file(filename);
        file << "Dataset,Criterion,MaxDepth,Trial,Accuracy,NodeCount,ActualDepth,TrainingTime(ms)" << endl;
        
        for (const auto& result : results) {
            for (int i = 0; i < result.accuracies.size(); i++) {
                file << result.dataset << "," << result.criterion << "," << result.maxDepth << ","
                     << i+1 << "," << result.accuracies[i] << "," << result.nodeCounts[i] << ","
                     << result.depths[i] << "," << result.trainingTimes[i] << endl;
            }
        }
        file.close();
        cout << "Detailed results saved to " << filename << endl;
    }
    
    void generateReport() {
        cout << "\n=== DECISION TREE ANALYSIS REPORT ===" << endl;
        cout << "=======================================" << endl;
        
        // Group results by dataset
        map<string, vector<ExperimentResult*>> datasetResults;
        for (auto& result : results) {
            datasetResults[result.dataset].push_back(&result);
        }
        
        for (const auto& dataset : datasetResults) {
            cout << "\n--- " << dataset.first << " Dataset ---" << endl;
            cout << "Criterion\tMaxDepth\tAccuracy\tStdDev\tNodes\tDepth\tTime(ms)" << endl;
            cout << "---------------------------------------------------------------" << endl;
            
            for (const auto& result : dataset.second) {
                cout << result->criterion << "\t\t" << result->maxDepth << "\t\t"
                     << fixed << setprecision(4) << result->avgAccuracy << "\t\t"
                     << result->stdDeviation << "\t" << result->avgNodes << "\t"
                     << result->avgDepth << "\t" << result->avgTrainingTime << endl;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    cout << "Starting Decision Tree Analysis..." << endl;
    
    // Load datasets
    auto adultData = read_data("Datasets/adult.data", false, false);
    auto irisData = read_data("Datasets/Iris.csv", true, true);
    
    // Limit adult dataset for faster computation
    if (adultData.size() > 3000) {
        adultData.resize(3000);
    }
    
    ExperimentReporter reporter;
    
    // Test different criteria
    vector<string> criteria = {"IG", "IGR", "NWIG"};
    vector<int> maxDepths = {0, 2, 3, 4, 5, 7, 10, 12, 15};
    
    cout << "Running experiments..." << endl;
    
    // Run experiments for each dataset
    for (const string& criterion : criteria) {
        for (int maxDepth : maxDepths) {
            cout << "\nTesting " << criterion << " with max depth " << maxDepth << "..." << endl;
            
            // Adult dataset
            reporter.runExperiment("Adult", criterion, maxDepth, adultData, 20);
            
            // Iris dataset
            reporter.runExperiment("Iris", criterion, maxDepth, irisData, 20);
        }
    }
    
    // Save results
    reporter.saveResultsToCSV("results_summary.csv");
    reporter.saveDetailedResults("results_detailed.csv");
    
    // Generate report
    reporter.generateReport();
    
    cout << "\nAnalysis complete! Check results_summary.csv and results_detailed.csv" << endl;
    cout << "Run 'python generate_plots.py' to create visualizations." << endl;
    
    return 0;
}
