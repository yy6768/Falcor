add_plugin(ReSTIRPass)

target_sources(ReSTIRPass PRIVATE
    ReSTIRPass.cpp
    ReSTIRPass.h
)

target_source_group(ReSTIRPass "RenderPasses")
