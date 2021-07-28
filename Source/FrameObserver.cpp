/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameObserver.cpp

  Description: The frame observer that is used for notifications from VimbaCPP
               regarding the arrival of a newly acquired frame.

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

#include <afxwin.h>
#include <FrameObserver.h>
#include "VmbTransform.h"

namespace AVT {
namespace VmbAPI {
namespace Examples {

//
// This is our callback routine that will be executed on every received frame.
// Triggered by the API.
//
// Parameters:
//  [in]    pFrame          The frame returned from the API
//
void FrameObserver::FrameReceived( const FramePtr pFrame )
{
    bool bQueueDirectly = true;
    VmbFrameStatusType eReceiveStatus;

    CWinApp* pApp = AfxGetApp();
    CWnd* pMainWin = NULL;
    if (NULL != pApp)
    {
        pMainWin = pApp->GetMainWnd();
        if (NULL != pMainWin)
        {
        }
    }

    if( VmbErrorSuccess == pFrame->GetReceiveStatus( eReceiveStatus ) )
    {
        VmbUint64_t iFrameID = 0;
        VmbUchar_t* pBuffer;
        VmbUchar_t* pColorBuffer = NULL;
        VmbErrorType err = pFrame->GetImage(pBuffer);

        err = pFrame->GetFrameID(iFrameID);
        if (err != VmbErrorSuccess)
            iFrameID = 0;

        VmbUint32_t nSize;
        err = pFrame->GetImageSize(nSize);

        // Lock the frame queue
        m_FramesMutex.Lock();

        m_FrameID = iFrameID;

        // Convert mono to bgr if necessary
        if (VmbPixelFormatMono8 == m_nPixelFormat)
        {
            pColorBuffer = new VmbUchar_t[nSize * NUM_COLORS];
            MonoToBGR(pBuffer, pColorBuffer, nSize);
            pBuffer = pColorBuffer;
        } 
        // Copy it
        // We need that because MFC might repaint the view after we have released the frame already
        CopyToImage(pBuffer, &m_Image);

        if (NULL != pColorBuffer)
        {
            delete[] pColorBuffer;
            pColorBuffer = NULL;
        } 


        // Unlock frame queue
        m_FramesMutex.Unlock();

        // And notify the view about it
        if(pMainWin)
            pMainWin->PostMessage(WM_FRAME_READY, eReceiveStatus, m_FrameID);
    }
    else {
        m_FrameID = 0;
        // And notify the view about it
        if (pMainWin)
            pMainWin->PostMessage(WM_FRAME_READY, eReceiveStatus, m_FrameID);
    }

    m_pCamera->QueueFrame(pFrame);
}

//
// After the view has been notified about a new frame it can pick it up.
// It is then removed from the internal queue
//
// Returns:
//  A shared pointer to the latest frame
//
/*
FramePtr FrameObserver::GetFrame()
{
    // Lock frame queue
    m_FramesMutex.Lock();
    // Pop the frame from the queue
    FramePtr res;
    if( ! m_Frames.empty() )
    {
        res = m_Frames.front();
        m_Frames.pop();
    }
    // Unlock the frame queue
    m_FramesMutex.Unlock();
    return res;
}
*/

bool FrameObserver::GetCImage(CImage* pOutImage)
{
    // Lock frame queue
    m_FramesMutex.Lock();

    //newImage = m_Image;
    // https://stackoverflow.com/questions/9571304/how-to-copy-a-cimage-object
    m_Image.BitBlt(pOutImage->GetDC(), 0, 0); 

    // Unlock the frame queue
    m_FramesMutex.Unlock();

    return true;
}


//
// Copies the content of a byte buffer to a MFC image with respect to the image's alignment
//
// Parameters:
//  [in]    pInbuffer       The byte buffer as received from the cam
//  [out]   OutImage        The filled MFC image
//
void FrameObserver::CopyToImage(VmbUchar_t* pInBuffer, CImage* pOutImage)
{
    if (NULL != *pOutImage)
    {
        VmbUchar_t* pCursor = (VmbUchar_t*)pOutImage->GetBits();
        int             nHeight = m_nHeight;
        int             nWidth = m_nWidth * NUM_COLORS;
        int             nStride = pOutImage->GetPitch() - nWidth;

        if (0 < nStride)
        {
            for (int y = 0; y < nHeight; ++y)
            {
                for (int x = 0; x < nWidth; ++x)
                {
                    *pCursor = *pInBuffer;
                    ++pCursor;
                    ++pInBuffer;
                }
                // Consider stride
                pCursor += nStride;
            }
        }
        else
        {
            memcpy(pOutImage->GetBits(), pInBuffer, nWidth * nHeight);
        }
    }
}


//
// Converts mono 8 bit to mono bgr
//
// Parameters:
//  [in]    pInBuffer       The input mono 8 byte buffer to convert
//  [out]   pOutBuffer      The output bgr byte buffer
//  [in]    nInSize         The size of the input buffer
//
void FrameObserver::MonoToBGR(VmbUchar_t* pInBuffer, VmbUchar_t* pOutBuffer, VmbUint32_t nInSize)
{
    while (0 < nInSize--)
    {
        for (int i = 0; i < NUM_COLORS; ++i)
        {
            *pOutBuffer = *pInBuffer;
            ++pOutBuffer;
        }

        ++pInBuffer;
    }
}



}}} // namespace AVT::VmbAPI::Examples
