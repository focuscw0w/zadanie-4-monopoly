import subprocess
import os

def read_file(file_path):
    with open(file_path, 'r') as file:
        return file.read()

def preprocess_output(output):
    return ' '.join(output.split())

def compare_outputs(output1, output2):
    lines_output1 = output1.split('\n')
    lines_output2 = output2.split('\n')
    
    while lines_output1 and not lines_output1[-1]:
        lines_output1.pop()
    while lines_output2 and not lines_output2[-1]:
        lines_output2.pop()

    num_lines_output1 = len(lines_output1)
    num_lines_output2 = len(lines_output2)
    
    if num_lines_output1 != num_lines_output2:
        return False, num_lines_output1, num_lines_output2, None, None
    
    for line1, line2 in zip(lines_output1, lines_output2):
        if preprocess_output(line1) != preprocess_output(line2):
            return False, num_lines_output1, num_lines_output2, line1, line2
    
    return True, num_lines_output1, num_lines_output2, None, None

def run_tests(exe_path, test_folder, print_output=False):
    test_files = [f.path for f in os.scandir(test_folder) if f.is_file() and f.name.startswith('stdin_')]
    
    if not test_files:
        print("No input files found in the specified test folder.")
        return
    
    test_files.sort()
    
    for stdin_path in test_files:
        stdout_path = stdin_path.replace("stdin_", "stdout_")
        parameters_path = stdin_path.replace("stdin_", "parameters_")
        
        if not (os.path.exists(stdout_path) and os.path.exists(parameters_path)):
            print(f"Missing files for test case {stdin_path}. Skipping.")
            continue
        
        input_data = read_file(stdin_path)
        expected_output = read_file(stdout_path).strip()
        parameters = read_file(parameters_path).strip()
        
        process = subprocess.Popen([exe_path] + parameters.split(), 
                                   stdin=subprocess.PIPE, 
                                   stdout=subprocess.PIPE, 
                                   stderr=subprocess.PIPE, 
                                   text=True)
        output, errors = process.communicate(input=input_data)
        
        if process.returncode != 0:
            print(f"Test case {stdin_path}: Execution failed with error:\n{errors}")
            continue
        
        result, num_lines_output, num_lines_expected, diff_output, diff_expected = compare_outputs(output, expected_output)
        if result:
            print(f"\n\nTEST - {stdin_path} - PASSED")
        else:
            print(f"\n\nTEST - {stdin_path} - FAILED")
            print(f"\n-- YOUR OUTPUT --\n{diff_output}")
            print(f"\n-- EXPECTED OUTPUT --\n{diff_expected}")
            print(f"\nNumber of lines in your output: {num_lines_output}")
            print(f"Number of lines in expected output: {num_lines_expected}")
        
        if print_output:
            print("\nOutput:")
            print(output)

run_tests('/Users/filipj169/Documents/Personal/Škola/1.ročník - ls/prog2/zadanie 4/src/z4', 's1', print_output=False)
#run_tests('/Users/gabrielkanocz/Desktop/z4_tester/z4', 's2', print_output=False)
#run_tests('/Users/gabrielkanocz/Desktop/z4_tester/z4', 's3', print_output=False)
#run_tests('/Users/gabrielkanocz/Desktop/z4_tester/z4', 's4', print_output=False)
#run_tests('/Users/gabrielkanocz/Desktop/z4_tester/z4', 's5', print_output=False)
#run_tests('/Users/gabrielkanocz/Desktop/z4_tester/z4', 's6', print_output=False)
#run_tests('/Users/gabrielkanocz/Desktop/z4_tester/z4', 's7', print_output=False)
#run_tests('/Users/gabrielkanocz/Desktop/z4_tester/z4', 's8', print_output=False)
#run_tests('/Users/gabrielkanocz/Desktop/z4_tester/z4', 's9', print_output=False)

