#include "localizestrings.h"
#include "GUIWindowSlideShow.h"
#include "application.h"
#include "picture.h"
#include "settings.h"
#include "guiFontManager.h"
#include "util.h"
#include "sectionloader.h"

#define MAX_RENDER_METHODS 9
#define MAX_ZOOM_FACTOR    10
#define MAX_PICTURE_WIDTH  2048//1024
#define MAX_PICTURE_HEIGHT 2048//1024

#define LABEL_ROW1			10
#define LABEL_ROW2			11
#define LABEL_ROW2_EXTRA	12

CGUIWindowSlideShow::CGUIWindowSlideShow(void)
:CGUIWindow(0)
{
  m_pTextureBackGround=NULL;
  m_pTextureCurrent=NULL;
  Reset();
  srand( timeGetTime() );

}

CGUIWindowSlideShow::~CGUIWindowSlideShow(void)
{
	Reset();
  
}


bool CGUIWindowSlideShow::IsPlaying() const
{
  if (m_pTextureBackGround) return true;
  return false;
}

void CGUIWindowSlideShow::Reset()
{
	m_bShowInfo=false;
  m_bSlideShow=false;
  m_bPause=false;
 
  m_iRotate=0;
  m_iZoomFactor=1;
  m_iPosX=0;
  m_iPosY=0;
  m_dwFrameCounter=0;
	m_iCurrentSlide=-1;
	m_strBackgroundSlide="";
	m_strCurrentSlide="";
	m_lSlideTime=0;
	if (m_pTextureBackGround)
	{
		m_pTextureBackGround->Release();
		m_pTextureBackGround=NULL;
	}
  if (m_pTextureCurrent)
  {
		m_pTextureCurrent->Release();
		m_pTextureCurrent=NULL;
  }
  m_vecSlides.erase(m_vecSlides.begin(),m_vecSlides.end());
}

void CGUIWindowSlideShow::Add(const CStdString& strPicture)
{
	m_vecSlides.push_back(strPicture);
}



IDirect3DTexture8* CGUIWindowSlideShow::GetNextSlide(DWORD& dwWidth, DWORD& dwHeight, CStdString& strSlide)
{
	if (! m_vecSlides.size()) return NULL;

	m_iCurrentSlide++;
	if ( m_iCurrentSlide >= (int) m_vecSlides.size() ) 
	{
		m_iCurrentSlide=0;
	}
  m_iRotate=0;
  m_iZoomFactor=1;
  m_iPosX=0;
  m_iPosY=0;

	strSlide=m_vecSlides[m_iCurrentSlide];
	CPicture picture;

	IDirect3DTexture8* pTexture=picture.Load(strSlide,0,MAX_PICTURE_WIDTH,MAX_PICTURE_HEIGHT);
	dwWidth=picture.GetWidth();
	dwHeight=picture.GetHeight();
	return pTexture;
}

IDirect3DTexture8* CGUIWindowSlideShow::GetPreviousSlide(DWORD& dwWidth, DWORD& dwHeight, CStdString& strSlide)
{
	if (! m_vecSlides.size()) return NULL;

	m_iCurrentSlide--;
	if ( m_iCurrentSlide < 0 ) 
	{
		m_iCurrentSlide=m_vecSlides.size()-1;
	}
  m_iRotate=0;
  m_iZoomFactor=1;
  m_iPosX=0;
  m_iPosY=0;

	strSlide=m_vecSlides[m_iCurrentSlide];
	CPicture picture;

	IDirect3DTexture8* pTexture=picture.Load(strSlide,0,MAX_PICTURE_WIDTH,MAX_PICTURE_HEIGHT);
	dwWidth=picture.GetWidth();
	dwHeight=picture.GetHeight();
	return pTexture;
}


void  CGUIWindowSlideShow::ShowNext()
{
  if (m_bSlideShow) return;
  if (m_pTextureBackGround)
    m_pTextureBackGround->Release();

	m_pTextureBackGround=GetNextSlide(m_dwWidthBackGround,m_dwHeightBackGround, m_strBackgroundSlide);
	
}

