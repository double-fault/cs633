import matplotlib.pyplot as plt
import numpy as np

time_point_v1= {
    8: [0.455269, 0.458390, 0.458927],
    16: [0.457434, 0.459548, 0.461475],
    32: [0.617473, 0.493020, 0.545275],
    64: [1.439103, 1.478587, 1.428333]
}

time_point_v2 = {
    8: [0.039324, 0.036215, 0.034397],
    16: [0.029669, 0.030415, 0.031667],
    32: [0.630115, 0.229125, 0.474021],
    64: [1.499877, 1.502457, 1.520435]
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
plt.savefig('7_point_stencil.png', dpi=300)
plt.show()