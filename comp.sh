#!/bin/bash
pkill tens
pkill units
make clean
make build
cd build
./countdown

