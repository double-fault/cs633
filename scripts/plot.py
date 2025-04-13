import os
import math
import re
import matplotlib.pyplot as plt

def extract_metadata(rel_path):
    subdir, filename = os.path.split(rel_path)
    version = subdir.strip()  # Expecting "v0", "v1", or "v2"
    pattern = r"output_((?:\d+_){3}\d+)_(\d+)_+v\d"
    match = re.search(pattern, filename)
    if match:
        core_config = match.group(1)
        processes = int(match.group(2))
    else:
        core_config = "unknown"
        processes = -1
    return version, processes, core_config

def format_summary_row(version, processes, core_config, avg_values, std_pct_values):
    formatted_values = [
        f"{avg:.4f} ({round(std)}%)"
        for avg, std in zip(avg_values, std_pct_values)
    ]
    err_values = []
    for avg, std in zip(avg_values, std_pct_values):
        if avg != 0:
            err = (std / 100.0) * avg
        else:
            err = 0.0
        err_values.append(err)
        
    return {
        'version': version,
        'processes': processes,
        'core_config': core_config,
        'values': formatted_values,
        'val3': avg_values[2],  # For the primary plot.
        'err3': err_values[2],
        'avg_values': avg_values,  # List of 3 average values.
        'err_values': err_values   # List of 3 error values.
    }

def process_file(rel_path, src_dirs, dst_dir, summary_data):
    file_paths = [os.path.join(src, rel_path) for src in src_dirs]
    contents = []
    for fp in file_paths:
        with open(fp, 'r') as f:
            contents.append(f.readlines())
    
    try:
        value_lists = [
            list(map(float, lines[2].strip().split()))
            for lines in contents
        ]
        avg_values = [sum(col) / len(col) for col in zip(*value_lists)]
        avg_line = " ".join("{:.6f}".format(val) for val in avg_values) + "\n"
        
        std_pct_values = []
        for col_vals in zip(*value_lists):
            mean = sum(col_vals) / len(col_vals)
            std = math.sqrt(sum((x - mean) ** 2 for x in col_vals) / len(col_vals))
            std_pct = (std / mean * 100) if mean != 0 else 0.0
            std_pct_values.append(std_pct)
        std_line = " ".join("{:.6f}".format(val) for val in std_pct_values) + "\n"
    
    except Exception as e:
        print(f"Error processing file {rel_path}: {e}")
        return
    
    new_content = contents[0].copy()  # Use the first source file as base.
    new_content[2] = avg_line
    new_content.append(std_line)
    
    output_path = os.path.join(dst_dir, rel_path)
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'w') as f:
        f.writelines(new_content)
    
    version, processes, core_config = extract_metadata(rel_path)
    summary_data.append(format_summary_row(version, processes, core_config, avg_values, std_pct_values))

def generate_plots(summary_data, dst_dir):
    groups = {}
    for row in summary_data:
        groups.setdefault(row['core_config'], []).append(row)
    
    plot_dir = os.path.join(dst_dir, "plots")
    os.makedirs(plot_dir, exist_ok=True)
    
    for core_config, rows in groups.items():
        v0_data = [r for r in rows if r['version'] == 'v0']
        v1_data = [r for r in rows if r['version'] == 'v1']
        v2_data = [r for r in rows if r['version'] == 'v2']
        
        if not (v1_data and v2_data):
            continue
        
        v1_data.sort(key=lambda r: r['processes'])
        v2_data.sort(key=lambda r: r['processes'])
        
        x_v1 = [r['processes'] for r in v1_data]
        y_v1 = [r['val3'] for r in v1_data]
        yerr_v1 = [r['err3'] for r in v1_data]
        
        x_v2 = [r['processes'] for r in v2_data]
        y_v2 = [r['val3'] for r in v2_data]
        yerr_v2 = [r['err3'] for r in v2_data]
        
        v0_value = v0_data[0]['val3'] if v0_data else None
        
        plt.figure(figsize=(8, 6))
        plt.errorbar(x_v1, y_v1, yerr=yerr_v1, fmt='-o', color='red', 
                     label='v1: without parallel i/o', capsize=5, capthick=2)
        plt.errorbar(x_v2, y_v2, yerr=yerr_v2, fmt='-o', color='blue', 
                     label='v2: with parallel i/o', capsize=5, capthick=2)
        if v0_value is not None:
            plt.axhline(y=v0_value, color='green', linestyle='dotted', linewidth=2,
                        label='v0: single process baseline')
        plt.xlabel("Number of Processes")
        plt.ylabel("Total time (s)")
        plt.title(f"Test case: {core_config}")
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plot_path = os.path.join(plot_dir, f"{core_config}_line_plot.png")
        plt.savefig(plot_path)
        plt.close()
        print(f"Saved primary plot for test case {core_config} to {plot_path}")

