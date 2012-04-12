//	Transcendence.h
//
//	Main header file

#ifndef INCL_TRANSCENDENCE
#define INCL_TRANSCENDENCE

#ifndef INCL_TSE
#include "TSE/TSE.h"
#endif

#define DEBUGLOG

#include "CSoundMgr.h"

#define ShowCursor SDL_ShowCursor

class CDockScreen;
class CTranscendenceWnd;

extern int g_cxScreen;
extern int g_cyScreen;
const int g_iColorDepth =					16;		//	Desired color depth

const int g_iFramesPerSecond =				30;		//	Desired frames per second
const int FRAME_RATE_COUNT =				51;		//	number of statistics to keep (for debugging)

#define OBJID_CPLAYERSHIPCONTROLLER	MakeOBJCLASSID(100)

extern CTranscendenceWnd *g_pTrans;
extern const CItem g_DummyItem;
extern CItemListManipulator g_DummyItemListManipulator;

struct SCommandLineOptions
	{
	bool bDebugMode;				//	Program in debug mode
	bool bDebugGame;				//	Game instance in debug mode
	bool bNoDebugLog;				//	Do not write out a debug log
	bool bDebugVideo;				//	Write out video information
	bool bDebugNonExclusive;		//	Non-exclusive DirectX mode
	bool bDebugManualBlt;			//	Do not use DirectX Blt

	bool bNoSound;					//	No sound (either music or sound effects)
	bool bDirectX;					//	Use DirectX to access screen directly
	bool bWindowedMode;				//	Use a window (instead of full screen)
	bool bUseTDB;					//	Force use of .TDB
	};

struct SFontTable
	{
	CG16bitFont Small;				//	10 pixels
	CG16bitFont Medium;				//	13 pixels
	CG16bitFont MediumBold;			//	13 pixels bold
	CG16bitFont MediumHeavyBold;	//	14 pixels bold
	CG16bitFont Large;				//	16 pixels
	CG16bitFont LargeBold;			//	16 pixels bold
	CG16bitFont Header;				//	19 pixels
	CG16bitFont HeaderBold;			//	19 pixels bold
	CG16bitFont SubTitle;			//	26 pixels
	CG16bitFont SubTitleBold;		//	26 pixels bold
	CG16bitFont SubTitleHeavyBold;	//	28 pixels bold
	CG16bitFont Title;				//	56 pixels

	CG16bitFont Console;			//	Fixed-width font

	COLORREF wTitleColor;				//	Large text color
	COLORREF wTextColor;				//	Color for large blocks
	COLORREF wHelpColor;				//	Faded help text
	COLORREF wBackground;				//	Almost black background
	COLORREF wSectionBackground;		//	Ligher section background
	COLORREF wSelectBackground;			//	Selection background

	COLORREF wAltRedColor;				//	Complementary alternate color
	COLORREF wAltRedBackground;			//	Complementary background
	COLORREF wAltGreenColor;			//	Complementary alternate color
	COLORREF wAltGreenBackground;		//	Complementary background
	COLORREF wAltYellowColor;			//	Complementary alternate color
	COLORREF wAltYellowBackground;		//	Complementary background
	COLORREF wAltBlueColor;				//	Complementary alternate color
	COLORREF wAltBlueBackground;		//	Complementary background
	};

//	Functions

void AnimateMainWindow ();
CString ComposePlayerNameString (const CString &sString, const CString &sPlayerName, int iGenome, ICCItem *pArgs = NULL);
const CG16bitFont &GetFontByName (const SFontTable &Fonts, const CString &sFontName);
ALERROR InitCodeChainExtensions (CCodeChain &CC);
CString TransPath (const CString &sPath);

class CIntroShipController : public CObject, public IShipController
	{
	public:
		CIntroShipController (void);
		CIntroShipController (CTranscendenceWnd *pTrans, IShipController *pDelegate);
		virtual ~CIntroShipController (void);

		inline void SetShip (CShip *pShip) { m_pShip = pShip; }

		virtual void Behavior (void) { m_pDelegate->Behavior(); }
		virtual int GetCombatPower (void) { return m_pDelegate->GetCombatPower(); }
		virtual ManeuverTypes GetManeuver (void) { return m_pDelegate->GetManeuver(); }
		virtual bool GetThrust (void) { return m_pDelegate->GetThrust(); }
		virtual bool GetReverseThrust (void) { return m_pDelegate->GetReverseThrust(); }
		virtual bool GetStopThrust (void) { return m_pDelegate->GetStopThrust(); }
		virtual bool GetMainFire (void) { return m_pDelegate->GetMainFire(); }
		virtual bool GetMissileFire (void) { return m_pDelegate->GetMissileFire(); }
		virtual bool GetDeviceActivate (void) { return m_pDelegate->GetDeviceActivate(); }
		virtual int GetFireDelay (void) { return m_pDelegate->GetFireDelay(); }
		virtual int GetFireRateAdj (void) { return m_pDelegate->GetFireRateAdj(); }
		virtual CSpaceObject *GetBase (void) { return m_pDelegate->GetBase(); }
		virtual CSpaceObject *GetEscortPrincipal (void) { return m_pDelegate->GetEscortPrincipal(); }
		virtual CSpaceObject *GetOrderGiver (void) { return m_pShip; }
		virtual CSpaceObject *GetTarget (void) { return m_pDelegate->GetTarget(); }
		virtual bool GetWeaponFire (int iDev) { return m_pDelegate->GetWeaponFire(iDev); }

		virtual void AddOrder(OrderTypes Order, CSpaceObject *pTarget, DWORD dwData, bool bAddBefore = false) { m_pDelegate->AddOrder(Order, pTarget, dwData, bAddBefore); }
		virtual void CancelAllOrders (void) { m_pDelegate->CancelAllOrders(); }
		virtual void CancelCurrentOrder (void) { m_pDelegate->CancelCurrentOrder(); }
		virtual OrderTypes GetCurrentOrderEx (CSpaceObject **retpTarget = NULL, DWORD *retdwData = NULL) { return m_pDelegate->GetCurrentOrderEx(retpTarget, retdwData); }

		//	Events

		virtual void OnArmorRepaired (int iSection) { m_pDelegate->OnArmorRepaired(iSection); }
		virtual void OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage) { m_pDelegate->OnAttacked(pAttacker, Damage); }
		virtual void OnDamaged (CSpaceObject *pCause, int iSection, const DamageDesc &Damage) { m_pDelegate->OnDamaged(pCause, iSection, Damage); }
		virtual void OnDestroyed (DestructionTypes iCause, CSpaceObject *pCause, bool *ioResurrectPending);
		virtual void OnDocked (CSpaceObject *pObj) { m_pDelegate->OnDocked(pObj); }
		virtual void OnDockedObjChanged (void) { m_pDelegate->OnDockedObjChanged(); }
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate) { m_pDelegate->OnEnterGate(pDestNode, sDestEntryPoint, pStargate); }
		virtual void OnFuelLowWarning (int iSeq) { m_pDelegate->OnFuelLowWarning(iSeq); }
		virtual void OnMessage (CSpaceObject *pSender, const CString &sMsg) { m_pDelegate->OnMessage(pSender, sMsg); }
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx) { m_pDelegate->OnObjDestroyed(Ctx); }
		virtual void OnWeaponStatusChanged (void) { m_pDelegate->OnWeaponStatusChanged(); }

	private:
		CTranscendenceWnd *m_pTrans;
		IShipController *m_pDelegate;
		CShip *m_pShip;
	};

enum TargetTypes
	{
	targetEnemies,
	targetFriendlies,
	};

struct SNewGameSettings
	{
	CString sPlayerName;
	GenomeTypes iPlayerGenome;
	DWORD dwAdventure;
	DWORD dwPlayerShip;
	};

