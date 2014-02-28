//	CObjectEffectList.cpp
//
//	CObjectEffectList class
//	Copyright (c) 2014 by George Moromisato. All Rights Reserved.

#include "PreComp.h"

CObjectEffectList::~CObjectEffectList (void)

//	CObjectEffectList destructor

	{
	CleanUp();
	}

void CObjectEffectList::CleanUp (void)

//	CleanUp
//
//	Clean up effects

	{
	int i;

	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		if (m_FixedEffects[i].pPainter)
			m_FixedEffects[i].pPainter->Delete();

	m_FixedEffects.DeleteAll();
	}

void CObjectEffectList::Init (const CObjectEffectDesc &Desc, const TArray<IEffectPainter *> &Painters)

//	Init
//
//	Initialize the effect list

	{
	int i;
	ASSERT(Desc.GetEffectCount() == Painters.GetCount());

	CleanUp();
	
	m_FixedEffects.InsertEmpty(Desc.GetEffectCount());
	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		m_FixedEffects[i].pPainter = Painters[i];
	}

void CObjectEffectList::Move (bool *retbBoundsChanged)

//	Move
//
//	Effects move

	{
	int i;
	bool bBoundsChanged = false;

	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		if (m_FixedEffects[i].pPainter)
			{
			bool bChanged;
			m_FixedEffects[i].pPainter->OnMove(&bChanged);
			if (bChanged)
				bBoundsChanged = true;
			}

	if (retbBoundsChanged)
		*retbBoundsChanged = bBoundsChanged;
	}

void CObjectEffectList::Paint (SViewportPaintCtx &Ctx, const CObjectEffectDesc &Desc, DWORD dwEffects, CG16bitImage &Dest, int x, int y)

//	Paint
//
//	Paint the effects in the dwEffects mask

	{
	int i;
	int iObjRotation = Ctx.iRotation;

	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		{
		const CObjectEffectDesc::SEffectDesc &EffectDesc = Desc.GetEffectDesc(i);

		if (EffectDesc.PosCalc.PaintFirst(Ctx.iVariant) != Ctx.bInFront
				&& m_FixedEffects[i].pPainter)
			{
			//	Figure out where to paint the effect

			int xPainter, yPainter;
			EffectDesc.PosCalc.GetCoordFromDir(Ctx.iVariant, &xPainter, &yPainter);

			//	Compute the rotation (180 for thruster effects)

			Ctx.iRotation = AngleMod(iObjRotation + EffectDesc.iRotation + 180);

			//	Paint

			if (dwEffects & EffectDesc.iType)
				m_FixedEffects[i].pPainter->Paint(Dest,
						x + xPainter,
						y + yPainter,
						Ctx);
			else
				m_FixedEffects[i].pPainter->PaintFade(Dest,
						x + xPainter,
						y + yPainter,
						Ctx);
			}
		}

	Ctx.iRotation = iObjRotation;
	}

void CObjectEffectList::PaintAll (SViewportPaintCtx &Ctx, const CObjectEffectDesc &Desc, CG16bitImage &Dest, int x, int y)

//	PaintAll
//
//	Paint all effects

	{
	DWORD dwAll = CObjectEffectDesc::effectThrustLeft
			| CObjectEffectDesc::effectThrustRight
			| CObjectEffectDesc::effectThrustMain;

	//	Set tick to 1 so we don't blink

	int iOldTick = Ctx.iTick;
	Ctx.iTick = 1;

	//	Paint

	Paint(Ctx, Desc, dwAll,	Dest, x, y);

	//	Done

	Ctx.iTick = iOldTick;
	}

void CObjectEffectList::Update (const CObjectEffectDesc &Desc, int iRotation, DWORD dwEffects)

//	Update
//
//	Update effects

	{
	int i;
	SEffectUpdateCtx PainterCtx;

	for (i = 0; i < m_FixedEffects.GetCount(); i++)
		if (m_FixedEffects[i].pPainter)
			{
			//	If bFade is TRUE then it means that we don't emit new particles
			//	out of particle effects (other effects are hidden).

			PainterCtx.bFade = ((dwEffects & Desc.GetEffectDesc(i).iType) == 0);

			//	Compute the position of the effect (relative to the object center)
			//	so we can emit at that location. We need this because some effects
			//	(particle effects) are centered on the object, not the emit position.

			int xEmit;
			int yEmit;
			Desc.GetEffectDesc(i).PosCalc.GetCoord(iRotation, &xEmit, &yEmit);
			PainterCtx.vEmitPos = CVector(xEmit * g_KlicksPerPixel, -yEmit * g_KlicksPerPixel);

			//	Update

			m_FixedEffects[i].pPainter->OnUpdate(PainterCtx);
			}
	}