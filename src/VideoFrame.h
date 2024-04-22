#ifndef VIDEO_FRAME_H_
#define VIDEO_FRAME_H_

#include <cstddef>
#include <vector>
#include <string>
#include <memory>

#include "ThreadPool.h"

class VideoFrame
{
public:
  enum Optimization
  {
    Auto,
    C, 
    SSE,
    AVX
  };

  VideoFrame(std::size_t width = 0, std::size_t height = 0);
  VideoFrame(const std::string& fileName);

  void save(const std::string& fileName);

  bool applyWR(std::shared_ptr<VideoFrame> preference, double alpha, bool key, ThreadPool &threadPool = ThreadPool(0), Optimization optimization = Auto);

  std::size_t width() const;
  std::size_t height() const;
  std::size_t stride(int plane) const;
  uint8_t* data(int plane);

private:
  std::size_t                       m_width;
  std::size_t                       m_height;
  std::vector<std::size_t>          m_strides;
  std::vector<std::vector<uint8_t>> m_data;
};


#endif
