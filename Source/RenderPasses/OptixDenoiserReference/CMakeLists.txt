add_plugin(OptixDenoiserReference)

target_sources(OptixDenoiserReference PRIVATE
    OptixDenoiserReference.cpp
    OptixDenoiserReference.h
)

target_source_group(OptixDenoiserReference "RenderPasses")
