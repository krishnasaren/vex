#include "vex/Sema/MROResolver.h"
namespace vex {
bool MROResolver::computeMRO(ClassDecl*) { return true; }
const std::vector<ClassDecl*>& MROResolver::getMRO(ClassDecl* c) {
    return cache_[c];
}
FnDecl* MROResolver::lookupMethod(ClassDecl*, std::string_view) { return nullptr; }
FieldDecl* MROResolver::lookupField(ClassDecl*, std::string_view) { return nullptr; }
bool MROResolver::c3Merge(std::vector<std::vector<ClassDecl*>>&, std::vector<ClassDecl*>&) { return true; }
}
