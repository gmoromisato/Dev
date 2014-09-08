//	CGSelectorArea.cpp
//
//	CGSelectorArea class

#include "PreComp.h"
#include "Transcendence.h"

const int ITEM_ICON_WIDTH =					64;
const int ITEM_ICON_HEIGHT =				64;

const int ITEM_ENTRY_WIDTH =				140;
const int ITEM_ENTRY_HEIGHT =				(ITEM_ICON_HEIGHT + 40);

const int ITEM_ENTRY_PADDING_TOP =			2;
const int ITEM_ENTRY_PADDING_LEFT =			4;
const int ITEM_ENTRY_PADDING_RIGHT =		4;

CGSelectorArea::CGSelectorArea (const CVisualPalette &VI) :
		m_VI(VI),
		m_pSource(NULL),
		m_iCursor(-1)

//	CGSelectorArea constructor

	{
	}

CGSelectorArea::~CGSelectorArea (void)

//	CGSelectorArea destructor

	{
	}

void CGSelectorArea::CalcRegionRect (const SEntry &Entry, int xCenter, int yCenter, RECT *retrcRect)

//	CalcRegionRect
//
//	Calculates the region rectangle

	{
	retrcRect->left = xCenter + Entry.rcRect.left;
	retrcRect->top = yCenter + Entry.rcRect.top;
	retrcRect->right = retrcRect->left + RectWidth(Entry.rcRect);
	retrcRect->bottom = retrcRect->top + RectHeight(Entry.rcRect);
	}

void CGSelectorArea::CleanUp (void)

//	CleanUp
//
//	Reset

	{
	m_Regions.DeleteAll();
	m_pSource = NULL;
	m_iCursor = -1;
	}

bool CGSelectorArea::FindRegionInDirection (EDirections iDir, int *retiIndex) const

//	FindRegionInDirection
//
//	Looks for the region in the specified direction from the currently selected
//	region.
//
//	Returns TRUE if the region is found.

	{
	int i;

	//	Get the current position of the selected region. If we have no selection,
	//	then we take an arbitrary point at the edge of the area.

	int xCur;
	int yCur;
	if (m_iCursor != -1)
		{
		xCur = m_Regions[m_iCursor].rcRect.left;
		yCur = m_Regions[m_iCursor].rcRect.top;
		}
	else
		{
		const RECT &rcRect = GetRect();
		switch (iDir)
			{
			case moveDown:
				xCur = rcRect.left + (RectWidth(rcRect) / 2);
				yCur = rcRect.top;
				break;

			case moveLeft:
				xCur = rcRect.right;
				yCur = rcRect.top + (RectHeight(rcRect) / 2);
				break;

			case moveRight:
				xCur = rcRect.left;
				yCur = rcRect.top + (RectHeight(rcRect) / 2);
				break;

			case moveUp:
				xCur = rcRect.left + (RectWidth(rcRect) / 2);
				yCur = rcRect.bottom;
				break;

			default:
				ASSERT(false);
				return false;
			}
		}

	//	Loop over all regions and find the one the is closest to the
	//	given region.

	int iBest = -1;
	int xBestDist;
	int yBestDist;
	for (i = 0; i < m_Regions.GetCount(); i++)
		if (i != m_iCursor)
			{
			const SEntry &Entry = m_Regions[i];
			int xDist = Absolute(Entry.rcRect.left - xCur);
			int yDist = Absolute(Entry.rcRect.top - yCur);
			bool bCloser = false;

			//	See if this entry is better than the best so far

			switch (iDir)
				{
				case moveDown:
					bCloser = (Entry.rcRect.top > yCur
							&& (iBest == -1 
								|| yDist < yBestDist 
								|| (yDist == yBestDist && xDist < xBestDist)));
					break;

				case moveLeft:
					bCloser = (Entry.rcRect.left < xCur
							&& (iBest == -1
								|| xDist < xBestDist
								|| (xDist == xBestDist && yDist < yBestDist)));
					break;

				case moveRight:
					bCloser = (Entry.rcRect.left > xCur
							&& (iBest == -1
								|| xDist < xBestDist
								|| (xDist == xBestDist && yDist < yBestDist)));
					break;

				case moveUp:
					bCloser = (Entry.rcRect.top < yCur
							&& (iBest == -1 
								|| yDist < yBestDist 
								|| (yDist == yBestDist && xDist < xBestDist)));
					break;

				default:
					ASSERT(false);
					return false;
				}

			if (bCloser)
				{
				iBest = i;
				xBestDist = xDist;
				yBestDist = yDist;
				}
			}

	//	Found?

	if (iBest == -1)
		return false;

	if (retiIndex)
		*retiIndex = iBest;

	return true;
	}

