from falcor import *
import math
import time

def render_graph_PathTracer():
    g = RenderGraph("PathTracer")
    PathTracer = createPass("PathTracer", {'samplesPerPixel': 1})
    g.addPass(PathTracer, "PathTracer")
    VBufferRT = createPass("VBufferRT", {'samplePattern': 'Stratified', 'sampleCount': 16, 'useAlphaTest': True})
    g.addPass(VBufferRT, "VBufferRT")
    AccumulatePass = createPass("AccumulatePass", {'enabled': True, 'precisionMode': 'Single'})
    g.addPass(AccumulatePass, "AccumulatePass")
    ToneMapper = createPass("ToneMapper", {'autoExposure': False, 'exposureCompensation': 0.0})
    g.addPass(ToneMapper, "ToneMapper")
    
    # 添加边
    g.addEdge("VBufferRT.vbuffer", "PathTracer.vbuffer")
    g.addEdge("VBufferRT.viewW", "PathTracer.viewW")
    g.addEdge("VBufferRT.mvec", "PathTracer.mvec")
    g.addEdge("PathTracer.color", "AccumulatePass.input")
    g.addEdge("AccumulatePass.output", "ToneMapper.src")
    g.markOutput("ToneMapper.dst")
    
    return g


PathTracer = render_graph_PathTracer()
try:
    m.addGraph(PathTracer)
except NameError:
    print("Failed to add graph, ensure 'm' is defined.")


last_capture_time=0


while True:  # 使用合适的条件来控制循环
    current_time = time.time()
    
    
    # 每秒截取一次纹理
    if current_time - last_capture_time >= 1:
        texture = m.getOutputTexture()  # 捕获当前渲染结果的纹理
        captured_textures.append(texture)
        last_capture_time = current_time  # 更新最后截取时间



