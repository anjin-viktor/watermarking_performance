#include "VideoFrame.h"
#include "WatermarkReference.h"
#include "Detector.h"

#include <boost/program_options.hpp>

#include <iostream>


int main(int argc, char** argv)
{
	namespace po = boost::program_options;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("gen_reference", "generate reference pattern (call with reference_max and output file as arguments)")
		("embed", "embed watermark reference pattern (call with alpha and input, reference and output files as arguments)")
		("detect", "detect watermark reference pattern (call with threshold and input and reference files as arguments)")
		("reference_max", po::value<int>()->default_value(50), "reference pattern maximum value (from 0 to 255)")
		("alpha", po::value<double>()->default_value(1.0), "reference pattern embedding multiplier")
		("threshold", po::value<double>()->default_value(0.01), "detector threshold value")
		("robustness_threshold", po::value<double>()->default_value(0.00), "threshold with robustness against filtering")
		("value,v", po::value<bool>()->default_value(true), "embedded value")
		("in,i", po::value<std::string>(), "input file")
		("out,o", po::value<std::string>(), "output file")
		("reference,r", po::value<std::string>(), "reference file")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count("gen_reference"))
	{
		if (!vm.count("in") || !vm.count("out"))
		{
			std::cout << "Arguments `in` and `out` are expected. Use help for additional info";
			return 1;
		}

		std::shared_ptr<VideoFrame> pframe = std::make_shared<VideoFrame>(vm["in"].as<std::string>(), VideoFrame::Grayscale);
		if (!pframe)
		{
			std::cout << "Error opening input file `" + vm["in"].as<std::string>() + "`";
			return 1;
		}

		auto preference = WR::createRandom(pframe->width(), pframe->height(), vm["reference_max"].as<int>(), VideoFrame::Grayscale);

		preference->DCTSharpening(vm["robustness_threshold"].as<double>(), vm["reference_max"].as<int>());
		preference->save(vm["out"].as<std::string>());

/*		std::vector<float> dctData = preference->fDCT();
		float threshold = vm["robustness_threshold"].as<double>();
		for (int i = 0; i < dctData.size(); i++)
		{
			if (dctData[i] < threshold)
			{
				if (rand() % 2)
					dctData[i] = 0;
				else 
					dctData[i] += threshold;
			}
		}

		VideoFrame frameIDCT = VideoFrame::iDCT(dctData, pframe->width(), pframe->height());
		
		frameIDCT.save(vm["out"].as<std::string>());*/
	}
	else if (vm.count("embed"))
	{
		if (!vm.count("in") || !vm.count("out") || !vm.count("reference"))
		{
			std::cout << "Arguments `in`, `out` and `reference` are expected. Use help for additional info";
			return 1;
		}

		std::shared_ptr<VideoFrame> pframe = std::make_shared<VideoFrame>(vm["in"].as<std::string>(), VideoFrame::Grayscale);
		if (!pframe)
		{
			std::cout << "Error opening input file `" + vm["in"].as<std::string>() + "`";
			return 1;
		}

		std::shared_ptr<VideoFrame> preference = std::make_shared<VideoFrame>(vm["reference"].as<std::string>(), VideoFrame::Grayscale);
		if (!preference)
		{
			std::cout << "Error opening reference file `" + vm["reference"].as<std::string>() + "`";
			return 1;
		}

		pframe->applyWR(preference, vm["alpha"].as<double>(), vm["value"].as<bool>());
		pframe->save(vm["out"].as<std::string>());
	}
	else if (vm.count("detect"))
	{
		if (!vm.count("in") || !vm.count("reference"))
		{
			std::cout << "Arguments `in`, `out` and `reference` are expected. Use help for additional info";
			return 1;
		}

		std::shared_ptr<VideoFrame> pframe = std::make_shared<VideoFrame>(vm["in"].as<std::string>());
		if (!pframe)
		{
			std::cout << "Error opening input file `" + vm["in"].as<std::string>() + "`";
			return 1;
		}

		std::shared_ptr<VideoFrame> preference = std::make_shared<VideoFrame>(vm["reference"].as<std::string>());
		if (!preference)
		{
			std::cout << "Error opening reference file `" + vm["reference"].as<std::string>() + "`";
			return 1;
		}

		Detector::Result resTrue = Detector::LinearCorrelation(pframe, preference, vm["threshold"].as<double>());

		if (resTrue == Detector::TRUE)
			std::cout << "TRUE" << std::endl;
		else if (resTrue == Detector::FALSE)
			std::cout << "FALSE" << std::endl;
		else
			std::cout << "NOT DETECTED" << std::endl;
	}
	else
	{
		std::cout << desc << "\n";
	}

	return 0;
}
