#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <stdexcept>

#include "boost/program_options.hpp"

namespace po = boost::program_options;

void enable_active_desktop();
void register_task_scheduler();
void print_help(const po::options_description&);

constexpr int ERR_NONE = 0;
constexpr int ERR_BAD_ARGS = 1;
constexpr int ERR_EXCEPTION = 2;

const std::map<std::string, std::function<void()>> Modes
{
	{ "run", enable_active_desktop },
	{ "install", register_task_scheduler }
};

int main(int argc, char* argv[])
{
	po::options_description options{ "Options" };
	options.add_options()
		("help,h", "Shows this help message.")
		("mode,m", po::value<std::string>()->default_value("run"), 
			"The mode that the program is running under. Supported commands are:\n\n"
			"- \"run\": \tRuns the fix needed to enable wallpaper fade transitions\n"
			"- \"install\": \tInstalls a scheduled task using Windows Task Scheduler that'll run this fix on logins.");

	po::positional_options_description positional;
	positional.add("mode", 1);

	try {
		po::variables_map vm;
		po::store(
			po::command_line_parser(argc, argv)
				.options(options)
				.positional(positional)
				.run(),
			vm);
		po::notify(vm);

		if (vm.count("help")) {
			// The user wants to print out the help information for this command line.
			print_help(options);
			return ERR_NONE;
		}

		std::string mode = vm.at("mode").as<std::string>();
		auto modeToInvoke = Modes.find(mode);
		if (modeToInvoke == Modes.end()) {
			// The mode specified does not exist.
			std::cerr << "Error: Unknown mode \"" << mode << "\"\n\n";
			print_help(options);
			return ERR_BAD_ARGS;
		}

		modeToInvoke->second();
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n\n";
		print_help(options);
		return ERR_EXCEPTION;
	}

	return ERR_NONE;
}

void enable_active_desktop()
{
	std::cout << "TODO: Active Desktop";
}


void print_help(const po::options_description& options)
{
	// Print out the description
	std::cout << "Fix wallpaper fading transition issues by enabling Active Desktop.\n\n";

	// Print out the usage
	std::cout
		<< "Usage:\n"
		<< "  TransitionFixer.exe -h | --help\n"
		<< "  TransitionFixer.exe -m | --mode (run | install)\n\n";

	std::cout << options << std::endl;
}

void register_task_scheduler()
{
	std::cout << "TODO: Register Task Scheduler";
}