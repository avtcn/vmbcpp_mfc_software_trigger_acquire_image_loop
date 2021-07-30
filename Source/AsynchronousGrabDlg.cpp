/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AsynchronousGrabDlg.cpp

  Description: MFC dialog class for the GUI of the AsynchronousGrab example of
               VimbaCPP.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include <stdafx.h>
#include <AsynchronousGrab.h>
#include <AsynchronousGrabDlg.h>
#include "VmbTransform.h"
#include <iostream>
#include <sstream>
#include <string>
#define NUM_COLORS 3
#define BIT_DEPTH 8

using AVT::VmbAPI::FramePtr;
using AVT::VmbAPI::CameraPtrVector;
// Ctor
CAsynchronousGrabDlg::CAsynchronousGrabDlg( CWnd* pParent )
    : CDialog( CAsynchronousGrabDlg::IDD, pParent )
    , m_bIsStreaming( false )
{
    m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

BEGIN_MESSAGE_MAP( CAsynchronousGrabDlg, CDialog )
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED( IDC_BUTTON_STARTSTOP, &CAsynchronousGrabDlg::OnBnClickedButtonStartstop )

    // Here we add the event handlers for Vimba events
    ON_MESSAGE( WM_FRAME_READY, OnFrameReady )
    ON_MESSAGE( WM_CAMERA_LIST_CHANGED, OnCameraListChanged )
	ON_BN_CLICKED(IDC_BUTTON_OPEN_CAMERA, &CAsynchronousGrabDlg::OnBnClickedButtonOpenCamera)
	ON_BN_CLICKED(IDC_BUTTON_SW_TRIGGER, &CAsynchronousGrabDlg::OnBnClickedButtonSwTrigger)
	ON_BN_CLICKED(IDC_BUTTON_SW_TRIGGER_AUTO, &CAsynchronousGrabDlg::OnBnClickedButtonSwTriggerAuto)
	ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_CLOSE_CAMERA, &CAsynchronousGrabDlg::OnBnClickedButtonCloseCamera)
    
    ON_BN_CLICKED(IDC_BUTTON_OPEN_CAM_SYNC, &CAsynchronousGrabDlg::OnBnClickedButtonOpenCamSync)
    ON_BN_CLICKED(IDC_BUTTON_SYNC_ACQUIRE_IMG, &CAsynchronousGrabDlg::OnBnClickedButtonSyncAcquireImg)
    ON_BN_CLICKED(IDC_BUTTON_CLOSE_CAM_SYNC, &CAsynchronousGrabDlg::OnBnClickedButtonCloseCamSync)
    ON_BN_CLICKED(IDC_BUTTON_ACQUIRE_IMG_AUTO, &CAsynchronousGrabDlg::OnBnClickedButtonAcquireImgAuto)
END_MESSAGE_MAP()

BOOL CAsynchronousGrabDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetIcon( m_hIcon, TRUE );
    SetIcon( m_hIcon, FALSE );

    // Start Vimba
    VmbErrorType err = m_ApiController.StartUp();
    string_type DialogTitle( _TEXT( "AsynchronousGrab (MFC version) Vimba V" ) );
    SetWindowText( ( DialogTitle+m_ApiController.GetVersion() ).c_str() );
    Log( _TEXT( "Starting Vimba" ), err );
    if( VmbErrorSuccess == err )
    {
        // Initially get all connected cameras
        UpdateCameraListBox();
        string_stream_type strMsg;
        strMsg << "Cameras found..." << m_cameras.size();
        Log( strMsg.str() );
    }

    CButton* m_ctlCheck1 = (CButton*)GetDlgItem(IDC_CHECK_SHOW_IMAGE_EACH_CAPTURE);
    m_ctlCheck1->SetCheck(1);
    
    return TRUE;
}

void CAsynchronousGrabDlg::OnSysCommand( UINT nID, LPARAM lParam )
{
    if( SC_CLOSE == nID )
    {
        // if we are streaming stop streaming
        if (true == m_bIsStreaming) {


            OnBnClickedButtonStartstop();

        }


        // Before we close the application we stop Vimba
        m_ApiController.ShutDown();
    }

    CDialog::OnSysCommand( nID, lParam );
}