class CPlayerShipController : public CObject, public IShipController
	{
	public:
		CPlayerShipController (void);
		inline void SetShip (CShip *pShip) { m_pShip = pShip; }

		void Cargo (void);
		inline void Charge (int iCredits) { m_iCredits -= iCredits; }
		inline void ClearShowHelpDock (void) { m_bShowHelpDock = false; }
		inline void ClearShowHelpMap (void) { m_bShowHelpMap = false; }
		inline void ClearShowHelpAutopilot (void) { m_bShowHelpAutopilot = false; }
		inline void ClearShowHelpGate (void) { m_bShowHelpGate = false; }
		inline void ClearShowHelpUse (void) { m_bShowHelpUse = false; }
		inline void ClearShowHelpRefuel (void) { m_bShowHelpRefuel = false; }
		inline void ClearShowHelpEnableDevice (void) { m_bShowHelpEnableDevice = false; }
		inline void ClearShowHelpSwitchMissile (void) { m_bShowHelpSwitchMissile = false; }
		inline void ClearShowHelpFireMissile (void) { m_bShowHelpFireMissile = false; }

		void Communications (CSpaceObject *pObj, MessageTypes iMsg, DWORD dwData = 0);
		void DestroyPlayer (const CString &sEpitaph);
		void Dock (void);
		inline bool DockingInProgress (void) { return m_pStation != NULL; }
		void Gate (void);
		inline CShipClass *GetBestEnemiesDestroyedClass (void) { return m_pBestEnemyDestroyed; }
		inline int GetBestEnemiesDestroyed (void) { return m_iBestEnemyDestroyedCount; }
		inline int GetCredits (void) { return m_iCredits; }
		inline int GetCargoSpace (void) { return (int)(m_pShip->GetCargoSpaceLeft() + 0.5); }
		inline int GetEnemiesDestroyed (void) { return m_iEnemiesDestroyed; }
		inline GenomeTypes GetGenome (void) const { return m_iGenome; }
		inline int GetInsuranceClaims (void) { return m_iInsuranceClaims; }
		inline CSpaceObject *GetSelectedTarget (void) { return m_pTarget; }
		inline CShip *GetShip (void) { return m_pShip; }
		inline DWORD GetStartingShipClass (void) const { return m_dwStartingShipClass; }
		inline int GetSystemsVisited (void) { return m_iSystemsVisited; }
		inline CTranscendenceWnd *GetTrans (void) { return m_pTrans; }
		inline void IncScore (int iBonus) { m_iScore += iBonus; }
		inline void IncSystemsVisited (void) { m_iSystemsVisited++; }
		void InsuranceClaim (void);
		inline void Insure (void) { m_bInsured = true; }
		inline bool IsMapHUDActive (void) { return m_bMapHUD; }
		inline bool IsInsured (void) { return m_bInsured; }
		inline void Payment (int iCredits) { m_iCredits += iCredits; }
		void ReadyNextWeapon (void);
		void ReadyNextMissile (void);
		inline int GetScore (void) { return m_iScore; }
		void SetDestination (CSpaceObject *pTarget);
		inline void SetGenome (GenomeTypes iGenome) { m_iGenome = iGenome; }
		inline void SetMapHUD (bool bActive) { m_bMapHUD = bActive; }
		inline void SetStartingShipClass (DWORD dwUNID) { m_dwStartingShipClass = dwUNID; }
		void SetTarget (CSpaceObject *pTarget);
		void SelectNearestTarget (void);
		void SelectNextFriendly (void);
		void SelectNextTarget (void);
		inline void SetActivate (bool bActivate) { m_bActivate = bActivate; }
		void SetFireMain (bool bFire);
		void SetFireMissile (bool bFire);
		inline void SetTrans (CTranscendenceWnd *pTrans) { m_pTrans = pTrans; }
		void TransferGateFollowers (CSystem *pSystem, CSpaceObject *pStargate);
		void Undock (void);
		void Update (int iTick);

		//	Fleet formation methods
		DWORD GetCommsStatus (void);
		bool HasFleet (void);
		void FleetOrder (MessageTypes iOrder, DWORD dwData2);

		//	IShipController virtuals
		virtual void AddOrder(OrderTypes Order, CSpaceObject *pTarget, DWORD dwData, bool bAddBefore = false);
		virtual void CancelAllOrders (void);
		virtual void CancelCurrentOrder (void);
		virtual CString DebugCrashInfo (void);
		virtual int GetCombatPower (void);
		virtual ManeuverTypes GetManeuver (void);
		virtual bool GetThrust (void);
		virtual CSpaceObject *GetTarget (void);
		virtual bool GetReverseThrust (void);
		virtual bool GetStopThrust (void);
		virtual bool GetMainFire (void);
		virtual bool GetMissileFire (void);
		virtual CSpaceObject *GetOrderGiver (void) { return m_pShip; }
		virtual bool GetDeviceActivate (void);
		virtual int GetFireDelay (void) { return (int)((5.0 / STD_SECONDS_PER_UPDATE) + 0.5); }
		virtual bool GetWeaponFire (int iDev);
		virtual bool IsPlayer (void) { return true; }
		virtual void ReadFromStream (SLoadCtx &Ctx);
		virtual void SetManeuver (IShipController::ManeuverTypes iManeuver) { m_iManeuver = iManeuver; }
		virtual void SetThrust (bool bThrust) { m_bThrust = bThrust; }
		virtual void WriteToStream (IWriteStream *pStream);

		//	Events
		virtual void OnArmorRepaired (int iSection);
		virtual void OnBlindnessChanged (bool bBlind);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual void OnComponentChanged (ObjectComponentTypes iComponent);
		virtual void OnDamaged (CSpaceObject *pCause, int iSection, const DamageDesc &Damage);
		virtual bool OnDestroyCheck (DestructionTypes iCause, CSpaceObject *pCause);
		virtual void OnDestroyed (DestructionTypes iCause, CSpaceObject *pCause, bool *ioResurrectPending);
		virtual void OnDeviceEnabledDisabled (int iDev, bool bEnable);
		virtual void OnDeviceFailure (CInstalledDevice *pDev, int iType);
		virtual void OnDocked (CSpaceObject *pObj);
		virtual void OnDockedObjChanged (void);
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		virtual void OnFuelLowWarning (int iSeq);
		virtual void OnLifeSupportWarning (int iSecondsLeft);
		virtual void OnMessage (CSpaceObject *pSender, const CString &sMsg);
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx);
		virtual void OnProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program);
		virtual void OnRadiationWarning (int iSecondsLeft);
		virtual void OnRadiationCleared (void);
		virtual void OnReactorOverloadWarning (int iSeq);
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx);
		virtual void OnWeaponStatusChanged (void);
		virtual void OnWreckCreated (CSpaceObject *pWreck);

	private:
		void ClearFireAngle (void);
		CSpaceObject *FindDockTarget (void);
		void InitTargetList (TargetTypes iTargetType, bool bUpdate = false);
		void UpdateHelp (int iTick);

		CTranscendenceWnd *m_pTrans;
		CShip *m_pShip;
		CSpaceObject *m_pStation;
		CSpaceObject *m_pTarget;
		CSpaceObject *m_pDestination;
		CSpaceObject *m_pWreck;
		CObjectArray m_GateFollowers;
		CSpaceObjectTable m_TargetList;

		int m_iLastHelpTick;
		int m_iLastHelpUseTick;
		int m_iLastHelpFireMissileTick;
		bool m_bShowHelpDock;
		bool m_bShowHelpMap;
		bool m_bShowHelpAutopilot;
		bool m_bShowHelpGate;
		bool m_bShowHelpUse;
		bool m_bShowHelpRefuel;
		bool m_bShowHelpEnableDevice;
		bool m_bShowHelpSwitchMissile;
		bool m_bShowHelpFireMissile;

		ManeuverTypes m_iManeuver;
		bool m_bThrust;
		bool m_bFireMain;
		bool m_bFireMissile;
		bool m_bActivate;

		bool m_bMapHUD;							//	Show HUD on map

		int m_iCredits;							//	Credits available to player

		int m_iScore;							//	Total score for player
		int m_iSystemsVisited;					//	Number of star systems visited
		int m_iEnemiesDestroyed;				//	Number of enemy ships destroyed
		CShipClass *m_pBestEnemyDestroyed;		//	Class of most powerful ship destroyed
		int m_iBestEnemyDestroyedCount;			//	Number of powerful enemy ships destroyed

		GenomeTypes m_iGenome;					//	Player genome
		DWORD m_dwStartingShipClass;			//	Starting ship class

		int m_iInsuranceClaims;					//	Number of times player has been resurrected by insurance
		bool m_bInsured;						//	TRUE if player is insured against death

		CString m_sDeferredDestruction;			//	Destroy when we undock

		DWORD m_dwFormationPlace;				//	Temporary

	friend class CObjectClass<CPlayerShipController>;
	};

