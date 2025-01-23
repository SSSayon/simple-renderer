#!/usr/bin/env bash

cmake -B build
cmake --build build

mkdir -p output
# build/final testcases/cornell_box_five_balls.txt five_balls
build/final testcases/cornell_box_three_balls_one_glass.txt three_balls_one_glass
# build/final testcases/scene01_basic.txt output/scene01.bmp
# build/final testcases/scene02_cube.txt output/scene02.bmp
# build/final testcases/scene03_sphere.txt output/scene03.bmp
# build/final testcases/scene04_axes.txt output/scene04.bmp
# build/final testcases/scene05_bunny_200.txt output/scene05.bmp
# build/final testcases/scene06_bunny_1k.txt output/scene06.bmp
# build/final testcases/scene07_shine.txt output/scene07.bmp
