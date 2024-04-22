#include <boost/test/unit_test.hpp>

#include "Utils.h"
#include "VideoFrame.h"
#include "WatermarkReference.h"

BOOST_AUTO_TEST_SUITE(watermark_reference);

BOOST_AUTO_TEST_CASE(create_random)
{
  uint8_t threahold = 5;
  auto preference = WR::createRandom(100, 200, threahold);

  BOOST_CHECK_EQUAL(preference->width(), 100);
  BOOST_CHECK_EQUAL(preference->height(), 200);

  uint8_t* pdata = preference->data(0);

  int stride = (int)preference->width() * 3;
  for (int i = 0; i < preference->height(); i++)
  {
    for (int j = 0; j < preference->width(); j++)
    {
      uint8_t b = pdata[i * stride + j * 3];
      uint8_t g = pdata[i * stride + j * 3 + 1];
      uint8_t r = pdata[i * stride + j * 3 + 2];

      BOOST_CHECK(b < threahold);
      BOOST_CHECK(g < threahold);
      BOOST_CHECK(r < threahold);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END();
