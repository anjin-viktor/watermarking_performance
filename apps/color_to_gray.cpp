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
		("in,i", po::value<std::string>(), "input file")
		("out,o", po::value<std::string>(), "output file")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (!vm.count("in") || !vm.count("out") || vm.count("help"))
	{
		std::cout << desc << "\n";
		return 0;
	}

	std::shared_ptr<VideoFrame> pframe = std::make_shared<VideoFrame>(vm["in"].as<std::string>(), VideoFrame::Grayscale);
	pframe->save(vm["out"].as<std::string>());

	return 0;
}
