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
#include <Core/RuntimeException.hpp>
#include <iostream>

#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2)) // GCC 4.2+
// missing initializer for member 'X'
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

////////////////////////////////////////////////////////////////////////////////
// The table of command line switches.

enum
{
	USAGE	= 0,	//!< Show the program options syntax.
	VERSION	= 1,	//!< Show the program version and copyright.
	PORT	= 2,	//!< The COM port number.
	TEST	= 3,	//!< Test if the port exists.
	ECHO	= 4,	//!< Echo the input to stdout as well as the port.
	DEFAULTS = 5,	//!< List the port default settings.
	MANUAL	= 6,	//!< Show the manual.
};

static Core::CmdLineSwitch s_switches[] =
{
	{ USAGE,	TXT("?"),	NULL,			Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the program options syntax")	},
	{ USAGE,	TXT("h"),	TXT("help"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the program options syntax")	},
	{ VERSION,	TXT("v"),	TXT("version"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the program version")			},
	{ PORT,		TXT("p"),	TXT("port"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::SINGLE,	TXT("port"),	TXT("The COM port number to write to")		},
	{ TEST,		NULL,		TXT("test"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Test if the COM port exists")			},
	{ ECHO,		NULL,		TXT("echo"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Echo the port output to the screen")	},
	{ DEFAULTS,	NULL,		TXT("defaults"), Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("List the port default settings")		},
	{ MANUAL,	NULL,		TXT("manual"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,		NULL,			TXT("Display the manual")					},
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
		const std::string buffer = std::string(T2A(line)) + std::string("\r\n");
		const DWORD length = buffer.length();
		DWORD written = 0;

		if (!::WriteFile(device, buffer.data(), length, &written, nullptr))
			throw WCL::Win32Exception(::GetLastError(), TXT("Failed to write to serial port"));

		if (written != length)
			throw Core::RuntimeException(Core::fmt(TXT("Failed to write %u bytes to the serial port, only wrote %u"), length, written));

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
//! Convert the parity value to a string.

static tstring formatParity(BYTE parity)
{
	switch (parity)
	{
		case NOPARITY:		return TXT("None");
		case ODDPARITY:		return TXT("Odd");
		case EVENPARITY:	return TXT("Even");
		case MARKPARITY:	return TXT("Mark");
		case SPACEPARITY:	return TXT("Space");
		default:	return Core::fmt(TXT("??? <value=%d>"), static_cast<int>(parity));
	}
}

////////////////////////////////////////////////////////////////////////////////
//! Convert the stop bits value to a string.

static tstring formatStopBits(BYTE stopBits)
{
	switch (stopBits)
	{
		case ONESTOPBIT:	return TXT("1");
		case ONE5STOPBITS:	return TXT("1.5");
		case TWOSTOPBITS:	return TXT("2");
		default:			return Core::fmt(TXT("??? <value=%d>"), static_cast<int>(stopBits));
	}
}

////////////////////////////////////////////////////////////////////////////////
//! Test if the serial port can be opened.

static int listDefaults(uint portNumber, tostream& out)
{
	const tstring filename = Core::fmt(TXT("\\\\.\\COM%u"), portNumber);
	const HANDLE device = ::CreateFile(filename.c_str(), GENERIC_READWRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (device == INVALID_HANDLE_VALUE)
		throw WCL::Win32Exception(::GetLastError(), TXT("Failed to open serial port"));

	DCB dcb = { sizeof(DCB) };

	if (!::GetCommState(device, &dcb))
		throw WCL::Win32Exception(::GetLastError(), TXT("Failed to retrieve serial port state"));

	out << TXT("Baud Rate: ") << static_cast<int>(dcb.BaudRate) << std::endl;
	out << TXT("Data Bits: ") << static_cast<int>(dcb.ByteSize) << std::endl;
	out << TXT("Parity   : ") << formatParity(dcb.Parity) << std::endl;
	out << TXT("Stop Bits: ") << formatStopBits(dcb.StopBits) << std::endl;

	::CloseHandle(device);

	return EXIT_SUCCESS;
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
	// Request for the manual?
	else if (m_parser.isSwitchSet(MANUAL))
	{
		showManual(err);
		return EXIT_SUCCESS;
	}

	if (!m_parser.isSwitchSet(PORT))
		throw Core::CmdLineException(TXT("No COM port number specified [--port]"));

	const uint portNumber = Core::parse<uint>(m_parser.getSwitchValue(PORT));

	if ((portNumber < 1) || (portNumber > 9) )
		throw Core::CmdLineException(TXT("Invalid COM port number, expecting 1-9"));

	if (m_parser.isSwitchSet(TEST))
		return testPort(portNumber, out, err);

	if (m_parser.isSwitchSet(DEFAULTS))
		return listDefaults(portNumber, out);

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
