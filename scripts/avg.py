import os
import math
import re

def extract_metadata(rel_path):
    subdir, filename = os.path.split(rel_path)
    version = subdir.strip()  # Expecting version as "v0", "v1", or "v2"
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
    values = [
        f"{avg:.4f} ({round(std)}%)"
        for avg, std in zip(avg_values, std_pct_values)
    ]
    return {
        'version': version,
        'processes': processes,
        'core_config': core_config,
        'values': values
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
    
    new_content = contents[0].copy()  # Base content from the first file
    new_content[2] = avg_line
    new_content.append(std_line)
    
    output_path = os.path.join(dst_dir, rel_path)
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'w') as f:
        f.writelines(new_content)
    
    version, processes, core_config = extract_metadata(rel_path)
    summary_data.append(format_summary_row(version, processes, core_config, avg_values, std_pct_values))

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
        header = f"{'Version':<8}  {'Processes':<10}  {'CoreConfig':<15}  {'Value1':<16}  {'Value2':<16}  {'Value3':<16}"
        f.write(header + "\n")
        f.write("-" * len(header) + "\n")
        for row in summary_data:
            values_str = "  ".join(f"{val:<16}" for val in row['values'])
            f.write(f"{row['version']:<8}  {row['processes']:<10}  {row['core_config']:<15}  {values_str}\n")
    
    print(f"\nSummary written to: {summary_path}")

if __name__ == '__main__':
    main()