ICCItem *CGSelectorArea::GetEntryAtCursor (void)

//	GetEntryAtCursor
//
//	Returns a struct representing the selection. The struct has the following
//	fields:
//
//	category: If this is an empty slot, this describes the category of item
//		that can be installed.
//	item: The item selected (if appropriate)
//	type: One of the following values:
//		'installedItem: An installed item (armor or device)
//		'emptySlot: An empty device slot

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	if (m_iCursor == -1)
		return CC.CreateNil();

	const SEntry &Entry = m_Regions[m_iCursor];

	ICCItem *pResult = CC.CreateSymbolTable();
	CCSymbolTable *pStruct = (CCSymbolTable *)pResult;

	switch (Entry.iType)
		{
		case typeEmptySlot:
			pStruct->SetStringValue(CC, CONSTLIT("type"), CONSTLIT("emptySlot"));
			switch (Entry.iSlotType)
				{
				case devPrimaryWeapon:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatWeapon));
					break;

				case devMissileWeapon:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatLauncher));
					break;

				case devShields:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatShields));
					break;

				case devDrive:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatDrive));
					break;

				case devCargo:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatCargoHold));
					break;

				case devReactor:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatReactor));
					break;

				default:
					pStruct->SetStringValue(CC, CONSTLIT("category"), GetItemCategoryID(itemcatMiscDevice));
				}
			break;

		case typeInstalledItem:
			{
			pStruct->SetStringValue(CC, CONSTLIT("type"), CONSTLIT("installedItem"));
			ICCItem *pItem = ::CreateListFromItem(CC, Entry.pItemCtx->GetItem());
			pStruct->SetValue(CC, CONSTLIT("item"), pItem);
			pItem->Discard(&CC);
			break;
			}

		default:
			ASSERT(false);
			return CC.CreateNil();
		}

	return pResult;
	}

const CItem &CGSelectorArea::GetItemAtCursor (void)

//	GetItemAtCursor
//
//	Returns the currently selected item (if any)

	{
	if (m_iCursor == -1)
		return CItem::NullItem();

	const SEntry &Entry = m_Regions[m_iCursor];
	switch (Entry.iType)
		{
		case typeInstalledItem:
			return Entry.pItemCtx->GetItem();

		default:
			return CItem::NullItem();
		}
	}

bool CGSelectorArea::LButtonDown (int x, int y)

//	LButtonDown
//
//	Handle mouse button

	{
	int i;

	//	All coordinates are relative to the center of the area.

	const RECT &rcRect = GetRect();
	int xCenter = rcRect.left + (RectWidth(rcRect) / 2);
	int yCenter = rcRect.top + RectHeight(rcRect) / 2;

	//	See if we've clicked on something

	for (i = 0; i < m_Regions.GetCount(); i++)
		{
		const SEntry &Entry = m_Regions[i];

		//	Convert the rect to destination coordinates

		RECT rcRegion;
		CalcRegionRect(Entry, xCenter, yCenter, &rcRegion);

		//	If we clicked in the region, signal an action

		if (x >= rcRegion.left && x < rcRegion.right
				&& y >= rcRegion.top && y < rcRegion.bottom)
			SignalAction(i);
		}

	return false;
	}

bool CGSelectorArea::MoveCursor (EDirections iDir)

//	MoveCursor
//
//	Moves the cursor in the specified direction. Returns TRUE if the cursor
//	moved successfully.

	{
	switch (iDir)
		{
		case moveDown:
		case moveLeft:
		case moveRight:
		case moveUp:
			if (!FindRegionInDirection(iDir, &m_iCursor))
				return false;

			Invalidate();
			return true;

		case moveNext:
			if (m_iCursor + 1 >= m_Regions.GetCount())
				return false;

			m_iCursor++;
			Invalidate();
			return true;

		case movePrev:
			if (m_iCursor <= 0)
				return false;

			m_iCursor--;
			Invalidate();
			return true;

		//	Invalid

		default:
			ASSERT(false);
			return false;
		}
	}

