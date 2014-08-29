//	CGAreas.h
//
//	CGAreas
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#define ITEM_LIST_AREA_PAGE_UP_ACTION			(0xffff0001)
#define ITEM_LIST_AREA_PAGE_DOWN_ACTION			(0xffff0002)

class CGItemListArea : public AGArea
	{
	public:
		CGItemListArea (void);
		~CGItemListArea (void);

		void CleanUp (void);
		inline void DeleteAtCursor (int iCount) { if (m_pListData) m_pListData->DeleteAtCursor(iCount); InitRowDesc(); Invalidate(); }
		inline int GetCursor (void) { return (m_pListData ? m_pListData->GetCursor() : -1); }
		ICCItem *GetEntryAtCursor (void);
		inline const CItem &GetItemAtCursor (void) { return (m_pListData ? m_pListData->GetItemAtCursor() : g_DummyItem); }
		inline CItemListManipulator &GetItemListManipulator (void) { return (m_pListData ? m_pListData->GetItemListManipulator() : g_DummyItemListManipulator); }
		inline IListData *GetList (void) const { return m_pListData; }
		inline CSpaceObject *GetSource (void) { return (m_pListData ? m_pListData->GetSource() : NULL); }
		inline bool IsCursorValid (void) { return (m_pListData ? m_pListData->IsCursorValid() : false); }
		bool MoveCursorBack (void);
		bool MoveCursorForward (void);
		inline void ResetCursor (void) { if (m_pListData) m_pListData->ResetCursor(); Invalidate(); }
		inline void SetCursor (int iIndex) { if (m_pListData) m_pListData->SetCursor(iIndex); Invalidate(); }
		inline void SetFilter (const CItemCriteria &Filter) { if (m_pListData) m_pListData->SetFilter(Filter); InitRowDesc(); Invalidate(); }
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void SetList (CCodeChain &CC, ICCItem *pList);
		void SetList (CSpaceObject *pSource);
		void SetList (CItemList &ItemList);
		inline void SetRowHeight (int cyHeight) { m_cyRow = Max(1, cyHeight); }
		inline void SetUIRes (const CUIResources *pUIRes) { m_pUIRes = pUIRes; }
		inline void SyncCursor (void) { if (m_pListData) m_pListData->SyncCursor(); Invalidate(); }

		//	AGArea virtuals
		virtual bool LButtonDown (int x, int y);
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);
		virtual void Update (void);

	private:
		enum ListTypes
			{
			listNone,
			listItem,
			listCustom,
			};

		struct SRowDesc
			{
			int yPos;							//	Position of the row (sum of height of previous rows)
			int cyHeight;						//	Height of this row
			};

		int CalcRowHeight (int iRow);
		void InitRowDesc (void);
		int FindRow (int y);
		void FormatDisplayAttributes (TArray<SDisplayAttribute> &Attribs, const RECT &rcRect, int *retcyHeight);
		void PaintCustom (CG16bitImage &Dest, const RECT &rcRect, bool bSelected);
		void PaintDisplayAttributes (CG16bitImage &Dest, TArray<SDisplayAttribute> &Attribs);
		void PaintItem (CG16bitImage &Dest, const CItem &Item, const RECT &rcRect, bool bSelected);

		IListData *m_pListData;
		ListTypes m_iType;

		const CUIResources *m_pUIRes;
		const SFontTable *m_pFonts;
		int m_iOldCursor;						//	Cursor pos
		int m_yOffset;							//	Painting offset for smooth scroll
		int m_yFirst;							//	coord of first row relative to list rect
		int m_cyRow;							//	Row height

		int m_cyTotalHeight;					//	Total heigh of all rows
		TArray<SRowDesc> m_Rows;
	};

class CGDrawArea : public AGArea
	{
	public:
		CGDrawArea (void);

		inline CG16bitImage &GetCanvas (void) { CreateImage(); return m_Image; }

		//	AGArea virtuals
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);

	private:
		void CreateImage (void);

		CG16bitImage m_Image;

		WORD m_wBackColor;
		bool m_bTransBackground;
	};

class CGNeurohackArea : public AGArea
	{
	public:
		CGNeurohackArea (void);
		~CGNeurohackArea (void);

		void CleanUp (void);
		void SetData (int iWillpower, int iDamage);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }

		//	AGArea virtuals
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);
		virtual void Update (void);

	private:
		struct SNode
			{
			int x;
			int y;
			int iWidth;
			int iSphereSize;
			int iRootDist;
			int iDamageLevel;

			SNode *pParent;
			SNode *pNext;
			SNode *pPrev;
			SNode *pFirstChild;
			};

		void CreateBranch (SNode *pParent, int iDirection, int iGeneration, int iWidth, const RECT &rcRect);
		void CreateNetwork (const RECT &rcRect);
		SNode *CreateNode (SNode *pParent, int x, int y);
		void PaintBranch (CG16bitImage &Dest, SNode *pNode, SNode *pNext = NULL);
		void PaintSphere (CG16bitImage &Dest, int x, int y, int iRadius, WORD wGlowColor);

		const SFontTable *m_pFonts;

		SNode *m_pNetwork;
		int m_iNodeCount;
		int m_iNodeAlloc;

		SNode **m_pRootNodes;
		int m_iRootCount;
		int m_iRootAlloc;

		SNode **m_pTerminalNodes;
		int m_iTerminalCount;
		int m_iTerminalAlloc;

		SNode **m_pActiveNodes;
		int m_iActiveCount;
		int m_iActiveAlloc;

		int m_iWillpower;
		int m_iDamage;
	};
