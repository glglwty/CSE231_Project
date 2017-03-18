
include_directories(.)
add_llvm_loadable_module( CSE231
  ReachingDefinitionAnalysis.cpp
  PLUGIN_TOOL
  opt
  )
