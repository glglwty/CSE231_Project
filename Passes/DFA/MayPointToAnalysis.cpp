//
// Created by glglwty on 3/17/17.
//

#include <iostream>
#include <unordered_map>
#include <set>
#include "231DFA.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstVisitor.h"

using namespace llvm;

inline void ass(bool b) {
    if (!b) {
        std::cerr << "fatal error" << std::endl;
        //*(int*)0 = 0;
    }
}

inline void ass(bool b, const char *message) {
    if (!b) {
        std::cerr << "fatal error: " << message << std::endl;
        //*(int*)0 = 0;
    }
}

struct DFAId {
    bool isM;
    unsigned id;

    std::string to_string() const {
        return (isM ? "M" : "R") + std::to_string(id);
    }

    struct Hash {
        std::size_t operator()(const DFAId &k) const {
            return std::hash<unsigned>()(k.id) ^ std::hash<bool>()(k.isM);
        }
    };

    bool operator==(const DFAId &that) const {
        return isM == that.isM && id == that.id;
    }

    bool operator<(const DFAId &that) const {
        if (isM != that.isM) {
            return !isM;
        }
        return id < that.id;
    }
};

struct MayPointToInfo : public Info {
    std::map<DFAId, std::set<DFAId>> m;

    void print() override {
        for (auto &pi : m) {
            std::cerr << pi.first.to_string() << "->(";
            for (auto &t : pi.second) {
                std::cerr << t.to_string() << "/";
            }
            std::cerr << ")|";
        }
        std::cerr << std::endl;
    }

    static bool equals(const MayPointToInfo *info1, const MayPointToInfo *info2) {
        ass(info1 && info2, "nullptr in equals");
        if (info1->m.size() != info2->m.size()) {
            return false;
        }
        for (auto &pi : info1->m) {
            auto it = info2->m.find(pi.first);
            if (it == info2->m.end() || pi.second.size() != it->second.size() ||
                !std::equal(pi.second.begin(), pi.second.end(), it->second.begin())) {
                return false;
            }
        }
        return true;
    }

    static Info *join(const MayPointToInfo *info1, const MayPointToInfo *info2, MayPointToInfo *result) {
        ass(info1 && info2 && result, "nullptr in join");
        result->m.clear();
        for (auto &pi : info1->m) {
            auto it = info2->m.find(pi.first);
            if (it == info2->m.end()) {
                result->m.emplace(pi.first, pi.second);
            } else {
                auto &target = result->m[pi.first];
                std::set_union(pi.second.begin(), pi.second.end(), it->second.begin(), it->second.end(),
                               std::inserter(target, target.begin()));
            }
        }
        for (auto &pi : info2->m) {
            if (info1->m.find(pi.first) == info1->m.end()) {
                result->m.emplace(pi.first, pi.second);
            }
        }
        return result;
    }

};

class MayPointToAnalysis : public DataFlowAnalysis<MayPointToInfo, true> {
public:
    static MayPointToInfo MayPointToNothing;

    MayPointToAnalysis() : DataFlowAnalysis(MayPointToNothing, MayPointToNothing) {}

    struct Visitor : public InstVisitor<Visitor> {
        MayPointToInfo info;
        MayPointToAnalysis &parent;

        Visitor(MayPointToInfo &&info_in, MayPointToAnalysis &parent) : info(std::move(info_in)), parent(parent) {}

        unsigned f(Instruction *I) {
            return parent.getInstrToIndex().at(I);
        }

        void insert_all(DFAId from_id, DFAId to_id) {
            auto it = info.m.find(from_id);
            if (it != info.m.end()) {
                auto &target = info.m[to_id];
                std::copy(it->second.begin(), it->second.end(), std::inserter(target, target.begin()));
            }
        }

        void visitAllocaInst(AllocaInst &I) {
            auto id = f(&I);
            info.m[{false, id}].emplace(DFAId{true, id});
        }

        void visitBitCastInst(BitCastInst &I) {
            //std::cout << "operands of bitcast" << I.getNumOperands() << std::endl;
            if (auto pi = dyn_cast<Instruction>(I.getOperand(0))) {
                insert_all(DFAId{false, f(pi)}, DFAId{false, f(&I)});
            } else {
                //ass(false, "the parameter is not an instruction");
            }
        }

