clang++ fuck.cpp -S -emit-llvm -o fuck.ll
opt -load /LLVM_ROOT/build/lib/CSE231.so -cse231-cdi -S < fuck.ll -o instrumented.ll
/solution/opt -cse231-cdi -S < fuck.ll -o instrumented_sol.ll
llc instrumented.ll
g++ instrumented.s lib231.cpp main.cpp
./a.out
/solution/opt -cse231-cdi -S < fuck.ll -o instrumented_sol.ll
llc instrumented_sol.ll
g++ instrumented_sol.s lib231.cpp main.cpp
./a.out
