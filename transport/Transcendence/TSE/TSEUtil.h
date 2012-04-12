//	TSEUtil.h
//
//	Transcendence Utilities

#ifndef INCL_TSE_UTIL
#define INCL_TSE_UTIL

//	Utility inlines

inline int Angle2Direction (int iAngle)
	{
	return ((g_RotationRange - (iAngle / g_RotationAngle)) + 5) % g_RotationRange;
	}
inline int Angle2Direction (int iAngle, int iRotationCount)
	{
	return ((iRotationCount - (iAngle / (360 / iRotationCount))) + (iRotationCount / 4)) % iRotationCount;
	}
inline int AlignToRotationAngle (int iAngle) 
	{ 
	return (((g_RotationRange - Angle2Direction(iAngle)) + 5) * g_RotationAngle) % 360; 
	}
inline bool AreAnglesAligned (int iDesiredAngle, int iActualAngle, int iTolerance)
	{
	if (iActualAngle < iDesiredAngle)
		iActualAngle += 360;

	int iOffset = 180 - abs((iActualAngle - iDesiredAngle) - 180);

	return (iOffset <= iTolerance);
	}
inline int AngleRange (int iMinAngle, int iMaxAngle)
	{
	if (iMinAngle > iMaxAngle)
		return (iMaxAngle + 360 - iMinAngle);
	else
		return (iMaxAngle - iMinAngle);
	}
inline int AngleBearing (int iDir, int iTarget)
	{
	int iBearing = AngleRange(iDir, iTarget);
	if (iBearing > 180)
		return iBearing - 360;
	else
		return iBearing;
	}
inline Metric Ticks2Seconds (int iTicks) { return (Metric)iTicks * g_SecondsPerUpdate; }

//	Debugging Support

enum ProgramStates
	{
	psUnknown,							//	No idea what the program is doing now, but
										//		it is none of the other states.
	psAnimating,						//	Program is in Animate() function, but not in
										//		anything more specific.
	psPaintingSRS,						//	We are in Universe.PaintPOV()
	psUpdating,							//	In Universe.Update()
	psUpdatingEvents,					//	In Universe.Update(); updating event objects
	psUpdatingBehavior,					//	In Universe.Update(); updating obj behavior
	psUpdatingObj,						//	In Universe.Update(); updating obj
	psUpdatingMove,						//	In Universe.Update(); updating obj motion
	psUpdatingEncounters,				//	In Universe.Update(); updating random encounters
	psUpdatingExtended,					//	In System.UpdateExtended(); updating obj
	psDestroyNotify,					//	Calling OnObjDestroyed
	psUpdatingReactorDisplay,			//	In CReactorDisplay::Update
	psPaintingMap,						//	In CTranscendenceWnd::PaintMap
	psPaintingLRS,						//	In CTranscendenceWnd::PaintLRS
	psPaintingArmorDisplay,				//	In CArmorDisplay::Paint
	psPaintingMessageDisplay,			//	In CMessageDisplay::Paint
	psPaintingReactorDisplay,			//	In CReactorDisplay::Paint
	psPaintingTargetDisplay,			//	In CTargetDisplay::Paint
	psPaintingDeviceDisplay,			//	In CDeviceCounterDisplay::Paint
	psStargateEnter,					//	In CTranscendenceWnd::EnterStargate
	psStargateEndGame,					//	In EnterStargate, end game
	psStargateLoadingSystem,			//	In EnterStargate, loading system
	psStargateCreatingSystem,			//	In EnterStargate, creating system
	psStargateUpdateExtended,			//	In EnterStargate, updating system
	psStargateTransferringGateFollowers,//	In EnterStargate, transferring gate followers
	psStargateSavingSystem,				//	In EnterStargate, saving the system
	psStargateFlushingSystem,			//	In EnterStargate, flushing old system
	psStargateGarbageCollecting,		//	In EnterStargate, garbage collecting images
	psStargateLoadingBitmaps,			//	In EnterStargate, loading images
	psStargateEnterDone,				//	In EnterStargate, done
	};

#ifdef DEBUG_PROGRAMSTATE
extern ProgramStates g_iProgramState;
extern CSpaceObject *g_pProgramObj;
extern CTimedEvent *g_pProgramEvent;
inline void SetProgramState (ProgramStates iState) { g_iProgramState = iState; g_pProgramObj = NULL; }
inline void SetProgramState (ProgramStates iState, CSpaceObject *pObj) { g_iProgramState = iState; g_pProgramObj = pObj; }
inline void SetProgramObj (CSpaceObject *pObj) { g_pProgramObj = pObj; }
inline void SetProgramEvent (CTimedEvent *pEvent) { g_pProgramEvent = pEvent; }

