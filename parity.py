#!/usr/bin/env python3

"""
Calculates the correlation for N pair, up to N_max, using the parity criteria.

2016.11.22 Alessandro Cere
"""


import glob
import numpy as np
import subprocess

from math import pi
from uncertainties import unumpy


sink_file = 'par_chsh.dat'
sink_file_err = 'par_chsh_err.dat'
N_max = 20

angles = [x / 100. for x in range(0, 15, 2)] + \
    [.15] + [x / 100. for x in range(16, 25, 2)]
angles = np.array([angle / pi * 180 for angle in angles])


def pp_corr(filename, N=3):
    cmd = './parity {} '.format(N) + filename
    a = subprocess.check_output(cmd, shell=True)
    return float(a.strip())


def bell_value(a0b0, a0b1, a1b0, a1b1, corr, N=1):
    """
    returns bell-operator estimated value
    for a given correlation function
    """
    return np.sum([corr(k, N) * s
                   for k, s
                   in zip([a0b0, a0b1, a1b0, a1b1],
                          [1, -1, 1, 1])])


def bell_value_err(a0b0_file, a0b1_file, a1b0_file, a1b1_file, corr, N=1,
                   trials=1):
    """
    returns bell-operator estimated value
    for a given correlation function
    including an estimate of the errors using the bootstrapping technique
    """
    files = [a0b0_file, a0b1_file, a1b0_file, a1b1_file]
    # To prevent unnecessary shuffling of single bit cases
    if N == 1:
        trials = 1
    correlations = [[corr(k, N) for _ in range(trials)] for k in files]

    # Calculates the correlation and the associated error from
    # the obtained distribution
    corr_u = unumpy.uarray(np.mean(correlations, 1), np.std(correlations, 1))
    return np.sum([k * s
                   for k, s
                   in zip(corr_u,
                          [1, -1, 1, 1])])


if __name__ == '__main__':
    # d = glob.glob('meas*')
    d = glob.glob('measurements/meas*.dat')

    # first raw of both output files contains the angle
    with open(sink_file, 'w') as sink, open(sink_file_err, 'w') as sink_err:
        sink.write(('angle' + '\t{:.4f}' * len(angles) + '\n').format(*angles))
        sink_err.write(('angle' + '\t{:.4f}' * len(angles) +
                        '\n').format(*angles))

    # Main loop over chunk size N
    for N in range(1, N_max):
        bells = [bell_value_err(*d[k * 4: k * 4 + 4], pp_corr, N, 1000)
                 for k
                 in range(len(angles))]
        # Write of the result for each N as separate rows
        with open(sink_file, 'a') as sink, \
                open(sink_file_err, 'a') as sink_err:
            sink.write(
                ('{}' + '\t{:.5f}' * len(angles) +
                 '\n').format(N, *unumpy.nominal_values(bells)))
            sink_err.write(
                ('{}' + '\t{:.5f}' * len(angles) +
                 '\n').format(N, *unumpy.std_devs(bells)))
        # for visual feedback
        print(N)
