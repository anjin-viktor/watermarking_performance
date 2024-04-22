#include "Detector.h"
#include "VideoFrame.h"

#include <cmath>

Detector::Result Detector::LinearCorrelation(std::shared_ptr<VideoFrame> pFrame, std::shared_ptr<VideoFrame> pFrameNoise, double threshold)
{
  if (!pFrame || !pFrameNoise)
    return Detector::FAILED;

  if(pFrame->width() != pFrameNoise->width() || pFrame->height() != pFrameNoise->height())
    return Detector::FAILED;

  uint8_t* pdata = pFrame->data(0);
  uint8_t* pnoise = pFrameNoise->data(0);

  std::size_t width = pFrame->width();
  std::size_t stride = width * 3;
  std::size_t height = pFrame->height();
  int result = 0;

  double meanRf = 0, meanRn = 0, meanGf = 0, meanGn = 0, meanBf = 0, meanBn = 0;
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      meanBf += pdata[i * stride + j * 3];
      meanBn += pnoise[i * stride + j * 3];

      meanGf += pdata[i * stride + j * 3 + 1];
      meanGn += pnoise[i * stride + j * 3 + 1];

      meanRf += pdata[i * stride + j * 3 + 2];
      meanRn += pnoise[i * stride + j * 3 + 2];
    }
  }

  meanBf /= height * width;
  meanBn /= height * width;
  meanGf /= height * width;
  meanGn /= height * width;
  meanRf /= height * width;
  meanRn /= height * width;

  double numB = 0, sqrBf = 0, sqrBn = 0;
  double numG = 0, sqrGf = 0, sqrGn = 0;
  double numR = 0, sqrRf = 0, sqrRn = 0;


  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      numB += ((double)pdata[i * stride + j * 3] - meanBf) * ((double)pnoise[i * stride + j * 3] - meanBn);
      numG += ((double)pdata[i * stride + j * 3 + 1] - meanGf) * ((double)pnoise[i * stride + j * 3 + 1] - meanGn);
      numR += ((double)pdata[i * stride + j * 3 + 2] - meanRf) * ((double)pnoise[i * stride + j * 3 + 2] - meanRn);

      sqrBf += std::pow(pdata[i * stride + j * 3] - meanBf, 2);
      sqrBn += std::pow(pnoise[i * stride + j * 3] - meanBn, 2);

      sqrGf += std::pow(pdata[i * stride + j * 3 + 1] - meanGf, 2);
      sqrGn += std::pow(pnoise[i * stride + j * 3 + 1] - meanGn, 2);

      sqrRf += std::pow(pdata[i * stride + j * 3 + 2] - meanRf, 2);
      sqrRn += std::pow(pnoise[i * stride + j * 3 + 2] - meanRn, 2);
    }
  }

  double corrB = numB / std::sqrt(sqrBf) / std::sqrt(sqrBn);
  double corrG = numG / std::sqrt(sqrGf) / std::sqrt(sqrGn);
  double corrR = numR / std::sqrt(sqrRf) / std::sqrt(sqrRn);

  double corr = (corrB + corrG + corrR) / 3;

  Detector::Result res = Detector::NO_WATERMARK;
  if (corr < -threshold)
    res = Detector::FALSE;
  else if(corr > threshold)
    res = Detector::TRUE;
   
  return res;
}