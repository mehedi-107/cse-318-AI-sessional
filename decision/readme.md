# Decision Tree Analysis Report Generator

This project implements a comprehensive decision tree analysis system that compares different splitting criteria (Information Gain, Information Gain Ratio, and Normalized Weighted Information Gain) across multiple datasets.

## Files Overview

- `tree.cpp`: Core decision tree implementation
- `report.cpp`: Experiment runner and report generator  
- `generate_plots.py`: Python script for visualization
- `run_analysis.bat`: Batch script to run complete analysis
- `requirements.txt`: Python dependencies

## Quick Start

### Option 1: Automated (Recommended)
```bash
# Run the complete analysis
./run_analysis.bat
```

### Option 2: Manual Steps
```bash
# Step 1: Install Python dependencies
pip install -r requirements.txt

# Step 2: Compile the report generator
g++ -o report report.cpp -std=c++17

# Step 3: Run experiments
./report.exe

# Step 4: Generate plots
python generate_plots.py
```

## What the Analysis Does

### 1. Experiments
- Tests 3 splitting criteria: **IG**, **IGR**, **NWIG**
- Tests 5 depth limits: **0** (no limit), **3**, **5**, **10**, **15**
- Runs on 2 datasets: **Adult** and **Iris**
- Performs 10 trials per configuration for statistical significance

### 2. Metrics Collected
- **Accuracy**: Classification accuracy on test set
- **Standard Deviation**: Measure of consistency
- **Node Count**: Tree complexity measure
- **Tree Depth**: Actual depth achieved
- **Training Time**: Time to build the tree

### 3. Generated Outputs

#### CSV Files
- `results_summary.csv`: Aggregated results by configuration
- `results_detailed.csv`: Individual trial results

#### Visualizations
- `accuracy_comparison.png`: Accuracy comparison across criteria
- `model_complexity.png`: Complexity vs accuracy analysis
- `training_time.png`: Training time comparison
- `detailed_analysis.png`: Distribution analysis
- `depth_effect.png`: Effect of max depth on performance

#### Console Report
- Performance comparison table
- Best configuration identification
- Statistical summary

## Analysis Features

### Splitting Criteria Comparison
- **Information Gain (IG)**: Standard measure based on entropy reduction
- **Information Gain Ratio (IGR)**: IG normalized by intrinsic value
- **Normalized Weighted Information Gain (NWIG)**: Custom weighted measure

### Depth Analysis
- **Depth 0**: No pruning (full tree)
- **Depth 3-15**: Various pruning levels
- **Impact**: Shows overfitting vs underfitting trade-offs

### Statistical Robustness
- Multiple trials per configuration
- Mean and standard deviation calculation
- Box plots for distribution analysis

## Key Insights the Analysis Provides

1. **Best Criteria**: Which splitting criterion works best for each dataset
2. **Optimal Depth**: What depth limit provides best accuracy
3. **Complexity Trade-offs**: How tree size affects performance
4. **Training Efficiency**: Which methods are fastest
5. **Consistency**: Which approaches are most reliable

## Expected Runtime
- **Iris dataset**: ~1-2 minutes
- **Adult dataset**: ~10-15 minutes (limited to 3000 samples)
- **Total analysis**: ~15-20 minutes

This analysis provides comprehensive insights into decision tree performance across different configurations, helping you choose the best approach for your specific use case. 