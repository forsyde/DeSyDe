#!/usr/bin/env python
#----------------------------------------
# This script generates all the necessary basic experimentation
# files by using available templates. It also runs then as
# given by the user.
#----------------------------------------

# reading arguments from the command-line
import argparse
# obvious file and folder handling.
import os
# execute the binary given
import subprocess
# get elapsed time and wait for mutex
import time


# arguments from command line and their parsing
parser = argparse.ArgumentParser(description='Manage and execute DeSyDe experiments.')
parser.add_argument('bin', type=str, nargs=1, help='specify the DeSyDe binary to be used as seen from the bin_path option.')
parser.add_argument('--platform', type=str, default="TDN-NoC", help="Select which kind of platform the tests should be ran on. Possible arguments: 'TDN-NoC'")
parser.add_argument('--size', type=str, help="Select the size of the chosen platform, e.g. X and Y for TDN NoC ones. Separate multiple values with comma.")
parser.add_argument('--slots', type=str, help="Select the TDN slots of the chosen platform, e.g. 2 for TDN NoC ones. Separate multiple values with comma.")
parser.add_argument('--applications', type=str, help="Select the combination of applications the test should run, e.g. 'so-cy-ra'. Can be given out of order. Separate multiple values with comma.")
# parser.add_argument('experiments', type=str, nargs='*', help='Integers or ranges (inclusive) representing the experiment to run, separated by spaces. e.g. 1 4-7 6 5-20. No specification means to run all found in current folder.')
parser.add_argument('-bp', '--bin_path', type=str, default='/var/forsyde/bin', help="the folder in which the DeSyDe binaries can be found. Default: /var/forsyde/bin")
args = parser.parse_args()

# bulk of the script: find the bin, get experiments and run them.
original_dir = os.getcwd()
if not os.path.exists(os.path.join(args.bin_path, args.bin[0])):
  print('A DeSyDe binary could not be found on the path provided.')
  quit()
bin_dir = os.path.join(args.bin_path, args.bin[0])
print('Running with binary {0}'.format(bin_dir))

print('Checking and waiting for mutex file to be clear')
while os.path.exists('run.lock'):
  time.sleep(60)  

# create mutex file
open('run.lock', 'w').close()
# run stuff
if args.platform == "TDN-NoC":
  experiments_present = dict()
  for root, dirs, fs in os.walk("TDN-NoC"):
    if "config.cfg" in fs:
      info = root.split(os.path.sep)
      # add the experiments present based on a tuple of their size and app combination
      experiments_present[tuple(info)] = root

  print('Num of experiments found: {0}'.format(len(experiments_present)))
  experiments_to_run = experiments_present.keys()
  # support for multiple sizes is done via commas
  if args.size:
    sizes = args.size.split(",")
    experiments_to_run = [k for k in experiments_to_run if k[2] in sizes]
  if args.applications:
    apps = args.applications.split(",")
    experiments_to_run = [k for k in experiments_to_run if k[4] in apps]
  if args.slots:
    slots = args.slots.split(",")
    experiments_to_run = [k for k in experiments_to_run if k[3] in slots]
  print('Running {0} experiments.'.format(len(experiments_to_run)))
  for exp in experiments_to_run:
    exp_name = experiments_present[exp]
    runs = [int(f.split('-')[-1]) for f in os.listdir(exp_name) if 'run' in f and '-' in f]
    if len(runs) > 0:
      run = max(runs) + 1
    else:
      run = 1
    print('Running experiment {0} on run {1}'.format(exp, run))
    run_folder = os.path.join(exp_name, "run-{0}".format(run))
    try:
      os.makedirs(os.path.join(run_folder, 'out'))
    except:
      pass
    start = time.time()
    subprocess.call([
      bin_dir,
      '--config', 
      '{0}/config.cfg'.format(exp_name), 
      '--dse.th_prop',
      'MCR',
      '--output',
      '{0}/'.format(run_folder) 
    ])
    duration = time.time() - start
    print('Duration: {0} minutes and {1} seconds.'.format(int(duration) // 60, int(duration) % 60))
    print('End of experiment {0} on run {1}'.format(exp, run))
else:
  print("Platform given not implemented. Stopping execution.")
# delete mutex file
if os.path.exists('run.lock'):
  os.remove('run.lock')
