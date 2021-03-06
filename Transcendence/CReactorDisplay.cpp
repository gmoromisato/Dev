//	CReactorDisplay.cpp
//
//	CReactorDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define NORMAL_COLOR				RGB(0, 128, 0)
#define WARNING_COLOR				RGB(255, 255, 0)
#define CRITICAL_COLOR				RGB(255, 0, 0)

#define FUEL_MARKER_COUNT			20
#define FUEL_MARKER_HEIGHT			5
#define FUEL_MARKER_WIDTH			12
#define FUEL_MARKER_GAP				1

#define TEXT_COLOR					CG16bitImage::RGBValue(150,180,255)
#define DAMAGED_TEXT_COLOR			CG16bitImage::RGBValue(128,0,0)

CReactorDisplay::CReactorDisplay (void) : m_pPlayer(NULL),
		m_iTickCount(0),
		m_pFonts(NULL),
		m_iOverloading(0)

//	CReactorDisplay contructor

	{
	}

CReactorDisplay::~CReactorDisplay (void)

//	CReactorDisplay destructor

	{
	CleanUp();
	}

void CReactorDisplay::CleanUp (void)

//	CleanUp
//
//	Delete relevant stuff

	{
	m_pPlayer = NULL;
	}

ALERROR CReactorDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initialize

	{
	ALERROR error;

	CleanUp();

	m_pPlayer = pPlayer;
	m_rcRect = rcRect;

	//	Create the off-screen buffer

	if (error = m_Buffer.CreateBlank(RectWidth(rcRect), RectHeight(rcRect), false, DEFAULT_TRANSPARENT_COLOR))
		return error;

	m_Buffer.SetTransparentColor();

	return NOERROR;
	}

void CReactorDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	Dest.ColorTransBlt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			255,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CReactorDisplay::Update (void)

//	Update
//
//	Updates the buffer from data

	{
	if (m_pPlayer == NULL)
		return;

	CShip *pShip = m_pPlayer->GetShip();
	const CPlayerSettings *pSettings = pShip->GetClass()->GetPlayerSettings();
	const SReactorImageDesc &ReactorDesc = pSettings->GetReactorDesc();

	int yOffset = (RectHeight(m_rcRect) - RectHeight(ReactorDesc.ReactorImage.GetImageRect())) / 2;

	//	Paint the background

	ReactorDesc.ReactorImage.PaintImageUL(m_Buffer, 0, yOffset, 0, 0);

	//	Calculate fuel values

	int iFuelLeft = pShip->GetFuelLeft();
	int iMaxFuel = pShip->GetMaxFuel();
	int iFuelLevel = (iMaxFuel > 0 ? (Min((iFuelLeft * 100 / iMaxFuel) + 1, 100)) : 0);

	//	Paint the fuel level

	CG16bitImage *pFuelImage = NULL;
	RECT rcSrcRect;
	bool bBlink;
	if (iFuelLevel < 15)
		{
		pFuelImage = &ReactorDesc.FuelLowLevelImage.GetImage(NULL_STR);
		rcSrcRect = ReactorDesc.FuelLowLevelImage.GetImageRect();
		bBlink = true;
		}
	else
		{
		pFuelImage = &ReactorDesc.FuelLevelImage.GetImage(NULL_STR);
		rcSrcRect = ReactorDesc.FuelLevelImage.GetImageRect();
		bBlink = false;
		}

	if (!bBlink || ((m_iTickCount % 2) == 0))
		{
		int cxFuelLevel = RectWidth(rcSrcRect) * iFuelLevel / 100;
		m_Buffer.ColorTransBlt(rcSrcRect.left,
				rcSrcRect.top,
				cxFuelLevel,
				RectHeight(rcSrcRect),
				255,
				*pFuelImage,
				ReactorDesc.xFuelLevelImage,
				yOffset + ReactorDesc.yFuelLevelImage);
		}

	//	Paint fuel level text

	m_Buffer.Fill(ReactorDesc.rcFuelLevelText.left,
			yOffset + ReactorDesc.rcFuelLevelText.top,
			RectWidth(ReactorDesc.rcFuelLevelText),
			RectHeight(ReactorDesc.rcFuelLevelText),
			DEFAULT_TRANSPARENT_COLOR);

	CString sFuelLevel = strPatternSubst(CONSTLIT("fuel"));
	int cxWidth = m_pFonts->Small.MeasureText(sFuelLevel);
	m_pFonts->Small.DrawText(m_Buffer,
			ReactorDesc.rcFuelLevelText.left,
			yOffset + ReactorDesc.rcFuelLevelText.top,
			m_pFonts->wHelpColor,
			sFuelLevel,
			0);

	//	Calculate the power level

	int iPowerUsage = pShip->GetPowerConsumption();
	int iMaxPower = pShip->GetMaxPower();
	int iPowerLevel;
	if (iMaxPower)
		iPowerLevel = Min((iPowerUsage * 100 / iMaxPower) + 1, 120);
	else
		iPowerLevel = 0;

	if (iPowerLevel >= 100)
		m_iOverloading++;
	else
		m_iOverloading = 0;

	bBlink = (m_iOverloading > 0);

	//	Paint the power level

	if (!bBlink || ((m_iOverloading % 2) == 1))
		{
		CG16bitImage *pPowerImage = &ReactorDesc.PowerLevelImage.GetImage(NULL_STR);
		rcSrcRect = ReactorDesc.PowerLevelImage.GetImageRect();
		int cxPowerLevel = RectWidth(rcSrcRect) * iPowerLevel / 120;
		m_Buffer.ColorTransBlt(rcSrcRect.left,
				rcSrcRect.top,
				cxPowerLevel,
				RectHeight(rcSrcRect),
				255,
				*pPowerImage,
				ReactorDesc.xPowerLevelImage,
				yOffset + ReactorDesc.yPowerLevelImage);
		}

	//	Paint power level text

	m_Buffer.Fill(ReactorDesc.rcPowerLevelText.left,
			yOffset + ReactorDesc.rcPowerLevelText.top,
			RectWidth(ReactorDesc.rcPowerLevelText),
			RectHeight(ReactorDesc.rcPowerLevelText),
			DEFAULT_TRANSPARENT_COLOR);

	CString sPowerLevel = strPatternSubst(CONSTLIT("power usage"));
	int cyHeight;
	cxWidth = m_pFonts->Small.MeasureText(sPowerLevel, &cyHeight);
	m_pFonts->Small.DrawText(m_Buffer,
			ReactorDesc.rcPowerLevelText.left,
			yOffset + ReactorDesc.rcPowerLevelText.bottom - cyHeight,
			m_pFonts->wHelpColor,
			sPowerLevel,
			0);

	//	Paint the reactor name (we paint on top of the levels)

	WORD wColor;
	if (pShip->GetReactorDesc()->fDamaged)
		wColor = DAMAGED_TEXT_COLOR;
	else
		wColor = m_pFonts->wTitleColor;

	CString sReactorName = strPatternSubst(CONSTLIT("%s (%s)"), 
			pShip->GetReactorName(),
			ReactorPower2String(iMaxPower));

	m_pFonts->Medium.DrawText(m_Buffer,
			ReactorDesc.rcReactorText.left,
			yOffset + ReactorDesc.rcReactorText.top,
			wColor,
			sReactorName,
			0);

	m_iTickCount++;
	}

