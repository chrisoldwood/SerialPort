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
	TEST	= 3,	//!< Test if the port exists.
	ECHO	= 4,	//!< Echo the input to stdout as well as the port.
};

static Core::CmdLineSwitch s_switches[] = 
{
	{ USAGE,	TXT("?"),	NULL,			Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the program options syntax")	},
	{ USAGE,	TXT("h"),	TXT("help"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the program options syntax")	},
	{ VERSION,	TXT("v"),	TXT("version"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the program version")			},
	{ PORT,		TXT("p"),	TXT("port"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("port"),	TXT("The COM port number to write to")		},
	{ TEST,		NULL,		TXT("test"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Test if the COM port exists")			},
	{ ECHO,		NULL,		TXT("echo"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Echo the port output to the screen")	},
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
//! Write text from stdin to the serial port.

static int writeText(uint portNumber, tistream& in, bool echo, tostream& out)
{
	const tstring filename = Core::fmt(TXT("\\\\.\\COM%u"), portNumber); 
	const HANDLE device = ::CreateFile(filename.c_str(), GENERIC_READWRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (device == INVALID_HANDLE_VALUE)
		throw WCL::Win32Exception(::GetLastError(), TXT("Failed to open serial port"));

	for (tstring line; std::getline(in, line);)
	{
		const std::string buffer = T2A(line);
		if (!::WriteFile(device, buffer.data(), buffer.size(), nullptr, nullptr))
			throw WCL::Win32Exception(::GetLastError(), TXT("Failed to write to serial port"));

		if (echo)
			out << line << std::endl;
	}

	::CloseHandle(device);

	return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! Test if the serial port can be opened.

static int testPort(uint portNumber, tostream& out, tostream& err)
{
	const tstring filename = Core::fmt(TXT("\\\\.\\COM%u"), portNumber); 
	const HANDLE device = ::CreateFile(filename.c_str(), GENERIC_READWRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

	if (device != INVALID_HANDLE_VALUE)
	{
		out << TXT("Port opened successfully.") << std::endl;
		return EXIT_FAILURE;
	}
	else
	{
		err << TXT("Failed to open port.") << std::endl;
		::CloseHandle(device);
		return EXIT_SUCCESS;
	}
}

////////////////////////////////////////////////////////////////////////////////
//! Run the application.

int SerialPortApp::run(int argc, tchar* argv[], tistream& in, tostream& out, tostream& err)
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

	if (m_parser.isSwitchSet(TEST))
		return testPort(portNumber, out, err);

	const bool echo = m_parser.isSwitchSet(ECHO);

	return writeText(portNumber, in, echo, out);
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
