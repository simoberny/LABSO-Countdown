#!/bin/bash
pkill countdown
pkill tens
pkill units
make clean
make assets
cd build
./countdown