def generate_extra_value_plots(summary_data, dst_dir):
    groups = {}
    for row in summary_data:
        if row['version'] in ['v1', 'v2']:
            key = (row['core_config'], row['version'])
            groups.setdefault(key, []).append(row)
    
    value_plot_dir = os.path.join(dst_dir, "value_plots")
    os.makedirs(value_plot_dir, exist_ok=True)
    
    for (core_config, version), rows in groups.items():
        rows.sort(key=lambda r: r['processes'])
        x = [r['processes'] for r in rows]
        y1 = [r['avg_values'][0] for r in rows]
        y2 = [r['avg_values'][1] for r in rows]
        y3 = [r['avg_values'][2] for r in rows]
        yerr1 = [r['err_values'][0] for r in rows]
        yerr2 = [r['err_values'][1] for r in rows]
        yerr3 = [r['err_values'][2] for r in rows]
        
        plt.figure(figsize=(8, 6))
        plt.errorbar(x, y1, yerr=yerr1, fmt='-o', color='red', capsize=5, capthick=2, label="Data I/O Time")
        plt.errorbar(x, y2, yerr=yerr2, fmt='-o', color='green', capsize=5, capthick=2, label="Computation Time")
        plt.errorbar(x, y3, yerr=yerr3, fmt='-o', color='blue', capsize=5, capthick=2, label="Total Time")
        
        plt.xlabel("Number of Processes")
        plt.ylabel("Time (s)")
        
        if version == 'v1':
            suffix = " - without parallel i/o"
        elif version == 'v2':
            suffix = " - with parallel i/o"
        else:
            suffix = ""
        
        plt.title(f"Test case: {core_config} - {version}{suffix}")
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plot_path = os.path.join(value_plot_dir, f"{core_config}_{version}_values_plot.png")
        plt.savefig(plot_path)
        plt.close()
        print(f"Saved extra values plot for {core_config} {version} to {plot_path}")

def main():
    src_dirs = ['results', 'results2', 'results3']
    dst_dir = 'results_avg'
    summary_data = []
    
    for root, _, files in os.walk(src_dirs[0]):
        for file in files:
            rel_path = os.path.relpath(os.path.join(root, file), src_dirs[0])
            process_file(rel_path, src_dirs, dst_dir, summary_data)
    
    summary_data.sort(key=lambda x: (x['core_config'], x['processes'], int(x['version'][1:])))
    
    summary_path = os.path.join(dst_dir, 'summary.txt')
    with open(summary_path, 'w') as f:
        header = (f"{'Version':<8}  {'Processes':<10}  {'CoreConfig':<15}  "
                  f"{'Value1':<16}  {'Value2':<16}  {'Value3':<16}")
        f.write(header + "\n")
        f.write("-" * len(header) + "\n")
        for row in summary_data:
            values_str = "  ".join(f"{val:<16}" for val in row['values'])
            f.write(f"{row['version']:<8}  {row['processes']:<10}  {row['core_config']:<15}  {values_str}\n")
    
    print(f"\nSummary written to: {summary_path}")
    
    generate_plots(summary_data, dst_dir)
    
    generate_extra_value_plots(summary_data, dst_dir)

if __name__ == '__main__':
    main()

