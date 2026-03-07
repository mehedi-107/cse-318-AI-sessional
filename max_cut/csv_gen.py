import csv
import graph
header = [
    "Name", "|V| or n", "|E| or m",
    "Simple Randomized or Randomized-1",
    "Simple Greedy or Greedy-1",
    "Semi-greedy-1",
    "Simple local or local-1 - No. of iterations",
    "Simple local or local-1 - Average value",
    "GRASP-1 - No. of iterations",
    "GRASP-1 - Best value",
    "Known best solution or upper bound"
]



with open("max_cut_results.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(header)
    writer.writerows(rows)
