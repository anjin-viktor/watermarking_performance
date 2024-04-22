#define BOOST_TEST_MODULE video_frame
#include <boost/test/unit_test.hpp>

#include <memory>

#include "Utils.h"
#include "VideoFrame.h"
#include "WatermarkReference.h"
#include "ThreadPool.h"

BOOST_AUTO_TEST_SUITE(video_frame);

BOOST_AUTO_TEST_CASE(open)
{
  VideoFrame frame(getSourceDir(__FILE__) + "images/sea_640.jpg");
  BOOST_CHECK_EQUAL(frame.width(), 480);
  BOOST_CHECK_EQUAL(frame.height(), 640);

  uint8_t* pdata = frame.data(0);
  int b0 = pdata[0];
  int g0 = pdata[1];
  int r0 = pdata[2];

  BOOST_CHECK_EQUAL(r0, 0);
  BOOST_CHECK_EQUAL(g0, 83);
  BOOST_CHECK_EQUAL(b0, 92);

  frame.save(getSourceDir(__FILE__) + "out/open_save.png");

  VideoFrame frameStored(getSourceDir(__FILE__) + "out/open_save.png");
  BOOST_CHECK_EQUAL(frameStored.width(), 480);
  BOOST_CHECK_EQUAL(frameStored.height(), 640);

  pdata = frameStored.data(0);
  b0 = pdata[0];
  g0 = pdata[1];
  r0 = pdata[2];

  BOOST_CHECK_EQUAL(r0, 0);
  BOOST_CHECK_EQUAL(g0, 83);
  BOOST_CHECK_EQUAL(b0, 92);
}

BOOST_AUTO_TEST_CASE(pixel_data)
{
  int width = 100, height = 200;
  VideoFrame frame(width, height);


  BOOST_CHECK_EQUAL(frame.width(), width);
  BOOST_CHECK_EQUAL(frame.height(), height);

  uint8_t* pdata = frame.data(0);
  int stride = width * 3;
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      bool isBlack = i < 10 && j < 10;
      uint8_t val = isBlack ? 0 : 255;

      pdata[i * stride + j * 3] = val;
      pdata[i * stride + j * 3 + 1] = val;
      pdata[i * stride + j * 3 + 2] = val;
    }
  }

  frame.save(getSourceDir(__FILE__) + "out/pixel_data.png");

  VideoFrame frameStored(getSourceDir(__FILE__) + "out/pixel_data.png");
  BOOST_CHECK_EQUAL(frameStored.width(), 100);
  BOOST_CHECK_EQUAL(frameStored.height(), 200);

  pdata = frameStored.data(0);
  int b0 = pdata[0];
  int g0 = pdata[1];
  int r0 = pdata[2];

  BOOST_CHECK_EQUAL(r0, 0);
  BOOST_CHECK_EQUAL(g0, 0);
  BOOST_CHECK_EQUAL(b0, 0);

  int b15 = pdata[15 * frameStored.width() * 3 + 15 * 3];
  int g15 = pdata[15 * frameStored.width() * 3 + 15 * 3 + 1];
  int r15 = pdata[15 * frameStored.width() * 3 + 15 * 3 + 2];

  BOOST_CHECK_EQUAL(r15, 255);
  BOOST_CHECK_EQUAL(g15, 255);
  BOOST_CHECK_EQUAL(b15, 255);
}

BOOST_AUTO_TEST_CASE(applay_watermark_reference)
{
  int width = 100, height = 200;
  VideoFrame frame(width, height);

  uint8_t* pdata = frame.data(0);
  std::size_t stride = width * 3;
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      uint8_t val = 100;
      pdata[i * stride + j * 3] = val;
      pdata[i * stride + j * 3 + 1] = val;
      pdata[i * stride + j * 3 + 2] = val;
    }
  }

  auto preference = WR::createRandom(width, height, 10);

  VideoFrame frameOrig = frame;

  frame.applyWR(preference, 1.0, true);
  frame.applyWR(preference, 1.0, false);

  pdata = frame.data(0);
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width * 3; j++)
    {
      BOOST_CHECK_EQUAL(pdata[i * stride + j], 100);
    }
  }
}