void CAsynchronousGrabDlg::OnBnClickedButtonStartstop()
{
    VmbErrorType err;
    int nRow = m_ListBoxCameras.GetCurSel();

    if( false == m_bIsStreaming )
    {
        if( -1 < nRow )
        {
            // Start acquisition
            err = m_ApiController.StartContinuousImageAcquisition( m_cameras[nRow] );
            // Set up image for MFC picture box
            if (VmbErrorSuccess == err )
            {
                m_Image.Create(m_ApiController.GetWidth(), -m_ApiController.GetHeight(), NUM_COLORS * BIT_DEPTH);
                m_ClearBackground = true;
            }
            Log(_TEXT("Starting Acquisition"), err);
            m_bIsStreaming = VmbErrorSuccess == err;
        }
        else
        {
            Log(_TEXT("Please select a camera."));
        }
    }
    else
    {
        m_bIsStreaming = false;
        // Stop acquisition
        err = m_ApiController.StopContinuousImageAcquisition();
        if( NULL != m_Image )
        {
            m_Image.Destroy();
        }
        Log( _TEXT( "Stopping Acquisition" ), err );
    }

    if( false == m_bIsStreaming )
    {
        m_ButtonStartStop.SetWindowText( _TEXT( "Start Image Acquisition" ) );
    }
    else
    {
        m_ButtonStartStop.SetWindowText( _TEXT( "Stop Image Acquisition" ) );
    }
}

//
// This event handler is triggered through a MFC message posted by the frame observer
//
// Parameters:
//  [in]    status          The frame receive status (complete, incomplete, ...)
//  [in]    lParam          [Unused, demanded by MFC signature]
//
// Returns:
//  Nothing, always returns 0
//
LRESULT CAsynchronousGrabDlg::OnFrameReady( WPARAM status, LPARAM lParam )
{
    if( true == m_bIsStreaming )
    {
        // Pick up frame
        bool ret = m_ApiController.GetCImage(&m_Image);

        ticksEnd = GetTickCount();
        double fps = lParam * 1.0 / ((ticksEnd - ticksStart) / 1000);

		// The waiting frame is coming now ...
		m_bWaitingFrame = false;

        VmbUint64_t iFrameID = lParam;

        // See if it is not corrupt
        if( VmbFrameStatusComplete == status )
        {
            Log(_TEXT("Received image, frame id "), iFrameID);
            TRACE("+++++++> Received image, frame id %08lld, sw click count %ld, fps %f\n", iFrameID, m_nClickCount, fps);

            // Display it
            RECT rect;
            m_PictureBoxStream.GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect, false);
        }
        else
        {
            // If we receive an incomplete image we do nothing but logging
            Log(_TEXT("Failure in receiving image id "), iFrameID);
            TRACE("-------> Failure in receiving image, frame id %08lld, sw click count %ld, fps %f\n", iFrameID, m_nClickCount, fps);
        } 
        
    }

    return 0;
}

//
// This event handler is triggered through a MFC message posted by the camera observer
//
// Parameters:
//  [in]    reason          The reason why the callback of the observer was triggered (plug-in, plug-out, ...)
//  [in]    lParam          [Unused, demanded by MFC signature]
//
// Returns:
//  Nothing, always returns 0
//
LRESULT CAsynchronousGrabDlg::OnCameraListChanged( WPARAM reason, LPARAM lParam )
{
    bool bUpdateList = false;

    // We only react on new cameras being found and known cameras being unplugged
    if( AVT::VmbAPI::UpdateTriggerPluggedIn == reason )
    {
        Log( _TEXT( "Camera list changed. A new camera was discovered by Vimba." ) );
        bUpdateList = true;
    }
    else if( AVT::VmbAPI::UpdateTriggerPluggedOut == reason )
    {
        Log( _TEXT( "Camera list changed. A camera was disconnected from Vimba." ) );
        if( true == m_bIsStreaming )
        {
            OnBnClickedButtonStartstop();
        }
        bUpdateList = true;
    }

    if( true == bUpdateList )
    {
        UpdateCameraListBox();
    }

    m_ButtonStartStop.EnableWindow( 0 < m_cameras.size() || m_bIsStreaming );

    return 0;
}

