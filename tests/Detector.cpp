#include <boost/test/unit_test.hpp>

#include "Utils.h"
#include "VideoFrame.h"
#include "WatermarkReference.h"
#include "Detector.h"

BOOST_AUTO_TEST_SUITE(watermark_detector);

BOOST_AUTO_TEST_CASE(linear_correlation)
{
  std::shared_ptr<VideoFrame> pframe = std::make_shared<VideoFrame>(getSourceDir(__FILE__) + "images/sea_640.jpg");
  std::shared_ptr<VideoFrame> pframeTrue = std::make_shared<VideoFrame>(getSourceDir(__FILE__) + "images/sea_640.jpg");
  std::shared_ptr<VideoFrame> pframeFalse = std::make_shared<VideoFrame>(getSourceDir(__FILE__) + "images/sea_640.jpg");

  uint8_t threahold = 50;
  auto preference = WR::createRandom(pframe->width(), pframe->height(), threahold);


  pframeTrue->applyWR(preference, 0.1, true);
  pframeFalse->applyWR(preference, 0.1, false);
  
/*  pframeTrue->save(getSourceDir(__FILE__) + "out/true.jpg");
  pframeFalse->save(getSourceDir(__FILE__) + "out/false.jpg");
  preference->save(getSourceDir(__FILE__) + "out/reference.png");*/

/*  std::shared_ptr<VideoFrame> pframe = std::make_shared<VideoFrame>(getSourceDir(__FILE__) + "out/sea_640_transcodec.jpg");
  std::shared_ptr<VideoFrame> pframeTrue = std::make_shared<VideoFrame>(getSourceDir(__FILE__) + "out/true.jpg");
  std::shared_ptr<VideoFrame> pframeFalse = std::make_shared<VideoFrame>(getSourceDir(__FILE__) + "out/false.jpg");
  std::shared_ptr<VideoFrame> preference = std::make_shared<VideoFrame>(getSourceDir(__FILE__) + "out/reference.png");
  */

  Detector::Result resTrue = Detector::LinearCorrelation(pframeTrue, preference, 0.01);
  Detector::Result resFalse = Detector::LinearCorrelation(pframeFalse, preference, 0.01);
  Detector::Result resNoWatermark = Detector::LinearCorrelation(pframe, preference, 0.01);

  BOOST_CHECK_EQUAL(resTrue, Detector::TRUE);
  BOOST_CHECK_EQUAL(resFalse, Detector::FALSE);
  BOOST_CHECK_EQUAL(resNoWatermark, Detector::NO_WATERMARK);
}

BOOST_AUTO_TEST_SUITE_END();
