
include_directories(.)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
add_llvm_loadable_module( CSE231
  ReachingDefinitionAnalysis.cpp
  LivenessAnalysis.cpp
  MayPointToAnalysis.cpp
  PLUGIN_TOOL
  opt
  )
