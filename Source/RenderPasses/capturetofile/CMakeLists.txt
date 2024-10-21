add_plugin(capturetofile)

target_sources(capturetofile PRIVATE
    capturetofile.cpp
    capturetofile.h
)

target_source_group(capturetofile "RenderPasses")
