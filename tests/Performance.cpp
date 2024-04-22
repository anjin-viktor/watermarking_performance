#include <boost/test/unit_test.hpp>

#include "Utils.h"
#include "VideoFrame.h"
#include "WatermarkReference.h"

#include <chrono>
#include <vector>

BOOST_AUTO_TEST_SUITE(watermark_detector);

std::size_t performanceTestItem(std::vector<std::shared_ptr<VideoFrame>> pframes, std::shared_ptr<VideoFrame> preference, int testsCount, int internalThreads)
{
  std::size_t fullDuration = 0;
  ThreadPool threadPool(internalThreads);
  for (int i = 0; i < testsCount; i++)
  {
    std::vector<bool> bits(pframes.size());
    for (int j = 0; j < pframes.size(); j++)
      bits[j] = rand() % 2;

    std::vector<std::shared_ptr<VideoFrame>> pframesTest(pframes.size());
    for (int j = 0; j < pframes.size(); j++)
    {
//      pframesTest[j] = std::make_shared<VideoFrame>(*pframes[j]);
      pframesTest[j] = pframes[j];
    }

    double alpha = 0.1;

    auto start = std::chrono::high_resolution_clock::now();

    for (int j = 0; j < pframes.size(); j++)
      pframesTest[j]->applyWR(preference, alpha, bits[j], threadPool);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
//    std::cout << i << ": " << duration.count() << " microseconds" << std::endl;
    fullDuration += duration.count();
  }

  return fullDuration;
}

void performanceTest(std::string testName, std::vector<std::string> names, int framesInTest, int testsCnt)
{
  std::size_t processorCount = std::thread::hardware_concurrency();

  for (int j = 0; j <= processorCount * 2; j += 1)
  {
    int streamsCnt = j == 0 ? 1 : j;

    for (int i = 0; i <= processorCount * 2; i += 1)
    {
      ThreadPool threadPool(streamsCnt);
      std::vector<std::shared_ptr<VideoFrame>> pframes(framesInTest);
      for (std::size_t k = 0; k < names.size(); k++)
        pframes[k] = std::make_shared<VideoFrame>(names[k]);
      for(std::size_t k = names.size(); k<framesInTest; k++)
        pframes[k] = std::make_shared<VideoFrame>(*pframes[k % names.size()]);

      std::vector<std::future<size_t>> results;

      uint8_t threahold = 50;
      auto preference = WR::createRandom(pframes[0]->width(), pframes[0]->height(), threahold);

      for (int task = 0; task < streamsCnt; task++)
      {
        int frames = framesInTest / streamsCnt;
        if (task == streamsCnt - 1 && streamsCnt > 1)
          frames = framesInTest - framesInTest / streamsCnt * (streamsCnt - 1);
        std::vector<std::shared_ptr<VideoFrame>> pframesTest(frames);
        for (int k = 0; k < frames; k++)
        {
          pframesTest[k] = pframes[task * (framesInTest / streamsCnt) + k];
        }

        results.emplace_back(threadPool.enqueue(performanceTestItem, pframesTest, preference, testsCnt, i));
      }

      std::size_t fullDuration = 0;
      for (auto&& result : results)
        fullDuration += result.get();
      float fullDuratuonMs = (float)fullDuration / 1000;
      float frameDuration = fullDuratuonMs / framesInTest / testsCnt / streamsCnt;

      std::cout << streamsCnt << " parallel " << testName << " " << i << " thread test duration : " << frameDuration << " msec per frame" << std::endl;
    }
  }
}

BOOST_AUTO_TEST_CASE(performance, * boost::unit_test::disabled())
{
  std::vector<std::string> namesSD = { getSourceDir(__FILE__) + "images/agriculture-sd.jpg",
    getSourceDir(__FILE__) + "images/blue-sd.jpg",
    getSourceDir(__FILE__) + "images/crane-sd.jpg",
    getSourceDir(__FILE__) + "images/field-sd.jpg",
    getSourceDir(__FILE__) + "images/twilight-sd.jpg",
  };

  std::vector<std::string> namesHD = { getSourceDir(__FILE__) + "images/agriculture-hd.jpg",
    getSourceDir(__FILE__) + "images/blue-hd.jpg",
    getSourceDir(__FILE__) + "images/crane-hd.jpg",
    getSourceDir(__FILE__) + "images/field-hd.jpg",
    getSourceDir(__FILE__) + "images/twilight-hd.jpg",
  };

  std::vector<std::string> names4K = { getSourceDir(__FILE__) + "images/agriculture-4k.jpg",
    getSourceDir(__FILE__) + "images/blue-4k.jpg",
    getSourceDir(__FILE__) + "images/crane-4k.jpg",
    getSourceDir(__FILE__) + "images/field-4k.jpg",
    getSourceDir(__FILE__) + "images/twilight-4k.jpg",
  };

  std::size_t processorCount = std::thread::hardware_concurrency();

  performanceTest("SD", namesSD, 1000, 100);
  performanceTest("HD", namesHD, 500, 10);
  performanceTest("4k", names4K, 200, 25);
}


