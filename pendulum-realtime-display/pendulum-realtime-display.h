
// imu-data-display.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CimudatadisplayApp:
// See imu-data-display.cpp for the implementation of this class
//

class CimudatadisplayApp : public CWinApp
{
public:
	CimudatadisplayApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CimudatadisplayApp theApp;