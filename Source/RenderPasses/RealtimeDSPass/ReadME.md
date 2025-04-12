# Introduction
- 参考NPPD/RTDS 定义神经网络模型
- 其中Encoder（双层全连接）和Unet通过Onnx导入TensorRT加入或者DirectML解析ONNX实现
- 把Splat操作转换成HLSL compute shader实现
```python
def splat(img, kernel, size):
    h = img.shape[2]
    w = img.shape[3]
    total = torch.zeros_like(img)

    img = F.pad(img, [(size - 1) // 2] * 4)
    kernel = F.pad(kernel, [(size - 1) // 2] * 4)

    for i in range(size):
        for j in range(size):
            total += (
                img[:, :, i: i + h, j: j + w]
                * kernel[:, i * size + j, None, i: i + h, j: j + w]
            )

    return total
```