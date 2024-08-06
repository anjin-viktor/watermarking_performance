#ifndef WATERMARK_REFERENCE_H_
#define WATERMARK_REFERENCE_H_

#include "VideoFrame.h"
#include <memory>

class VideoFrame;

namespace WR
{
  std::shared_ptr<VideoFrame> createRandom(std::size_t width, std::size_t height, uint8_t threshold, VideoFrame::ColorFormat colorFormat = VideoFrame::Color);
}

#endif
