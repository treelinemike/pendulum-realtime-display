
// imu-data-displayDlg.cpp : implementation file
//
// MFC multithreading tutorial: https://www.tutorialspoint.com/mfc/mfc_multithreading.htm
// High-speed ChartCtrl project: https://www.codeproject.com/Articles/14075/High-speed-Charting-Control
// MFC Serial port wrapper: https://www.codeproject.com/Articles/992/Serial-library-for-C
// Serial port functionality (not used, doesn't work with MFC): https://msdn.microsoft.com/en-us/library/system.io.ports.serialport(v=vs.90).aspx
// Specifiying COM ports > 10: https://support.microsoft.com/en-us/help/115831/howto-specify-serial-ports-larger-than-com9

#include <Afxwin.h>
#include "stdafx.h"
#include "pendulum-realtime-display.h"
#include "pendulum-realtime-displayDlg.h"
#include "afxdialogex.h"
#include <cmath>
#include <deque>
#include "ChartCtrl.h"
#include "ChartLineSerie.h"
#include "ChartPointsSerie.h"
#include "ChartAxisLabel.h"
#include "SerialMFC.h"
#include "CString"
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CimudatadisplayDlg dialog

// global variables
double t = 0;
double Gx, Gy, Gz, Ax, Ay, Az;
unsigned long int encCount = 0;
unsigned long int encSub = 0;
double theta = 0;
double theta_dot = 0;
BOOL stopNow;
const double pi = 3.14159265358979323846;
CSerial serial;
unsigned int latestByte;
DWORD dwBytesRead = 0;
BYTE  abBuffer[100];
int errorCode = 0;
unsigned int microTime;
std::deque<unsigned int> serialBytes;
BOOL firstRun = true;
unsigned int plotMaxTime;

CimudatadisplayDlg::CimudatadisplayDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IMUDATADISPLAY_DIALOG, pParent){
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CimudatadisplayDlg::DoDataExchange(CDataExchange* pDX){
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_ctrlStatus);
	// Note: comments below with AFX_... are important!
	//{{AFX_DATA_MAP(CChartDemoDlg)
	//DDX_Control(pDX, IDC_CUSTOM3, m_ChartCtrl);
	DDX_Control(pDX, IDC_ENCDISP, m_ChartCtrlEncoder);
	DDX_Control(pDX, IDC_THETADOT, m_ChartCtrlThetaDot);
	DDX_Control(pDX, IDC_PHASEPLOT, m_ChartCtrlPhasePlot);
	//}}AFX_DATA_MAP
	//ON_WM_SERIAL(OnSerialMsg);
	/*
	DDX_Control(pDX, IDC_GYROPLOT, m_ChartCtrlGyro);
	DDX_Control(pDX, IDC_AZAX, m_ChartCtrlAZAX);
	DDX_Control(pDX, IDC_GXGZ, m_ChartCtrlGXGZ);
	/**/
	DDX_Control(pDX, IDC_COM_PORT_SELECT, m_ComboBox);
	DDX_Control(pDX, IDC_BUTTON1, m_CtrlButton1);
	DDX_Control(pDX, IDC_BUTTON2, m_CtrlButton2);
	DDX_Control(pDX, IDC_BUTTON3, m_CtrlButton3);
	DDX_Control(pDX, IDC_TITLE_TEXT, m_TitleText);
	DDX_Control(pDX, IDC_EDIT1, m_DisplayTime);
}

BEGIN_MESSAGE_MAP(CimudatadisplayDlg, CDialogEx)
	ON_WM_TIMER() // IMPORTANT
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CimudatadisplayDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CimudatadisplayDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CimudatadisplayDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CimudatadisplayDlg::OnBnClickedButton3)
	ON_STN_CLICKED(IDC_TITLE_TEXT, &CimudatadisplayDlg::OnStnClickedTitleText)
END_MESSAGE_MAP()


// CimudatadisplayDlg message handlers