//
// Copies the content of a byte buffer to a MFC image with respect to the image's alignment
//
// Parameters:
//  [in]    pInbuffer       The byte buffer as received from the cam
//  [in]    ePixelFormat    The pixel format of the frame
//  [out]   OutImage        The filled MFC image
//
void CAsynchronousGrabDlg::CopyToImage( VmbUchar_t *pInBuffer, VmbPixelFormat_t ePixelFormat, CImage &OutImage )
{
    const int               nHeight         = m_ApiController.GetHeight();
    const int               nWidth          = m_ApiController.GetWidth();
    const int               nStride         = OutImage.GetPitch();
    const int               nBitsPerPixel   = OutImage.GetBPP();
    VmbError_t              Result;
    if( ( nWidth*nBitsPerPixel ) /8 != nStride )
    {
        Log( _TEXT( "Vimba only supports stride that is equal to width." ), VmbErrorWrongType );
        return;
    }
    VmbImage                SourceImage,DestinationImage;
    SourceImage.Size        = sizeof( SourceImage );
    DestinationImage.Size   = sizeof( DestinationImage );

    SourceImage.Data        = pInBuffer;
    DestinationImage.Data   = OutImage.GetBits();

    Result = VmbSetImageInfoFromPixelFormat( ePixelFormat, nWidth, nHeight, &SourceImage );
    if( VmbErrorSuccess != Result )
    {
        Log( _TEXT( "Error setting source image info." ), static_cast<VmbErrorType>( Result ) );
        return;
    }
    static const std::string DisplayFormat( "BGR24" );
    Result = VmbSetImageInfoFromString( DisplayFormat.c_str(),DisplayFormat.size(), nWidth,nHeight, &DestinationImage );
    if( VmbErrorSuccess != Result )
    {
        Log( _TEXT( "Error setting destination image info." ),static_cast<VmbErrorType>( Result ) );
        return;
    }
    Result = VmbImageTransform( &SourceImage, &DestinationImage,NULL,0 );
    if( VmbErrorSuccess != Result )
    {
        Log( _TEXT( "Error transforming image." ), static_cast<VmbErrorType>( Result ) );
    }
}

//
// Queries and lists all known camera
//
void CAsynchronousGrabDlg::UpdateCameraListBox()
{
    // Get all cameras currently connected to Vimba
    CameraPtrVector cameras = m_ApiController.GetCameraList();

    // Simply forget about all cameras known so far
    m_ListBoxCameras.ResetContent();
    m_cameras.clear();

    // And query the camera details again
    for(    CameraPtrVector::const_iterator iter = cameras.begin();
            cameras.end() != iter;
            ++iter )
    {
        std::string strCameraName;
        std::string strCameraID;
        if( VmbErrorSuccess != (*iter)->GetName( strCameraName ) )
        {
            strCameraName = "[NoName]";
        }
        // If for any reason we cannot get the ID of a camera we skip it
        if( VmbErrorSuccess == (*iter)->GetID( strCameraID ) )
        {
            
            std::string strInfo = strCameraName + " " + strCameraID;
            m_ListBoxCameras.AddString( CString( strInfo.c_str() ) );
            m_cameras.push_back( strCameraID );
        }
        else
        {
            Log( _TEXT("Could not get camera ID") );
        }
    }

    // Select first cam if none is selected
    if (    -1 == m_ListBoxCameras.GetCurSel()
         && 0 < m_cameras.size() )
    {
        m_ListBoxCameras.SetCurSel( 0 );
    }

    m_ButtonStartStop.EnableWindow( 0 < m_cameras.size() || m_bIsStreaming );
}

//
// Prints out a given logging string, error code and the descriptive representation of that error code
//
// Parameters:
//  [in]    strMsg          A given message to be printed out
//  [in]    eErr            The API status code
//
void CAsynchronousGrabDlg::Log( string_type strMsg, VmbErrorType eErr )
{
    strMsg += _TEXT( "..." ) + m_ApiController.ErrorCodeToMessage( eErr );
    m_ListLog.InsertString( 0, strMsg.c_str() );
}

