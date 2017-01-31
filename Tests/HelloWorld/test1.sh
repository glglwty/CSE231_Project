clang++ fuck.cpp -S -emit-llvm -o fuck.ll
opt -load /LLVM_ROOT/build/lib/CSE231.so -cse231-csi < fuck.ll > /dev/null
/solution/opt -cse231-csi -S < fuck.ll > /dev/null
