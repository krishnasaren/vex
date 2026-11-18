#include "vex/Sema/DeriveExpander.h"
namespace vex {
bool DeriveExpander::run(ModuleDecl*) { return true; }
bool DeriveExpander::expand(DeclNode*, std::string_view) { return true; }
bool DeriveExpander::deriveDebug(StructDecl*) { return true; }
bool DeriveExpander::deriveDebug(EnumDecl*) { return true; }
bool DeriveExpander::deriveClone(StructDecl*) { return true; }
bool DeriveExpander::deriveClone(EnumDecl*) { return true; }
bool DeriveExpander::deriveEq(StructDecl*) { return true; }
bool DeriveExpander::deriveEq(EnumDecl*) { return true; }
bool DeriveExpander::deriveHash(StructDecl*) { return true; }
bool DeriveExpander::deriveDefault(StructDecl*) { return true; }
bool DeriveExpander::deriveIter(StructDecl*) { return true; }
bool DeriveExpander::deriveDisplay(StructDecl*) { return true; }
bool DeriveExpander::deriveDisplay(EnumDecl*) { return true; }
}
