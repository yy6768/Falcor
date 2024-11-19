from pathlib import WindowsPath, PosixPath
from falcor import *

def render_graph_DefaultRenderGraph():
    g = RenderGraph('DefaultRenderGraph')
    g.create_pass('OptixDenoiser', 'OptixDenoiser', {'enabled': True, 'blend': 0.0, 'model': 'Temporal', 'denoiseAlpha': False})
    g.create_pass('OptixDenoiserReference', 'OptixDenoiserReference', {})
    g.create_pass('ExrtoTexture', 'ExrtoTexture', {})
    g.add_edge('ExrtoTexture.color', 'OptixDenoiser.color')
    g.add_edge('ExrtoTexture.diffuse', 'OptixDenoiser.albedo')
    g.add_edge('ExrtoTexture.normal', 'OptixDenoiser.normal')
    g.add_edge('OptixDenoiser.output', 'OptixDenoiserReference.color')
    g.add_edge('ExrtoTexture.motion', 'OptixDenoiser.mvec')
    g.add_edge('OptixDenoiser', 'OptixDenoiserReference')
    g.mark_output('OptixDenoiser.output')
    return g

DefaultRenderGraph = render_graph_DefaultRenderGraph()
try: m.addGraph(DefaultRenderGraph)
except NameError: None
