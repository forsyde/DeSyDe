#!/usr/bin/env python

#----------------------------------------
# This script processes all experiments
# given by the user and outputs then in a pandas table
#----------------------------------------
#----------------------------------------
# Author: Rodolfo Jordao <jordao@kth.se>
# Copyright: Copyright 2019, Rodolfo Jordao
# License: MIT <https://opensource.org/licenses/MIT>
# Version: 0.1.1
#----------------------------------------

# reading arguments from the command-line
import argparse
# obvious file and folder handling.
import os
# execute the binary given
import subprocess
# get elapsed time
import time
# do the data!
import pandas as pd
# regular exp to catch results
import re
# read XML files
import xml.etree.ElementTree as ET

# helper func
def ms_to_time_tuple(time_ms):
  time_s = time_ms // 1000
  time_ms = time_ms % 1000
  time_min = time_s // 60
  time_s = time_s % 60
  time_h = time_min // 60
  time_min = time_min % 60
  return "{0:2d}:{1:2d}:{2:2d}.{3:3d}".format(time_h, time_min, time_s, time_ms)


# arguments from command line and their parsing
parser = argparse.ArgumentParser(description='Process DeSyDe experiments.')
parser.add_argument('--platform', type=str, default="TDN-NoC", help="Select which kind of platform the tests should be ran on. Possible arguments: 'TDN-NoC'")
parser.add_argument('--size', type=str, help="Select the size of the chosen platform, e.g. X and Y for TDN NoC ones. Separate multiple values with comma.")
parser.add_argument('--slots', type=str, help="Select the TDN slots of the chosen platform, e.g. 2 for TDN NoC ones. Separate multiple values with comma.")
parser.add_argument('--applications', type=str, help="Select the combination of applications the test should run, e.g. 'so-cy-ra'. Can be given out of order. Separate multiple values with comma.")
parser.add_argument('--sort', type=str, default='P,TDN-slots,mesh,sols-found', help="Select the sorting fields, in order, for the table output. Default: 'P,TDN-slots,mesh,sols-found'. Separate multiple values with comma.")
args = parser.parse_args()

if args.platform == "TDN-NoC":
  print('Finding all possible experiments...')
  experiments_present = dict()
  for root, dirs, fs in os.walk("TDN-NoC"):
    if "config.cfg" in fs:
      info = root.split(os.path.sep)
      # add the experiments present based on a tuple of their size and app combination
      experiments_present[tuple(info)] = root

  print('Num of experiments found: {0}'.format(len(experiments_present)))
  experiments_to_run = experiments_present.keys()
  results = {
    'platform': [],
    'P': [],
    'mesh': [],
    'TDN-slots': [],
    'apps': [],
    'actors': [],
    'channels': [],
    'branches': [],
    'reductions': [],
    'run': [],
    'sols-found': [],
    'runtime': [],
    'timeout': [],
    'first': [],
    'last': [],
  }
  for exp in experiments_to_run:
    exp_name = experiments_present[exp]
    runs = [os.path.join(exp_name, f) for f in os.listdir(exp_name) if 'run' in f and '-' in f]
    outs = [os.path.join(r, os.path.join('out', 'out.txt')) for r in runs]
    non_empty = [o for o in outs if os.path.isfile(o) and os.path.getsize(o) > 0]
    if len(non_empty) > 0:
      # get some characteristics of each experiments, such as actor number and channel number
      actors = 0
      channels = 0
      branches = dict()
      reductions = dict()
      for root, folders, apps in os.walk(exp_name):
        if 'sdfs' in root:
          for app in apps:
            xml = ET.parse(os.path.join(root, app))
            actors += sum(1 for i in xml.iter('actor'))
            for channel in xml.iter('channel'):
              channels += 1
              if not channel.attrib['srcActor'] in branches:
                branches[channel.attrib['srcActor']] = set()
              branches[channel.attrib['srcActor']] = branches[channel.attrib['srcActor']].union({channel.attrib['dstActor']})
              if not channel.attrib['dstActor'] in reductions:
                reductions[channel.attrib['dstActor']] = set()
              reductions[channel.attrib['dstActor']] = reductions[channel.attrib['dstActor']].union({channel.attrib['srcActor']})
    for o in non_empty:
      with open(o) as f:
        text = f.read()
        # some tests may still be running, do a minor check
        if 'ended' in text:
          results['platform'].append(exp[0])
          results['P'].append(exp[1])
          results['mesh'].append(exp[2])
          results['TDN-slots'].append(exp[3])
          results['apps'].append(exp[4])
          results['actors'].append(actors)
          results['channels'].append(channels)
          results['branches'].append(sum(1 for i in branches if len(branches[i]) > 1))
          results['reductions'].append(sum(1 for i in reductions if len(reductions[i]) > 1))
          run_num = int(re.search('run-(\d+)', o).group(1))
          results['run'].append(run_num)
          sols = int(re.search('(\d) solutions found', text).group(1))
          runtime = int(re.search('search ended after: (\d+) s', text).group(1))
          results['sols-found'].append(sols)
          results['runtime'].append(ms_to_time_tuple(1000*runtime))
          results['timeout'].append('due to time-out' in text)
          if sols > 0:
            m = re.findall('Solution number: (\d+), after (\d+) ms', text)
            time = ms_to_time_tuple(int(m[0][1]))
            results['first'].append(time)
            if len(m) > 1:
              time = ms_to_time_tuple(int(m[1][1]))
              results['last'].append(time)
            else:
              results['last'].append('00:00:00.000')
          else:
            results['first'].append('00:00:00.000')
            results['last'].append('00:00:00.000')
  results = pd.DataFrame(results)
  if args.size:
    results = results[results.mesh.isin(args.size.split(','))]
  if args.applications:
    results = results[results.apps.isin(args.applications.split(','))]
  if args.slots:
    results = results[results['TDN-slots'].isin(args.slots.split(','))]
  results = results.sort_values(by=args.sort.split(','))
  print(results)
else:
  print("Platform given not implemented. Stopping execution.")