class CMessageDisplay : public CObject
	{
	public:
		CMessageDisplay (void);

		void ClearAll (void);
		void DisplayMessage (CString sMessage, COLORREF wColor);
		void Paint (CG16bitImage &Dest);
		void Update (void);

		inline void SetBlinkTime (int iTime) { m_iBlinkTime = iTime; }
		inline void SetFadeTime (int iTime) { m_iFadeTime = iTime; }
		inline void SetFont (CG16bitFont *pFont) { m_pFont = pFont; }
		inline void SetRect (RECT &rcRect) { m_rcRect = rcRect; }
		inline void SetSteadyTime (int iTime) { m_iSteadyTime = iTime; }

	private:
		enum Constants
			{
			MESSAGE_QUEUE_SIZE = 5,
			};

		enum State
			{
			stateClear,						//	Blank (stays permanently)
			stateNormal,					//	Normal (stays permanently)
			stateBlinking,					//	Blinking (for m_iBlinkTime)
			stateSteady,					//	Normal (for m_iSteadyTime)
			stateFading						//	Fade to black (for m_iFadeTime)
			};

		struct SMessage
			{
			CString sMessage;				//	Message to paint
			int x;							//	Location of message
			State iState;					//	current state (blinking, etc)
			int iTick;						//	Tick count for this message
			COLORREF wColor;					//	Color to paint
			};

		inline int Next (int iPos) { return ((iPos + 1) % MESSAGE_QUEUE_SIZE); }
		inline int Prev (int iPos) { return ((iPos + MESSAGE_QUEUE_SIZE - 1) % MESSAGE_QUEUE_SIZE); }

		RECT m_rcRect;

		CG16bitFont *m_pFont;				//	Font to use (not owned)
		int m_iBlinkTime;
		int m_iSteadyTime;
		int m_iFadeTime;

		int m_iFirstMessage;
		int m_iNextMessage;
		SMessage m_Messages[MESSAGE_QUEUE_SIZE];

		int m_cySmoothScroll;
	};

class CButtonDisplay : public CObject
	{
	public:
		CButtonDisplay (void);

		void LButtonDown (int x, int y);
		void LButtonUp (int x, int y);
		void MouseMove (int x, int y);
		void Paint (CG16bitImage &Dest);

	private:
		CDockScreen *m_pController;
		RECT m_rcRect;

		CString m_sLabel;
		DWORD m_dwTag;
	};

class IListData
	{
	public:
		virtual ~IListData (void) { }
		virtual void DeleteAtCursor (int iCount) { }
		virtual int GetCount (void) { return 0; }
		virtual int GetCursor (void) { return -1; }
		virtual CString GetDescAtCursor (void) { return NULL_STR; }
		virtual ICCItem *GetEntryAtCursor (CCodeChain &CC) { return CC.CreateNil(); }
		virtual const CItem &GetItemAtCursor (void) { return g_DummyItem; }
		virtual CItemListManipulator &GetItemListManipulator (void) { return g_DummyItemListManipulator; }
		virtual CSpaceObject *GetSource (void) { return NULL; }
		virtual CString GetTitleAtCursor (void) { return NULL_STR; }
		virtual bool IsCursorValid (void) { return false; }
		virtual bool MoveCursorBack (void) { return false; }
		virtual bool MoveCursorForward (void) { return false; }
		virtual void PaintImageAtCursor (CG16bitImage &Dest, int x, int y) { }
		virtual void ResetCursor (void) { }
		virtual void SetCursor (int iCursor) { }
		virtual void SetFilter (const CItemCriteria &Filter) { }
	};

class CItemListWrapper : public IListData
	{
	public:
		CItemListWrapper (CSpaceObject *pSource);

		virtual void DeleteAtCursor (int iCount) { m_ItemList.DeleteAtCursor(iCount); m_pSource->InvalidateItemListAddRemove(); }
		virtual int GetCount (void) { return m_ItemList.GetCount(); }
		virtual int GetCursor (void) { return m_ItemList.GetCursor(); }
		virtual const CItem &GetItemAtCursor (void) { return m_ItemList.GetItemAtCursor(); }
		virtual CItemListManipulator &GetItemListManipulator (void) { return m_ItemList; }
		virtual CSpaceObject *GetSource (void) { return m_pSource; }
		virtual bool IsCursorValid (void) { return m_ItemList.IsCursorValid(); }
		virtual bool MoveCursorBack (void) { return m_ItemList.MoveCursorBack(); }
		virtual bool MoveCursorForward (void) { return m_ItemList.MoveCursorForward(); }
		virtual void ResetCursor (void) { m_ItemList.ResetCursor(); }
		virtual void SetCursor (int iCursor) { m_ItemList.SetCursor(iCursor); }
		virtual void SetFilter (const CItemCriteria &Filter) { m_ItemList.SetFilter(Filter); }

	private:
		CSpaceObject *m_pSource;
		CItemListManipulator m_ItemList;
	};

class CListWrapper : public IListData
	{
	public:
		CListWrapper (CCodeChain *pCC, ICCItem *pList);
		virtual ~CListWrapper (void) { m_pList->Discard(m_pCC); }

		virtual int GetCount (void) { return m_pList->GetCount(); }
		virtual int GetCursor (void) { return m_iCursor; }
		virtual CString GetDescAtCursor (void);
		virtual ICCItem *GetEntryAtCursor (CCodeChain &CC);
		virtual CString GetTitleAtCursor (void);
		virtual bool IsCursorValid (void) { return (m_iCursor != -1); }
		virtual bool MoveCursorBack (void);
		virtual bool MoveCursorForward (void);
		virtual void PaintImageAtCursor (CG16bitImage &Dest, int x, int y);
		virtual void ResetCursor (void) { m_iCursor = -1; }
		virtual void SetCursor (int iCursor) { m_iCursor = iCursor; }

	private:
		CCodeChain *m_pCC;
		ICCItem *m_pList;

		int m_iCursor;
	};

#define ITEM_LIST_AREA_PAGE_UP_ACTION			(0xffff0001)
#define ITEM_LIST_AREA_PAGE_DOWN_ACTION			(0xffff0002)

class CGItemListArea : public AGArea
	{
	public:
		CGItemListArea (void);
		~CGItemListArea (void);

		void CleanUp (void);
		inline void DeleteAtCursor (int iCount) { if (m_pListData) m_pListData->DeleteAtCursor(iCount); Invalidate(); }
		ICCItem *GetEntryAtCursor (void);
		inline const CItem &GetItemAtCursor (void) { return (m_pListData ? m_pListData->GetItemAtCursor() : g_DummyItem); }
		inline CItemListManipulator &GetItemListManipulator (void) { return (m_pListData ? m_pListData->GetItemListManipulator() : g_DummyItemListManipulator); }
		inline CSpaceObject *GetSource (void) { return (m_pListData ? m_pListData->GetSource() : NULL); }
		inline bool IsCursorValid (void) { return (m_pListData ? m_pListData->IsCursorValid() : false); }
		bool MoveCursorBack (void);
		bool MoveCursorForward (void);
		inline void ResetCursor (void) { if (m_pListData) m_pListData->ResetCursor(); Invalidate(); }
		inline void SetCursor (int iIndex) { if (m_pListData) m_pListData->SetCursor(iIndex); Invalidate(); }
		inline void SetFilter (const CItemCriteria &Filter) { if (m_pListData) m_pListData->SetFilter(Filter); Invalidate(); }
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void SetList (CCodeChain &CC, ICCItem *pList);
		void SetList (CSpaceObject *pSource);

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

		void PaintCustom (CG16bitImage &Dest, const RECT &rcRect, bool bSelected);
		void PaintItem (CG16bitImage &Dest, const CItem &Item, const RECT &rcRect, bool bSelected);
		void PaintItemModifier (CG16bitImage &Dest, 
								const CString &sLabel,
								COLORREF rgbBackground,
								RECT *ioRect);

		IListData *m_pListData;
		ListTypes m_iType;

		const SFontTable *m_pFonts;
		int m_iOldCursor;						//	Cursor pos
		int m_yOffset;							//	Painting offset for smooth scroll
		int m_yFirst;							//	coord of first row
	};

