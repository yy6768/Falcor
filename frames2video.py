import os

import cv2


def frames_to_video(frame_dir, output_video_path, fps=30):
    """
    将帧连接在一起生成视频。

    参数:
    frame_dir (str): 帧所在的目录。
    output_video_path (str): 输出视频文件的路径。
    fps (int): 视频的帧率。
    """
    # 获取所有帧文件
    frame_files = sorted([os.path.join(frame_dir, f) for f in os.listdir(
        frame_dir) if f.endswith('.png') or f.endswith('.jpg')])

    if not frame_files:
        raise ValueError(f"No frames found in directory {frame_dir}")

    # 读取第一帧以获取帧的尺寸
    first_frame = cv2.imread(frame_files[0])
    height, width, _ = first_frame.shape

    # 创建视频写入器
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # 使用 mp4 编码器
    writer = cv2.VideoWriter(output_video_path, fourcc, fps, (width, height))

    for frame_file in frame_files:
        frame = cv2.imread(frame_file)
        writer.write(frame)

    writer.release()
    print(f"Video saved to {output_video_path}")


if __name__ == "__main__":
    frames_to_video("G://data//denoised_with_temporal_1spp",
                    "G://data//denoised_with_temporal_1spp/bistro1_optix.mp4")
