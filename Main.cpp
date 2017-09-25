////////////////////////////////////////////////////////////////////////////////
//! \file   Main.cpp
//! \brief  The console application entry point.
//! \author Chris Oldwood

#include "Common.hpp"
#include <tchar.h>
#include "SerialPortApp.hpp"
#include <Core/tiostream.hpp>

//! The application.
static SerialPortApp g_app;

////////////////////////////////////////////////////////////////////////////////
//! The process entry point.

int _tmain(int argc, tchar* argv[])
{
	return g_app.main(argc, argv, tcin, tcout, tcerr);
}