class CDockScreen : public CObject,
					public IScreenController
	{
	public:
		CDockScreen (void);
		virtual ~CDockScreen (void);

		void CleanUpScreen (void);
		void HandleChar (char chChar);
		void HandleKeyDown (int iVirtKey);
		ALERROR InitScreen (CTranscendenceWnd *pTrans, 
							RECT &rcRect, 
							CSpaceObject *pLocation, 
							CXMLElement *pDesc, 
							const CString &sPane,
							AGScreen **retpScreen);
		inline bool IsFirstOnInit (void) { return m_bFirstOnInit; }
		inline void ResetFirstOnInit (void) { m_bFirstOnInit = true; }
		void ResetList (void);
		void SetListFilter (const CItemCriteria &Filter);

		//	Methods used by script code
		void DeleteCurrentItem (int iCount);
		void EnableAction (int iAction, bool bEnabled = true);
		int GetCounter (void);
		const CItem &GetCurrentItem (void);
		ICCItem *GetCurrentListEntry (void);
		inline CItemListManipulator &GetItemListManipulator (void) { return m_pItemListControl->GetItemListManipulator(); }
		CString GetTextInput (void);
		bool IsCurrentItemValid (void);
		void SetActionLabel (int iAction, const CString &sLabel, const CString &sKey = NULL_STR);
		void SetDescription (const CString &sDesc);
		ALERROR SetDisplayText (const CString &sID, const CString &sText);
		void SetCounter (int iCount);
		void SetTextInput (const CString &sText);
		void ShowAction (int iAction, bool bShow = true);
		void ShowPane (const CString &sName);
		void ShowScreen (const CString &sName, const CString &sPane);
		inline void Undock (void) { m_pPlayer->Undock(); }

		//	IScreenController virtuals
		virtual void Action (DWORD dwTag, DWORD dwData = 0);

	private:
		enum EControlTypes
			{
			ctrlText,
			ctrlImage,
			};

		struct SDisplayControl
			{
			EControlTypes iType;
			CString sID;
			AGArea *pArea;
			ICCItem *pCode;
			};

		ALERROR CreateBackgroundImage (CXMLElement *pDesc);
		ALERROR CreateItemPickerControl (CXMLElement *pDesc, AGScreen *pScreen, const RECT &rcRect);
		ALERROR CreateTitleAndBackground (CXMLElement *pDesc, AGScreen *pScreen, const RECT &rcRect);
		CString EvalString (const CString &sString, bool bPlain = false);
		bool EvalBool (const CString &sString);
		CString EvalInitialPane (void);
		void ExecuteAction (CXMLElement *pAction);
		SDisplayControl *FindDisplayControl (const CString &sID);
		ALERROR InitCodeChain (CTranscendenceWnd *pTrans, CSpaceObject *pStation);
		ALERROR InitCustomList (void);
		ALERROR InitDisplay (CXMLElement *pDisplayDesc, AGScreen *pScreen, const RECT &rcScreen);
		ALERROR InitFonts (void);
		ALERROR InitItemList (void);
		void PositionActionButtons (int yPos);
		void SelectNextItem (bool *retbMore = NULL);
		void SelectPrevItem (void);
		void ShowDisplay (void);
		void ShowItem (void);
		void UpdateCredits (void);

		CTranscendenceWnd *m_pTrans;
		const SFontTable *m_pFonts;
		CUniverse *m_pUniv;
		CPlayerShipController *m_pPlayer;
		CSpaceObject *m_pLocation;
		CXMLElement *m_pDesc;
		AGScreen *m_pScreen;
		bool m_bFirstOnInit;

		//	Title and header
		CG16bitImage *m_pBackgroundImage;
		CGTextArea *m_pCredits;
		CGTextArea *m_pCargoSpace;
		bool m_bFreeBackgroundImage;

		//	Item list variables
		CGItemListArea *m_pItemListControl;
		CItemCriteria m_ItemCriteria;

		//	Display controls
		int m_iControlCount;
		SDisplayControl *m_pControls;

		//	Counter variables;
		CGTextArea *m_pCounter;
		bool m_bReplaceCounter;

		//	Text input variables
		CGTextArea *m_pTextInput;

		//	Panes
		CXMLElement *m_pPanes;
		CXMLElement *m_pCurrentPane;
		CXMLElement *m_pCurrentActions;
		RECT m_rcPane;
		CGFrameArea *m_pCurrentFrame;
		CGTextArea *m_pFrameDesc;
		bool m_bInShowPane;
		bool m_bNoListNavigation;
	};

class CArmorDisplay
	{
	public:
		CArmorDisplay (void);
		~CArmorDisplay (void);

		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CPlayerShipController *pPlayer, const RECT &rcRect);
		void Paint (CG16bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void SetSelection (int iSelection);
		void Update (void);

	private:
		void CleanUp (void);

		CUniverse *m_pUniverse;
		CPlayerShipController *m_pPlayer;

		RECT m_rcRect;
		CG16bitImage m_Buffer;
		const SFontTable *m_pFonts;
		int m_iSelection;
	};

#define MAX_SCORES			100

class CHighScoreList
	{
	public:
#if 0
		struct Entry
			{
			int iScore;
			int iResurrectCount;
			int iSystemsVisited;
			int iEnemiesDestroyed;
			CShipClass *pBestEnemyDestroyed;
			int iBestEnemyDestroyedCount;
			int iGenome;
			CString sName;
			CString sShipClass;
			CString sEpitaph;
			CString sTime;

			bool bEscapedHumanQuarantineZone;
			bool bDebugGame;
			};
#endif

		CHighScoreList (void);

		ALERROR Load (const CString &sFilename);
		ALERROR Save (const CString &sFilename);

		int AddEntry (const SGameStats &NewEntry);

		inline int GetCount (void) const { return m_iCount; }
		inline const SGameStats &GetEntry (int iIndex) const { return m_List[iIndex]; }
		inline int GetMostRecentPlayerGenome (void) const { return m_iMostRecentPlayerGenome; }
		inline const CString &GetMostRecentPlayerName (void) const { return m_sMostRecentPlayerName; }
		void SetMostRecentPlayerName (const CString &sName);
		void SetMostRecentPlayerGenome (int iGenome);

	private:
		int LoadGenome (const CString &sAttrib);

		bool m_bModified;
		int m_iCount;
		SGameStats m_List[MAX_SCORES];
		int m_iMostRecentPlayerGenome;
		CString m_sMostRecentPlayerName;
	};

class CHighScoreDisplay
	{
	public:
		CHighScoreDisplay (void);
		~CHighScoreDisplay (void);

		void CleanUp (void);
		ALERROR Init (const RECT &rcRect, CHighScoreList *pList, int iHighlight = -1);
		void Paint (CG16bitImage &Dest);
		void SelectNext (void);
		void SelectPrevious (void);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void Update (void);

	private:
		int ComputeFirstEntry (int iHighlight);

		CHighScoreList *m_pList;
		int m_iFirstEntry;
		int m_iSelectedEntry;

		RECT m_rcRect;
		CG16bitImage m_Buffer;
		const SFontTable *m_pFonts;
		RECT m_rcHighlight;
	};

class CTextCrawlDisplay
	{
	public:
		CTextCrawlDisplay (void);
		~CTextCrawlDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (const RECT &rcRect, const CString &sText);
		void Paint (CG16bitImage &Dest);
		inline void SetFont (const CG16bitFont *pFont) { m_pFont = pFont; }
		void Update (void);

	private:
		RECT m_rcRect;
		RECT m_rcText;
		int m_yPos;
		const CG16bitFont *m_pFont;
		CStringArray m_EpilogLines;
	};

#define MAX_MENU_ITEMS				100

class CMenuData
	{
	public:
		CMenuData (void);

		inline void AddMenuItem (const CString &sKey,
						  const CString &sLabel,
						  DWORD dwData,
						  DWORD dwData2 = 0) { AddMenuItem(sKey, sLabel, NULL, NULL_STR, dwData, dwData2); }
		void AddMenuItem (const CString &sKey,
						  const CString &sLabel,
						  const CObjectImageArray *pImage,
						  const CString &sExtra,
						  DWORD dwData,
						  DWORD dwData2 = 0);
		inline void SetTitle (const CString &sTitle) { m_sTitle = sTitle; }

		bool FindItemData (const CString &sKey, DWORD *retdwData = NULL, DWORD *retdwData2 = NULL);
		inline int GetCount (void) const { return m_iCount; }
		inline DWORD GetItemData (int iIndex) const { return m_List[iIndex].dwData; }
		inline const CObjectImageArray *GetItemImage (int iIndex) const { return m_List[iIndex].pImage; }
		inline const CString &GetItemExtra (int iIndex) const { return m_List[iIndex].sExtra; }
		inline const CString &GetItemKey (int iIndex) const { return m_List[iIndex].sKey; }
		inline const CString &GetItemLabel (int iIndex) const { return m_List[iIndex].sLabel; }
		inline const CString &GetTitle (void) { return m_sTitle; }
		inline void RemoveAll (void) { m_iCount = 0; }

	private:
		struct Entry
			{
			CString sKey;
			CString sLabel;
			const CObjectImageArray *pImage;
			CString sExtra;

			DWORD dwData;
			DWORD dwData2;
			};

		CString m_sTitle;
		int m_iCount;
		Entry m_List[MAX_MENU_ITEMS];
	};

