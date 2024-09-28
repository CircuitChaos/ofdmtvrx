#pragma once

#include <string>
#include "log.h"

class Cli {
public:
	Cli(int argc, char *const argv[]);

	bool needExit() const;
	Logger::Level getLogLevel() const;
	std::string getInputFile() const;
	std::string getOutputDir() const;
	bool getPrintAudioLevel() const;

private:
	bool m_needExit{false};
	Logger::Level m_logLevel{Logger::LL_NORM};
	std::string m_inputFile{};
	std::string m_outputDir{};
	bool m_printAudioLevel{false};

	void showVersion();
	void showHelp();
};
