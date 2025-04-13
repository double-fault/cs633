import matplotlib.pyplot as plt
import numpy as np

time_point_v1 = {
    8: [0.137890, 0.139215, 0.136076],
    16: [0.137886, 0.137692, 0.135860],
    32: [0.602007, 0.555104, 0.222633],
    64: [1.319793, 1.247160, 1.256953]
}

time_point_v2 = {
    8: [0.016050, 0.015966, 0.016639],
    16: [0.014681, 0.015253, 0.016277],
    32: [0.654879, 0.127795, 0.598746],
    64: [1.469130, 1.469685, 1.476908]
}

def compute_stats(time_dict):
    processor_counts = sorted(time_dict.keys())
    means = [np.mean(time_dict[count]) for count in processor_counts]
    stds = [np.std(time_dict[count]) for count in processor_counts]
    return processor_counts, means, stds

np_counts, mean_point_v1, std_point_v1 = compute_stats(time_point_v1)
_, mean_point_v2, std_point_v2 = compute_stats(time_point_v2)

plt.style.use('default')
plt.rcParams.update({'font.size': 12})

plt.figure(figsize=(8, 5))
plt.errorbar(np_counts, mean_point_v2, yerr=std_point_v2, 
             fmt='-o', capsize=5, label='with_parallel_io', color='red')
plt.errorbar(np_counts, mean_point_v1, yerr=std_point_v1, 
             fmt='-s', capsize=5, label='without_parallel_io', color='blue')

plt.xlabel('Processor Count (np)', fontsize=12)
plt.ylabel('Execution Time (s)', fontsize=12)
plt.title('Scalability Results: Execution Time vs. Processors', fontsize=14)
plt.xticks(np_counts)
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend(fontsize=10)


plt.tight_layout()
plt.savefig('3_point_stencil.png', dpi=300)
plt.show()