class CMenuDisplay
	{
	public:
		CMenuDisplay (void);
		~CMenuDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CMenuData *pMenu, const RECT &rcRect);
		inline void Invalidate (void) { m_bInvalid = true; }
		void Paint (CG16bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }

	private:
		void ComputeMenuRect (RECT *retrcRect);
		void Update (void);

		CMenuData *m_pMenu;	

		RECT m_rcRect;
		CG16bitImage m_Buffer;
		const SFontTable *m_pFonts;
		bool m_bInvalid;
	};

class CPickerDisplay
	{
	public:
		CPickerDisplay (void);
		~CPickerDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		int GetSelection (void);
		ALERROR Init (CMenuData *pMenu, const RECT &rcRect);
		inline void Invalidate (void) { m_bInvalid = true; }
		void Paint (CG16bitImage &Dest);
		inline void ResetSelection (void) { m_iSelection = 0; }
		void SelectNext (void);
		void SelectPrev (void);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		inline void SetHelpText (const CString &sText) { m_sHelpText = sText; }

	private:
		void PaintSelection (CG16bitImage &Dest, int x, int y);
		void Update (void);

		CMenuData *m_pMenu;

		RECT m_rcRect;
		CG16bitImage m_Buffer;
		const SFontTable *m_pFonts;
		int m_iSelection;
		bool m_bInvalid;
		CString m_sHelpText;
	};

#define MAX_BUTTONS					10

class CButtonBarData
	{
	public:
		CButtonBarData (void);
		~CButtonBarData (void);

		enum AlignmentTypes
			{
			alignCenter,
			alignLeft,
			alignRight,
			};

		enum StyleTypes
			{
			styleLarge,							//	128x64 button
			styleMedium,						//	64x64 button
			};

		void AddButton (DWORD dwCmdID,
						const CString &sLabel,
						const CString &sDescription,
						const CString &sKey,
						int iImageIndex,
						AlignmentTypes iAlign,
						StyleTypes iStyle = styleLarge);
		void CleanUp (void);
		int FindButton (const CString &sKey);
		inline AlignmentTypes GetAlignment (int iIndex) { return m_Buttons[iIndex].iAlign; }
		inline const RECT &GetButtonRect (int iIndex) { return m_Buttons[iIndex].rcRect; }
		inline DWORD GetCmdID (int iIndex) { return m_Buttons[iIndex].dwCmdID; }
		inline int GetCount (void) { return m_iCount; }
		inline const CString &GetDescription (int iIndex) { return m_Buttons[iIndex].sDescription; }
		inline const CG16bitImage &GetImage (void) { return m_Images; }
		void GetImageSize (int iIndex, RECT *retrcRect);
		inline int GetImageIndex (int iIndex) { return m_Buttons[iIndex].iImageIndex; }
		inline const CString &GetKey (int iIndex) { return m_Buttons[iIndex].sKey; }
		inline const CString &GetLabel (int iIndex) { return m_Buttons[iIndex].sLabel; }
		inline StyleTypes GetStyle (int iIndex) { return m_Buttons[iIndex].iStyle; }
		inline bool GetVisible (int iIndex) { return ((m_Buttons[iIndex].dwFlags & FLAG_VISIBLE) ? true : false); }
		int HitTest (int x, int y);
		ALERROR Init (void);

		inline void SetButtonRect (int iIndex, const RECT &rcRect) { m_Buttons[iIndex].rcRect = rcRect; }
		inline void SetDescription (int iIndex, const CString &sDescription) { m_Buttons[iIndex].sDescription = sDescription; }
		inline void SetLabel (int iIndex, const CString &sLabel) { m_Buttons[iIndex].sLabel = sLabel; }
		inline void SetVisible (int iIndex, bool bVisible) { if (bVisible) m_Buttons[iIndex].dwFlags |= FLAG_VISIBLE; else m_Buttons[iIndex].dwFlags &= ~FLAG_VISIBLE; }

	private:
		enum Flags
			{
			FLAG_VISIBLE = 0x00000001,
			};

		struct Entry
			{
			DWORD dwCmdID;
			CString sLabel;
			CString sDescription;
			CString sKey;
			int iImageIndex;
			AlignmentTypes iAlign;
			StyleTypes iStyle;
			DWORD dwFlags;

			RECT rcRect;
			};

		int m_iCount;
		Entry m_Buttons[MAX_BUTTONS];
		CG16bitImage m_Images;
	};

class CButtonBarDisplay
	{
	public:
		CButtonBarDisplay (void);
		~CButtonBarDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CTranscendenceWnd *pTrans, 
					  CButtonBarData *pData, 
					  const RECT &rcRect);
		inline void Invalidate (void) { }
		bool OnChar (char chChar);
		bool OnKeyDown (int iVirtKey);
		bool OnLButtonDoubleClick (int x, int y);
		bool OnLButtonDown (int x, int y);
		void OnMouseMove (int x, int y);
		void Paint (CG16bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void Update (void);

	private:
		void ComputeButtonRects (void);
		void ComputeButtonSize (int iIndex, int *retcxWidth, int *retcyHeight);
		int ComputeButtonWidth (CButtonBarData::AlignmentTypes iAlignment);
		void ComputeButtonRects (CButtonBarData::AlignmentTypes iAlignment, int x, int y);
		void GetImageRect (int iIndex, bool bSelected, RECT *retrcRect);
		int FindButtonAtPoint (const POINT &pt);

		CTranscendenceWnd *m_pTrans;
		CButtonBarData *m_pButtons;
		RECT m_rcRect;
		const SFontTable *m_pFonts;

		int m_iSelected;
	};

class CDeviceCounterDisplay
	{
	public:
		CDeviceCounterDisplay (void);
		~CDeviceCounterDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CPlayerShipController *pPlayer, const RECT &rcRect);
		inline void Invalidate (void) { m_bInvalid = true; }
		void Paint (CG16bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void Update (void);

	private:
		void PaintDevice (CInstalledDevice *pDevice, int x);

		CPlayerShipController *m_pPlayer;

		RECT m_rcRect;
		RECT m_rcBuffer;
		CG16bitImage m_Buffer;
		const SFontTable *m_pFonts;
		bool m_bInvalid;
		bool m_bEmpty;
	};

class CPlayerDisplay
	{
	public:
		CPlayerDisplay (void);
		~CPlayerDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CTranscendenceWnd *pTrans, const RECT &rcRect, bool bShowDebugOption);
		inline void Invalidate (void) { m_bInvalid = true; }
		bool OnChar (char chChar);
		bool OnLButtonDoubleClick (int x, int y);
		bool OnLButtonDown (int x, int y);
		void OnMouseMove (int x, int y);
		bool OnKeyDown (int iVirtKey);
		void Paint (CG16bitImage &Dest);
		void Update (void);

	private:
		struct SPlayerShip
			{
			CShipClass *pClass;
			CString sName;
			};

		void PaintBuffer (void);
		void PaintOption (int iPos, const RECT &rcIcon, const CString &sLabel);

		CTranscendenceWnd *m_pTrans;
		CString m_sName;
		bool m_bMale;
		bool m_bMusicOn;
		bool m_bShowDebugOption;

		CString m_sEditBuffer;
		bool m_bEditing;
		bool m_bClearAll;

		bool m_bInvalid;
		RECT m_rcRect;
		RECT m_rcIcon;
		RECT m_rcName;
		RECT m_rcGenderOption;
		RECT m_rcMusicOption;
		RECT m_rcDebugModeOption;
		CG16bitImage m_IconImage;
		CG16bitImage m_Buffer;
		const SFontTable *m_pFonts;
	};

class CReactorDisplay
	{
	public:
		CReactorDisplay (void);
		~CReactorDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CPlayerShipController *pPlayer, const RECT &rcRect);
		void Paint (CG16bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }
		void Update (void);

	private:
		CPlayerShipController *m_pPlayer;

		RECT m_rcRect;
		CG16bitImage m_Buffer;
		const SFontTable *m_pFonts;
		int m_iTickCount;
		int m_iOverloading;
	};

