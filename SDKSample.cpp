// =STS=> SDKSample.cpp[4270].aa00   closed   SMID:1 
/////////////////////////////////////////////////////////////////////////////
//
// (c) Copyright 2011 - 2012 Blackrock Microsystems
//
// $Workfile: SDKSample.cpp $
// $Archive: /Cerebus/Human/WindowsApps/SDKSample/SDKSample.cpp $
// $Revision: 1 $
// $Date: 3/29/11 9:23a $
// $Author: Ehsan $
//
// $NoKeywords: $
//
/////////////////////////////////////////////////////////////////////////////
//
// PURPOSE: cbmex SDK example application
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <afxmt.h>
#include "SDKSample.h"
#include "SDKSampleDlg.h"
#include "Wincon.h"
#include "Dragonfly.h"
#include "message_defs.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include<string.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CSDKSampleApp

BEGIN_MESSAGE_MAP(CSDKSampleApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSDKSampleApp construction

class spikeDataHolder {
public:
	
	int spikesRecieved;
	const int maxSpikesStored = 500;
	cbPKT_SPK spikeArray[500];
	
	void saveSpike(ofstream* f, cbPKT_SPK spike) {

	}
	
	spikeDataHolder() {
		spikesRecieved = 0;
	}

};

bool testDragonflyMessaging(Dragonfly_Module* mod) {

	MDF_TEST_DATA test_message;
	test_message.a = 1;
	test_message.b = 2;


	CMessage msg(MT_TEST_DATA);
	msg.SetData(&test_message, sizeof(test_message));
	mod->SendMessageDF(&msg);
	
	return true;

}


void connectToMessageLogger() {
	MDF_SAVE_MESSAGE_LOG spike_message_log;
	spike_message_log.pathname_length = 18;
	char pathname[] = "C:\dragonfly_logs";
	strcpy(spike_message_log.pathname, pathname);
}

void spikesCallback(UINT32 nInstacne, const cbSdkPktType type, const void* pEventData, void* pCallbackData)
{
	spikeDataHolder * pDlg = reinterpret_cast<spikeDataHolder *>(pCallbackData);
	//cbPKT_SPK spikesArray[500] = pDlg;
	
	switch (type)
	{
	case cbSdkPkt_PACKETLOST:
		break;
	case cbSdkPkt_SPIKE:
		if (pDlg && pEventData)
		{
			cbPKT_SPK spk = *reinterpret_cast<const cbPKT_SPK *>(pEventData);
			// Note: Callback should return fast, so it is better to buffer it  here
			//       and use another thread to handle data
			//pDlg->AddSpike(spk);
			//spikesArray[spikeIndex] = spk;
			//pass to some array of spikes?
			int index = pDlg->spikesRecieved;

			if (index >= pDlg->maxSpikesStored){
				index = 0;
				pDlg->spikesRecieved = 0;
			}

			pDlg->spikesRecieved += 1;

			pDlg->spikeArray[index] = spk;
		}
		break;
	default:
		break;
	}
	return;
}

void registerCallBack(spikeDataHolder* sData)
{
	cbPKT_SPK spikesArray[500];
	void* point= &spikesArray;
	cbSdkResult res = cbSdkRegisterCallback(0, CBSDKCALLBACK_SPIKE, spikesCallback, point);
	if (res != CBSDKRESULT_SUCCESS)
	{
		std::cout << res << std::endl;
		return;
	}
	std::cout << "Successfully listening to the spikes" << std::endl;
}

void BindStdHandlesToConsole()
{
	// Redirect the CRT standard input, output, and error handles to the console
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	//Clear the error state for each of the C++ standard stream objects. We need to do this, as
	//attempts to access the standard streams before they refer to a valid target will cause the
	//iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
	//to always occur during startup regardless of whether anything has been read from or written to
	//the console or not.
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
}

void LaunchDebugConsolse() {

	AllocConsole();
	BindStdHandlesToConsole();
}

void initFileStreaming(ofstream* f) {
	f->open("spikeData.txt");

	std::chrono::time_point<std::chrono::system_clock> currentTime;
	currentTime = std::chrono::system_clock::now();
	std::time_t startTime = std::chrono::system_clock::to_time_t(currentTime);

	*f << "Recording spike data at "<< std::ctime(&startTime);
	//f->close();
}
void saveSpike(ofstream* f, cbPKT_SPK spk) {
	*f << spk.time; //Time
	*f << spk.chid << std::endl; //Channel
	for (int i = 0; i < cbMAX_PNTS; i++) {
		*f << spk.wave[i];
	}
}

Dragonfly_Module * init_Dragonfly() {
	Dragonfly_Module * mod = new Dragonfly_Module(MID_PRODUCER, 0);
	cout << "Attempted to send dragonfly message" << std::endl;
	mod->ConnectToMMM();
	mod->Subscribe(MT_TEST_DATA);
	mod->Subscribe(MT_EXIT);

	std::cout << "Request running...\n" << std::endl;
	return mod;
}

void send_Spike_Message(Dragonfly_Module* mod, MDF_SPIKE spikeMessage) {
	CMessage msg(MT_SPIKE);
	msg.SetData(&spikeMessage, sizeof(spikeMessage));
	mod->SendMessageDF(&msg);
}

MDF_SPIKE createSpikeMessage(cbPKT_SPK spk) {
	MDF_SPIKE spikeMessage;
	spikeMessage.chid = spk.chid;
	spikeMessage.time = spk.time;
	return spikeMessage;
}

void init(){

	LaunchDebugConsolse();

	cbSdkVersion ver;
	cbSdkResult res = cbSdkGetVersion(0, &ver);
		
	std::cout << "Using Library " << ver.major<<"." <<ver.minor<<std::endl;
	std::cout<< "Attempting to connect to NSP" << std::endl;


	Dragonfly_Module * mod = init_Dragonfly();



	//bool messaging_working = testMessaging(mod);

	//cbSdkResult res = cbSdkOpen(0);
	res = cbSdkOpen(0);
	if (res != CBSDKRESULT_SUCCESS)
	{
		return;
	}
	cbSdkConnectionType conType;
	cbSdkInstrumentType instType;
	// Return the actual openned connection
	res = cbSdkGetType(0, &conType, &instType);
	if (res != CBSDKRESULT_SUCCESS)
	{
		return;
	}

	res = cbSdkGetVersion(0, &ver);
	if (res != CBSDKRESULT_SUCCESS)
	{
		return;
	}

	if (conType < 0 || conType > CBSDKCONNECTION_CLOSED)
		conType = CBSDKCONNECTION_CLOSED;
	if (instType < 0 || instType > CBSDKINSTRUMENT_COUNT)
		instType = CBSDKINSTRUMENT_COUNT;




	res = cbSdkGetVersion(0, &ver);
	CString strOld, strNew;

	if (res == CBSDKRESULT_SUCCESS)
	{
		res = cbSdkGetType(0, &conType, &instType);
		if (res == CBSDKRESULT_SUCCESS)
		{
			if (instType != CBSDKINSTRUMENT_LOCALNSP && instType != CBSDKINSTRUMENT_NSP)
				strOld = "nPlay";
			if (conType == CBSDKCONNECTION_CENTRAL)
				strOld += "(Central)";
		}
		strNew.Format("%s v%u.%02u.%02u.%02u", strOld, ver.nspmajor, ver.nspminor, ver.nsprelease, ver.nspbeta);
		std::cout << "Connected to" <<strNew<< std::endl;
	}
	else {
		std::cout << "Failed to connect";
	}

	//cbSdkResult cbSdkSetChannelConfig(UINT32 nInstance, UINT16 channel, cbPKT_CHANINFO * chaninfo);
	cbPKT_CHANINFO chaninfo;
	UINT16 channel = 1;
	res = cbSdkGetChannelConfig(0, channel, &chaninfo);
	std::cout<<"Spike threshold " << chaninfo.spkthrlevel <<" "<< chaninfo.spkthrlimit<< std::endl;


	
	spikeDataHolder* sData = new spikeDataHolder();
	cout << "Spikeindex " << (*sData).spikesRecieved << std::endl;

	std::cout << "Registering call back " << std::endl;
	registerCallBack(sData);

	ofstream* f = new ofstream();
	initFileStreaming(f);

	f->close();

}


CSDKSampleApp::CSDKSampleApp()
{
	
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CExampleApp object

CSDKSampleApp theApp;


// CSDKSampleApp initialization

// Author & Date:   Ehsan Azar     29 March 2011
// Purpose: Initialize SDK example application,
//           and make sure at most one instance runs.
// Outputs:
//   returns the error code
BOOL CSDKSampleApp::InitInstance()
{
	
    CMutex cbSDKSampleAppMutex(TRUE, "cbSDKSampleAppMutex");
    CSingleLock cbSDKSampleLock(&cbSDKSampleAppMutex);

	// We let only one instance of nPlay GUI
    if (!cbSDKSampleLock.Lock(0))
        return FALSE;

	
	init();
	

	CWinApp::InitInstance();

	CSDKSampleDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

//void main() {
//
//	LaunchDebugConsolse();
//	init();
//}