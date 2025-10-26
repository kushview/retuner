#!/usr/bin/sh
rm -rf build
cmake -Bbuild . -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target all -j2
python3 utils/artifacts.py
