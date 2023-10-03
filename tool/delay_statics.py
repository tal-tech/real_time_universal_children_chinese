#!/usr/bin/env python
import sys
import os

def GetLabel(file_path):
    label = {}
    with open(file_path) as f:
        while True:
            line = f.readline().strip()
            if not line:
                break
            seqs = line.split()
            if len(seqs) != 2:
                continue
            if label.has_key(seqs[0]):
                continue
            label[seqs[0]] = float(seqs[1]) / 1000.0 # supporse label is ms

    return label

def GetRes(file_path):
    res = {}
    with open(file_path) as f:
        while True:
            line = f.readline().strip()
            if not line:
                break
            seqs = line.split()
            if len(seqs) != 2:
                continue
            if res.has_key(seqs[0]):
                res[seqs[0]].append(float(seqs[1]))
            else:
                res[seqs[0]] = []
                res[seqs[0]].append(float(seqs[1]))

    return res
def main(argv):
    if(len(argv) < 3):
        print("Delay statics tool")
        print("Usage: {} label_path res_path".format(argv[0]))
        return
    label = GetLabel(argv[1])
    res = GetRes(argv[2])
    delays = []
    oov = 0
    res_num = 0
    for item in res.keys():
        #print("{} {}".format(item, res[item]))
        if not label.has_key(item):
            #print(item)
            oov += 1
            continue
        for i in range(0, len(res[item])):
            res_num += 1
            res_delay = res[item][i]
            label_start = label[item]
            if res_delay >= label_start:
                delays.append(res_delay - label_start)
                break
    print("{} oov wav files".format(oov))
    print("{} wav files, aver_delay:{:.4f}s".format(len(delays), sum(delays) / len(delays)))

if __name__ == "__main__":
    main(sys.argv)

