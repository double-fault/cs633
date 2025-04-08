import numpy as np

def count_local_extrema_and_globals(filename, nx, ny, nz):
    """
    Reads a file with nx*ny*nz rows and k columns (one column per volume),
    reshapes each column into a 3D volume (assuming file order: (x, y, z) with x varying fastest),
    and for each volume calculates:
      - Count of local minima (only interior voxels)
      - Count of local maxima (only interior voxels)
      - Global minimum (over all voxels)
      - Global maximum (over all voxels)
    """
    # Load data from file.
    data = np.loadtxt(filename)
    num_voxels, k = data.shape
    if num_voxels != nx * ny * nz:
        raise ValueError(f"Expected {nx*ny*nz} rows, but got {num_voxels}.")
    
    results = []  # will store tuples: (minima_count, maxima_count, global_min, global_max)
    
    for col in range(k):
        # Reshape column data into a 3D volume.
        # The file order is (x, y, z) with x varying fastest.
        volume = data[:, col].reshape((nz, ny, nx)).transpose(2, 1, 0)
        
        # Global min and max (over the entire volume, including boundaries)
        global_min = volume.min()
        global_max = volume.max()
        
        # Only consider interior voxels (indices 1 to -1 along each dimension)
        inner = volume[1:-1, 1:-1, 1:-1]
        left   = volume[:-2, 1:-1, 1:-1]
        right  = volume[2:, 1:-1, 1:-1]
        down   = volume[1:-1, :-2, 1:-1]
        up     = volume[1:-1, 2:, 1:-1]
        back   = volume[1:-1, 1:-1, :-2]
        front  = volume[1:-1, 1:-1, 2:]
        
        # Check for local minima (interior only)
        is_minima = (inner < left) & (inner < right) & \
                    (inner < down) & (inner < up) & \
                    (inner < back) & (inner < front)
                    
        # Check for local maxima (interior only)
        is_maxima = (inner > left) & (inner > right) & \
                    (inner > down) & (inner > up) & \
                    (inner > back) & (inner > front)
        
        minima_count = np.sum(is_minima)
        maxima_count = np.sum(is_maxima)
        
        results.append((minima_count, maxima_count, global_min, global_max))
    
    return results

# Example usage:
if __name__ == "__main__":
    # Set your volume dimensions and the input file name.
    nx, ny, nz = 64, 64, 96  # dimensions of each volume
    filename = "data_64_64_96_7.txt"
    
    # Get results for each volume.
    results = count_local_extrema_and_globals(filename, nx, ny, nz)
    
    for i, (minima_count, maxima_count, global_min, global_max) in enumerate(results, start=1):
        print(f"Volume {i}:")
        print(f"  Local Minima Count (interior only): {minima_count}")
        print(f"  Local Maxima Count (interior only): {maxima_count}")
        print(f"  Global Minimum Value: {global_min}")
        print(f"  Global Maximum Value: {global_max}")

