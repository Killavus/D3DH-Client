import argparse
import os
from shutil import copyfile 

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Copy frames from the same batch to one directory')
    parser.add_argument('input_dir', type=str,
                        help='input directory to get frames from')
    parser.add_argument('output_dir', type=str,
                        help='output directory to save frames')
    
    args = parser.parse_args()
    if os.path.isdir(args.output_dir):
        print ("OUTPUT DIRECTORY ALREADY EXISTS. EXITING")
        exit(-1)

    os.mkdir(args.output_dir)
    
    for filename in os.listdir(args.input_dir):
        batch_it = filename.split('.')[-2].split('_')[-1]
        batch_path = args.output_dir + '/' + batch_it
        
        if not os.path.isdir(batch_path):
            os.mkdir(batch_path)
            
        copyfile(args.input_dir + '/' + filename, batch_path + '/' + filename)
