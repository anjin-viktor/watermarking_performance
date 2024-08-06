#include "WatermarkReference.h"
#include <memory.h>

std::shared_ptr<VideoFrame> WR::createRandom(std::size_t width, std::size_t height, uint8_t threshold, VideoFrame::ColorFormat colorFormat)
{
  auto pframe = std::make_shared<VideoFrame>(width, height, colorFormat);

  uint8_t *pdata = pframe->data(0);
  int bytesPerPixel = colorFormat == VideoFrame::Color ? 3 : 1;
  memset(pdata, 0, pframe->height() * pframe->stride(0) * bytesPerPixel);

  std::size_t frameSize = pframe->height() * pframe->stride(0) * bytesPerPixel;
  for (int i = 0; i < frameSize; i++)
  {
    uint8_t val = rand() % threshold;
    pdata[i] = val;
  }

  return pframe;
}
