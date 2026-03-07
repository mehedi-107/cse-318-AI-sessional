import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

data = pd.read_csv('2105052.csv')
data = data[:10:]

def remove_dash(value):
    return int(str(value).split('(')[0])

processed_data = data.rename(columns={
    'Simple Randomized or Randomized-1': 'Randomized',
    'Simple Greedy or Greedy-1': 'Greedy',
    'Semi-greedy-1': 'Semi-Greedy',
    'Simple local or local-1 - Average value': 'Local Search',
    'GRASP-1 - Best value': 'GRASP'
})

processed_data['GRASP'] = processed_data['GRASP'].apply(remove_dash)
processed_data['Semi-Greedy'] = processed_data['Semi-Greedy'].apply(remove_dash)
algorithms = ['Randomized', 'Greedy', 'Semi-Greedy', 'Local Search', 'GRASP']
plot_data = processed_data[['Name'] + algorithms]

colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd']
bar_width = 0.15
n_graphs = len(plot_data)
index = np.arange(n_graphs)
plt.figure(figsize=(16, 8))
for i, algo in enumerate(algorithms):
    plt.bar(
        index + (i * bar_width),
        plot_data[algo],
        width=bar_width,
        color=colors[i],
        label=algo
    )
plt.title('Max Cut Results Comparison', fontsize=14)
plt.xlabel('Graph Instances', fontsize=12)
plt.ylabel('Solution Values', fontsize=12)
plt.xticks(
    index + bar_width * 2,
    plot_data['Name'],
    rotation=0,
    fontsize=10
)
plt.ylim(-2000, 12000)
plt.legend(bbox_to_anchor=(1.02, 1), loc='upper left')
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.show()