#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
g++ -O2 -std=c++17 main.cpp -o spraysheetcut -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
echo "Built: ./spraysheetcut"
