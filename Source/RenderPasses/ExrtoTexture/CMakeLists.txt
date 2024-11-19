add_plugin(ExrtoTexture)

target_sources(ExrtoTexture PRIVATE
    ExrtoTexture.cpp
    ExrtoTexture.h
)

target_source_group(ExrtoTexture "RenderPasses")
