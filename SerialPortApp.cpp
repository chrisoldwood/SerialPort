////////////////////////////////////////////////////////////////////////////////
//! \file   SerialPortApp.cpp
//! \brief  The SerialPortApp class definition.
//! \author Chris Oldwood

#include "Common.hpp"
#include "SerialPortApp.hpp"
#include <WCL/Path.hpp>
#include <WCL/VerInfoReader.hpp>
#include <WCL/Win32Exception.hpp>
#include <Core/CmdLineException.hpp>
#include <Core/AnsiWide.hpp>

////////////////////////////////////////////////////////////////////////////////
// The table of command line switches.

enum
{
	USAGE	= 0,	//!< Show the program options syntax.
	VERSION	= 1,	//!< Show the program version and copyright.
	PORT	= 2,	//!< The COM port number.
};

static Core::CmdLineSwitch s_switches[] = 
{
	{ USAGE,	TXT("?"),	NULL,			Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the program options syntax")	},
	{ USAGE,	TXT("h"),	TXT("help"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the program options syntax")	},
	{ VERSION,	TXT("v"),	TXT("version"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the program version")			},
	{ PORT,		TXT("p"),	TXT("port"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("port"),	TXT("The COM port number to write to")		},
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

int SerialPortApp::run(int argc, tchar* argv[], tistream& in, tostream& out, tostream& /*err*/)
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

	if (!m_parser.isSwitchSet(PORT))
		throw Core::CmdLineException(TXT("No COM port number specified [--port]"));

	const uint portNumber = Core::parse<uint>(m_parser.getSwitchValue(PORT));

	if ((portNumber < 1) || (portNumber > 9) )
		throw Core::CmdLineException(TXT("Invalid COM port number, expecting 1-9"));

	const tstring filename = Core::fmt(TXT("\\\\.\\COM%u"), portNumber); 
	const HANDLE device = ::CreateFile(filename.c_str(), GENERIC_READWRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (device == INVALID_HANDLE_VALUE)
		throw WCL::Win32Exception(::GetLastError(), TXT("Failed to open serial port"));

	for (tstring line; std::getline(in, line);)
	{
		const std::string buffer = T2A(line);
		if (!::WriteFile(device, buffer.data(), buffer.size(), nullptr, nullptr))
			throw WCL::Win32Exception(::GetLastError(), TXT("Failed to write to serial port"));
	}

	::CloseHandle(device);

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
