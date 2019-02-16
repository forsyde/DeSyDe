#!/usr/bin/env python
#----------------------------------------
# This script generates all the necessary basic experimentation
# files by using available templates, in case, exp_1.
#----------------------------------------

# reading arguments from the command-line
import argparse
# obvious file and folder handling.
import os
import shutil
# execute the binary given
import subprocess
# get elapsed time
import time
# get sqrt
import math
# handle XML
import xml.etree.ElementTree as ET

# get partition sets in order to test all application combinations
def partition_set_no_empty(s):
  partitions = [s]
  if len(s) > 1:
    for e in s:
      for p in partition_set_no_empty([i for i in s if i != e]):
        if not p in partitions:
          partitions.append(p)
  return partitions

# arguments from command line and their parsing
parser = argparse.ArgumentParser(description='Generate DeSyDe experiments.')
parser.add_argument('experiments', type=str, nargs='+', help='Integers or ranges (inclusive) representing the number of TDN NoCs to generate, separated by spaces. e.g. 1 4-7 6 5-20.')
args = parser.parse_args()

experiments_to_gen = set()
if len(args.experiments) > 0:
  for exp in args.experiments:
    try:
      if '-' in exp:
        splitted = exp.split('-')
        b = int(splitted[0])
        e = int(splitted[1])
        experiments_to_gen = experiments_to_gen | set(range(b, e+1))
      else:
        experiments_to_gen = experiments_to_gen | set([int(exp)])
    except ValueError as e:
      print 'Experiment argument {0} cannot be parsed. Is it correct?'.format(exp)
      quit()

experiments_to_gen = sorted(list(experiments_to_gen))
experiments_to_gen_str = [str(s) for s in experiments_to_gen]
print('Generating experiments for size: {0}'.format(' '.join(experiments_to_gen_str)))
for exp in experiments_to_gen:
  min_div = 1
  remain = exp
  for num in range(1, int(math.sqrt(exp))+1):
    if exp % num == 0:
      # dimensions of the TDN NoC in question. Later on, more parameters may be adjusted
      x = num
      y = exp // num
      print('Dimensions for {0} NoCs cores: {1} X {2}'.format(exp, x, y))
      # get all apps listed on sdfs folder
      apps = os.listdir(os.path.join('template', 'sdfs'))
      # first two letters after last _ and before first .
      apps_names = {a: (a.split('.')[0]).split('_')[-1][0:2] for a in apps}
      apps_combs = partition_set_no_empty(apps)
      # explore all combinations
      for app_comb in apps_combs:
        joint_name = '-'.join(sorted([apps_names[a] for a in app_comb]))
        print('Generating app combination: {0}'.format(joint_name))
        # count the total number of channels
        channels = 0
        for app in app_comb:
          app_xml = ET.parse(os.path.join(os.path.join('template', 'sdfs'), app))
          for c in app_xml.iter('channel'):
            channels += 1
        # with this knowdlege, start writtin the templates
        for channel_num in range(1,channels+1):
          base_folder = 'TDN-NoC/{0}/{1}x{2}/{3}/{4}'.format(exp, x, y, channel_num, joint_name)
          # setup the test folder if it does not exist
          try:
            shutil.copytree('template', base_folder)
          except:
            pass
          try:
            xml = ET.parse(os.path.join(os.path.join('template', 'xmls'), 'platform.xml'))
            for proc in xml.iter('processor'):
              proc.attrib['number'] = str(exp)
            for tdn_noc in xml.iter('TDN_NoC'):
              tdn_noc.attrib['name'] = '{0}x{1}TDN_{2}slots'.format(x, y, channel_num)
              tdn_noc.attrib['cycles'] = str(channel_num)
              tdn_noc.attrib['maxCyclesPerProc'] = str(channel_num)
              tdn_noc.attrib['x-dimension'] = str(x)
              tdn_noc.attrib['y-dimension'] = str(y)
            xml.write(os.path.join(os.path.join(base_folder, 'xmls'), 'platform.xml'))
            # delete app entries not assigned to this run and exp
            for app in apps:
              target_file = os.path.join(os.path.join(base_folder, 'sdfs'), app) 
              if not app in app_comb and os.path.exists(target_file):
                os.remove(target_file)
                with open(os.path.join('template', 'config.cfg'), 'r') as f:
                  cfg = f.read()
                  cfg = cfg.replace('template', base_folder)
                  with open(os.path.join(base_folder, 'config.cfg'), 'w') as out:
                    out.write(cfg)
          except IOError:
            raise
            quit()
  print('End generation of experiments for {0}'.format(exp))
