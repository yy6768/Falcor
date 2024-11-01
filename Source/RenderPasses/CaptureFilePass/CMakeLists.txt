add_plugin(CaptureFilePass)

target_sources(CaptureFilePass PRIVATE
    CaptureFilePass.cpp
    CaptureFilePass.h
)

target_source_group(CaptureFilePass "RenderPasses")
