#!/usr/bin/env bash

source venv/bin/activate

for instances_per_class in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
  do
    for cv in 0 1 2 3 4 5 6 7 8 9
      do
        CUDA_VISIBLE_DEVICES="" python train.py DATA-${instances_per_class}-CV-${cv} $cv --max_data_per_class $instances_per_class
      done
  done