void CAsynchronousGrabDlg::Log( string_type strMsg, VmbUint64_t iFrameID )
{
	strMsg += _TEXT("...") + std::to_wstring(iFrameID);
    m_ListLog.InsertString( 0, strMsg.c_str() );
}

void CAsynchronousGrabDlg::Log(string_type strMsg, long nClickCount, long nErrorCount)
{
    strMsg += _TEXT("..., Click: ") + std::to_wstring(nClickCount) + _TEXT(", Error: ") + std::to_wstring(nErrorCount);
    m_ListLog.InsertString(0, strMsg.c_str());
}

void CAsynchronousGrabDlg::Log(string_type strMsg, long nClickCount, long nErrorCount, double fps)
{
    strMsg += _TEXT("..., Click: ") + std::to_wstring(nClickCount) + _TEXT(", Error: ") + std::to_wstring(nErrorCount) 
        + _TEXT(", FPS: ") + std::to_wstring(fps) 
        + _TEXT(", Gap in ms: ") + std::to_wstring(1000.0/fps) 
        ;
    m_ListLog.InsertString(0, strMsg.c_str());
    OutputDebugString((strMsg.append(_TEXT("\n"))).c_str());
}


//
// Prints out a given logging string
//
// Parameters:
//  [in]    strMsg          A given message to be printed out
//
void CAsynchronousGrabDlg::Log( string_type strMsg )
{
    m_ListLog.InsertString( 0, strMsg.c_str() );
}

//
// The remaining functions are MFC intrinsic only
//

HCURSOR CAsynchronousGrabDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>( m_hIcon );
}

void CAsynchronousGrabDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CAMERAS, m_ListBoxCameras);
	DDX_Control(pDX, IDC_LIST_LOG, m_ListLog);
	DDX_Control(pDX, IDC_BUTTON_STARTSTOP, m_ButtonStartStop);
	DDX_Control(pDX, IDC_PICTURE_STREAM, m_PictureBoxStream);
	DDX_Control(pDX, IDC_BUTTON_OPEN_CAMERA, m_BtnOpenCamera);
	DDX_Control(pDX, IDC_BUTTON_SW_TRIGGER_AUTO, m_BtnSWTriggerAuto);
}

template <typename T>
CRect fitRect( T w, T h, const CRect &dst)
{
    double sw = static_cast<double>( dst.Width() ) / w;
    double sh = static_cast<double>( dst.Height() ) / h;
    double s = min( sw, sh );
    T new_w = static_cast<T>( w * s );
    T new_h = static_cast<T>( h * s );
    T off_w = (1 + dst.Width() - new_w) /2;
    T off_h = (1 + dst.Height() - new_h) /2;
    return CRect(off_w,off_h, off_w + new_w, off_h + new_h );
}

void CAsynchronousGrabDlg::OnPaint()
{
    if( IsIconic() )
    {
        CPaintDC dc( this );

        SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>( dc.GetSafeHdc() ), 0 );

        int cxIcon = GetSystemMetrics( SM_CXICON );
        int cyIcon = GetSystemMetrics( SM_CYICON );
        CRect rect;
        GetClientRect( &rect );
        int x = ( rect.Width() - cxIcon + 1 ) / 2;
        int y = ( rect.Height() - cyIcon + 1 ) / 2;
        dc.DrawIcon( x, y, m_hIcon );
    }
    else
    {
        CDialog::OnPaint();

        if (NULL != m_Image && m_bIsStreaming)
        {
            CPaintDC dc( &m_PictureBoxStream );
            CRect rect;
            m_PictureBoxStream.GetClientRect( &rect );
            if( m_ClearBackground)
            {
                m_ClearBackground = false;
                CBrush clearBrush( GetSysColor( COLOR_BTNFACE) );
                dc.FillRect( rect, &clearBrush);
            }
            rect = fitRect( m_Image.GetWidth(), m_Image.GetHeight(), rect );
            // HALFTONE enhances image quality but decreases performance
            dc.SetStretchBltMode( HALFTONE );
            m_Image.StretchBlt( dc.m_hDC, rect );
        }
    }
}