std::size_t performanceOptimizationTest(std::string testName, std::vector<std::string> names, int framesInTest, int testsCount, VideoFrame::Optimization optimization)
{
  std::vector<std::shared_ptr<VideoFrame>> pframes(names.size());
  for (std::size_t k = 0; k < names.size(); k++)
    pframes[k] = std::make_shared<VideoFrame>(names[k]);

  std::size_t fullDuration = 0;

  for (int i = 0; i < testsCount; i++)
  {
    std::vector<bool> bits(pframes.size());
    for (int j = 0; j < pframes.size(); j++)
      bits[j] = rand() % 2;

    std::vector<std::shared_ptr<VideoFrame>> pframesTest(framesInTest);
    for (int j = 0; j < pframesTest.size(); j++)
      pframesTest[j] = std::make_shared<VideoFrame>(*pframes[j % pframes.size()]);

    uint8_t threahold = 50;
    auto preference = WR::createRandom(pframes[0]->width(), pframes[0]->height(), threahold);

    double alpha = 0.1;

    auto start = std::chrono::high_resolution_clock::now();

    for (int j = 0; j < pframesTest.size(); j++)
      pframesTest[j]->applyWR(preference, alpha, bits[j], ThreadPool(0), optimization);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    fullDuration += duration.count();
  }

  float fullDuratuonMs = (float)fullDuration / 1000;
  float frameDuration = fullDuratuonMs / framesInTest / testsCount;

  std::cout << testName << ": " << frameDuration << " msec per frame" << std::endl;


  return fullDuration;
}
  
BOOST_AUTO_TEST_CASE(performance_optimizations, *boost::unit_test::disabled())
{
  std::vector<std::string> namesSD = { getSourceDir(__FILE__) + "images/agriculture-sd.jpg",
    getSourceDir(__FILE__) + "images/blue-sd.jpg",
    getSourceDir(__FILE__) + "images/crane-sd.jpg",
    getSourceDir(__FILE__) + "images/field-sd.jpg",
    getSourceDir(__FILE__) + "images/twilight-sd.jpg",
  };

  std::vector<std::string> namesHD = { getSourceDir(__FILE__) + "images/agriculture-hd.jpg",
    getSourceDir(__FILE__) + "images/blue-hd.jpg",
    getSourceDir(__FILE__) + "images/crane-hd.jpg",
    getSourceDir(__FILE__) + "images/field-hd.jpg",
    getSourceDir(__FILE__) + "images/twilight-hd.jpg",
  };

  std::vector<std::string> names4K = { getSourceDir(__FILE__) + "images/agriculture-4k.jpg",
    getSourceDir(__FILE__) + "images/blue-4k.jpg",
    getSourceDir(__FILE__) + "images/crane-4k.jpg",
    getSourceDir(__FILE__) + "images/field-4k.jpg",
    getSourceDir(__FILE__) + "images/twilight-4k.jpg",
  };

  std::size_t processorCount = std::thread::hardware_concurrency();

  performanceOptimizationTest("SD-C", namesSD, 100, 100, VideoFrame::C);
  performanceOptimizationTest("SD-SSE", namesSD, 100, 100, VideoFrame::SSE);
  performanceOptimizationTest("SD-AVX", namesSD, 100, 100, VideoFrame::AVX);

  performanceOptimizationTest("HD-C", namesHD, 50, 50, VideoFrame::C);
  performanceOptimizationTest("HD-SSE", namesHD, 50, 50, VideoFrame::SSE);
  performanceOptimizationTest("HD-AVX", namesHD, 50, 50, VideoFrame::AVX);

  performanceOptimizationTest("4k-C", names4K, 25, 25, VideoFrame::C);
  performanceOptimizationTest("4k-SSE", names4K, 25, 25, VideoFrame::SSE);
  performanceOptimizationTest("4k-AVX", names4K, 25, 25, VideoFrame::AVX);
}

BOOST_AUTO_TEST_SUITE_END();
