#include <unistd.h>
#include "cli.h"
#include "version.h"
#include "throw.h"

Cli::Cli(int argc, char *const argv[])
{
	int opt;
	while((opt = getopt(argc, argv, ":hvl:i:o:pn")) != -1 && !m_needExit) {
		switch(opt) {
			case '?':
				xthrow("Option -%c not recognized; use -h for help", optopt);
				break;

			case ':':
				xthrow("Option -%c requires argument; use -h for help", optopt);
				break;

			case 'h':
				showHelp();
				break;

			case 'v':
				showVersion();
				break;

			case 'l':
				xassert(std::string(optarg).size() == 1, "Option -l requires one character");
				switch(optarg[0]) {
					case 'd':
						m_logLevel = Logger::LL_DBG;
						break;

					case 'v':
						m_logLevel = Logger::LL_VERB;
						break;

					case 'n':
						m_logLevel = Logger::LL_NORM;
						break;

					case 'e':
						m_logLevel = Logger::LL_ERR;
						break;

					default:
						xthrow("Unrecognized log level for -l; use -h for help");
						break;
				}
				break;

			case 'i':
				m_inputFile = optarg;
				break;

			case 'o':
				m_outputDir = optarg;
				break;

			case 'p':
				m_printAudioLevel = true;
				break;

			case 'n':
				m_suppressX = true;
				break;

			default:
				break;
		}
	}

	xassert(argc == optind, "Excessive arguments");
}

bool Cli::needExit() const
{
	return m_needExit;
}

Logger::Level Cli::getLogLevel() const
{
	return m_logLevel;
}

std::string Cli::getInputFile() const
{
	return m_inputFile;
}

std::string Cli::getOutputDir() const
{
	return m_outputDir;
}

bool Cli::getPrintAudioLevel() const
{
	return m_printAudioLevel;
}

bool Cli::getSuppressX() const
{
	return m_suppressX;
}

void Cli::showVersion()
{
	printf("ofdmtvrx %s built %s\n", version::getVersion().c_str(), version::getBuild().c_str());
	if(version::withX()) {
		printf("This build supports X Window System\n");
	}
	else {
		printf("This build doesn't support X Window System\n");
	}
	printf("Newest version: https://github.com/CircuitChaos/ofdmtvrx/\n");
	m_needExit = true;
}

void Cli::showHelp()
{
	const char help[] = "Syntax: ofdmrx [flags] [-l <logging level>] [-i <input file>] [-o <output dir>]\n"
	                    "\n"
	                    "Flags:\n"
	                    "  -h: show help (this screen)\n"
	                    "  -v: show version\n"
	                    "  -p: print peak input audio level once per second (with normal logging level)\n"
	                    "  -n: don't use X Window System (console only)\n"
	                    "\n"
	                    "Logging levels:\n"
	                    "  d: debug, all diagnostic info is printed\n"
	                    "  v: verbose, various states and decoding errors are printed\n"
	                    "  n: normal, only reasonably important info is printed\n"
	                    "  e: error, only serious errors are printed\n"
	                    "\n"
	                    "If input file is not specified, then stdin will be used.\n"
	                    "If output dir is not specified, then current dir will be used.\n";

	printf("%s", help);
	if(!version::withX()) {
		printf("-n has no effect in this build\n");
	}

	m_needExit = true;
}
