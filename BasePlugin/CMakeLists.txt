add_library(LLVMBasePlugin MODULE 
            source/BasePlugin.cpp
            source/BasePluginRegister.cpp)

llvm_map_components_to_libnames(llvm_libs core)
target_link_libraries(LLVMBasePlugin ${llvm_libs})
target_include_directories(LLVMBasePlugin PRIVATE headers)

set_target_properties(LLVMBasePlugin PROPERTIES PREFIX "")
set_target_properties(LLVMBasePlugin PROPERTIES CXX_STANDARD 17)

install(TARGETS LLVMBasePlugin DESTINATION pass_plugins)