void  CGUIWindowSlideShow::ShowPrevious()
{
  if (m_bSlideShow) return;
  if (m_pTextureBackGround)
    m_pTextureBackGround->Release();

  m_pTextureBackGround=GetPreviousSlide(m_dwWidthBackGround,m_dwHeightBackGround, m_strBackgroundSlide);
}


void CGUIWindowSlideShow::Select(const CStdString& strPicture)
{
  for (int i=0; i < (int)m_vecSlides.size(); ++i)
  {
    CStdString& strSlide=m_vecSlides[i];
    if (strSlide==strPicture)
    {
      m_iCurrentSlide=i-1;
      return;
    }
  }
}


bool CGUIWindowSlideShow::InSlideShow() const
{
  return m_bSlideShow;
}

void  CGUIWindowSlideShow::StartSlideShow()
{
  m_bSlideShow=true;
}

void CGUIWindowSlideShow::Render()
{
  m_dwFrameCounter++;
  int iSlides= m_vecSlides.size();
  if (!iSlides) return;

  if (m_bSlideShow || !m_pTextureBackGround) 
  {

    if (iSlides > 1 || m_pTextureBackGround==NULL)
    {
      if (m_pTextureCurrent==NULL)
      {
	      DWORD dwTimeElapsed = timeGetTime() - m_lSlideTime;
				if (dwTimeElapsed >= (DWORD)g_stSettings.m_iSlideShowStayTime || 
            m_pTextureBackGround==NULL)
	      {
          if (!m_bPause|| m_pTextureBackGround==NULL)
          {
		        // get next picture
					  m_pTextureCurrent=GetNextSlide(m_dwWidthCurrent,m_dwHeightCurrent, m_strCurrentSlide);
            m_dwFrameCounter=0;
            int iNewMethod;
            do
            {
              iNewMethod=rand() % MAX_RENDER_METHODS;
            } while ( iNewMethod==m_iTransistionMethod);
            m_iTransistionMethod=iNewMethod;
          }
	      }
      }
    }

		// render background
		if (!m_pTextureBackGround) 
		{
			if (!m_pTextureCurrent) return;
			m_pTextureBackGround=m_pTextureCurrent;
			m_dwWidthBackGround=m_dwWidthCurrent;
			m_dwHeightBackGround=m_dwHeightCurrent;
			m_strBackgroundSlide=m_strCurrentSlide;
			m_pTextureCurrent=NULL;
			m_lSlideTime=timeGetTime();
		}
	}

	g_graphicsContext.Get3DDevice()->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0x0, 1.0f, 0L );

	int x,y,width,height;
	GetOutputRect(m_dwWidthBackGround, m_dwHeightBackGround, x, y, width, height);
	CPicture picture;
	picture.RenderImage(m_pTextureBackGround,
											x+m_iPosX,                 
											y+m_iPosY,
											width*m_iZoomFactor, height*m_iZoomFactor,
											m_dwWidthBackGround,
											m_dwHeightBackGround);

	if (m_pTextureCurrent) 
	{
		// render the new picture
		bool bResult;
		switch (m_iTransistionMethod)
		{
			case 0:
				bResult=RenderMethod1();
			break;
			case 1:
				bResult=RenderMethod2();
			break;
			case 2:
				bResult=RenderMethod3();
			break;
			case 3:
				bResult=RenderMethod4();
			break;
			case 4:
				bResult=RenderMethod5();
			break;
			case 5:
				bResult=RenderMethod6();
			break;
			case 6:
				bResult=RenderMethod7();
			break;
			case 7:
				bResult=RenderMethod8();
			break;
			case 8:
				bResult=RenderMethod9();
			break;
		}
	  
		if (bResult)
		{
			if (!m_pTextureCurrent) return;
			if (m_pTextureBackGround)
			{
				m_pTextureBackGround->Release();
				m_pTextureBackGround=NULL;
			}
			m_pTextureBackGround=m_pTextureCurrent;
			m_dwWidthBackGround=m_dwWidthCurrent;
			m_dwHeightBackGround=m_dwHeightCurrent;
			m_strBackgroundSlide=m_strCurrentSlide;
			m_pTextureCurrent=NULL;  
			m_lSlideTime=timeGetTime();
		}
	}

	RenderPause();

	if (!m_bShowInfo && m_iZoomFactor == 1)
		return;

	if (m_iZoomFactor > 1)
	{
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW1); 
			msg.SetLabel(""); 
			OnMessage(msg);
		}
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW2); 
			msg.SetLabel(""); 
			OnMessage(msg);
		}
		CStdString strZoomInfo;
		strZoomInfo.Format("%ix (%i,%i)", m_iZoomFactor, m_iPosX, m_iPosY);
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW2_EXTRA); 
			msg.SetLabel(strZoomInfo); 
			OnMessage(msg);
		}
	}
	if ( m_bShowInfo )
	{
		CStdString strFileInfo, strSlideInfo;
		strFileInfo.Format("%ix%i %s", m_dwWidthBackGround, m_dwHeightBackGround,CUtil::GetFileName(m_strBackgroundSlide));
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW1); 
			msg.SetLabel(strFileInfo); 
			OnMessage(msg);
		}
		strSlideInfo.Format("%i/%i", m_iCurrentSlide ,m_vecSlides.size());
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW2); 
			msg.SetLabel(strSlideInfo); 
			OnMessage(msg);
		}
		if (m_iZoomFactor == 1)
		{
			CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW2_EXTRA); 
			msg.SetLabel(""); 
			OnMessage(msg);
		}
	}
	CGUIWindow::Render();
}

