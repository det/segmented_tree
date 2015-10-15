#!/usr/bin/env python3
import csv
import subprocess
import sys

import numpy
from matplotlib import pyplot
from matplotlib import cm

bin_dir = sys.argv[1]
output_dir = sys.argv[2]

trials = 5

containers = ["segmented_tree_seq", "btree_seq", "bpt_sequence", "avl_array",
              "deque", "vector"]

single_8_labels = ["256", "7936", "246016", "7626496"]
single_8_args = [(256, 2107779313, 15865477950454414828),
                 (7936, 976634119, 3238950223561105499),
                 (246016, 3515491141, 5644467892570804156),
                 (7626496, 1106530671, 6965094249249093704)]

single_64_labels = ["32", "992", "30752", "953312"]
single_64_args = [(32, 2397254571, 4723602420748635361),
                  (992, 463092544, 12966777589746855639),
                  (30752, 430452927, 751509891372566603),
                  (953312, 3109453262, 10176667110359292238)]

range_8_labels = ["1x7626496", "31x246016", "961x7936", "29791x256"]
range_8_args = [(1, 7626496, 1319121800, 13937110459523125406),
                (31, 246016, 3037209825, 7094631787510567342),
                (961, 7936, 2984486723, 3526507821286439548),
                (29791, 256, 197924070, 9499073426478457076)]

range_64_labels = ["1x953312", "31x30752", "961x992", "29791x32"]
range_64_args = [(1, 953312, 235951511, 7803621008785366632),
                 (31, 30752, 1082972474, 11846815057285548515),
                 (961, 992, 5659033, 14482810490810820797),
                 (29791, 32, 3727649439, 10804193997107502541)]

def graph(size, kind, name, results, labels):
  N = len(labels)
  M = len(containers)
  ind = numpy.arange(N)
  border = 0.05
  width = (1.0 - border * 2) / M
  fig, ax = pyplot.subplots()

  legend_ids = []
  legend_names = []
  rects_list = []
  n = 0
  for container in containers:
    times = results[container]
    rects = ax.bar(ind + width * n + border, times, width, color=cm.jet(n / M))
    legend_ids.append(rects[0])
    legend_names.append(container)
    rects_list.append(rects)
    n += 1

  ax.set_xlabel('Container size')
  ax.set_ylabel('Milliseconds')
  ax.set_yscale('log')
  ax.set_title('%s of std::uint%d_t' % (name, size))
  ax.set_xticks(ind + width * M / 2 + border)
  ax.set_xticklabels(labels)
  ax.legend(legend_ids, legend_names, loc='upper left')

  pyplot.savefig('%s/%s-%s-%d.svg' % (output_dir,
                                   name.lower().replace(' ', '_'),
                                   kind,
                                   size))
  pyplot.close()

def raw(size, kind, name, results, labels):
  file = open('%s/%s-%s-%d.txt' % (output_dir,
                                   name.lower().replace(' ', '_'),
                                   kind,
                                   size),
               mode='w')
  file.write('#container,')
  file.write(','.join(labels))
  file.write('\n')
  for container in containers:
    times = results[container]
    file.write('%s,%s\n' % (container, ','.join([str(i) for i in times])))
  file.close()
    
def handle_sizes(size, kind, labels, args_list):
  blah = {}
  for args in args_list:
    for container in containers:
      best = {}
      for i in range(trials):
        pipe = subprocess.Popen(
            ["%s/bench_%s_%s_%d" % (bin_dir, kind, container, size)] +
                [str(i) for i in args],
            stdout = subprocess.PIPE, universal_newlines = True)

        measurements = list(csv.reader(pipe.stdout))
        for (benchmark, string) in measurements:
          ms = float(string)
          if benchmark in best: best[benchmark] = min(best[benchmark], ms)
          else: best[benchmark] = ms

      for (benchmark, ms) in best.items(): 
        if benchmark not in blah: blah[benchmark] = {}
        if container not in blah[benchmark]: blah[benchmark][container] = []
        blah[benchmark][container].append(ms)

  for (name, results) in blah.items():
    graph(size, kind, name, results, labels)
    raw(size, kind, name, results, labels)

handle_sizes(8, "single", single_8_labels, single_8_args)
handle_sizes(64, "single", single_64_labels, single_64_args)
handle_sizes(8, "range", range_8_labels, range_8_args)
handle_sizes(64, "range", range_64_labels, range_64_args)