        void visitGetElementPtrInst(GetElementPtrInst &I) {
            //std::cout << "operands of getelemptr" << I.getNumOperands() << std::endl;
            if (auto pi = dyn_cast<Instruction>(I.getOperand(0))) {
                insert_all(DFAId{false, f(pi)}, DFAId{false, f(&I)});
            } else {
                //ass(false, "the parameter is not an instruction");
            }
        }

        void visitLoadInst(LoadInst &I) {
            auto id = f(&I);
            if (auto pi = dyn_cast<Instruction>(I.getOperand(0))) {
                auto it_rp_xs = info.m.find(DFAId{false, f(pi)});
                if (it_rp_xs != info.m.end()) {
                    for (auto x : it_rp_xs->second) {
                        insert_all(x, DFAId{false, id});
                    }
                }
            } else {
                //ass(false, "the value being loaded is not an instruction");
            }
        }

        void visitStoreInst(StoreInst &I) {
            auto pvi = dyn_cast<Instruction>(I.getOperand(0)), ppi = dyn_cast<Instruction>(I.getOperand(1));
            if (pvi && ppi) {
                auto id = f(pvi);
                auto it_rp_ys = info.m.find(DFAId{false, f(ppi)});
                if (it_rp_ys != info.m.end()) {
                    for (auto y : it_rp_ys->second) {
                        insert_all(DFAId{false, id}, y);
                    }
                }
            } else {
                //ass(false, "the parameter in store instruction is not an instruction");
            }

        }

        void visitSelectInst(SelectInst &I) {
            auto pv1 = dyn_cast<Instruction>(I.getOperand(1)), pv2 = dyn_cast<Instruction>(I.getOperand(2));
            auto id = f(&I);
            if (pv1 && pv2) {
                insert_all(DFAId{false, f(pv1)}, DFAId{false, id});
                insert_all(DFAId{false, f(pv2)}, DFAId{false, id});
            }
        }

        void visitPHINode(PHINode &I) {
            for (Instruction *p = &I; p && isa<PHINode>(p); p = p->getNextNode()) {
                auto id = f(p);
                auto phi = cast<PHINode>(p);
                for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
                    if (auto instr = dyn_cast<Instruction>(phi->getIncomingValue(i))) {
                        insert_all(DFAId{false, f(instr)}, DFAId{false, id});
                    } else {
                        //ass(false, "incoming value is not instruction");
                    }
                }
            }
        }
    };


    void flowfunction(Instruction *I, std::vector<unsigned> &IncomingEdges, std::vector<unsigned> &OutgoingEdges,
                      std::vector<MayPointToInfo *> &Infos) override {
        //std::cout << "flow function in" << std::endl;
        MayPointToInfo input_info;
        auto my_id = getInstrToIndex().at(I);
        for (auto income_node : IncomingEdges) {
            MayPointToInfo in_info_new;
            MayPointToInfo::join(getEdgeToInfo().at(std::make_pair(income_node, my_id)), &input_info, &in_info_new);
            input_info = std::move(in_info_new);
        }
        Visitor v(std::move(input_info), *this);
        v.visit(I);
        for (unsigned i = 0; i < OutgoingEdges.size(); i++) {
            Infos.emplace_back(new MayPointToInfo(v.info));
        }
        //std::cout << "flow function out" << std::endl;
    }

};

class MayPointToAnalysisPass : public FunctionPass {
public:
    static char ID;

    MayPointToAnalysisPass() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
        //std::cout << "begin running runonFunction" << std::endl;
        MayPointToAnalysis analysis;
        analysis.runWorklistAlgorithm(&F);
        //std::cout << "finished analysis" << std::endl;
        analysis.print();
        return false;
    }

};

MayPointToInfo MayPointToAnalysis::MayPointToNothing;
char MayPointToAnalysisPass::ID = 0;
static RegisterPass<MayPointToAnalysisPass> _("cse231-maypointto", "part3maypointto",
                                              true /* Only looks at CFG */,
                                              true /* Analysis Pass */);