// Open camera in Software trigger mode
void CAsynchronousGrabDlg::OnBnClickedButtonOpenCamera()
{
    VmbErrorType err;
    int nRow = m_ListBoxCameras.GetCurSel();

    if( false == m_bIsStreaming )
    {
        if( -1 < nRow )
        {
            // Start acquisition in Sofeware trigger
			err = m_ApiController.StartContinuousImageAcquisition(m_cameras[nRow], 1);
            // Set up image for MFC picture box
            if (    VmbErrorSuccess == err
                 && NULL == m_Image )
            {
                m_Image.Create(  m_ApiController.GetWidth(),
                                -m_ApiController.GetHeight(),
                                NUM_COLORS * BIT_DEPTH );
                m_ClearBackground = true;
            }
            Log( _TEXT( "Starting Acquisition in Software trigger mode" ), err );
            m_bIsStreaming = VmbErrorSuccess == err;
        }
        else
        {
            Log( _TEXT( "Please select a camera." ) );
        }
    }
    else
    {
        m_bIsStreaming = false;

        if (m_CaptureTimer != 0) {
            KillTimer(m_CaptureTimer);
            m_CaptureTimer = 0;
        } 

        // Stop acquisition
        err = m_ApiController.StopContinuousImageAcquisition();
        if( NULL != m_Image )
        {
            m_Image.Destroy();
        }
        Log( _TEXT( "Stopping Acquisition in Software trigger mode" ), err );
    }

    if( false == m_bIsStreaming )
    {
        //m_ButtonStartStop.SetWindowText( _TEXT( "Start Image Acquisition" ) );
		m_BtnOpenCamera.EnableWindow(false);
    }
    else
    {
    }
}


void CAsynchronousGrabDlg::OnBnClickedButtonSwTrigger()
{
    /*
	if (m_bWaitingFrame)
	{ 
		Log(_TEXT("WARNING: still waiting for new frame ..."));
		return;
	}*/
	
	m_bWaitingFrame = true;

	m_ApiController.TriggerSoftwareCapture();
	string_type strMsg = _TEXT("Emit software trigger command to camera ..., m_nClickCount == ");
	Log(strMsg + std::to_wstring(m_nClickCount));

	// Increase count
	m_nClickCount++;

}


void CAsynchronousGrabDlg::OnBnClickedButtonSwTriggerAuto()
{ 
	if (m_CaptureTimer == 0) {
		Log(_TEXT("Start software trigger automatically ...")); 

        // Set software trigger frequency
        float fps = 2.0;
		m_CaptureTimer = SetTimer(1, 1000/fps, NULL); // one event every 1000 ms = 1 s 

		m_BtnSWTriggerAuto.SetWindowText(_TEXT("Stop SW Trigger Auto"));

        ticksStart = GetTickCount();
	} 
	else
	{
		Log(_TEXT("Stop software trigger automatically!")); 
		KillTimer(m_CaptureTimer);
		m_CaptureTimer = 0;
		m_BtnSWTriggerAuto.SetWindowText(_TEXT("Start SW Trigger Auto"));

		m_bWaitingFrame = false;
	}


}


void CAsynchronousGrabDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == m_CaptureTimer)
    {

        // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
        Log(_TEXT("Timer Event 1 ..."));
        OnBnClickedButtonSwTrigger();
    }

    if (nIDEvent == m_SyncImagingTimer)
    {
        OnBnClickedButtonSyncAcquireImg();
        
    }

	CDialog::OnTimer(nIDEvent);
}


void CAsynchronousGrabDlg::OnTimer2(UINT_PTR nIDEvent)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    Log(_TEXT("Timer Event 2..."));
    OnBnClickedButtonSwTrigger();

    CDialog::OnTimer(nIDEvent);
}




