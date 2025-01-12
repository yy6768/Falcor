add_falcor_executable(OptixDenoiserApp)

target_sources(OptixDenoiserApp PRIVATE
    OptixDenoiserApp.cpp
    OptixDenoiserApp.h
)

target_copy_shaders(OptixDenoiserApp Samples/OptixDenoiserApp)

target_source_group(OptixDenoiserApp "Samples")
