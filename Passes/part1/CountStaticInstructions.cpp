#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include <map>
#include <iostream>
#include <string>

using namespace llvm;

namespace {
struct TestPass : public FunctionPass {
  static char ID;
  TestPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    std::map <int, int> mp;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      mp[I->getOpcode()] ++;
    }
    for (auto& pi : mp) {
      std::cerr << Instruction::getOpcodeName(pi.first) << '\t' << pi.second << '\n';
    }
    return false;
  }
}; // end of struct TestPass
}  // end of anonymous namespace

char TestPass::ID = 0;
static RegisterPass<TestPass> X("cse231-csi", "project1section1",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
