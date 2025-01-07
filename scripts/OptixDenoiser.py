from pathlib import WindowsPath, PosixPath
from falcor import *

def render_graph_g():
    g = RenderGraph('g')
    g.create_pass('OptixDenoiser', 'OptixDenoiser', {'enabled': True, 'blend': 0.0, 'model': 'Temporal', 'denoiseAlpha': False})
    g.create_pass('ToneMapper', 'ToneMapper', {'outputSize': 'Default', 'useSceneMetadata': True, 'exposureCompensation': 0.0, 'autoExposure': False, 'filmSpeed': 100.0, 'whiteBalance': False, 'whitePoint': 6500.0, 'operator': 'Aces', 'clamp': True, 'whiteMaxLuminance': 1.0, 'whiteScale': 11.199999809265137, 'fNumber': 1.0, 'shutter': 1.0, 'exposureMode': 'AperturePriority'})
    g.create_pass('ExrtoTexture', 'ExrtoTexture', {})
    g.create_pass('OptixDenoiserReference', 'OptixDenoiserReference', {})
    g.add_edge('ExrtoTexture.color', 'OptixDenoiser.color')
    g.add_edge('ExrtoTexture.diffuse', 'OptixDenoiser.albedo')
    g.add_edge('ExrtoTexture.normal', 'OptixDenoiser.normal')
    g.add_edge('ExrtoTexture.motion', 'OptixDenoiser.mvec')
    g.add_edge('OptixDenoiser.output', 'ToneMapper.src')
    g.add_edge('ToneMapper.dst', 'OptixDenoiserReference.color')
    g.add_edge('ToneMapper', 'OptixDenoiserReference')
    g.add_edge('ExrtoTexture', 'OptixDenoiser')
    g.mark_output('ToneMapper.dst')
    return g

g = render_graph_g()
try: m.addGraph(g)
except NameError: None
