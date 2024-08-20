#!/usr/bin/env python3

import csv
import os
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

prod_data = defaultdict(list)
cons_data = defaultdict(list)

with open("prod_data.csv") as pd:
    rd = csv.reader(pd, delimiter=';')
    for row in rd:
        key = row[0]
        if key.lower() == "size":
            continue

        prod_data[key].append(float(row[-1]) / 1000.)

cons_files = [entry for entry in os.listdir('.')
              if entry.startswith('cons_data') and entry.endswith('.csv')]

for cons_file in cons_files:
    with open(cons_file) as pd:
        rd = csv.reader(pd, delimiter=';')
        for row in rd:
            key = row[0]
            if key.lower() == "size":
                continue

            cons_data[key].append(float(row[1]) / 1000.)

fig, axs = plt.subplots(nrows=len(ORDER), ncols=1, layout='constrained')

send_stats = calc_stats(prod_data)
recv_stats = calc_stats(cons_data)

for n, key in enumerate(ORDER):
    send = send_stats[key]
    recv = recv_stats[key]
    send['label'] = "Send"
    recv['label'] = "Recv"
    stats = [send, recv]
    axs[n].bxp(stats)
#    axs[n].set_yscale('symlog')
    axs[n].set_title(f"{key} array")
    axs[n].set_ylabel('Time (ms)')
    # axs[n].tick_params(axis='x', labelrotation=45)


plt.show()
