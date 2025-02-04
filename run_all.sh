#!/usr/bin/env bash

SPP=16
BVH=true

while getopts ":s:n" opt; do
  case ${opt} in
    s )
      SPP=$OPTARG
      ;;
    n )
      BVH=false
      ;;
    \? )
      echo "Usage: $0 [-s <SPP>] [-n]" # -s: samples per pixel, -n: disable bvh
      exit 1
      ;;
  esac
done

cmake -B build
cmake --build build

mkdir -p output

# whitted
# build/whitted testcases/whitted_cornell_box_five_balls.txt whitted_five_balls
# build/whitted testcases/whitted_cornell_box_three_balls_one_glass.txt whitted_three_balls_one_glass

# lambert
# build/pathtracing testcases/cornell_box.txt cornell_box/cornell_box uniform_naive $SPP $BVH
# build/pathtracing testcases/cornell_box.txt cornell_box/cornell_box cosine_naive $SPP $BVH
# build/pathtracing testcases/cornell_box.txt cornell_box/cornell_box cosine_nee $SPP $BVH
# build/pathtracing testcases/cornell_box.txt cornell_box/cornell_box cosine_mis $SPP $BVH

# modified phong
# build/pathtracing testcases/cornell_box_three_balls.txt cornell_box_three_balls phong_naive $SPP $BVH
# build/pathtracing testcases/cornell_box_three_balls.txt cornell_box_three_balls phong_nee $SPP $BVH
# build/pathtracing testcases/cornell_box_three_balls.txt cornell_box_three_balls phong_mis $SPP $BVH

# mis
# build/pathtracing testcases/mis_test.txt mis_test11 naive $SPP $BVH
# build/pathtracing testcases/mis_test.txt mis_test11 nee $SPP $BVH
# build/pathtracing testcases/mis_test.txt mis_test11 mis $SPP $BVH

# smooth normal
# build/pathtracing testcases/cornell_box_bunny.txt bunny mis $SPP $BVH

# depth of field
# build/pathtracing testcases/cornell_box_dof.txt cornell_box_dof mis $SPP $BVH

# texture
# build/pathtracing testcases/cornell_box_cow.txt cow mis $SPP $BVH
# build/pathtracing testcases/cornell_box_cow_and_bunny.txt cow_and_bunny mis $SPP $BVH
# build/pathtracing testcases/cornell_box_texture.txt cornell_box_texture mis $SPP $BVH 

# reflection & refraction
# build/pathtracing testcases/cornell_box_reflection_refraction.txt reflection_refraction mis $SPP $BVH

# cook-torrance
# build/pathtracing testcases/cornell_box_balls.txt cornell_box_balls uniform_mis $SPP $BVH
# build/pathtracing testcases/cornell_box_balls.txt cornell_box_balls cook_mis $SPP $BVH
