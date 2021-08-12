set(LLVM_LINK_COMPONENTS support)

add_clang_executable(ASYSIM
    ASYSIM.cpp
  )
target_link_libraries(ASYSIM
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  )
