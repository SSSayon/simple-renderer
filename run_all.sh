#!/usr/bin/env bash

if [ $# -gt 1 ]; then
    echo "Usage: $0 <SPP>(optional)"
    exit 1
fi

SPP=$1

cmake -B build
cmake --build build

mkdir -p output

# build/whitted testcases/whitted_cornell_box_five_balls.txt whitted_five_balls
# build/whitted testcases/whitted_cornell_box_three_balls_one_glass.txt whitted_three_balls_one_glass

# build/pathtracing testcases/cornell_box.txt cornell_box cosine_naive $SPP
# build/pathtracing testcases/cornell_box.txt cornell_box cosine_nee $SPP
# build/pathtracing testcases/cornell_box.txt cornell_box cosine_mis $SPP

# build/pathtracing testcases/cornell_box_three_balls.txt cornell_box_three_balls phong_naive $SPP
# build/pathtracing testcases/cornell_box_three_balls.txt cornell_box_three_balls phong_nee $SPP
# build/pathtracing testcases/cornell_box_three_balls.txt cornell_box_three_balls phong_mis $SPP

# build/pathtracing testcases/mis_test.txt mis_test11 naive $SPP
# build/pathtracing testcases/mis_test.txt mis_test11 nee $SPP
# build/pathtracing testcases/mis_test.txt mis_test11 mis $SPP

# build/pathtracing testcases/cornell_box_bunny.txt bunny mis $SPP

# build/pathtracing testcases/cornell_box_dof.txt cornell_box_dof mis $SPP

# build/pathtracing testcases/cornell_box_texture.txt cornell_box_texture mis $SPP

build/pathtracing testcases/cornell_box_three_balls_one_glass.txt three_balls_one_glass mis $SPP
