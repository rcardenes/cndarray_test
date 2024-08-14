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

prod_total = defaultdict(list)
prod_data = defaultdict(list)
cons_data = defaultdict(list)
cons_total = defaultdict(list)

with open("producer_data_4.csv") as pd:
    rd = csv.reader(pd, delimiter=';')
    for row in rd:
        key = row[0]
        if key.lower() == "size":
            continue

        prod_total[key].append(sum(float(k) for k in row[2:]) / 1000.)
        prod_data[key].append(float(row[-1]) / 1000.)

with open("consumer_data_4.csv") as pd:
    rd = csv.reader(pd, delimiter=';')
    for row in rd:
        key = row[0]
        if key.lower() == "size":
            continue

        cons_data[key].append(float(row[1]) / 1000.)
        cons_total[key].append(sum(float(k) for k in row[1:]) / 1000.)

fig, axs = plt.subplots(nrows=len(ORDER), ncols=1, layout='constrained')

ptot_stats = calc_stats(prod_total)
send_stats = calc_stats(prod_data)
recv_stats = calc_stats(cons_data)
ctot_stats = calc_stats(cons_total)

for n, key in enumerate(ORDER):
    ptot = ptot_stats[key]
    send = send_stats[key]
    recv = recv_stats[key]
    ctot = ctot_stats[key]
    ptot['label'] = "Ser+Send"
    send['label'] = "Send"
    recv['label'] = "Recv"
    ctot['label'] = "Recv+Parse"
    stats = [ptot, send, recv, ctot]
    axs[n].bxp(stats)
#    axs[n].set_yscale('symlog')
    axs[n].set_title(f"{key} array")
    axs[n].set_ylabel('Time (ms)')
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