void CGSelectorArea::Paint (CG16bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint the area

	{
	int i;

	const int iCornerRadius = 4;
	const int xBackMargin = 3;
	const int yBackMargin = 3;
	const int iSelectionWidth = 2;

	//	All coordinates are relative to the center of the area.

	int xCenter = rcRect.left + (RectWidth(rcRect) / 2);
	int yCenter = rcRect.top + RectHeight(rcRect) / 2;

	//	Loop over all regions and paint

	for (i = 0; i < m_Regions.GetCount(); i++)
		{
		const SEntry &Entry = m_Regions[i];

		//	Convert the rect to destination coordinates

		RECT rcRegion;
		CalcRegionRect(Entry, xCenter, yCenter, &rcRegion);

		//	Paint the background of the entry

		WORD wBackColor = (m_iCursor == i ? m_VI.GetColor(colorAreaDialogInputFocus) : m_VI.GetColor(colorAreaDialog));
		DWORD dwBackOpacity = (m_iCursor == i ? 255 : 220);

		::DrawRoundedRectTrans(Dest, 
				rcRegion.left - xBackMargin, 
				rcRegion.top - yBackMargin, 
				RectWidth(rcRegion) + 2 * xBackMargin,
				RectHeight(rcRegion) + 2 * yBackMargin,
				iCornerRadius + xBackMargin, 
				wBackColor,
				dwBackOpacity);

		//	Paint the actual region

		switch (Entry.iType)
			{
			case typeEmptySlot:
				PaintEmptySlot(Dest, rcRegion, Entry);
				break;

			case typeInstalledItem:
				PaintInstalledItem(Dest, rcRegion, Entry);
				break;
			}

		//	Draw outline if we're selected

		if (m_iCursor == i)
			{
			::DrawRoundedRectOutline(Dest,
					rcRegion.left,
					rcRegion.top,
					RectWidth(rcRegion),
					RectHeight(rcRegion),
					iCornerRadius,
					iSelectionWidth,
					m_VI.GetColor(colorAreaDialogHighlight));
			}
		}
	}

void CGSelectorArea::PaintEmptySlot (CG16bitImage &Dest, const RECT &rcRect, const SEntry &Entry)

//	PaintEmptySlot
//
//	Paints the device slot

	{
	//	Paint the slot icon

	const CG16bitImage &SlotImage = m_VI.GetImage(imageSlotIcon);
	int xIcon = rcRect.left + (RectWidth(rcRect) - ITEM_ICON_WIDTH) / 2;
	int yIcon = rcRect.top + ITEM_ENTRY_PADDING_TOP;
	DrawBltTransformed(Dest,
			xIcon + (ITEM_ICON_WIDTH / 2),
			yIcon + (ITEM_ICON_HEIGHT / 2),
			(Metric)ITEM_ICON_WIDTH / (Metric)SlotImage.GetWidth(),
			(Metric)ITEM_ICON_HEIGHT / (Metric)SlotImage.GetHeight(),
			0.0,
			SlotImage,
			0,
			0,
			SlotImage.GetWidth(),
			SlotImage.GetHeight());

	//	Paint the name of the slot

	CString sName;
	switch (Entry.iSlotType)
		{
		case devPrimaryWeapon:
			sName = CONSTLIT("weapon slot");
			break;

		case devMissileWeapon:
			sName = CONSTLIT("launcher slot");
			break;

		case devShields:
			sName = CONSTLIT("shield generator slot");
			break;

		case devCargo:
			sName = CONSTLIT("cargo hold slot");
			break;

		case devReactor:
			sName = CONSTLIT("reactor slot");
			break;

		case devDrive:
			sName = CONSTLIT("drive slot");
			break;

		default:
			sName = CONSTLIT("device slot");
		}

	RECT rcText;
	rcText.left = rcRect.left + ITEM_ENTRY_PADDING_LEFT;
	rcText.right = rcRect.right - ITEM_ENTRY_PADDING_RIGHT;
	rcText.top = yIcon + ITEM_ICON_HEIGHT;
	rcText.bottom = rcRect.bottom;

	m_VI.GetFont(fontMedium).DrawText(Dest, 
			rcText,
			m_VI.GetColor(colorTextHighlight),
			sName,
			0,
			CG16bitFont::AlignCenter);
	}

void CGSelectorArea::PaintInstalledItem (CG16bitImage &Dest, const RECT &rcRect, const SEntry &Entry)

//	PaintInstalledItem
//
//	Paints the installed item.

	{
	const CItem &Item = Entry.pItemCtx->GetItem();
	if (Item.GetType() == NULL)
		return;

	CSpaceObject *pSource = Entry.pItemCtx->GetSource();
	CInstalledArmor *pArmor = Entry.pItemCtx->GetArmor();
	CInstalledDevice *pDevice = Entry.pItemCtx->GetDevice();

	//	Paint the item icon

	bool bGrayed = (pDevice && !pDevice->IsEnabled());
	int xIcon = rcRect.left + (RectWidth(rcRect) - ITEM_ICON_WIDTH) / 2;
	int yIcon = rcRect.top + ITEM_ENTRY_PADDING_TOP;
	DrawItemTypeIcon(Dest, xIcon, yIcon, Item.GetType(), ITEM_ICON_WIDTH, ITEM_ICON_HEIGHT, bGrayed);

	//	Paint the name of the item below.

	RECT rcText;
	rcText.left = rcRect.left + ITEM_ENTRY_PADDING_LEFT;
	rcText.right = rcRect.right - ITEM_ENTRY_PADDING_RIGHT;
	rcText.top = yIcon + ITEM_ICON_HEIGHT;
	rcText.bottom = rcRect.bottom;

	m_VI.GetFont(fontMedium).DrawText(Dest, 
			rcText,
			m_VI.GetColor(colorTextHighlight),
			Item.GetNounPhrase(nounShort | nounNoModifiers),
			0,
			CG16bitFont::AlignCenter);

	//	If this is an armor segment, then paint HP, etc.

	if (pArmor)
		{
		int x = rcRect.right - ITEM_ENTRY_PADDING_RIGHT;
		int y = rcRect.top + ITEM_ENTRY_PADDING_TOP;

		//	HP

		CString sHP = strFromInt(pArmor->GetHitPoints());
		m_VI.GetFont(fontLarge).DrawText(Dest,
				x,
				y,
				m_VI.GetColor(colorTextHighlight),
				sHP,
				CG16bitFont::AlignRight);
		y += m_VI.GetFont(fontLarge).GetHeight();

		//	Damage

		int iMaxHP = pArmor->GetMaxHP(pSource);
		if (iMaxHP != pArmor->GetHitPoints() && iMaxHP > 0)
			{
			int iPercent = ((1000 * pArmor->GetHitPoints() / iMaxHP) + 5) / 10;
			CString sPercent = strPatternSubst(CONSTLIT("%d%%"), iPercent);

			m_VI.GetFont(fontMedium).DrawText(Dest,
					x,
					y,
					m_VI.GetColor(colorTextDockWarning),
					sPercent,
					CG16bitFont::AlignRight);
			y += m_VI.GetFont(fontMedium).GetHeight();
			}

		//	Modifiers

		if (pArmor->GetMods().IsNotEmpty())
			{
			CString sMods = Item.GetEnhancedDesc(pSource);
			if (!sMods.IsBlank())
				{
				bool bIsDisadvantage = *sMods.GetASCIIZPointer() == '-';
				WORD wBackColor = (bIsDisadvantage ? m_VI.GetColor(colorAreaDisadvantage) : m_VI.GetColor(colorAreaAdvantage));
				WORD wTextColor = (bIsDisadvantage ? m_VI.GetColor(colorTextDisadvantage) : m_VI.GetColor(colorTextAdvantage));

				PaintModifier(Dest, x, y, sMods, wTextColor, wBackColor, &y);
				}
			}
		}

	//	If this is not a device, then nothing to do

	else if (pDevice == NULL)
		{
		}

	//	If this is a shield generator, then paint shield stuff

	else if (pDevice->GetCategory() == itemcatShields)
		{
		int x = rcRect.right - ITEM_ENTRY_PADDING_RIGHT;
		int y = rcRect.top + ITEM_ENTRY_PADDING_TOP;

		//	HP

		if (pDevice->IsEnabled())
			{
			int iHP;
			int iMaxHP;
			pDevice->GetStatus(pSource, &iHP, &iMaxHP);

			CString sHP = strFromInt(iHP);
			m_VI.GetFont(fontLarge).DrawText(Dest,
					x,
					y,
					m_VI.GetColor(colorTextHighlight),
					sHP,
					CG16bitFont::AlignRight);
			y += m_VI.GetFont(fontLarge).GetHeight();

			//	Shield level

			if (iMaxHP != iHP && iMaxHP > 0)
				{
				int iPercent = ((1000 * iHP / iMaxHP) + 5) / 10;
				CString sPercent = strPatternSubst(CONSTLIT("%d%%"), iPercent);

				m_VI.GetFont(fontMedium).DrawText(Dest,
						x,
						y,
						m_VI.GetColor(colorTextShields),
						sPercent,
						CG16bitFont::AlignRight);
				y += m_VI.GetFont(fontMedium).GetHeight();
				}
			}
		else
			PaintModifier(Dest, x, y, CONSTLIT("disabled"), m_VI.GetColor(colorTextNormal), DEFAULT_TRANSPARENT_COLOR, &y);

		//	Damaged

		if (pDevice->IsDamaged())
			PaintModifier(Dest, x, y, CONSTLIT("damaged"), m_VI.GetColor(colorTextDisadvantage), m_VI.GetColor(colorAreaDisadvantage), &y);

		//	Modifiers

		if (pDevice->GetEnhancements() != NULL)
			{
			CString sMods = pDevice->GetEnhancedDesc(pSource, &Item);
			if (!sMods.IsBlank())
				{
				bool bIsDisadvantage = *sMods.GetASCIIZPointer() == '-';
				WORD wBackColor = (bIsDisadvantage ? m_VI.GetColor(colorAreaDisadvantage) : m_VI.GetColor(colorAreaAdvantage));
				WORD wTextColor = (bIsDisadvantage ? m_VI.GetColor(colorTextDisadvantage) : m_VI.GetColor(colorTextAdvantage));

				PaintModifier(Dest, x, y, sMods, wTextColor, wBackColor, &y);
				}
			}
		}
	}

void CGSelectorArea::PaintModifier (CG16bitImage &Dest, int x, int y, const CString &sText, WORD wColor, WORD wBackColor, int *rety)

//	PaintModifier
//
//	Paints a modifier block.

	{
	int cx = m_VI.GetFont(fontSmall).MeasureText(sText);
	if (wBackColor != DEFAULT_TRANSPARENT_COLOR)
		Dest.Fill(x - cx - 8,
				y,
				cx + 8,
				m_VI.GetFont(fontSmall).GetHeight(),
				wBackColor);

	m_VI.GetFont(fontSmall).DrawText(Dest,
			x - cx - 4,
			y,
			wColor,
			sText);

	if (rety)
		*rety = y + m_VI.GetFont(fontSmall).GetHeight() + 1;
	}

void CGSelectorArea::SetRegions (CSpaceObject *pSource, EConfigurations iConfig)

//	SetRegions
//
//	Sets the regions based on the given configuration.

	{
	CleanUp();

	m_pSource = pSource;

	switch (iConfig)
		{
		case configArmor:
			SetRegionsFromArmor(pSource);
			break;
		}

	Invalidate();
	}

void CGSelectorArea::SetRegionsFromArmor (CSpaceObject *pSource)

//	SetRegionsFromArmor
//
//	Generates regions showing armor and shields for the given ship.

	{
	int i;
	ASSERT(pSource);
	if (pSource == NULL)
		return;

	CShip *pShip = pSource->AsShip();
	if (pShip == NULL)
		return;

	CShipClass *pClass = pShip->GetClass();

	//	Compute some metrics.
	//
	//	We place the shield generator in the center and the armor segments in a
	//	circle around it.

	const RECT &rcRect = GetRect();
	int cxArea = RectWidth(rcRect);
	int cyArea = RectHeight(rcRect);

	const int iRadius = 200;

	//	Now add all the armor segments

	for (i = 0; i < pShip->GetArmorSectionCount(); i++)
		{
		SEntry *pEntry = m_Regions.Insert();
		CInstalledArmor *pArmor = pShip->GetArmorSection(i);

		pEntry->iType = typeInstalledItem;
		pEntry->pItemCtx = new CItemCtx(pShip, pArmor);

		//	Position the armor segment in a circle (add 90 degrees because the
		//	ship image points up).

		CShipClass::HullSection *pSection = pClass->GetHullSection(i);
		int iCenterAngle = 90 + AngleMiddle(pSection->iStartAt, AngleMod(pSection->iStartAt + pSection->iSpan));

		int xCenter;
		int yCenter;
		IntPolarToVector(iCenterAngle, iRadius, &xCenter, &yCenter);

		pEntry->rcRect.left = xCenter - (ITEM_ENTRY_WIDTH / 2);
		pEntry->rcRect.top = -yCenter - (ITEM_ENTRY_HEIGHT / 2);
		pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
		pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;
		}

	//	Add the shield generator last

	SEntry *pEntry = m_Regions.Insert();
	CInstalledDevice *pShields = pShip->GetNamedDevice(devShields);
	if (pShields)
		{
		pEntry->iType = typeInstalledItem;
		pEntry->pItemCtx = new CItemCtx(pShip, pShields);
		}
	else
		{
		pEntry->iType = typeEmptySlot;
		pEntry->iSlotType = devShields;
		}

	pEntry->rcRect.left = -ITEM_ENTRY_WIDTH / 2;
	pEntry->rcRect.top = -ITEM_ENTRY_HEIGHT / 2;
	pEntry->rcRect.right = pEntry->rcRect.left + ITEM_ENTRY_WIDTH;
	pEntry->rcRect.bottom = pEntry->rcRect.top + ITEM_ENTRY_HEIGHT;
	}

void CGSelectorArea::Update (void)

//	Update
//
//	Update the area

	{
	}