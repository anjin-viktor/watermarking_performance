#include "VideoFrame.h"

#include <opencv2/opencv.hpp>
#include <intrin.h>

VideoFrame::VideoFrame(std::size_t width, std::size_t height):
  m_width(width),
  m_height(height)
{
  std::size_t size = m_width * m_height * 3;
  m_data.resize(1);
  m_data[0].resize(size);
}

VideoFrame::VideoFrame(const std::string& fileName):
  m_width(0),
  m_height(0)
{
  cv::Mat image;
  image = cv::imread(fileName, cv::IMREAD_COLOR);

  m_width = image.cols;
  m_height = image.rows;


  m_data.resize(1);
  m_data[0].resize(m_width * m_height * 3);
  std::copy(image.data, image.data + m_width * m_height * 3, m_data[0].begin());

}

void VideoFrame::save(const std::string& fileName)
{
  cv::Mat image = cv::Mat((int)m_height, (int)m_width, CV_8UC3, &(m_data[0][0]));
  cv::imwrite(fileName, image);
}

std::size_t VideoFrame::width() const
{
  return m_width;
}

std::size_t VideoFrame::height() const
{
  return m_height;
}

std::size_t VideoFrame::stride(int plane) const
{
  return m_width;
}

uint8_t* VideoFrame::data(int plane)
{
  return &(m_data[0][plane]);
}

namespace
{
  void applyWRImpl_C(uint8_t* preference, uint8_t* pdata, std::size_t stride, std::size_t height, double alpha, bool key)
  {
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < stride; j++)
      {
        int val = pdata[i * stride + j];
        int wr = preference[i * stride + j] * alpha;
        if (alpha - 1.0 > std::numeric_limits<float>::epsilon())
          wr = (int)wr * alpha;
        if (key)
          val = std::min(255, val + wr);
        else
          val = std::max(0, val - wr);
        pdata[i * stride + j] = val;
      }
    }
  }

  void applyWRImpl_SSE(uint8_t* preference, uint8_t* pdata, std::size_t stride, std::size_t height, bool key)
  {
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < stride / 16; j++)
      {
        __m128i val = _mm_load_si128((__m128i*)(pdata + (i * stride + j * 16)));
        __m128i wr = _mm_load_si128((__m128i*)(preference + (i * stride + j * 16)));

        if (key)
        {
          val = _mm_adds_epu8(val, wr);
        }
        else
        {
          val = _mm_subs_epu8(val, wr);
        }

        _mm_store_si128((__m128i*)(pdata + (i * stride + j * 16)), val);
      }

      for (int j = ((int)stride / 16) * 16; j < stride; j++)
      {
        int val = pdata[i * stride + j];
        int wr = preference[i * stride + j];
        if (key)
          val = std::min(255, val + wr);
        else
          val = std::max(0, val - wr);
        pdata[i * stride + j] = val;
      }
    }
  }

  void applyWRImpl_AVX(uint8_t* preference, uint8_t* pdata, std::size_t stride, std::size_t height, bool key)
  {
    for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < stride / 32; j++)
      {
        __m256i val = _mm256_load_si256((__m256i*)(pdata + (i * stride + j * 32)));
        __m256i wr = _mm256_load_si256((__m256i*)(preference + (i * stride + j * 32)));

        if (key)
        {
          val = _mm256_adds_epu8(val, wr);
        }
        else
        {
          val = _mm256_subs_epu8(val, wr);
        }

        _mm256_store_si256((__m256i*)(pdata + (i * stride + j * 32)), val);
      }

      for (int j = ((int)stride / 32) * 32; j < stride; j++)
      {
        int val = pdata[i * stride + j];
        int wr = (int)(preference[i * stride + j]);
        if (key)
          val = std::min(255, val + wr);
        else
          val = std::max(0, val - wr);
        pdata[i * stride + j] = val;
      }
    }
  }

  void applyWRImpl(uint8_t *preference, uint8_t *pdata, std::size_t stride, std::size_t height, double alpha, bool key, VideoFrame::Optimization optimization)
  {
    if (optimization == VideoFrame::C || optimization == VideoFrame::Auto)
    {
      applyWRImpl_C(preference, pdata, stride, height, alpha, key);
    }
    else if (optimization == VideoFrame::SSE)
    {
      applyWRImpl_SSE(preference, pdata, stride, height, key);
    }
    else if (optimization == VideoFrame::AVX)
    {
      applyWRImpl_AVX(preference, pdata, stride, height, key);
    }
  }


}

bool VideoFrame::applyWR(std::shared_ptr<VideoFrame> preference, double alpha, bool key, ThreadPool& threadPool, VideoFrame::Optimization optimization)
{
  if (!preference)
    return false;

  if (m_width != preference->width() || m_height != preference->height())
    return false;


  uint8_t* pdata = &m_data[0][0];
  uint8_t* pwr = preference->data(0);
  std::size_t stride = m_width * 3;

  if (threadPool.size() == 0)
  {
    applyWRImpl(pwr, pdata, stride, m_height, alpha, key, optimization);
    return true;
  }

  std::size_t threads = threadPool.size();
  std::vector<std::future<void>> results;

  std::vector<std::size_t> tasksHeight(threads);
  for (int i = 0; i < threads - 1; i++)
    tasksHeight[i] = m_height / threads;
  tasksHeight[threads - 1] = m_height - m_height / threads * (threads - 1);

  std::size_t offset = 0;
  for (int i = 0; i < threads; i++)
  {
    results.emplace_back(threadPool.enqueue(applyWRImpl, pwr + offset, pdata + offset, stride, tasksHeight[i], alpha, key, optimization));
    offset += tasksHeight[i] * stride;
  }

  for (auto&& result : results)
    result.get();

  return true;
}

