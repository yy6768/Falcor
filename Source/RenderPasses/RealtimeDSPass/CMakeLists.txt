add_plugin(RealtimeDSPass)

target_sources(RealtimeDSPass PRIVATE
    RealtimeDSPass.cpp
    RealtimeDSPass.h
)

target_source_group(RealtimeDSPass "RenderPasses")
