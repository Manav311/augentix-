import json
import numpy as np
from collections import defaultdict
import sys

fmt = '%-32s %14.3f %6.2f%%'

FNAME = sys.argv[1]
KEY = None
if len(sys.argv) == 3:
    KEY = sys.argv[2]

with open(FNAME) as fp:
    ts = json.load(fp)
    key = 'inference_measurements_#1'
    im = ts['ArmNN'][key]
    measure_t = im['Wall clock time_#%s'%key.split('#')[1]]['raw'][0]
    key = 'Execute_#2'
    ex = im[key]
    ex_t = ex['Wall clock time_#%s'%key.split('#')[1]]['raw'][0]
    func_list = []
    t_list = []
    d = defaultdict(int)
    print('Total measure time: %f(ms)'%(measure_t/1000.))
    print('Total execute time: %f(ms)'%(ex_t/1000.))
    for i in ex:
        if isinstance(ex[i], str) or ex[i]['type'] != 'Event':
            continue
        t = ex[i]['Wall clock time_#%s'%i.split('#')[1]]['raw'][0]
        func_list += [i]
        t_list += [t]
        d[i.split('_')[0]] += t
    total = np.sum(t_list)
    if KEY is not None:
        for i, f in enumerate(func_list):
            if f.split('_')[0].find(KEY) > -1:
                print("%-3d %-42s %14.6f (ms) %6.2f%%" % (i, f, (t_list[i]/1000), t_list[i]/total*100))
    print('Total time from op: %f(ms)'%(np.sum(t_list)/1000))
    for i in d:
        print(fmt%(i, d[i]/1000., d[i] / ex_t * 100))
    print(fmt%('Total', np.sum(t_list)/1000., np.sum(t_list) / ex_t * 100))