// open from left->right
bool CGUIWindowSlideShow::RenderMethod1()
{
	bool bResult(false);

	int x,y,width,height;
	GetOutputRect(m_dwWidthCurrent, m_dwHeightCurrent, x, y, width, height);

	int iStep= width/g_stSettings.m_iSlideShowTransistionFrames;
	if (!iStep) iStep=1;
	int iExpandWidth=m_dwFrameCounter*iStep;
	if (iExpandWidth >= width)
	{
		iExpandWidth=width;
		bResult=true;
	}
	CPicture picture;
	picture.RenderImage(m_pTextureCurrent, x, y, iExpandWidth, height, m_dwWidthCurrent, m_dwHeightCurrent);
	return bResult;
}

// move into the screen from left->right
bool CGUIWindowSlideShow::RenderMethod2()
{
	bool bResult(false);
	int x,y,width,height;
	GetOutputRect(m_dwWidthCurrent, m_dwHeightCurrent, x, y, width, height);

	int iStep= width/g_stSettings.m_iSlideShowTransistionFrames;
	if (!iStep) iStep=1;
	int iPosX = m_dwFrameCounter*iStep - (int)width;
	if (iPosX >=x)
	{
		iPosX=x;
		bResult=true;
	}
	CPicture picture;
	picture.RenderImage(m_pTextureCurrent, iPosX, y, width, height, m_dwWidthCurrent, m_dwHeightCurrent);
	return bResult;
}

// move into the screen from right->left
bool CGUIWindowSlideShow::RenderMethod3()
{
	bool bResult(false);
	int x,y,width,height;
	GetOutputRect(m_dwWidthCurrent, m_dwHeightCurrent, x, y, width, height);

	int iStep= width/g_stSettings.m_iSlideShowTransistionFrames;
	if (!iStep) iStep=1;
	int posx = x + width - m_dwFrameCounter*iStep ;
	if (posx <=x )
	{
		posx = x;
		bResult=true;
	}
	CPicture picture;
	picture.RenderImage(m_pTextureCurrent,posx,y,width,height,m_dwWidthCurrent,m_dwHeightCurrent);
	return bResult;
}

