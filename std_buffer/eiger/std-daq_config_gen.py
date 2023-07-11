import argparse
import json
import re
import unittest


def parse_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    config = {
        'detsize': None,
        'hostnames': None,
        'dr': None,
        'udp_dstport': None,
        'deactivate': []
    }
    
    for line in lines:
        if line.startswith('detsize') and config['detsize'] is None:
            config['detsize'] = re.findall(r'\d+', line)
        elif line.startswith('hostname'):
            config['hostnames'] = re.findall(r'beb\d+', line)
        elif 'dr' in line:
            match = re.search(r'dr\s+(\S.*)', line)
            if match:
                config['dr'] = int(match.group(1))
        elif 'udp_dstport' in line and config['udp_dstport'] is None:
            match = re.search(r'udp_dstport\s+(\d+)', line)
            if match:
                config['udp_dstport'] = int(match.group(1))
        elif ':activate 0' in line:
            match = re.search(r'(\d+):activate 0', line)
            config['deactivate'].append(int(match.group(1)))
    return config


def generate_output_file(output_file, config, indexes, image_pixel_height, image_pixel_width, total_rows, total_columns):
    n_modules = len(config['hostnames']) * 2
    n_ms = int(len(config['hostnames'])/4)
    output_data = {
        "detector_name": f'EG{n_ms}M',
        "detector_type": "eiger",
        "n_modules": n_modules,
        "bit_depth": config['dr'],
        # Y and X in the detector vs std-daq are inverted: image is rotaate.
        "image_pixel_height": int(image_pixel_width),
        "image_pixel_width": int(image_pixel_height),
        "start_udp_port": config['udp_dstport'],
        "writer_user_id": 21206, # 21206 defaulted for now
        "submodule_info": [],
        "module_positions": {}
    }

    submodules_per_module = 2

    # clean up deactivate list
    remove_modules = []
    for deact_module in config['deactivate']:
        remove_modules.append(deact_module*2)
        remove_modules.append(deact_module*2+1)
    output_data['deactivate'] = remove_modules
    
    list_module_positions = get_module_xy_position(len(indexes),int(image_pixel_height), int(image_pixel_width), total_rows, total_columns)
    # remove deactivate modules from module_positions
    for module in remove_modules:
        list_module_positions.pop(module)
    # Generate submodule information
    for i, index in enumerate(indexes):
        hostname = config['hostnames'][i // submodules_per_module]
        submodule_info = {
            "hostname": hostname,
            "submodule": i,
            "port": int(config['udp_dstport']) + i,
            "row": index[0],
            "column": index[1],
            "activate": 1
        }
        if i in remove_modules:
            submodule_info['activate'] = 0

        output_data["submodule_info"].append(submodule_info)
    output_data['module_positions'] = list_module_positions

    # Write output file in JSON format
    with open(output_file, 'w') as file:
        json.dump(output_data, file, indent=4)

    print(f'Std-daq config file written to {output_file}')


def get_submodule_row_col(mod_index, index, total_size_x, total_size_y):
    submodule_size_x = 256
    submodule_size_y = 512

    module_size_x = 256
    module_size_y = 1024

    # submodule col is the same as module col
    submodule_col = mod_index % (total_size_x // submodule_size_x)

    # submodule row
    module_row = mod_index * module_size_x // total_size_x
    submodule_row = (module_row * 2) + (index % 2)
    

    return submodule_row, submodule_col


def calculate_submodule_index(detsize, hostnames):
    total_size_x = int(detsize[1])
    total_size_y = int(detsize[0])

    submodules_per_module = 2

    indexes = []
    for i, hostname in enumerate(hostnames):
        for submodule_index in range(i * submodules_per_module, (i + 1) * submodules_per_module):
            indexes.append(get_submodule_row_col(i, submodule_index, total_size_x, total_size_y))

    return indexes

def get_columns_total_size(n_columns, initial_size):
    x_small_gap = 2
    x_big_gap = 36
    total_size = initial_size  # Assign to a new variable
    for column in range(n_columns):
        if column % 2 == 1 and column < n_columns - 1:
            total_size += x_big_gap
        elif column % 2 == 0 and column < n_columns - 1:
            total_size += x_small_gap
    return total_size


def get_rows_total_size(n_rows, initial_size):
    y_small_gap = 2
    y_big_gap = 8
    total_size = initial_size
    for row in range(n_rows):
        if row % 2 == 1 and row < n_rows - 1:
            total_size += y_big_gap 
        elif row % 2 == 0:
            total_size += 3 * y_small_gap 
    return total_size

def calculate_total_image_size(detsize, columns, rows):
    total_size_x = int(detsize[1])
    total_size_y = int(detsize[0])
    total_size_x = get_columns_total_size(columns, total_size_x)
    total_size_y = get_rows_total_size(rows, total_size_y)
    return total_size_y, total_size_x

def get_module_xy_position(n_modules, total_size_x, total_size_y, total_rows, total_columns):
    # Calculate gaps
    x_small_gap = 2
    x_big_gap = 8
    y_small_gap = 2
    y_big_gap = 36
    
    # Calculate module size
    module_size_x = 512
    module_size_y = 256
    
    # Initialize variables and data structures
    increase_column = 0
    dict_results = {}
    current_x1 = 0
    current_x2 = module_size_x + x_small_gap - 1
    current_y1 = total_size_y - 1
    current_y2 = current_y1 - module_size_y + 1
    row = total_rows
    column = 0
    raw_add = 1
    y_big_counter = 0
    
    # Iterate over each module:
    # Starting in the lower left corner add first module
    # Second module goes on top
    # after two modules there in a vertical gap
    # after 12 modules the column is full, start a new column to the right (with horizontal gap)

    for i_mod in range(n_modules):
        if raw_add != 1:
            if i_mod % 2 == 0:  # Even modules
                if row != 0 and y_big_counter == 1:  # Add big y_big_gap
                    current_y1 = right_current_y1 - y_big_gap - 1
                    current_y2 = current_y1 - module_size_y + 1
                    dict_results[i_mod] = [current_x1, current_y2, current_x2, current_y1]
                    y_big_counter = 0
                else:
                    current_y2 = current_y2 - y_small_gap - 1
                    current_y1 = current_y2 - module_size_y + 1
                    dict_results[i_mod] = [current_x1, current_y2, current_x2, current_y1]
                    y_big_counter += 1
                    row -= 1
            else:  # Internal right side of the module (index + 1 same row and +1 column)
                column += 1
                right_current_x1 = current_x2 + x_small_gap + 1
                right_current_x2 = right_current_x1 + module_size_x + x_small_gap - 1
                right_current_y1 = current_y1
                right_current_y2 = current_y2
                dict_results[i_mod] = [right_current_x1, right_current_y2, right_current_x2, right_current_y1]
                column -= 1
            if row == 0:  # Last row
                if increase_column == 1:
                    row = total_rows
                    column += 2
                    current_y1 = total_size_y - 1
                    current_y2 = current_y1 - module_size_y + 1
                    current_x1 = right_current_x2 + x_big_gap + 1
                    current_x2 = current_x1 + module_size_x + x_small_gap - 1
                    y_big_counter = 0
                    increase_column = 0
                    raw_add = 1
                else:
                    increase_column = 1
        else:
            raw_add = 0
            dict_results[i_mod] = [current_x1, current_y2, current_x2, current_y1]
    
    # Return the dictionary of module positions
    return dict_results

def get_row_column(lst):
    rows = max(lst, key=lambda x: x[0])[0] + 1
    columns = max(lst, key=lambda x: x[1])[1] + 1
    return columns, rows

def main():
    parser = argparse.ArgumentParser(description='Std-DAQ Eiger configuration generator')
    parser.add_argument('input_file', type=str, help='Path to the Eiger detector config file')
    parser.add_argument('output_file', type=str, help='Path to the output file')

    args = parser.parse_args()
    input_file = args.input_file
    output_file = args.output_file
    
    config = parse_file(input_file)
    try:
        indexes = calculate_submodule_index(config['detsize'], config['hostnames'])
    except TypeError:
        raise ValueError("Error: 'detsize' or 'hostnames' cannot be None. Terminating the program.")

    columns, rows = get_row_column(indexes)
    image_pixel_height,image_pixel_width = calculate_total_image_size(config['detsize'], columns, rows)
    generate_output_file(output_file, config, indexes, image_pixel_height, image_pixel_width, rows, columns)



class SubmoduleIndexCalculatorTest(unittest.TestCase):
    def test_9M(self):
        detsize = ['3072', '3072']
        hostnames = ['beb0', 'beb1', 'beb2', 'beb3', 'beb4', 'beb5', 'beb6', 'beb7', 'beb8', 'beb9', 'beb10', 'beb11',
                    'beb12', 'beb13', 'beb14', 'beb15', 'beb16', 'beb17', 'beb18', 'beb19', 'beb20', 'beb21', 'beb22', 'beb23', 
                    'beb24', 'beb25', 'beb26', 'beb27', 'beb28', 'beb29', 'beb30', 'beb31', 'beb32', 'beb33', 'beb34', 'beb35']

        # row, column
        expected_indexes = [
                    (0, 0), (1, 0), (0, 1), (1, 1), (0, 2), (1, 2), (0, 3), (1, 3), (0, 4), (1, 4), (0, 5), (1, 5), (0, 6), (1, 6), (0, 7), (1, 7), (0, 8), (1, 8), (0, 9), (1, 9), (0, 10), (1, 10), (0, 11), (1, 11), 
                    (2, 0), (3, 0), (2, 1), (3, 1), (2, 2), (3, 2), (2, 3), (3, 3), (2, 4), (3, 4), (2, 5), (3, 5), (2, 6), (3, 6), (2, 7), (3, 7), (2, 8), (3, 8), (2, 9), (3, 9), (2, 10), (3, 10), (2, 11), (3, 11), 
                    (4, 0), (5, 0), (4, 1), (5, 1), (4, 2), (5, 2), (4, 3), (5, 3), (4, 4), (5, 4), (4, 5), (5, 5), (4, 6), (5, 6), (4, 7), (5, 7), (4, 8), (5, 8), (4, 9), (5, 9), (4, 10), (5, 10), (4, 11), (5, 11)]

        indexes = calculate_submodule_index(detsize, hostnames)
        self.assertEqual(indexes, expected_indexes)

    def test_6M(self):
        detsize = ['2048', '3072']
        hostnames = ['beb0', 'beb1', 'beb2', 'beb3', 'beb4', 'beb5', 'beb6', 'beb7', 'beb8', 'beb9', 'beb10', 'beb11',
                    'beb12', 'beb13', 'beb14', 'beb15', 'beb16', 'beb17', 'beb18', 'beb19', 'beb20', 'beb21', 'beb22', 'beb23'] 

        # row, column
        expected_indexes = [
            (0, 0), (1, 0), (0, 1), (1, 1), (0, 2), (1, 2), (0, 3), (1, 3), (0, 4), (1, 4), (0, 5), (1, 5), (0, 6), (1, 6), (0, 7), (1, 7), (0, 8), (1, 8), (0, 9), (1, 9), (0, 10), (1, 10), (0, 11), (1, 11), 
            (2, 0), (3, 0), (2, 1), (3, 1), (2, 2), (3, 2), (2, 3), (3, 3), (2, 4), (3, 4), (2, 5), (3, 5), (2, 6), (3, 6), (2, 7), (3, 7), (2, 8), (3, 8), (2, 9), (3, 9), (2, 10), (3, 10), (2, 11), (3, 11)]

        indexes = calculate_submodule_index(detsize, hostnames)
        self.assertEqual(indexes, expected_indexes)

    def test_3M(self):
        detsize = ['1024', '3072']
        hostnames = ['beb0', 'beb1', 'beb2', 'beb3', 'beb4', 'beb5', 'beb6', 'beb7', 'beb8', 'beb9', 'beb10', 'beb11'] 

        # row, column
        expected_indexes = [
            (0, 0), (1, 0), (0, 1), (1, 1), (0, 2), (1, 2), (0, 3), (1, 3), (0, 4), (1, 4), (0, 5), (1, 5), (0, 6), (1, 6), (0, 7), (1, 7), (0, 8), (1, 8), (0, 9), (1, 9), (0, 10), (1, 10), (0, 11), (1, 11)]

        indexes = calculate_submodule_index(detsize, hostnames)
        self.assertEqual(indexes, expected_indexes)

    def test_get_columns_total_size(self):
        n_columns_list = [i for i in range(12,0,-1)]
        initial_sizes_list = [size for size in range(3072,0,-256)]
        expected_results_list = [3264, 3006, 2714, 2456, 2164, 1906, 1614, 1356, 1064, 806, 514, 256]
        for index, n_columns in enumerate(n_columns_list):
            self.assertEqual(get_columns_total_size(n_columns, initial_sizes_list[index]), expected_results_list[index])

    def test_get_rows_total_size(self):
        n_rows_list = [i for i in range(6,0,-2)]
        initial_sizes_list = [size for size in range(3072,0,-1024)]
        expected_results_list = [3106, 2068, 1030]
        for index, n_row in enumerate(n_rows_list):
            self.assertEqual(get_rows_total_size(n_row, initial_sizes_list[index]), expected_results_list[index])

if __name__ == '__main__':
    # Run the unit tests
    # unittest.main()
    main()