#define DEBUG_SAVE_PROGRAMSTATE		\
	ProgramStates iDEBUG_SavedState = g_iProgramState;	\
	CSpaceObject *pDEBUG_SavedObj = g_pProgramObj;	\
	CTimedEvent *pDEBUG_SavedEvent = g_pProgramEvent;

#define DEBUG_RESTORE_PROGRAMSTATE	\
	g_iProgramState = iDEBUG_SavedState;	\
	g_pProgramObj = pDEBUG_SavedObj;	\
	g_pProgramEvent = pDEBUG_SavedEvent;

#else
inline void SetProgramState (ProgramStates iState) { }
inline void SetProgramState (ProgramStates iState, CSpaceObject *pObj) { }
inline void SetProgramObj (CSpaceObject *pObj) { }
inline void SetProgramEvent (CTimedEvent *pEvent) { }
#define DEBUG_SAVE_PROGRAMSTATE
#define DEBUG_RESTORE_PROGRAMSTATE
#endif

#ifdef DEBUG_PERFORMANCE
extern DWORD g_dwPerformanceTimer;
inline void DebugStartTimer (void) { g_dwPerformanceTimer = ::GetTickCount(); }
inline void DebugStopTimer (char *szTiming)
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "%s: %d ms\n", szTiming, ::GetTickCount() - g_dwPerformanceTimer);
	::OutputDebugString(szBuffer);
	}
#else
inline void DebugStartTimer (void) { }
inline void DebugStopTimer (char *szTiming) { }
#endif

#define TRY(f)	{try { error = f; } catch (...) { error = ERR_FAIL; }}

//	Game load/save structures

struct SUniverseLoadCtx
	{
	DWORD dwVersion;					//	See CUniverse.cpp for version history

	IReadStream *pStream;				//	Stream to load from
	};

struct SLoadCtx
	{
	SLoadCtx (void) : ObjMap(FALSE, TRUE),
		ForwardReferences(TRUE, FALSE) { }

	DWORD dwVersion;					//	See CSystem.cpp for version history

	IReadStream *pStream;				//	Stream to load from
	CSystem *pSystem;					//	System to load into

	CIDTable ObjMap;					//	Map of ID to objects.
	CIDTable ForwardReferences;			//	Map of ID to CIntArray of addresses
										//		that need CSpaceObject pointer
	};

//	Utility classes

class DiceRange
	{
	public:
		DiceRange (void) : m_iFaces(0), m_iCount(0), m_iBonus(0) { }
		DiceRange (int iFaces, int iCount, int iBonus);

		inline int GetAveValue (void) const { return (m_iCount * (m_iFaces + 1) / 2) + m_iBonus; }
		inline int GetBonus (void) const { return m_iBonus; }
		inline int GetCount (void) const { return m_iCount; }
		inline int GetFaces (void) const { return m_iFaces; }
		inline int GetMaxValue (void) const { return m_iFaces * m_iCount + m_iBonus; }
		inline int GetMinValue (void) const { return m_iCount + m_iBonus; }
		inline bool IsConstant (void) const { return (m_iFaces * m_iCount) == 0; }
		inline bool IsEmpty (void) const { return (m_iFaces == 0 && m_iCount == 0 && m_iBonus == 0); }
		int Roll (void) const;
		ALERROR LoadFromXML (const CString &sAttrib);
		void ReadFromStream (SLoadCtx &Ctx);
		CString SaveToXML (void) const;
		void SetConstant (int iValue) { m_iFaces = 0; m_iCount = 0; m_iBonus = iValue; }
		void WriteToStream (IWriteStream *pStream) const;

	private:
		int m_iFaces;
		int m_iCount;
		int m_iBonus;
	};