// move into the screen from up->bottom
bool CGUIWindowSlideShow::RenderMethod4()
{
	bool bResult(false);
	int x,y,width,height;
	GetOutputRect(m_dwWidthCurrent, m_dwHeightCurrent, x, y, width, height);
 
	int iStep= height/g_stSettings.m_iSlideShowTransistionFrames;
	if (!iStep) iStep=1;
	int posy = m_dwFrameCounter*iStep - height;
	if (posy >= y)
	{
		posy = y;
		bResult=true;
	}
	CPicture picture;
	picture.RenderImage(m_pTextureCurrent,x,posy,width,height,m_dwWidthCurrent,m_dwHeightCurrent);
	return bResult;
}

// move into the screen from bottom->top
bool CGUIWindowSlideShow::RenderMethod5()
{
	bool bResult(false);
	int x,y,width,height;
	GetOutputRect(m_dwWidthCurrent, m_dwHeightCurrent, x, y, width, height);

	int iStep= height/g_stSettings.m_iSlideShowTransistionFrames;
	if (!iStep) iStep=1;
	int posy = y+height-m_dwFrameCounter*iStep ;
	if (posy <=y)
	{
		posy=y;
		bResult=true;
	}
	CPicture picture;
	picture.RenderImage(m_pTextureCurrent,x,posy,width,height,m_dwWidthCurrent,m_dwHeightCurrent);
	return bResult;
}


// open from up->bottom
bool CGUIWindowSlideShow::RenderMethod6()
{
	bool bResult(false);
	int x,y,width,height;
	GetOutputRect(m_dwWidthCurrent, m_dwHeightCurrent, x, y, width, height);
  
	int iStep= height/g_stSettings.m_iSlideShowTransistionFrames;
	if (!iStep) iStep=1;
	int newheight=m_dwFrameCounter*iStep;
	if (newheight >= height)
	{
		newheight=height;
		bResult=true;
	}
	CPicture picture;
	picture.RenderImage(m_pTextureCurrent,x,y,width,newheight,m_dwWidthCurrent,m_dwHeightCurrent);
	return bResult;
}

// slide from left<-right
bool CGUIWindowSlideShow::RenderMethod7()
{
	bool bResult(false);
	int x,y,width,height;
	GetOutputRect(m_dwWidthCurrent, m_dwHeightCurrent, x, y, width, height);
  
	int iStep= width/g_stSettings.m_iSlideShowTransistionFrames;
	if (!iStep) iStep=1;
	int newwidth=m_dwFrameCounter*iStep;
	if (newwidth >=width)
	{
		newwidth=width;
		bResult=true;
	}

	//right align the texture
	int posx=x + width-newwidth;
	CPicture picture;
	picture.RenderImage(m_pTextureCurrent,posx,y,newwidth,height,m_dwWidthCurrent,m_dwHeightCurrent);
	return bResult;
}


// slide from down->up
bool CGUIWindowSlideShow::RenderMethod8()
{
	bool bResult(false);
	int x,y,width,height;
	GetOutputRect(m_dwWidthCurrent, m_dwHeightCurrent, x, y, width, height);
	  
	int iStep= height/g_stSettings.m_iSlideShowTransistionFrames;
	if (!iStep) iStep=1;
	int newheight=m_dwFrameCounter*iStep;
	if (newheight >=height)
	{
		newheight=height;
		bResult=true;
	}

	//bottom align the texture
	int posy=y + height-newheight;
	CPicture picture;
	picture.RenderImage(m_pTextureCurrent,x,posy,width,newheight,m_dwWidthCurrent,m_dwHeightCurrent);
	return bResult;
}

