#include <iostream>
#include <boost/program_options.hpp>

#include <Windows.h>

#include "exit_code.h"
#include "event_log.h"
#include "transition_fixer.h"

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	po::options_description opts("Allowed Options");
	opts.add_options()
		("help", "Show this help message")
#ifdef _DEBUG
		("break", po::bool_switch(), "Break as soon as the program starts")
#endif
		("mode", 
			po::value<std::string>()
				->default_value("run")
				->required(),
			"The mode the program will run as. Valid modes are:\n"
			"- run (default)\n"
			"- install\n"
			"- uninstall");

	po::positional_options_description pos;
	pos.add("mode", 1);

	try {
		po::variables_map vm;
		po::store(
			po::command_line_parser(argc, argv)
				.options(opts)
				.positional(pos)
				.run(), 
			vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << opts << std::endl;
			return ExitCode::ERR_SUCCESS;
		}

#ifdef _DEBUG
		// Do we need to start breaking?
		bool breakOnStart = vm["break"].as<bool>();
		if (breakOnStart) {
			DebugBreak();
		}
#endif

		// What are we trying to do?
		bool succeeded = false;
		std::string mode = vm["mode"].as<std::string>();
		if (mode == "install") {
			succeeded = InstallEventLogSource();
			if (succeeded) {
				LogInfo(L"Successfully installed Event Log source");
			}
		}
		else if (mode == "run") {
			succeeded = ApplyFadeFix();
			if (succeeded) {
				LogInfo(L"Successfully enabled Active Desktop");
			}
		}
		else if (mode == "uninstall") {
			succeeded = UninstallEventLogSource();
			if (succeeded) {
				LogInfo(L"Successfully removed Event Log source");
			}
		}
		else {
			std::cerr 
				<< "Error: Unrecognized mode '" << mode << "'\n"
				<< opts
				<< std::endl;

			return ExitCode::ERR_CMDLINE_ERROR;
		}

		if (succeeded) {
			return ExitCode::ERR_SUCCESS;
		}
		else {
			return ExitCode::ERR_FAILURE;
		}
	}
	catch (const po::error& ex) {
		std::cerr
			<< "Error: " << ex.what() << "\n"
			<< opts
			<< std::endl;

		return ExitCode::ERR_CMDLINE_ERROR;
	}
}