////////////////////////////////////////////////////////////////////////////////
//! \file   SerialPortApp.hpp
//! \brief  The SerialPortApp class declaration.
//! \author Chris Oldwood

// Check for previous inclusion
#ifndef SERIALPORTAPP_HPP
#define SERIALPORTAPP_HPP

#if _MSC_VER > 1000
#pragma once
#endif

#include <WCL/ConsoleApp.hpp>

////////////////////////////////////////////////////////////////////////////////
//! The application.

class SerialPortApp : public WCL::ConsoleApp
{
public:
	//! Default constructor.
	SerialPortApp();

	//! Destructor.
	virtual ~SerialPortApp();
	
protected:
	//
	// ConsoleApp methods.
	//

	//! Run the application.
	virtual int run(int argc, tchar* argv[], tistream& in, tostream& out, tostream& err);

	//! Get the name of the application.
	virtual tstring applicationName() const;

	//! Display the program options syntax.
	virtual void showUsage(tostream& out) const;

private:
	//
	// Members.
	//
	Core::CmdLineParser m_parser;		//!< The command line parser.
};

#endif // SERIALPORTAPP_HPP
