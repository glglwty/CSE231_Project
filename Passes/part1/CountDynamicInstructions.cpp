#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/IRBuilder.h>
#include <map>
#include <iostream>
#include <string>

using namespace llvm;

namespace {
  struct TestPass : public FunctionPass {
    static char ID;
    Module *m;

    TestPass() : FunctionPass(ID) {}

    bool doInitialization(Module &M) {
      m = &M;
      return false;
    }

    bool runOnFunction(Function &F) override {
      auto &context = F.getContext();
      auto int32_type = Type::getInt32Ty(context);
      auto pint32_type = Type::getInt32PtrTy(context);
      auto void_type = Type::getVoidTy(context);
      auto F_updateInstrInfo = cast<Function>(
        m->getOrInsertFunction("updateInstrInfo",
                               FunctionType::get(void_type, {int32_type, pint32_type, pint32_type}, false)));
      auto F_printOutInstrInfo = cast<Function>(
        m->getOrInsertFunction("printOutInstrInfo", FunctionType::get(void_type, false)));
      for (auto &blk : F) {
        std::map<int, int> mp;
        for (auto &inst : blk) {
          mp[inst.getOpcode()]++;
        }
        IRBuilder<> ib(&blk.front());
        auto size = ConstantInt::get(int32_type, mp.size());
        auto a_alloca = ib.CreateAlloca(int32_type, size);
        auto b_alloca = ib.CreateAlloca(int32_type, size);
        int index = 0;
        for (auto &pi :mp) {
          Value *index_array[] = {ConstantInt::get(int32_type, index++)};
          ib.CreateStore(ConstantInt::get(int32_type, pi.first), ib.CreateGEP(a_alloca, index_array));
          ib.CreateStore(ConstantInt::get(int32_type, pi.second), ib.CreateGEP(b_alloca, index_array));
        }
        ib.CreateCall(F_updateInstrInfo, {size, a_alloca, b_alloca});
        if (dyn_cast<ReturnInst>(&blk.back())) {
          ib.CreateCall(F_printOutInstrInfo);
        }
      }
      return true;
    }
  }; // end of struct TestPass
}  // end of anonymous namespace

char TestPass::ID = 0;
static RegisterPass <TestPass> X("cse231-cdi", "project1section2",
                                 false /* Only looks at CFG */,
                                 false /* Analysis Pass */);
