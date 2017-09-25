////////////////////////////////////////////////////////////////////////////////
//! \file   SerialPortApp.cpp
//! \brief  The SerialPortApp class definition.
//! \author Chris Oldwood

#include "Common.hpp"
#include "SerialPortApp.hpp"
#include <WCL/Path.hpp>
#include <WCL/VerInfoReader.hpp>

////////////////////////////////////////////////////////////////////////////////
// The table of command line switches.

enum
{
	USAGE	= 0,	//!< Show the program options syntax.
	VERSION	= 1,	//!< Show the program version and copyright.
};

static Core::CmdLineSwitch s_switches[] = 
{
	{ USAGE,	TXT("?"),	NULL,			Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,	NULL,	TXT("Display the program options syntax")	},
	{ USAGE,	TXT("h"),	TXT("help"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,	NULL,	TXT("Display the program options syntax")	},
	{ VERSION,	TXT("v"),	TXT("version"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,	NULL,	TXT("Display the program version")			},
};
static size_t s_switchCount = ARRAY_SIZE(s_switches);

////////////////////////////////////////////////////////////////////////////////
//! Default constructor.

SerialPortApp::SerialPortApp()
	: m_parser(s_switches, s_switches+s_switchCount)
{
}

////////////////////////////////////////////////////////////////////////////////
//! Destructor.

SerialPortApp::~SerialPortApp()
{
}

////////////////////////////////////////////////////////////////////////////////
//! Run the application.

int SerialPortApp::run(int argc, tchar* argv[], tistream& /*in*/, tostream& out, tostream& /*err*/)
{
	m_parser.parse(argc, argv, Core::CmdLineParser::ALLOW_UNIX_FORMAT);

	// Request for help?
	if (m_parser.isSwitchSet(USAGE))
	{
		showUsage(out);
		return EXIT_SUCCESS;
	}
	// Request for version?
	else if (m_parser.isSwitchSet(VERSION))
	{
		showVersion(out);
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! Get the name of the application.

tstring SerialPortApp::applicationName() const
{
	return TXT("SerialPort");
}

////////////////////////////////////////////////////////////////////////////////
//! Display the program options syntax.

void SerialPortApp::showUsage(tostream& out) const
{
	out << std::endl;
	out << TXT("USAGE: ") << applicationName() << (" [options] ...") << std::endl;
	out << std::endl;

	out << m_parser.formatSwitches(Core::CmdLineParser::UNIX);
}