class CAttributeDataBlock
	{
	public:
		CAttributeDataBlock (void);
		CAttributeDataBlock (const CAttributeDataBlock &Src);
		CAttributeDataBlock &operator= (const CAttributeDataBlock &Src);
		~CAttributeDataBlock (void);

		inline void DeleteAll (void) { CleanUp(); }
		bool FindObjRefData (CSpaceObject *pObj, CString *retsAttrib = NULL) const;
		const CString &GetData (const CString &sAttrib) const;
		CString GetDataAttrib (int iIndex) const { return m_pData->GetKey(iIndex); }
		int GetDataCount (void) const { return (m_pData ? m_pData->GetCount() : 0); }
		CSpaceObject *GetObjRefData (const CString &sAttrib) const;
		inline bool IsEmpty (void) { return (m_pData == NULL && m_pObjRefData == NULL); }
		bool IsEqual (const CAttributeDataBlock &Src);
		void OnObjDestroyed (CSpaceObject *pObj);
		void ReadFromStream (SLoadCtx &Ctx);
		void ReadFromStream (IReadStream *pStream);
		void SetData (const CString &sAttrib, const CString &sData);
		void SetFromXML (CXMLElement *pData);
		void SetObjRefData (const CString &sAttrib, CSpaceObject *pObj);
		void WriteToStream (IWriteStream *pStream, CSystem *pSystem = NULL);

	private:
		struct SObjRefEntry
			{
			CString sName;
			CSpaceObject *pObj;

			SObjRefEntry *pNext;
			};

		void CleanUp (void);
		void Copy (const CAttributeDataBlock &Copy);

		CSymbolTable *m_pData;					//	Opaque string data
		SObjRefEntry *m_pObjRefData;			//	Custom pointers to CSpaceObject *
	};

class CRandomEntryResults
	{
	public:
		CRandomEntryResults (void);
		~CRandomEntryResults (void);

		void AddResult (CXMLElement *pElement, int iCount);
		inline int GetCount (void) { return m_Results.GetCount(); }
		CXMLElement *GetResult (int iIndex);
		int GetResultCount (int iIndex);

	private:
		CStructArray m_Results;
	};

class CRandomEntryGenerator
	{
	public:
		virtual ~CRandomEntryGenerator (void);

		virtual void Generate (CRandomEntryResults &Results) = 0;
		inline CXMLElement *GetElement (void) { return m_pElement; }
		inline int GetPercent (void) { return m_iPercent; }
		inline void SetPercent (int iPercent) { m_iPercent = iPercent; }

		static ALERROR Generate (CXMLElement *pElement, CRandomEntryResults &Results);
		static ALERROR GenerateAsGroup (CXMLElement *pElement, CRandomEntryResults &Results);
		static ALERROR LoadFromXML (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator);
		static ALERROR LoadFromXMLAsGroup (CXMLElement *pElement, CRandomEntryGenerator **retpGenerator);

	protected:
		CRandomEntryGenerator (void);
		CRandomEntryGenerator (CXMLElement *pElement);
		inline int GetCount (void) { return m_Count.Roll(); }

	private:
		CXMLElement *m_pElement;
		int m_iPercent;					//	Either chance or probability
		DiceRange m_Count;				//	Count
	};

#include <vector>

class CSpaceObjectList
	{
	public:
		CSpaceObjectList (void);
		~CSpaceObjectList (void);

		void Add (CSpaceObject *pObj, int *retiIndex = NULL);
		void FastAdd (CSpaceObject *pObj, int *retiIndex = NULL);
		bool FindObj (CSpaceObject *pObj, int *retiIndex = NULL) const;
		int GetCount (void) const;
		inline CSpaceObject *GetObj (int iIndex) const { return m_pList[iIndex]; }
		inline bool IsEmpty (void) const { return !m_pList.size(); }
		void ReadFromStream (SLoadCtx &Ctx);
		void Remove (int iIndex);
		bool Remove (CSpaceObject *pObj);
		void RemoveAll (void);
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		std::vector<CSpaceObject *> m_pList;
	};

class CSpaceObjectTable
	{
	public:
		CSpaceObjectTable (void);

		void Add (const CString &sKey, CSpaceObject *pObj);
		bool Find (CSpaceObject *pObj, int *retiIndex = NULL);
		inline int GetCount (void) { return m_Table.GetCount(); }
		inline CSpaceObject *Get (int iIndex) { return (CSpaceObject *)m_Table.GetValue(iIndex); }
		inline void Remove (int iIndex) { m_Table.RemoveEntry(m_Table.GetKey(iIndex), NULL); }
		bool Remove (CSpaceObject *pObj);
		inline void RemoveAll (void) { m_Table.RemoveAll(); }

	private:
		CSymbolTable m_Table;
	};

//	Utility functions

CString AppendModifiers (const CString &sModifierList1, const CString &sModifierList2);
bool HasModifier (const CString &sModifierList, const CString &sModifier);

#endif

