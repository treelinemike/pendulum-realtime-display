
// imu-data-displayDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "ChartCtrl.h"
#include "ChartLineSerie.h"
#include "ChartPointsSerie.h"
#include "SerialMFC.h"

#define DLE_byte 0x10
#define STX_byte 0x02
#define ETX_byte 0x03
#define PKT_TYPE_ST_DATA 0x01
#define PKT_TYPE_BOSCH_DATA 0x02
#define PKT_TYPE_ST_CONFIG 0xA1
#define PKT_TYPE_BOSH_CONFIG 0xA2
#define PKT_TYPE_ENCODER_DATA 0xAA
#define DEFAULT_PLOT_MAX_TIME 30               // [seconds]
#define IMU_PLOT_TIMER_PERIOD_MS 50    // 10ms = 100Hz; 20ms = 50Hz; 50ms = 20Hz; 100ms = 10Hz

// CimudatadisplayDlg dialog
class CimudatadisplayDlg : public CDialogEx
{
// Construction
public:
	CimudatadisplayDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IMUDATADISPLAY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void CimudatadisplayDlg::OnTimer(UINT_PTR nIDEvent);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	CStatic m_ctrlStatus;
	/*
	CChartCtrl m_ChartCtrl;
	CChartLineSerie* pLineSeriesGx;
	CChartLineSerie* pLineSeriesGy;
	CChartLineSerie* pLineSeriesGz;
	CChartLineSerie* pLineSeriesAx;
	CChartLineSerie* pLineSeriesAy;
	CChartLineSerie* pLineSeriesAz;

	CChartPointsSerie* pPointSeriesAZAX;
	CChartPointsSerie* pPointSeriesGXGZ;
	/**/
	CChartLineSerie* pLineSeriesTheta;
	CChartLineSerie* pLineSeriesThetaDot;
	CChartLineSerie* pLineSeriesPhase;

	/*
	CChartCtrl m_ChartCtrlGyro;
	CChartCtrl m_ChartCtrlAZAX;
	CChartCtrl m_ChartCtrlGXGZ;
	/**/

	CChartCtrl m_ChartCtrlEncoder;
	CChartCtrl m_ChartCtrlThetaDot;
	CChartCtrl m_ChartCtrlPhasePlot;
	CComboBox m_ComboBox;
	CButton m_CtrlButton1;
	CButton m_CtrlButton2;
	CButton m_CtrlButton3;
	CStatic m_TitleText;
	CEdit m_DisplayTime;
	/*
	CChartStandardAxis* pBottomAxisGyro;
	CChartStandardAxis* pBottomAxis;      // for the accelerometer plot, though unlabeled (TODO: fix this)
	/**/
	CChartStandardAxis* pBottomAxisEncoder;
	CChartStandardAxis* pBottomAxisThetaDot;
	afx_msg void OnStnClickedTitleText();
};

float imuVal16(uint16_t twosVal, float dataRange);