#include "llvm/Passes/PassBuilder.h"

using namespace llvm;
using namespace std;

class SetNoRecursionPass : public PassInfoMixin<SetNoRecursionPass> {
public:
  SetNoRecursionPass(set<Function *> *PassNoRecursionSet);

  PreservedAnalyses run(LazyCallGraph::SCC &C, CGSCCAnalysisManager &AM,
                        LazyCallGraph &CG, CGSCCUpdateResult &UR);
private:
  set<Function *> *NoRecursionSet;
};