BOOL CimudatadisplayDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160, _T("Arial Bold"));
	m_TitleText.SetFont(m_Font1);
	
	plotMaxTime = DEFAULT_PLOT_MAX_TIME;
	CString sStatusMsg;
	sStatusMsg.Format(L"%2d", plotMaxTime);
	m_DisplayTime.SetWindowTextW(sStatusMsg);
	m_CtrlButton2.EnableWindow(false);

	/*
	// Configure Gyro Chart
	m_ChartCtrlGyro.EnableRefresh(false);
	pBottomAxisGyro = m_ChartCtrlGyro.CreateStandardAxis(CChartCtrl::BottomAxis);
	CChartStandardAxis* pLeftAxisGyro = m_ChartCtrlGyro.CreateStandardAxis(CChartCtrl::LeftAxis);
	pBottomAxisGyro->SetMinMax(0, plotMaxTime);
	pBottomAxisGyro->GetLabel()->SetText(_T("Time [s]"));
	pLeftAxisGyro->SetMinMax(-1000, 1000);
	pLeftAxisGyro->GetLabel()->SetText(_T("Angular Rate [deg/s]"));
	m_ChartCtrlGyro.EnableRefresh(true);


	// Configure Accelerometer Chart
	m_ChartCtrl.EnableRefresh(false);
	pBottomAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::BottomAxis);
	CChartStandardAxis* pLeftAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::LeftAxis);
	pBottomAxis->SetMinMax(0, plotMaxTime);
	pBottomAxis->GetLabel()->SetText(_T("Time [s]"));
	pLeftAxis->SetMinMax(-2, 2);
	pLeftAxis->GetLabel()->SetText(_T("Acceleration [g]"));
	m_ChartCtrl.EnableRefresh(true);

	// Configure AzAx Chart
	m_ChartCtrlAZAX.EnableRefresh(false);
	CChartStandardAxis* pBottomAxisAZAX =
		m_ChartCtrlAZAX .CreateStandardAxis(CChartCtrl::BottomAxis);
	CChartStandardAxis* pLeftAxisAZAX =
		m_ChartCtrlAZAX.CreateStandardAxis(CChartCtrl::LeftAxis);
	pBottomAxisAZAX->SetMinMax(-2, 2);
	pBottomAxisAZAX->GetLabel()->SetText(_T("Ax [g]"));
	pLeftAxisAZAX->SetMinMax(-2, 2);
	pLeftAxisAZAX->GetLabel()->SetText(_T("Az [g]"));
	m_ChartCtrlAZAX.EnableRefresh(true);

	// Configure GxGz Chart
	m_ChartCtrlGXGZ.EnableRefresh(false);
	CChartStandardAxis* pBottomAxisGXGZ =
		m_ChartCtrlGXGZ.CreateStandardAxis(CChartCtrl::BottomAxis);
	CChartStandardAxis* pLeftAxisGXGZ =
		m_ChartCtrlGXGZ.CreateStandardAxis(CChartCtrl::LeftAxis);
	pBottomAxisGXGZ->SetMinMax(-500, 500);
	pBottomAxisGXGZ->GetLabel()->SetText(_T("Omega Z [deg/s]"));
	pLeftAxisGXGZ->SetMinMax(-500, 500);
	pLeftAxisGXGZ->GetLabel()->SetText(_T("Omega X [deg/s]"));
	m_ChartCtrlGXGZ.EnableRefresh(true);
	/**/

	// Configure Encoder Chart
	m_ChartCtrlEncoder.EnableRefresh(false);
	pBottomAxisEncoder = m_ChartCtrlEncoder.CreateStandardAxis(CChartCtrl::BottomAxis);
	CChartStandardAxis* pLeftAxisEncoder = m_ChartCtrlEncoder.CreateStandardAxis(CChartCtrl::LeftAxis);
	pBottomAxisEncoder->SetMinMax(0, plotMaxTime);
	pBottomAxisEncoder->GetLabel()->SetText(_T("Time [s]"));
	pLeftAxisEncoder->SetMinMax(-4*pi, 4*pi);
	pLeftAxisEncoder->GetLabel()->SetText(_T("Theta [rad]"));
	m_ChartCtrlEncoder.EnableRefresh(true);

	// Configure Theta Dot Chart
	m_ChartCtrlThetaDot.EnableRefresh(false);
	pBottomAxisThetaDot = m_ChartCtrlThetaDot.CreateStandardAxis(CChartCtrl::BottomAxis);
	CChartStandardAxis* pLeftAxisThetaDot = m_ChartCtrlThetaDot.CreateStandardAxis(CChartCtrl::LeftAxis);
	pBottomAxisThetaDot->SetMinMax(0, plotMaxTime);
	pBottomAxisThetaDot->GetLabel()->SetText(_T("Time [s]"));
	pLeftAxisThetaDot->SetMinMax(-20 * pi, 20 * pi);
	pLeftAxisThetaDot->GetLabel()->SetText(_T("Theta Dot [rad/s]"));
	m_ChartCtrlThetaDot.EnableRefresh(true);

	// Configure Phase Portrait Chart
	m_ChartCtrlPhasePlot.EnableRefresh(false);
	CChartStandardAxis* pBottomAxisPhasePlot =
		m_ChartCtrlPhasePlot.CreateStandardAxis(CChartCtrl::BottomAxis);
	CChartStandardAxis* pLeftAxisPhasePlot =
		m_ChartCtrlPhasePlot.CreateStandardAxis(CChartCtrl::LeftAxis);
	pBottomAxisPhasePlot->SetMinMax(-2*pi, 2*pi);
	pBottomAxisPhasePlot->GetLabel()->SetText(_T("Theta [rad]"));
	pLeftAxisPhasePlot->SetMinMax(-50, 50);
	pLeftAxisPhasePlot->GetLabel()->SetText(_T("Theta Dot [rad/s]"));
	m_ChartCtrlPhasePlot.EnableRefresh(true);


	// temporary color variable for series
	COLORREF SerieColor;

	/*
	// Initialize Gx
	pLineSeriesGx = m_ChartCtrlGyro.CreateLineSerie();
	SerieColor = RGB(255, 0, 0);
	pLineSeriesGx->SetColor(SerieColor);
	pLineSeriesGx->SetWidth(3);
	pLineSeriesGx->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrlGyro.EnableRefresh(true);

	// Initialize Gy
	pLineSeriesGy = m_ChartCtrlGyro.CreateLineSerie();
	SerieColor = RGB(0, 179, 0);
	pLineSeriesGy->SetColor(SerieColor);
	pLineSeriesGy->SetWidth(3);
	pLineSeriesGy->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrlGyro.EnableRefresh(true);

	// Initialize Gz
	pLineSeriesGz = m_ChartCtrlGyro.CreateLineSerie();
	SerieColor = RGB(0, 0, 255);
	pLineSeriesGz->SetColor(SerieColor);
	pLineSeriesGz->SetWidth(3);
	pLineSeriesGz->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrlGyro.EnableRefresh(true);

	// Initialize Ax
	pLineSeriesAx = m_ChartCtrl.CreateLineSerie();
	SerieColor = RGB(255, 0, 0);
	pLineSeriesAx->SetColor(SerieColor);
	pLineSeriesAx->SetWidth(3);
	pLineSeriesAx->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrl.EnableRefresh(true);

	// Initialize Ay
	pLineSeriesAy = m_ChartCtrl.CreateLineSerie();
	SerieColor = RGB(0, 179, 0);
	pLineSeriesAy->SetColor(SerieColor);
	pLineSeriesAy->SetWidth(3);
	pLineSeriesAy->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrl.EnableRefresh(true);

	// Initialize Az
	pLineSeriesAz = m_ChartCtrl.CreateLineSerie();
	SerieColor = RGB(0, 0, 255);
	pLineSeriesAz->SetColor(SerieColor);
	pLineSeriesAz->SetWidth(3);
	pLineSeriesAz->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrl.EnableRefresh(true);

	// Initialize AZAX
	pPointSeriesAZAX = m_ChartCtrlAZAX.CreatePointsSerie();
	SerieColor = RGB(255, 0, 255);
	pPointSeriesAZAX->SetColor(SerieColor);
	pPointSeriesAZAX->SetBorderColor(SerieColor);
	pPointSeriesAZAX->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrlAZAX.EnableRefresh(true);

	// Initialize GXGZ
	pPointSeriesGXGZ = m_ChartCtrlGXGZ.CreatePointsSerie();
	SerieColor = RGB(255, 0, 255);
	pPointSeriesGXGZ->SetColor(SerieColor);
	pPointSeriesGXGZ->SetBorderColor(SerieColor);
	pPointSeriesGXGZ->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrlGXGZ.EnableRefresh(true);
	/**/

	// Initialize Theta Line Series
	pLineSeriesTheta = m_ChartCtrlEncoder.CreateLineSerie();
	SerieColor = RGB(0, 0, 255);
	pLineSeriesTheta->SetColor(SerieColor);
	pLineSeriesTheta->SetWidth(3);
	pLineSeriesTheta->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrlEncoder.EnableRefresh(true);

	// Initialize ThetaDot Line Series
	pLineSeriesThetaDot = m_ChartCtrlThetaDot.CreateLineSerie();
	SerieColor = RGB(255, 0, 0);
	pLineSeriesThetaDot->SetColor(SerieColor);
	pLineSeriesThetaDot->SetWidth(3);
	pLineSeriesThetaDot->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrlThetaDot.EnableRefresh(true);

	// Initialize Phase Portrait Line Series
	pLineSeriesPhase = m_ChartCtrlPhasePlot.CreateLineSerie();
	SerieColor = RGB(0, 179, 0);
	pLineSeriesPhase->SetColor(SerieColor);
	pLineSeriesPhase->SetWidth(3);
	pLineSeriesPhase->SetSeriesOrdering(poNoOrdering);
	m_ChartCtrlPhasePlot.EnableRefresh(true);

	unsigned int comIdx;
	CString comStr;
	for (comIdx = 1; comIdx < 255; ++comIdx) {
		comStr.Format(L"\\\\.\\COM%d",comIdx);
		errorCode = serial.Open(comStr);
		if (errorCode == 0) {
			comStr.Format(L"COM%d", comIdx);
			m_ComboBox.AddString(comStr);
		}
		serial.Close();
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CimudatadisplayDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CimudatadisplayDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// THIS IS WHERE THE MAGIC HAPPENS, this thread just runs in the background
UINT MyThreadProc(LPVOID Param) {


	// flush buffers
	serial.Purge();

	// start reading
	do
	{
		if (stopNow) {
			serial.Close();
			AfxEndThread(0);
		}

		dwBytesRead = 0;
		//serial.Read(abBuffer, 1, &dwBytesRead);
		serial.Read(abBuffer, sizeof(abBuffer), &dwBytesRead);
		if (dwBytesRead > 0) {
			unsigned int byteNum;
			for (byteNum = 0; byteNum < dwBytesRead; ++byteNum)
			{
				serialBytes.push_back(abBuffer[byteNum]);
			}			
			latestByte = serialBytes[serialBytes.size()-1];
		}

		//cout << "Size: " << serialBytes.size() << endl;

		// find the start of a message
		while ((serialBytes.size() > 1) && !((serialBytes[0] == DLE_byte) && (serialBytes[1] == STX_byte))) {
			
			// if buffer starts with two DLE bytes, get rid of them both
			if (((serialBytes[0] == DLE_byte) && (serialBytes[1] == DLE_byte))) {
				serialBytes.pop_front();
				serialBytes.pop_front();
			}
			else {
				// otherwise just get rid of first byte
				serialBytes.pop_front();
			}
		}
		

		if (serialBytes.size() > 2) { // we should have the start of a message
			
			BOOL msgFound = FALSE;
			unsigned int fwdIdx = 2;
			unsigned int revIdx;
			unsigned int numPrevDLE;
			BOOL atPrevDLE;


			while (!msgFound && (fwdIdx < serialBytes.size())) { // TODO: optimize; only run if latest byte added is an ETX and work backwards from that

																 // check whether this byte is an ETX
				if (serialBytes[fwdIdx] == ETX_byte) {

					// if so, see if we have an odd number of DLE bytes immediately preceeding it
					atPrevDLE = TRUE;
					numPrevDLE = 0;
					revIdx = fwdIdx - 1;
					while (atPrevDLE && (revIdx > 0)) {
						if (serialBytes[revIdx] == DLE_byte) {
							++numPrevDLE;
							--revIdx;
						}
						else {
							atPrevDLE = FALSE;
						}
					}
					// if we found an odd number of previous DLEs, this is a message
					if ((numPrevDLE > 0) && ((numPrevDLE % 2) != 0)) {
						msgFound = TRUE;
					}
				}

				// increment counter to move on to next byte
				++fwdIdx;
			}

			// if we found a message, extract it into its own structure
			if (msgFound) {
				
				std::deque<unsigned int> newMsg;
				unsigned int copyIdx;
				for (copyIdx = 0; copyIdx < fwdIdx; ++copyIdx) {

					// removed stuffed DLEs
					if ((serialBytes.size() > 1) && (serialBytes[0] == DLE_byte) && (serialBytes[1] == DLE_byte)) {
						serialBytes.pop_front();
						++copyIdx;
					}
					if (serialBytes.size() == 0) {
						//ASSERT HERE?
						//cout << "big error: 0 serial bytes" << endl;
					}
					newMsg.push_back(serialBytes[0]);
					serialBytes.pop_front();
				}
		
				if ((newMsg[2] == 0x01) || (newMsg[2] == 0x02)) {

					// PROCESS IMU PACKET
					// calculate delta t since last datapoint, then update current time
					unsigned int newMicroTime = (unsigned int)((((uint16_t)(newMsg[4])) << 8) + ((uint16_t)(newMsg[3])));
					if(firstRun){
						microTime = newMicroTime;
						//t = 0;  // set t=0 at time of declaration or reset; allows for start/stop functionality
						firstRun = false;
					} else {
						if (newMicroTime > microTime) {

							t += ((double)(newMicroTime - microTime))*(0.000016);
							microTime = newMicroTime;
						}else {
							t += ((double)((65535-microTime) + newMicroTime +1))*(0.000016);
							microTime = newMicroTime;
						}
					}
					/**/

					// per ST axes
					/*
					Gx = imuVal16((uint16_t)((((uint16_t)(newMsg[9])) << 8) + ((uint16_t)(newMsg[8]))), 1000.0F);
					Gy = imuVal16((uint16_t)((((uint16_t)(newMsg[11])) << 8) + ((uint16_t)(newMsg[10]))), 1000.0F);
					Gz = imuVal16((uint16_t)((((uint16_t)(newMsg[13])) << 8) + ((uint16_t)(newMsg[12]))), 1000.0F);
					Ax = imuVal16((uint16_t)((((uint16_t)(newMsg[15])) << 8) + ((uint16_t)(newMsg[14]))), 2.0F);
					Ay = imuVal16((uint16_t)((((uint16_t)(newMsg[17])) << 8) + ((uint16_t)(newMsg[16]))), 2.0F);
					Az = imuVal16((uint16_t)((((uint16_t)(newMsg[19])) << 8) + ((uint16_t)(newMsg[18]))), 2.0F);
					/**/

					/**/
					// per OrthoSensor axes
					Gy = imuVal16((uint16_t)((((uint16_t)(newMsg[9])) << 8) + ((uint16_t)(newMsg[8]))), 1000.0F);
					Gx = -1*imuVal16((uint16_t)((((uint16_t)(newMsg[11])) << 8) + ((uint16_t)(newMsg[10]))), 1000.0F);
					Gz = imuVal16((uint16_t)((((uint16_t)(newMsg[13])) << 8) + ((uint16_t)(newMsg[12]))), 1000.0F);
					Ay = imuVal16((uint16_t)((((uint16_t)(newMsg[15])) << 8) + ((uint16_t)(newMsg[14]))), 2.0F);
					Ax = -1*imuVal16((uint16_t)((((uint16_t)(newMsg[17])) << 8) + ((uint16_t)(newMsg[16]))), 2.0F);
					Az = imuVal16((uint16_t)((((uint16_t)(newMsg[19])) << 8) + ((uint16_t)(newMsg[18]))), 2.0F);
					/**/

				} else if ((newMsg[2] == 0xAA)) {
					// PROCESS ENCODER PACKET
					

					/**/
					// calculate delta t since last datapoint, then update current time
					unsigned int newMicroTime = (unsigned int)((((uint16_t)(newMsg[4])) << 8) + ((uint16_t)(newMsg[3])));
					double dt = -1;
					if (firstRun) {
						microTime = newMicroTime;
						//t = 0;  // set t=0 at time of declaration or reset; allows for start/stop functionality
						firstRun = false;
					}
					else {
						if (newMicroTime > microTime) {
							dt = ((double)(newMicroTime - microTime))*(0.000016);
						}
						else {
							dt = ((double)((65535 - microTime) + newMicroTime + 1))*(0.000016);
						}
						t += dt;
						microTime = newMicroTime;
					}

					// get encoder count
					unsigned long int newEncoderCount = (unsigned long int)( (((unsigned long int)(newMsg[7])) << 16) + (((unsigned long int)(newMsg[6])) << 8)  + ((unsigned long int)(newMsg[5])) );
					
					// convert to angle (including unwrapping)
					if (((long int)((long int)newEncoderCount - (long int)encCount) > 8000000)) {
						encSub = 16777216;
					}
					else if (( (long int)(  (long int)newEncoderCount - (long int)encCount) < -8000000)) {
						encSub = 0;
					}
					double newTheta = 2 * pi*(((double)newEncoderCount - (double)encSub) / 32768.0);
				
					// compute velocity (unfiltered)
					if (dt > 0) {
						theta_dot = (newTheta-theta)/dt;
					}

					// update current encoder count and angle
					encCount = newEncoderCount;
					theta = newTheta;
					
				} else {
					// discard message - TODO: this is a hack, figure out why messages are bad and fix bugs, may want to ASSERT()
					newMsg.clear();
				}
			}
		}
	} while (1);

	// done; we'll never get here, though thread may be terminated by parent
	return TRUE;
}

// change display only when timer expires, nifty... (effectively time compare interrupt)
void CimudatadisplayDlg::OnTimer(UINT_PTR nIDEvent) {
	// TODO: Add your message handler code here and/or call default
	CString sStatusMsg;
	//sStatusMsg.Format(L"Running: %d (%+0.2f,%+0.2f)", currValue, t, Az);

	// reset time to zero on domain overflow
	if (t > ((double)plotMaxTime)) {
		OnBnClickedButton3();
	}

	sStatusMsg.Format(L"Connected: t = %0.2f", t);
	m_ctrlStatus.SetWindowText(sStatusMsg);

	/*
	sStatusMsg.Format(L"Encoder count = %d", encCount);
	m_ctrlStatus.SetWindowText(sStatusMsg);
	/**/

	/*
	pLineSeriesGx->AddPoint(t, Gx);
	pLineSeriesGy->AddPoint(t, Gy);
	pLineSeriesGz->AddPoint(t, Gz);
	pLineSeriesAx->AddPoint(t, Ax);
	pLineSeriesAy->AddPoint(t, Ay);
	pLineSeriesAz->AddPoint(t, Az);
	pPointSeriesAZAX->AddPoint(Ax, Az);
	pPointSeriesGXGZ->AddPoint(Gz, Gx);
	/**/
	pLineSeriesTheta->AddPoint(t, theta);
	pLineSeriesThetaDot->AddPoint(t, theta_dot);
	pLineSeriesPhase->AddPoint(theta, theta_dot);
	CDialogEx::OnTimer(nIDEvent);
}



void CimudatadisplayDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	serial.Close();
	CDialogEx::OnOK();
}


void CimudatadisplayDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CString portStr, plotTimeStr;

	// make sure an appropriate COM port is actually selected
	if (m_ComboBox.GetCurSel() == -1){
		MessageBox(L"No COM port selected!", L"Error");
	}
	else {

		// open serial port
		m_ComboBox.GetLBText(m_ComboBox.GetCurSel(), portStr);
		errorCode = serial.Open(L"\\\\.\\" + portStr);
		serial.Setup(CSerial::EBaud115200, CSerial::EData8, CSerial::EParNone, CSerial::EStop1);
		serial.SetupHandshaking(CSerial::EHandshakeOff);


		if (errorCode) {
			portStr.Format(L"Error opening serial port: 0x%02X", errorCode);
			m_ctrlStatus.SetWindowText(portStr);
		}
		else {
			m_CtrlButton1.EnableWindow(false);
			m_CtrlButton2.EnableWindow(true);
			m_ComboBox.EnableWindow(false);
			m_DisplayTime.EnableWindow(false);

			firstRun = true;

			// update time axis upper limit
			m_DisplayTime.GetWindowTextW(plotTimeStr);
			plotMaxTime = _ttoi(plotTimeStr);
			/*
			pBottomAxisGyro->SetMinMax(0, plotMaxTime);
			pBottomAxis->SetMinMax(0, plotMaxTime);
			/**/
			pBottomAxisEncoder->SetMinMax(0, plotMaxTime);
			pBottomAxisThetaDot->SetMinMax(0, plotMaxTime);

			m_ctrlStatus.SetWindowText(L"Starting...");
			SetTimer(1234, IMU_PLOT_TIMER_PERIOD_MS, 0); // 3 times per second
			stopNow = false;

			AfxBeginThread(MyThreadProc, 0); // <<== START THE THREAD
		}
	}
}


void CimudatadisplayDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	stopNow = TRUE;
	KillTimer(1234);
	m_ctrlStatus.SetWindowText(L"Stopped");

	m_CtrlButton1.EnableWindow(true);
	m_CtrlButton2.EnableWindow(false);
	m_ComboBox.EnableWindow(true);
	m_DisplayTime.EnableWindow(true);

}


void CimudatadisplayDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	t = 0;
	firstRun = true;
	/*
	pLineSeriesGx->ClearSerie();
	pLineSeriesGy->ClearSerie();
	pLineSeriesGz->ClearSerie();
	pLineSeriesAx->ClearSerie();
	pLineSeriesAy->ClearSerie();
	pLineSeriesAz->ClearSerie();
	pPointSeriesAZAX->ClearSerie();
	pPointSeriesGXGZ->ClearSerie();
	/**/
	pLineSeriesTheta->ClearSerie();
	pLineSeriesThetaDot->ClearSerie();
	pLineSeriesPhase->ClearSerie();
}

// convert from bytes sent by IMU into actual float quantities
float imuVal16(uint16_t twosVal, float dataRange) {
	float dataVal;
	if (twosVal & ((uint16_t)0x8000)) {
		// negative value
		dataVal = ((float)(~((uint16_t)(twosVal - 1)) & 0xFFFF)*(-1 * dataRange)) / (32768);
	}
	else {
		// positive value
		dataVal = (twosVal*dataRange) / 32767;
	}

	return dataVal;
}



void CimudatadisplayDlg::OnStnClickedTitleText()
{
	// TODO: Add your control notification handler code here
}
