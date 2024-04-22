#ifndef DETECTOR_H_
#define DETECTOR_H_

#include <memory>

class VideoFrame;

namespace Detector
{
  enum Result
  {
    FAILED,
    TRUE,
    FALSE,
    NO_WATERMARK
  };

  Result LinearCorrelation(std::shared_ptr<VideoFrame> pFrame, std::shared_ptr<VideoFrame> pFrameNoise, double threshold);
};


#endif