#include "vex/Sema/DecoratorProcessor.h"
namespace vex {
bool DecoratorProcessor::run(ModuleDecl*) { return true; }
bool DecoratorProcessor::processDecl(DeclNode*) { return true; }
bool DecoratorProcessor::processInline(FnDecl*) { return true; }
bool DecoratorProcessor::processNoInline(FnDecl*) { return true; }
bool DecoratorProcessor::processExtern(FnDecl*) { return true; }
bool DecoratorProcessor::processTest(FnDecl*) { return true; }
bool DecoratorProcessor::processDeprecated(DeclNode*) { return true; }
bool DecoratorProcessor::processCold(FnDecl*) { return true; }
bool DecoratorProcessor::processHot(FnDecl*) { return true; }
bool DecoratorProcessor::processNoReturn(FnDecl*) { return true; }
bool DecoratorProcessor::processUnsafe(FnDecl*) { return true; }
bool DecoratorProcessor::processExport(DeclNode*) { return true; }
bool DecoratorProcessor::processLink(DeclNode*) { return true; }
}
