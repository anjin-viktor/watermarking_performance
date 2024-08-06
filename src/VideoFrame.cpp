#include "VideoFrame.h"

#include <opencv2/opencv.hpp>
#ifdef WIN32
#include <intrin.h>
#endif

VideoFrame::VideoFrame(std::size_t width, std::size_t height, ColorFormat colorFormat):
  m_width(width),
  m_height(height),
  m_colorFormat(colorFormat)
{
  std::size_t size = m_width * m_height;

  if (m_colorFormat == ColorFormat::Color)
    size *= 3;

  m_data.resize(1);
  m_data[0].resize(size);
}

VideoFrame::VideoFrame(const std::string& fileName, ColorFormat colorFormat):
  m_width(0),
  m_height(0),
  m_colorFormat(colorFormat)
{
  cv::Mat image;
  image = cv::imread(fileName, m_colorFormat == ColorFormat::Color ? cv::IMREAD_COLOR : cv::IMREAD_GRAYSCALE);

  m_width = image.cols;
  m_height = image.rows;

  std::size_t size = m_width * m_height;
  if (m_colorFormat == ColorFormat::Color)
    size *= 3;

  m_data.resize(1);
  m_data[0].resize(size);

  std::copy(image.data, image.data + size, m_data[0].begin());
}

void VideoFrame::save(const std::string& fileName)
{
  cv::Mat image = cv::Mat((int)m_height, (int)m_width, m_colorFormat == ColorFormat::Color ? CV_8UC3 : CV_8UC1, &(m_data[0][0]));
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

std::vector<float> VideoFrame::fDCT()
{
  VideoFrame res(m_width, m_height, m_colorFormat);
  std::vector<float> srcData;
  srcData.insert(srcData.end(), m_data[0].begin(), m_data[0].end());
  std::vector<float> dstData(srcData.size());

  cv::Mat src = cv::Mat((int)m_height, (int)m_width, CV_32F, &srcData[0]);
  cv::Mat dst = cv::Mat((int)m_height, (int)m_width, CV_32F, &dstData[0]);

  cv::dct(src, dst);

  return dstData;
}

void VideoFrame::DCTSharpening(float threshold, int referenceMax)
{
  const int blockSize = 8;
  std::size_t stride = m_width * (m_colorFormat == VideoFrame::Color ? 3 : 1);

  for (int i = 0; i + blockSize <= m_width; i += blockSize)
  {
    for (int j = 0; j + blockSize <= m_height; j += blockSize)
    {
      std::vector<float> srcData(blockSize * blockSize);
      for (int posX = 0; posX < blockSize; posX++)
      {
        for (int posY = 0; posY < blockSize; posY++)
          srcData[posX + posY * blockSize] = m_data[0][i + posX + (j + posY) * stride];
      }

      std::vector<float> dstData(srcData.size());
      cv::Mat src = cv::Mat((int)blockSize, (int)blockSize, CV_32F, &srcData[0]);
      cv::Mat dst = cv::Mat((int)blockSize, (int)blockSize, CV_32F, &dstData[0]);
      cv::dct(src, dst);

      for (int k = 0; k < dstData.size(); k++)
      {
        if (dstData[k] < threshold)
        {
          if (rand() % 2)
            dstData[k] = 0;
          else
          {
            int sign = dstData[k] > 0 ? 1 : -1;
            dstData[k] = threshold * sign;
          }
        }
      }

/*      for (int k1 = 0; k1 < blockSize; k1++)
      {
        for (int k2 = blockSize - k1 - 1; k2 < blockSize; k2++)
        {
          int idx = k1 * blockSize + k2;
          if (abs(dstData[idx]) < threshold)
          {
            if (rand() % 2)
              dstData[idx] = 0;
            else
            {
              int sign = dstData[idx] > 0 ? 1 : -1;
              dstData[idx] = threshold * sign;
            }
          }
        }
      }*/

      cv::idct(dst, src);

      for (int posX = 0; posX < blockSize; posX++)
      {
        for (int posY = 0; posY < blockSize; posY++)
        {
          int val = srcData[posX + posY * blockSize];
          if (val < 0)
            val = 0;
          else if (val > referenceMax)
            val = referenceMax;

          m_data[0][i + posX + (j + posY) * stride] = val;
        }
      }
    }
  }
}

VideoFrame VideoFrame::iDCT(std::vector<float> dctData, std::size_t width, std::size_t height)
{
  VideoFrame res(width, height, VideoFrame::Grayscale);
  std::vector<float> dstData(dctData.size());

  cv::Mat src = cv::Mat((int)height, (int)width, CV_32F, &dctData[0]);
  cv::Mat dst = cv::Mat((int)height, (int)width, CV_32F, &dstData[0]);

  cv::idct(src, dst);
  std::copy(dstData.begin(), dstData.end(), res.m_data[0].begin());

  return res;
}

namespace
{
  void applyWRImpl_C(uint8_t* preference, uint8_t* pdata, std::size_t width, std::size_t stride, std::size_t height, double alpha, bool key, bool bypassByRows = true)
  {
    if (bypassByRows)
    {
      for (int i = 0; i < height; i++)
      {
        for (int j = 0; j < width; j++)
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
    else
    {
      for (int j = 0; j < width; j++)
      {
        for (int i = 0; i < height; i++)
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
  }
#ifdef WIN32
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
#endif

  void applyWRImpl(uint8_t *preference, uint8_t *pdata, std::size_t width, std::size_t stride, std::size_t height, double alpha, bool key, VideoFrame::Optimization optimization, VideoFrame::ThreadingType threading)
  {
    if (optimization == VideoFrame::C || optimization == VideoFrame::Auto)
    {
      applyWRImpl_C(preference, pdata, width, stride, height, alpha, key, true/*, threading == VideoFrame::Rows*/);
    }
#ifdef WIN32
    else if (optimization == VideoFrame::SSE)
    {
      applyWRImpl_SSE(preference, pdata, stride, height, key);
    }
    else if (optimization == VideoFrame::AVX)
    {
      applyWRImpl_AVX(preference, pdata, stride, height, key);
    }
#else
    else
    {
      applyWRImpl_C(preference, pdata, stride, height, alpha, key, true);
    }

#endif
  }


}

bool VideoFrame::applyWR(std::shared_ptr<VideoFrame> preference, double alpha, bool key, VideoFrame::Optimization optimization)
{
  ThreadPool threadPool(0);
  return applyWR(preference, alpha, key, threadPool, optimization);
}


bool VideoFrame::applyWR(std::shared_ptr<VideoFrame> preference, double alpha, bool key, ThreadPool& threadPool, VideoFrame::Optimization optimization, VideoFrame::ThreadingType threading)
{
  if (!preference)
    return false;

  if (m_width != preference->width() || m_height != preference->height())
    return false;


  uint8_t* pdata = &m_data[0][0];
  uint8_t* pwr = preference->data(0);
  std::size_t stride = m_width * (m_colorFormat == VideoFrame::Color ? 3 : 1);

  if (threadPool.size() == 0)
  {
    applyWRImpl(pwr, pdata, stride, stride, m_height, alpha, key, optimization, threading);
    return true;
  }

  std::size_t threads = threadPool.size();
  std::vector<std::future<void>> results;

  std::vector<std::size_t> tasksHeight(threads);
  std::vector<uint8_t *> tasksPData(threads);
  std::vector<uint8_t*> tasksPwr(threads);
  std::vector<std::size_t> tasksWidth(threads);

  if (threading == VideoFrame::Rows)
  {
    std::size_t offset = 0;
    for (int i = 0; i < threads; i++)
    {
      tasksHeight[i] = m_height / threads;
      tasksPData[i] = pdata + offset;
      tasksPwr[i] = pwr + offset;
      tasksWidth[i] = stride;
      offset += tasksHeight[i] * stride;
    }
    tasksHeight[threads - 1] = m_height - m_height / threads * (threads - 1);
  }
  else
  {
    std::size_t offset = 0;
    for (int i = 0; i < threads; i++)
    {
      tasksHeight[i] = m_height;
      tasksPData[i] = pdata + offset;
      tasksPwr[i] = pwr + offset;
      tasksWidth[i] = stride / threads;
      offset += stride / threads;
    }
    tasksWidth[threads - 1] = stride - stride / threads * (threads - 1);
  }



  std::size_t offset = 0;
  for (int i = 0; i < threads; i++)
  {
    results.emplace_back(threadPool.enqueue(applyWRImpl, tasksPwr[i], tasksPData[i], tasksWidth[i], stride, tasksHeight[i], alpha, key, optimization, threading));
    offset += tasksHeight[i] * stride;
  }

  for (auto&& result : results)
    result.get();

  return true;
}