class CAdventureDescDisplay
	{
	public:
		CAdventureDescDisplay (void);
		~CAdventureDescDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CTranscendenceWnd *pTrans, const RECT &rcRect);
		inline void Invalidate (void) { m_bInvalid = true; }
		bool OnKeyDown (int iVirtKey);
		void Paint (CG16bitImage &Dest);
		void SelectNext (void);
		void SelectPrev (void);
		void Update (void);

	private:
		void PaintBuffer (void);

		CTranscendenceWnd *m_pTrans;
		int m_iCurrentIndex;

		bool m_bInvalid;
		RECT m_rcRect;
		CG16bitImage m_Buffer;
		const SFontTable *m_pFonts;
	};

class CShipClassDisplay
	{
	public:
		CShipClassDisplay (void);
		~CShipClassDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CTranscendenceWnd *pTrans, const RECT &rcRect, bool bShowDebugShips);
		inline void Invalidate (void) { m_bInvalid = true; }
		bool OnKeyDown (int iVirtKey);
		void Paint (CG16bitImage &Dest);
		void SelectNext (void);
		void SelectPrev (void);
		void Update (void);

	private:
		enum Constants
			{
			MAX_DATA_BOXES = 20,

			FLAG_ALIGN_LEFT =	0x00000001,
			FLAG_ALIGN_RIGHT =	0x00000002,
			FLAG_CONTINUES =	0x00000004,
			FLAG_PLAIN_HEADER = 0x00000008,

			SPECIAL_ICON_DEVICE_SLOTS =		1,
			SPECIAL_ICON_MAX =				256,
			};

		struct SPlayerShip
			{
			CShipClass *pClass;
			CString sName;
			};

		struct SDataBox
			{
			CItemType *pItemIcon;
			CString sNumber;
			CString sHeading;
			CString sDesc;

			DWORD dwFlags;
			};

		SDataBox *AddDataBox (void);
		void InitPlayerShipList (bool bShowDebugShips);
		void InitShipData (void);
		void PaintBuffer (void);
		void PaintDataBox (CG16bitImage &Dest, int x, int y, const SDataBox &DataBox, int *retcyHeight = NULL);
		void PaintNextDataBox (int x, int &y, const SDataBox &DataBox, int &yLastIcon);

		CTranscendenceWnd *m_pTrans;
		int m_iPlayerShip;

		int m_iPlayerShipCount;
		SPlayerShip *m_PlayerShipList;

		CG16bitImage *m_pShipImage;
		int m_iDataBoxCount;
		SDataBox m_DataBoxes[MAX_DATA_BOXES];
		int m_iLeftCount;
		int m_iRightCount;

		bool m_bInvalid;
		RECT m_rcRect;
		CG16bitImage m_Buffer;
		const SFontTable *m_pFonts;
		CG16bitImage m_Images;

		RECT m_rcCenterPane;
		RECT m_rcLeftPane;
		RECT m_rcRightPane;
		RECT m_rcBottomPane;
	};

class CCommandLineDisplay
	{
	public:
		CCommandLineDisplay (void);
		~CCommandLineDisplay (void);

		void CleanUp (void);
		inline void ClearInput (void) { m_sInput = NULL_STR; m_bInvalid = true; }
		inline const CString &GetInput (void) { return m_sInput; }
		inline int GetOutputLineCount (void) { return GetOutputCount(); }
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CTranscendenceWnd *pTrans, const RECT &rcRect);
		void Input (const CString &sInput);
		void InputBackspace (void);
		void InputEnter (void);
		void Output (const CString &sOutput, COLORREF wColor = 0);
		void Paint (CG16bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }

	private:
		enum Constants
			{
			MAX_LINES = 80,
			};

		void AppendOutput (const CString &sLine, COLORREF wColor);
		const CString &GetOutput (int iLine);
		COLORREF GetOutputColor (int iLine);
		int GetOutputCount (void);
		void Update (void);

		CTranscendenceWnd *m_pTrans;
		const SFontTable *m_pFonts;
		RECT m_rcRect;

		CString m_Output[MAX_LINES + 1];
		COLORREF m_OutputColor[MAX_LINES + 1];
		int m_iOutputStart;
		int m_iOutputEnd;
		CString m_sInput;

		CG16bitImage m_Buffer;
		bool m_bInvalid;
		int m_iCounter;
		RECT m_rcCursor;
	};

class CTargetDisplay
	{
	public:
		CTargetDisplay (void);
		~CTargetDisplay (void);

		void CleanUp (void);
		inline const RECT &GetRect (void) { return m_rcRect; }
		ALERROR Init (CPlayerShipController *pPlayer, const RECT &rcRect);
		inline void Invalidate (void) { m_bInvalid = true; }
		void Paint (CG16bitImage &Dest);
		inline void SetFontTable (const SFontTable *pFonts) { m_pFonts = pFonts; }

	private:
		void PaintDeviceStatus (CShip *pShip, DeviceNames iDev, int x, int y);
		void Update (void);

		CPlayerShipController *m_pPlayer;

		RECT m_rcRect;
		CG16bitImage m_Buffer;
		CG16bitImage *m_pBackground;
		const SFontTable *m_pFonts;
		bool m_bInvalid;
	};

class CGameFile
	{
	public:
		CGameFile (void);
		~CGameFile (void);

		void Close (void);
		ALERROR Create (const CString &sFilename);
		CString GenerateFilename (const CString &sName);
		inline int GetResurrectCount (void) { return m_Header.dwResurrectCount; }
		inline CString GetSystemName (void) { return CString(m_Header.szSystemName); }
		inline bool IsGameResurrect (void) { return ((m_Header.dwFlags & GAME_FLAG_RESURRECT) ? true : false); }
		inline bool IsUniverseValid (void) { return (m_Header.dwUniverse != INVALID_ENTRY); }
		ALERROR Open (const CString &sFilename);

		ALERROR LoadSystem (DWORD dwUNID, CSystem **retpSystem, DWORD dwObjID = 0xffffffff, CSpaceObject **retpObj = NULL);
		ALERROR LoadUniverse (CUniverse &Univ, DWORD *retdwSystemID, DWORD *retdwPlayerID, CString *retsError);
		ALERROR SaveSystem (DWORD dwUNID, CSystem *pSystem);
		ALERROR SaveUniverse (CUniverse &Univ, bool bCheckpoint);
		ALERROR SetGameResurrect (void);

	private:
		enum GameFlags
			{
			GAME_FLAG_RESURRECT =					0x00000001,
			GAME_FLAG_DEBUG =						0x00000002,
			};

		enum Consts
			{
			GAME_HEADER_MAX_SYSTEM_NAME =			128,
			INVALID_ENTRY =							0xffffffff,
			};

		struct SGameHeader
			{
			DWORD dwVersion;

			DWORD dwUniverse;				//	Location of universe data
			DWORD dwSystemMap;				//	Location of system directory. The system
											//		directory is an array of file IDs
											//		indexed by system UNID and prefixed
											//		by a count.
			char szSystemName[GAME_HEADER_MAX_SYSTEM_NAME];
			DWORD dwFlags;					//	Flags for game
			DWORD dwResurrectCount;			//	Number of times we're been resurrected

			DWORD dwSpare[7];
			};

		ALERROR LoadGameHeader (SGameHeader *retHeader);
		void LoadSystemMapFromStream (DWORD dwVersion, const CString &sStream);
		ALERROR SaveGameHeader (SGameHeader &Header);
		void SaveSystemMapToStream (CString *retsStream);

		int m_iRefCount;

		CDataFile *m_pFile;

		int m_iHeaderID;							//	Entry of header
		SGameHeader m_Header;						//	Loaded header
		CIDTable m_SystemMap;						//	Map from system ID to save file ID
	};

#define DEBUG_LINES_COUNT					51

#define CMD_CONTINUE_OLD_GAME				120
#define CMD_START_NEW_GAME					121
#define CMD_QUIT_GAME						122
#define CMD_SELECT_SHIP						123
#define CMD_SELECT_SHIP_CANCEL				124
#define CMD_NEXT_SHIP						125
#define CMD_PREV_SHIP						126

#define CMD_SELECT_ADVENTURE				201
#define CMD_SELECT_ADVENTURE_CANCEL			202
#define CMD_NEXT_ADVENTURE					203
#define CMD_PREV_ADVENTURE					204

#define CMD_LOAD_ADVENTURE					301

#define MAP_SCALE_COUNT						4