// grow from middle
bool CGUIWindowSlideShow::RenderMethod9()
{
	bool bResult(false);
	int x,y,width,height;
	GetOutputRect(m_dwWidthCurrent, m_dwHeightCurrent, x, y, width, height);
	int iStepX= width/g_stSettings.m_iSlideShowTransistionFrames;
	int iStepY= height/g_stSettings.m_iSlideShowTransistionFrames;
	if (!iStepX) iStepX=1;
	if (!iStepY) iStepY=1;
	int newheight=m_dwFrameCounter*iStepY;
	int newwidth=m_dwFrameCounter*iStepX;
	if (newheight >=height)
	{
		newheight=height;
		bResult=true;
	}
	if (newwidth >=width)
	{
		newwidth=width;
		bResult=true;
	}

	//center align the texture
	int posx = x + (width - newwidth)/2;
	int posy = y + (height - newheight)/2;
	CPicture picture;
	picture.RenderImage(m_pTextureCurrent,posx,posy,newwidth,newheight,m_dwWidthCurrent, m_dwHeightCurrent);													// posy in texture
	return bResult;
}

void CGUIWindowSlideShow::OnAction(const CAction &action)
{
	switch (action.wID)
	{
		case ACTION_PREVIOUS_MENU:
			m_gWindowManager.PreviousWindow();
		break;
		case ACTION_NEXT_PICTURE:
			if (m_iZoomFactor==1)
				ShowNext();
		break;
		case ACTION_PREV_PICTURE:
			if (m_iZoomFactor==1)
				ShowPrevious();
		break;
		case ACTION_MOVE_RIGHT:
			if (m_iZoomFactor!=1)
				m_iPosX-=25;
			m_lSlideTime=timeGetTime();
		break;
      
		case ACTION_MOVE_LEFT:
			if (m_iZoomFactor!=1)
				m_iPosX += 25;
			m_lSlideTime=timeGetTime();
		break;

		case ACTION_MOVE_DOWN:
			if (m_iZoomFactor > 1 ) m_iPosY-=25;
			m_lSlideTime=timeGetTime();
		break;

		case ACTION_MOVE_UP:
			if (m_iZoomFactor > 1 ) m_iPosY += 25;
			m_lSlideTime=timeGetTime();
		break;

		case ACTION_SHOW_CODEC:
			m_bShowInfo=!m_bShowInfo;
			m_lSlideTime=timeGetTime();
		break;

		case ACTION_PAUSE:
			if (m_bSlideShow) 
			{
				m_bPause=!m_bPause;
			}
			m_lSlideTime=timeGetTime();
		break;

		case ACTION_ZOOM_OUT:
			Zoom(m_iZoomFactor-1);
			m_lSlideTime=timeGetTime();
		break;

		case ACTION_ZOOM_IN:
			Zoom(m_iZoomFactor+1);
			m_lSlideTime=timeGetTime();
		break;

		case ACTION_ROTATE_PICTURE:
			m_iRotate++;
			if (m_iRotate>=4)
			{
				m_iRotate=0;
			}
			DoRotate();
			m_lSlideTime=timeGetTime();
		break;

		case ACTION_ZOOM_LEVEL_NORMAL:
		case ACTION_ZOOM_LEVEL_1:
		case ACTION_ZOOM_LEVEL_2:
		case ACTION_ZOOM_LEVEL_3:
		case ACTION_ZOOM_LEVEL_4:
		case ACTION_ZOOM_LEVEL_5:
		case ACTION_ZOOM_LEVEL_6:
		case ACTION_ZOOM_LEVEL_7:
		case ACTION_ZOOM_LEVEL_8:
		case ACTION_ZOOM_LEVEL_9:
			Zoom((action.wID - ACTION_ZOOM_LEVEL_NORMAL)+1);
			m_lSlideTime=timeGetTime();
		break;
		case ACTION_ANALOG_MOVE:
			float fX=2*action.fAmount1;
			float fY=2*action.fAmount2;
			if (fX||fY)
			{
				if (m_iZoomFactor>1)
				{
					m_iPosX -= (int)fX;
					m_iPosY += (int)fY;
				}
			}
			m_lSlideTime=timeGetTime();
		break;
	}
}