BOOST_AUTO_TEST_CASE(apply_wr_multithread)
{
  int width = 100, height = 200;
  VideoFrame frame(width, height);

  uint8_t* pdata = frame.data(0);
  std::size_t stride = width * 3;
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      uint8_t val = 100;
      pdata[i * stride + j * 3] = val;
      pdata[i * stride + j * 3 + 1] = val;
      pdata[i * stride + j * 3 + 2] = val;
    }
  }

  auto preference = WR::createRandom(width, height, 10);
  VideoFrame frameMT = frame;

  frame.applyWR(preference, 1.0, true);

  ThreadPool threadPool(8);
  frameMT.applyWR(preference, 1.0, true, threadPool);

  pdata = frame.data(0);
  uint8_t* pdataMT = frameMT.data(0);
  std::size_t size = stride * height;
  BOOST_CHECK_EQUAL_COLLECTIONS(pdata, pdata + size, pdataMT, pdataMT + size);
}

BOOST_AUTO_TEST_CASE(apply_wr_sse)
{
  int width = 500, height = 350;


  auto pframe = WR::createRandom(width, height, 0xFF);
  auto pframeSSE = std::make_shared<VideoFrame>(*pframe);
  auto preference = WR::createRandom(width, height, 10);

  pframe->applyWR(preference, 1.0, true, ThreadPool(0), VideoFrame::C);
  pframeSSE->applyWR(preference, 1.0, true, ThreadPool(0), VideoFrame::SSE);

  uint8_t *pdata = pframe->data(0);
  uint8_t* pdataSSE = pframeSSE->data(0);
  std::size_t stride = width * 3;
  std::size_t size = stride * height;

  BOOST_CHECK_EQUAL_COLLECTIONS(pdata, pdata + size, pdataSSE, pdataSSE + size);

  pframe = WR::createRandom(width, height, 0xFF);
  pframeSSE = std::make_shared<VideoFrame>(*pframe);

  pframe->applyWR(preference, 1.0, false, ThreadPool(0), VideoFrame::C);
  pframeSSE->applyWR(preference, 1.0, false, ThreadPool(0), VideoFrame::SSE);

  pdata = pframe->data(0);
  pdataSSE = pframeSSE->data(0);

  BOOST_CHECK_EQUAL_COLLECTIONS(pdata, pdata + size, pdataSSE, pdataSSE + size);
}

BOOST_AUTO_TEST_CASE(apply_wr_avx)
{
  int width = 500, height = 350;

  auto pframe = WR::createRandom(width, height, 0xFF);
  auto pframeAVX = std::make_shared<VideoFrame>(*pframe);
  auto preference = WR::createRandom(width, height, 10);

  pframe->applyWR(preference, 1.0, true, ThreadPool(0), VideoFrame::C);
  pframeAVX->applyWR(preference, 1.0, true, ThreadPool(0), VideoFrame::AVX);

  uint8_t* pdata = pframe->data(0);
  uint8_t* pdataAVX = pframeAVX->data(0);
  std::size_t stride = width * 3;
  std::size_t size = stride * height;

  BOOST_CHECK_EQUAL_COLLECTIONS(pdata, pdata + size, pdataAVX, pdataAVX + size);

  pframe = WR::createRandom(width, height, 0xFF);
  pframeAVX = std::make_shared<VideoFrame>(*pframe);

  pframe->applyWR(preference, 1.0, false, ThreadPool(0), VideoFrame::C);
  pframeAVX->applyWR(preference, 1.0, false, ThreadPool(0), VideoFrame::AVX);

  pdata = pframe->data(0);
  pdataAVX = pframeAVX->data(0);

  BOOST_CHECK_EQUAL_COLLECTIONS(pdata, pdata + size, pdataAVX, pdataAVX + size);
}


BOOST_AUTO_TEST_SUITE_END();
