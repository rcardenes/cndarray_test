#!/usr/bin/env python3

import csv
from collections import defaultdict

import numpy as np
import matplotlib
from matplotlib import pyplot as plt

ORDER = ("128x128", "256x256", "512x512")

def calc_stats(data):
    stats = {}

    for key in ORDER:
        kdata = np.array(data[key])

        med = np.median(kdata)
        q1 = np.percentile(kdata, 25)
        q3 = np.percentile(kdata, 75)
        s = kdata.std() * 3
        lo = med - s
        hi = med + s
        no_outliers = kdata[np.logical_and(kdata >= lo, kdata <= hi)]
        whislo = no_outliers.min()
        whishi = no_outliers.max()
        fliers = kdata[np.logical_or(kdata < lo, kdata > hi)]
        stats[key] = dict(
            med=med,
            q1=q1,
            q3=q3,
            whislo=whislo,
            whishi=whishi,
            fliers = ())

    return stats

prod_total = [defaultdict(list), defaultdict(list), defaultdict(list), defaultdict(list)]
prod_data = [defaultdict(list), defaultdict(list), defaultdict(list), defaultdict(list)]

with open("producer_data_7.csv") as pd:
    rd = csv.reader(pd, delimiter=';')
    for row in rd:
        key = row[0]
        if key.lower() == "size":
            continue

        prod_total[0][key].append(sum(float(k) for k in row[2:]) / 1000.)
        prod_data[0][key].append(float(row[-1]) / 1000.)

with open("parallel_producer_data_9.csv") as pd:
    rd = csv.reader(pd, delimiter=';')
    for row in rd:
        key = row[0]
        if key.lower() == "size":
            continue

        prod_total[1][key].append(sum(float(k) for k in row[2:]) / 1000.)
        prod_data[1][key].append(float(row[-1]) / 1000.)

with open("parallel_producer_data_10.csv") as pd:
    rd = csv.reader(pd, delimiter=';')
    for row in rd:
        key = row[0]
        if key.lower() == "size":
            continue

        prod_total[2][key].append(sum(float(k) for k in row[2:]) / 1000.)
        prod_data[2][key].append(float(row[-1]) / 1000.)

with open("parallel_producer_data_11.csv") as pd:
    rd = csv.reader(pd, delimiter=';')
    for row in rd:
        key = row[0]
        if key.lower() == "size":
            continue

        prod_total[3][key].append(sum(float(k) for k in row[2:]) / 1000.)
        prod_data[3][key].append(float(row[-1]) / 1000.)



fig, axs = plt.subplots(nrows=len(ORDER), ncols=4, layout='constrained', sharey='row')

ptot_stats = [calc_stats(p) for p in prod_total]
send_stats = [calc_stats(p) for p in prod_data]

for i, key in enumerate(ORDER):
    axs[i][0].set_ylabel(f'{key}\nTime (ms)')

for i, title in enumerate(['Serial', '16 threads', '32 threads', '64 threads']):
    axs[0][i].set_title(title)

for m, key in enumerate(ORDER):
    for n, (ptot, send) in enumerate(zip(ptot_stats, send_stats)):
        ptot = ptot[key]
        send = send[key]
        ptot['label'] = "Ser+Send"
        send['label'] = "Send"
        stats = [ptot, send]
        axs[m][n].bxp(stats)
#    axs[n].set_yscale('symlog')
#        axs[m][n].set_title(f"{key} array")
#        axs[m][n].set_ylabel('Time (ms)')
    # axs[n].tick_params(axis='x', labelrotation=45)

# axs[0].bxp(stats)
# axs[0].set_title('Producer (total)')
# axs[0].set_yscale('log')
# axs[0].set_ylabel('Time (ms)')
# axs[0].tick_params(axis='x', labelrotation=45)
#
# axs[1].bxp(stats)
# axs[1].set_title('Producer (send)')
# axs[1].set_yscale('log')
# axs[1].tick_params(axis='x', labelrotation=45)
#
# axs[2].bxp(stats)
# axs[2].set_title('Consumer (recv)')
# axs[2].set_yscale('log')
# axs[2].tick_params(axis='x', labelrotation=45)

plt.show()