bool CGUIWindowSlideShow::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
		case GUI_MSG_WINDOW_DEINIT:
		{
			Reset();
			if (message.GetParam1() != WINDOW_PICTURES)
			{
				CSectionLoader::Unload("CXIMAGE");
			}
			g_application.EnableOverlay();
		}
		break;

		case GUI_MSG_WINDOW_INIT:
		{
			CGUIWindow::OnMessage(message);
			if (message.GetParam1() != WINDOW_PICTURES)
			{
				CSectionLoader::Load("CXIMAGE");
			}
			g_application.DisableOverlay();
			return true;
		}
	}
	return CGUIWindow::OnMessage(message);
}

void CGUIWindowSlideShow::RenderPause()
{
  static DWORD dwCounter=0;
  dwCounter++;
  if (dwCounter > 25)
  {
    dwCounter=0;
  }
  if (!m_bPause) return;
  if (dwCounter <13) return;
	CGUIFont* pFont=g_fontManager.GetFont("font13");
  const WCHAR *szText;
	szText=g_localizeStrings.Get(112).c_str();
  pFont->DrawShadowText(500.0,60.0,0xff000000,szText);
}

void CGUIWindowSlideShow::DoRotate()
{
  if(m_pTextureBackGround)
  {
    m_pTextureBackGround->Release();
  }
  CPicture picture;
  m_pTextureBackGround=picture.Load(m_strBackgroundSlide, m_iRotate,MAX_PICTURE_WIDTH,MAX_PICTURE_HEIGHT);
  m_dwWidthBackGround=picture.GetWidth();
  m_dwHeightBackGround=picture.GetHeight();
  m_iZoomFactor=1;
  m_iPosX=m_iPosY=0;
	m_lSlideTime=timeGetTime();
}

void CGUIWindowSlideShow::GetOutputRect(const int iSourceWidth, const int iSourceHeight, int& x, int& y, int& width, int& height)
{
	// calculate aspect ratio correction factor
	D3DPRESENT_PARAMETERS params;
	g_application.GetD3DParameters(params);
	RESOLUTION iRes = CUtil::GetResolution(params);
	int iOffsetX1 = g_settings.m_movieCalibration[iRes].left;
	int iOffsetY1 = g_settings.m_movieCalibration[iRes].top;
	int iOffsetX2 = g_settings.m_movieCalibration[iRes].right;
	int iOffsetY2 = g_settings.m_movieCalibration[iRes].bottom;

	int iScreenWidth = resInfo[iRes].iWidth + iOffsetX2-iOffsetX1;
	int iScreenHeight = resInfo[iRes].iHeight + iOffsetY2-iOffsetY1;
	float fSourceFrameAR = (float)iSourceWidth/iSourceHeight;
	float fOutputFrameAR = fSourceFrameAR / resInfo[iRes].fPixelRatio;
	width = iScreenWidth;
	height = (int)(width / fOutputFrameAR);
	if (height > iScreenHeight)
	{
		height = iScreenHeight;
		width = (int)(height * fOutputFrameAR);
	}
	x = (resInfo[iRes].iWidth - width)/2 + iOffsetX1;
	y = (resInfo[iRes].iHeight - height)/2 + iOffsetY1;
}

void CGUIWindowSlideShow::Zoom(int iZoom)
{
	if (iZoom > MAX_ZOOM_FACTOR || iZoom < 1)
		return;
	int x,y,width,height;
	GetOutputRect(m_dwWidthBackGround,m_dwHeightBackGround,x,y,width,height);
	m_iPosX =-m_iPosX ;
	m_iPosY =-m_iPosY;
	float middlex = ((float)( m_iPosX+width/2 )) / ((float)m_iZoomFactor);
	float middley = ((float)( m_iPosY+height/2)) / ((float)m_iZoomFactor);
	m_iZoomFactor = iZoom;
	if (m_iZoomFactor == 1)
	{
		m_iPosX=0;
		m_iPosY=0;
	}
	else
	{
		m_iPosX=-(int) ( middlex*((float)m_iZoomFactor) ) +width/2;
		m_iPosY=-(int) ( middley*((float)m_iZoomFactor) )	+height/2;
	}
}