@echo off
echo Starting Decision Tree Analysis...
echo.

echo Step 1: Compiling report.cpp...
g++ -o report report.cpp -std=c++17
if %errorlevel% neq 0 (
    echo Error: Compilation failed!
    pause
    exit /b 1
)

echo Step 2: Running experiments...
echo This may take a few minutes...
report.exe
if %errorlevel% neq 0 (
    echo Error: Report generation failed!
    pause
    exit /b 1
)

echo.
echo Step 3: Generating plots with Python...
python generate_plots.py
if %errorlevel% neq 0 (
    echo Error: Plot generation failed!
    echo Make sure you have matplotlib, seaborn, pandas, and numpy installed
    echo Try: pip install matplotlib seaborn pandas numpy
    pause
    exit /b 1
)

echo.
echo Analysis complete!
echo Check the following files:
echo - results_summary.csv: Summary of all experiments
echo - results_detailed.csv: Detailed trial-by-trial results
echo - *.png: Generated plots and visualizations
echo.
pause