void CAsynchronousGrabDlg::OnBnClickedButtonCloseCamera()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CAsynchronousGrabDlg::OnBnClickedButton3()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CAsynchronousGrabDlg::OnBnClickedButtonOpenCamSync()
{
    VmbErrorType err;
    int nRow = m_ListBoxCameras.GetCurSel();

    if (false == m_bIsStreaming)
    {
        if (-1 < nRow)
        {
            // Start acquisition in Sofeware trigger
            err = m_ApiController.StartCamera(m_cameras[nRow], 1);
            // Set up image for MFC picture box
            if (VmbErrorSuccess == err
                && NULL == m_Image)
            {
                m_Image.Create(m_ApiController.GetWidth(),
                    -m_ApiController.GetHeight(),
                    NUM_COLORS * BIT_DEPTH);
                m_ClearBackground = true;
            }
            Log(_TEXT("Starting Acquisition in Sync mode ..."), err);
            m_bIsStreaming = false;

            m_nClickCount = 0;
        }
        else
        {
            Log(_TEXT("Please select a camera."));
        }
    }
    else
    {
        m_bIsStreaming = false;
        // Stop acquisition
        err = m_ApiController.StopContinuousImageAcquisition();
        //m_ApiController.ClearFrameQueue();
        if (NULL != m_Image)
        {
            m_Image.Destroy();
        }
        Log(_TEXT("Stopping Acquisition in Sync mode"), err);
    }

    if (false == m_bIsStreaming)
    {
        //m_BtnOpenCamera.EnableWindow(false);
    }
    else
    {
    }

}


void CAsynchronousGrabDlg::OnBnClickedButtonSyncAcquireImg()
{

    // Get the frame
    FramePtr pFrame;
    VmbErrorType err = m_ApiController.AcquireSingleImage(pFrame);

    m_nClickCount++;

    ticksEnd = GetTickCount();
    double fps = m_nClickCount *1.0 / ((ticksEnd - ticksStart) / 1000);

    
    bool bCheckedShow = false;// GetDlgItemInt(IDC_CHECK_SHOW_IMAGE_EACH_CAPTURE, NULL, 1);  //�������з���������bSignedΪ1������ֱֵ����int����ȥ����

    CButton* m_ctlCheck1 = (CButton*)GetDlgItem(IDC_CHECK_SHOW_IMAGE_EACH_CAPTURE);
    bCheckedShow = (m_ctlCheck1->GetCheck() == 1) ? true : false;

    // See if it is not corrupt
    if (VmbErrorSuccess == err)
    {
        VmbUchar_t* pBuffer;
        VmbUchar_t* pColorBuffer = NULL;
        VmbUint64_t iFrameID = 0;

        VmbErrorType err = pFrame->GetImage(pBuffer);
        err = pFrame->GetFrameID(iFrameID);
        Log(_TEXT("Image received. "), m_nClickCount, m_nErrorCount, fps);

        
        if (bCheckedShow && VmbErrorSuccess == err)
        {
            VmbUint32_t nSize;
            err = pFrame->GetImageSize(nSize);
            if (VmbErrorSuccess == err)
            {
                VmbPixelFormatType ePixelFormat = m_ApiController.GetPixelFormat();
                CopyToImage(pBuffer, ePixelFormat, m_Image);
                // Display it
                RECT rect;
                m_PictureBoxStream.GetWindowRect(&rect);
                ScreenToClient(&rect);
                InvalidateRect(&rect, false);
            }
        }
    }
    else
    {
        // If we receive an incomplete image we do nothing but logging
        
        m_nErrorCount++;
        Log(_TEXT("Failure in receiving sync image, error count: "), m_nErrorCount);
    }


    

}


void CAsynchronousGrabDlg::OnBnClickedButtonCloseCamSync()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    m_ApiController.StopContinuousImageAcquisition();
}


void CAsynchronousGrabDlg::OnBnClickedButtonAcquireImgAuto()
{
    UINT nGap = 50;
    if (m_SyncImagingTimer == 0) {
        Log(_TEXT("Start sync capture automatically ..."));
        m_SyncImagingTimer = SetTimer(2, nGap, NULL); // one event every nGap ms, 1000 ms = 1 s 
        //m_BtnSWTriggerAuto.SetWindowText(_TEXT("Stop SW Trigger Auto"));

        m_nClickCount = 0;
        ticksStart = GetTickCount();

    }
    else
    {
        Log(_TEXT("Stop sync capture loop!"));
        KillTimer(m_SyncImagingTimer);
        m_SyncImagingTimer = 0;
        //m_BtnSWTriggerAuto.SetWindowText(_TEXT("Start SW Trigger Auto"));       
    }

}
