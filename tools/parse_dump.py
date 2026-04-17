import sys
import subprocess
import os

def parse_dump(dump_path, pdb_dir):
    if not os.path.exists(dump_path):
        print(f"Dump file not found: {dump_path}")
        return
        
    # Using CDB (Microsoft Console Debugger) which is usually installed with Windows SDK
    cdb_path = r"C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\cdb.exe"
    if not os.path.exists(cdb_path):
        print("CDB not found. Please install Windows SDK Debugging Tools.")
        return
        
    # .sympath setup, .reload to load symbols, !analyze -v to analyze crash, q to quit
    cmd = f'"{cdb_path}" -y "{pdb_dir}" -z "{dump_path}" -c "!analyze -v; q"'
    
    print(f"Analyzing dump {dump_path} with PDBs from {pdb_dir}...")
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        
        # Extract the interesting part from the output
        output = result.stdout
        if "STACK_TEXT:" in output:
            stack_text = output.split("STACK_TEXT:")[1].split("SYMBOL_NAME:")[0]
            print("\n--- CRASH CALL STACK ---")
            print(stack_text.strip())
            print("------------------------\n")
        else:
            print("Could not extract call stack. Full output:")
            print(output[:1000] + "...\n(Truncated)")
            
    except Exception as e:
        print(f"Failed to analyze dump: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python parse_dump.py <path_to_dmp> <path_to_pdb_dir>")
        sys.exit(1)
        
    parse_dump(sys.argv[1], sys.argv[2])