class CTranscendenceWnd : public CUniverse::IDebug
	{
	public:
		CTranscendenceWnd ();
		virtual ~CTranscendenceWnd() { }

		void Animate (void);

		void Autopilot (bool bTurnOn);
		void ClearMessage (void);
		inline CString ComposePlayerNameString (const CString &sString, ICCItem *pArgs = NULL) { return ::ComposePlayerNameString(sString, m_sPlayerName, m_iPlayerGenome, pArgs); }
		inline void DamageFlash (void) { m_iDamageFlash += 2; }
		void DebugConsoleOutput (const CString &sOutput);
		void DisplayMessage (CString sMessage);
		void DoCommand (DWORD dwCmd);
		inline DWORD GetAdventure (void) { return m_dwAdventure; }
		inline bool GetDebugGame (void) { return m_Options.bDebugGame; }
		DWORD GetDefaultPlayerShip (void);
		inline const SFontTable &GetFonts (void) { return m_Fonts; }
		void GetMousePos (POINT *retpt);
		inline bool GetMusicOption (void) { return m_Prefs.bMusicOn; }
		inline CPlayerShipController *GetPlayer (void) { return m_pPlayer; }
		inline int GetPlayerGenome (void) { return m_iPlayerGenome; }
		inline int GetSoundVolumeOption (void) { return m_Prefs.iSoundVolume; }
		inline const CString &GetPlayerName (void) { return m_sPlayerName; }
		inline DWORD GetPlayerShip (void) { return m_dwPlayerShip; }
		inline const CString &GetRedirectMessage (void) { return m_sRedirectMessage; }
		void HideDockScreen (void);
		inline bool InAutopilot (void) { return m_bAutopilot; }
		inline bool InGameState (void) { return m_State == gsInGame; }
		inline bool InMap (void) { return m_bShowingMap; }
		inline bool InMenu (void) { return (m_CurrentMenu != menuNone || m_CurrentPicker != pickNone); }
		void OnObjDestroyed (CSpaceObject *pObjDestroyed, CSpaceObject *pDestroyer);
		void PlayerDestroyed (const CString &sText);
		void PlayerEnteredGate (CSystem *pSystem, 
							    CTopologyNode *pDestNode,
							    const CString &sDestEntryPoint);
		void RedirectDisplayMessage (bool bRedirect = true);
		void ResetDockScreenList (void) { m_CurrentDock.ResetList(); }
		void SelectArmor (int iSeg);
		void SetAdventure (DWORD dwAdventure) { m_dwAdventure = dwAdventure; }
		inline void SetCrawlText (const CString &sText) { m_sCrawlText = sText; }
		inline void SetCrawlImage (DWORD dwUNID) { m_dwCrawlImage = dwUNID; }
		void SetDebugGame (bool bDebugMode = true);
		void SetMusicOption (bool bMusicOn);
		void SetPlayerGenome (int iGenome);
		void SetPlayerName (const CString &sName);
		void SetPlayerShip (DWORD dwUNID);
		void SetSoundVolumeOption (int iVolume);
		ALERROR ShowDockScreen (CSpaceObject *pLocation, CXMLElement *pScreenDesc, const CString &sPane);
		inline void UpdateArmorDisplay (void) { m_ArmorDisplay.Update(); }
		inline void UpdateDeviceCounterDisplay (void) { m_DeviceDisplay.Invalidate(); }
		inline void UpdateWeaponStatus (void) { m_TargetDisplay.Invalidate(); }

		//	CUniverse::IDebug
		virtual void DebugOutput (const CString &sLine);
		virtual void GameOutput (const CString &sLine);

	private:
		enum BackgroundState
			{
			bsNone,
			bsLoadUniverse,
			bsCreateGame,
			bsQuit,
			};

		enum GameState
			{
			gsNone,
			gsLoading,
			gsIntro,
			gsSelectAdventure,
			gsSelectShip,
			gsProlog,
			gsInGame,
			gsDocked,
			gsHelp,
			gsEnteringStargate,
			gsLeavingStargate,
			gsDestroyed,
			gsEpilog,
			};

		enum MenuTypes
			{
			menuNone,
			menuGame,
			menuSelfDestructConfirm,
			menuOrders,
			menuCommsTarget,
			menuComms,
			menuInvoke,
			};

		enum PickerTypes
			{
			pickNone,
			pickUsableItem,
			pickPower,
			pickEnableDisableItem,
			};

		struct SPreferences
			{
			bool bMusicOn;
			int iSoundVolume;

			bool bModified;
			};

		ALERROR StartLoading (void);
		void AnimateLoading (void);
		ALERROR LoadUniverseDefinition (void);
		void PaintLoadingScreen (void);
		void StopLoading (void);
		static DWORD LoadUniverseThread (LPVOID pData);

		ALERROR StartIntro (void);
		void AnimateIntro (void);
		void OnDblClickIntro (int x, int y, DWORD dwFlags);
		void OnCharIntro (char chChar, DWORD dwKeyData);
		void OnKeyDownIntro (int iVirtKey, DWORD dwKeyData);
		void OnLButtonDownIntro (int x, int y, DWORD dwFlags);
		void OnMouseMoveIntro (int x, int y, DWORD dwFlags);
		void PaintOverwriteGameDlg (void);
		void PaintDlgButton (const RECT &rcRect, const CString &sText);
		void StopIntro (void);

		ALERROR StartSelectAdventure (void);
		void AnimateSelectAdventure (void);
		void OnCharSelectAdventure (char chChar, DWORD dwKeyData);
		void OnDblClickSelectAdventure (int x, int y, DWORD dwFlags);
		void OnKeyDownSelectAdventure (int iVirtKey, DWORD dwKeyData);
		void OnLButtonDownSelectAdventure (int x, int y, DWORD dwFlags);
		void OnMouseMoveSelectAdventure (int x, int y, DWORD dwFlags);
		void StopSelectAdventure (void);

		ALERROR StartSelectShip (void);
		void AnimateSelectShip (void);
		void OnCharSelectShip (char chChar, DWORD dwKeyData);
		void OnDblClickSelectShip (int x, int y, DWORD dwFlags);
		void OnKeyDownSelectShip (int iVirtKey, DWORD dwKeyData);
		void OnLButtonDownSelectShip (int x, int y, DWORD dwFlags);
		void OnMouseMoveSelectShip (int x, int y, DWORD dwFlags);
		void StopSelectShip (void);

		ALERROR StartProlog (void);
		void StopProlog (void);
		void AnimateCrawlScreen (void);
		ALERROR InitCrawlScreen (void);

		void DefineGameGlobals (void);
		ALERROR StartGame (void);
		ALERROR StartNewGame (const SNewGameSettings &Options, CString *retsError);
		ALERROR StartNewGameBackground (CString *retsError);
		ALERROR LoadOldGame (CString *retsError);
		ALERROR SaveGame (bool bCheckpoint);

		ALERROR StartHelp (void);
		void PaintHelpScreen (void);
		void StopHelp (void);

		ALERROR StartEpilog (void);
		void StopEpilog (void);
		ALERROR CreateRandomShip (CSystem *pSystem, CSovereign *pSovereign, CShip **retpShip);

		void BltScreen (void);
		void CleanUpDirectDraw (void);
		void CleanUpPlayerShip (void);
		void CleanUpPlayerVariables (void);
		void ClearDebugLines (void);
		CString ComputeLoadGameLabel (bool *retbSavedGame = NULL);
		void ComputeScreenSize (void);
		void EndDestroyed (void);
		void EnterStargate (void);
		ALERROR InitDirectDraw (int iColorDepth);
		ALERROR InitDisplays (void);
		void LeaveStargate (void);
		void LoadPreferences (void);
		void PaintDebugLines (void);
		void PaintFrameRate (void);
		void PaintLRS (void);
		void PaintMainScreenBorder (void);
		void PaintMap (void);
		void PaintSnow (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight);
		void PaintSRSSnow (void);
		void PaintWeaponStatus (void);
		void RecordFinalScore (const CString &sEpitaph, const CString &sEndGameReason, bool bEscaped);
		void ReportCrash (void);
		void ReportCrashEvent (CString *retsMessage);
		ALERROR RestartGame (void);
		void SavePreferences (void);
		void ShowErrorMessage (const CString &sError);

		void DoCommsMenu (int iIndex);
		void DoGameMenuCommand (DWORD dwCmd);
		void DoOrdersCommand (MessageTypes iOrder, DWORD dwData2);
		void DoSelfDestructConfirmCommand (DWORD dwCmd);
		void DoEnableDisableItemCommand (DWORD dwData);
		void DoInvocation (CPower *pPower);
		void DoUseItemCommand (DWORD dwData);
		void ShowCommsMenu (CSpaceObject *pObj);
		void ShowCommsTargetMenu (void);
		void ShowEnableDisablePicker (void);
		void ShowInvokeMenu (void);
		void ShowOrdersMenu (void);
		void ShowGameMenu (void);
		void ShowUsePicker (void);

		void OpenGameLog (void);
		void CloseGameLog (void);

		static int BackgroundThread (LPVOID pData);
		void BackgroundLoadUniverse (void);
		void BackgroundNewGame (void);
		void CreateBackgroundThread (void);
		void DestroyBackgroundThread (void);
		bool IsBackgroundDone (void) { return m_iBackgroundState == bsNone; }
		void SetBackgroundState (BackgroundState iState);

		LONG WMActivateApp (bool bActivate);
		LONG WMChar (char chChar, DWORD dwKeyData);
		LONG WMClose (void);
		LONG WMCreate (SCommandLineOptions *cmdOpts, CString *retsError);
		LONG WMDestroy (void);
		LONG WMDisplayChange (int iBitDepth, int cxWidth, int cyHeight);
		LONG WMKeyDown (int iVirtKey, DWORD dwKeyData);
		LONG WMKeyUp (int iVirtKey, DWORD dwKeyData);
		LONG WMLButtonDblClick (int x, int y, DWORD dwFlags);
		LONG WMLButtonDown (int x, int y, DWORD dwFlags);
		LONG WMLButtonUp (int x, int y, DWORD dwFlags);
		LONG WMMouseMove (int x, int y, DWORD dwFlags);
		LONG WMMove (int x, int y);
		LONG WMSize (int cxWidth, int cyHeight, int iSize);

		//	General
		CString m_sVersion;
		CString m_sCopyright;

		//	Game
		SCommandLineOptions m_Options;		//	Command line options
		SPreferences m_Prefs;				//	Game preferences
		CString m_sPlayerName;				//	Name of player
		int m_iPlayerGenome;				//	Player genome
		DWORD m_dwPlayerShip;				//	Player ship class
		DWORD m_dwAdventure;				//	Adventure UNID
		GameState m_State;					//	Game state
		bool m_bShowingMap;					//	Showing system map
		bool m_bAutopilot;					//	Autopilot is ON
		bool m_bPaused;						//	Game paused
		bool m_bPausedStep;					//	Step one frame
		bool m_bDebugConsole;				//	Showing debug console
		char m_chKeyDown;					//	Processed a WM_KEYDOWN (skip WM_CHAR)
		bool m_bNextWeaponKey;				//	Next weapon key is down
		bool m_bNextMissileKey;				//	Next missile key is down
		CUniverse m_Universe;
		CPlayerShipController *m_pPlayer;
		int m_iTick;
		CDockScreen m_CurrentDock;
		CIDTable m_DockScreens;
		AGScreen *m_pCurrentScreen;
		MenuTypes m_CurrentMenu;
		CMenuData m_MenuData;
		PickerTypes m_CurrentPicker;
		CGameFile m_GameFile;

		CSystem *m_pSystem;					//	Source system when gating
		CTopologyNode *m_pDestNode;			//	Destination when gating
		CString m_sDestEntryPoint;			//	Destination entry point when gating
		int m_iCountdown;					//	Miscellaneous timer
		CSpaceObject *m_pMenuObj;			//	Object during menu selection
		bool m_bRedirectDisplayMessage;		//	Redirect display msg to dock screen
		CString m_sRedirectMessage;			//	Redirected message

		//	Background thread
		SDL_sem *m_hWorkAvailableEvent;
		BackgroundState m_iBackgroundState;
		SDL_Thread *m_hBackgroundThread;

		//	Loading screen
		CG16bitImage m_TitleImage;
		CG16bitImage m_StargateImage;
		bool m_bTitleInvalid;
		CString m_sBackgroundError;

		//	Intro screen
		CHighScoreList *m_pHighScoreList;
		CHighScoreDisplay m_HighScoreDisplay;
		int m_iLastHighScore;
		CPlayerDisplay m_PlayerDisplay;
		DWORD m_dwIntroShipClass;
		int m_iLastShipCreated;
		CSystem *m_pIntroSystem;
		bool m_bSavedGame;
		RECT m_rcIntroTop;
		RECT m_rcIntroMain;
		RECT m_rcIntroBottom;
		CButtonBarData m_ButtonBar;
		CButtonBarDisplay m_ButtonBarDisplay;
		bool m_bOverwriteGameDlg;
		RECT m_rcOverwriteGameDlg;
		RECT m_rcOverwriteGameOK;
		RECT m_rcOverwriteGameCancel;

		//	Select adventure screen
		CAdventureDescDisplay m_AdventureDescDisplay;

		//	Select ship screen
		CShipClassDisplay m_ShipClassDisplay;

		//	Crawl screen
		bool m_bCrawlInvalid;
		CString m_sCrawlText;
		DWORD m_dwCrawlImage;
		CG16bitImage *m_pCrawlImage;
		CTextCrawlDisplay m_CrawlText;
		bool m_bContinue;

		//	Help screen
		bool m_bHelpInvalid;
		CG16bitImage m_HelpImage;
		GameState m_OldState;

		//	Performance options
		bool m_bTransparencyEffects;

		SFontTable m_Fonts;
		CSoundMgr m_SoundMgr;

		//	Main game structures
public:
		CG16bitImage &m_Screen;				//	Entire screen
private:
		RECT m_rcScreen;					//	Rect of entire screen
		RECT m_rcMainScreen;				//	Rect of main (1024x768) area
		RECT m_rcWindow;					//	Rect of main window in screen coordinates
		RECT m_rcWindowScreen;				//	Rect of screen within window
		bool m_bMinimized;

		CG16bitImage m_LRS;					//	Long-range scan
		RECT m_rcLRS;						//	Rect on screen where LRS goes
		CG16bitImage *m_pLargeHUD;			//	Background LRS image
		CG16bitImage *m_pSRSSnow;			//	SRS snow image
		CG16bitImage *m_pLRSBorder;			//	LRS border

		CArmorDisplay m_ArmorDisplay;		//	Armor display object
		CDeviceCounterDisplay m_DeviceDisplay;	//	Device counter display
		CMessageDisplay m_MessageDisplay;	//	Message display object
		CReactorDisplay m_ReactorDisplay;	//	Reactor status display object
		CTargetDisplay m_TargetDisplay;		//	Targeting computer display
		CMenuDisplay m_MenuDisplay;			//	Menu display
		CPickerDisplay m_PickerDisplay;		//	Picker display
		CCommandLineDisplay m_DebugConsole;	//	CodeChain debugging console
		int m_iDamageFlash;					//	0 = no flash; odd = recover; even = flash;
		Metric m_rMapScale[MAP_SCALE_COUNT];//	Map scale
		int m_iMapScale;					//	Map scale index
		int m_iMapZoomEffect;				//	0 = no zoom effect

		//	DirectX debugging
		int m_iFrameCount;					//	Number of frames so far
		int m_iFrameTime[FRAME_RATE_COUNT];	//	Last n frame times (in milliseconds)
		int m_iPaintTime[FRAME_RATE_COUNT];	//	Last n paint times (in milliseconds)
		int m_iUpdateTime[FRAME_RATE_COUNT];//	Last n update times (in milliseconds)
		int m_iBltTime[FRAME_RATE_COUNT];	//	Last n BltToScreen times (in milliseconds)
		int m_iTicks[FRAME_RATE_COUNT][10];
		int m_iStartAnimation;

		//	Debug info
#ifdef DEBUG
		CString m_DebugLines[DEBUG_LINES_COUNT];			//	Debug lines
		int m_iDebugLinesStart;
		int m_iDebugLinesEnd;
#endif
#ifdef DEBUGLOG
		CTextFileLog m_Log;
#endif
		CTextFileLog m_GameLog;

	friend ALERROR MainWndProc(CTranscendenceWnd *wnd, void *evt);
	friend ALERROR CreateMainWindow();
	friend void DestroyMainWindow(void);
	friend class CIntroShipController;
	};

//	Inlines

inline void CTranscendenceWnd::ClearMessage (void)
	{
	m_MessageDisplay.ClearAll();
	}

//	CodeChain context

struct CodeChainCtx
	{
	CUniverse *pUniv;
	CTranscendenceWnd *pTrans;
	CDockScreen *pScreen;
	};

#endif

