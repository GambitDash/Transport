//	TSE.h
//
//	Transcendence Space Engine

#ifndef INCL_TSE
#define INCL_TSE

#if 0
#ifndef INCL_JPEGUTIL
#include "JPEGUtil.h"
#endif
#endif

#ifndef INCL_XMLUTIL
#include "XMLUtil.h"
#endif

#if 0
#ifndef INCL_DIRECTXUTIL
#include "DirectXUtil.h"
#endif
#endif 

#ifndef _INC_MATH
#include <math.h>
#endif

#ifndef INCL_TSE_GEOMETRY
#include "Geometry.h"
#endif

//	Define some debugging symbols

#ifdef DEBUG
//#define DEBUG_SOURCE_LOAD_TRACE
//#define DEBUG_SHIP
//#define DEBUG_HENCHMAN
//#define DEBUG_LOAD
//#define DEBUG_COMBAT
//#define DEBUG_ALL_ITEMS
#define DEBUG_VECTOR
//#define DEBUG_PERFORMANCE
//#define DEBUG_NEBULA_PAINTING
#endif

//	We leave this defined because we want to get traces in the field in case
//	of a crash.
#define DEBUG_PROGRAMSTATE

//	Not sure about this feature yet...
//#define BACKGROUND_OBJECTS

//	Forward references

class CDataFile;

class CG16bitRegion;
#include "CG16bitFont.h"
#include "CRect.h"

class CG16bitFont;
class CSoundMgr;

class CArmorClass;
class CDeviceClass;
class CEffectCreator;
class CInstalledDevice;
class CItem;
class CItemListManipulator;
class CItemTable;
class CItemType;
class CPower;
class CRandomEntryResults;
class CResourceDb;
class CShip;
class CShipClass;
class CEnergyFieldType;
class CSovereign;
class CSpaceEnvironmentType;
class CSpaceObject;
class CStation;
class CStationType;
class CSystem;
class CVector;
class CTimedEvent;
class CTopologyNode;
class CUniverse;
class IEffectPainter;
class IShipController;
class IShipGenerator;
class CWeaponFireDesc;
class CTradingDesc;
class CWeaponClass;
class CDockScreenType;
class CMissile;

extern CUniverse *g_pUniverse;

#include "TQueue.h"
#include "CTimeDate.h"
#include "CCCodeChain.h"

//	Include other TSE files

#ifndef INCL_TSE_TYPES
#include "TSETypes.h"
#endif

#ifndef INCL_TSE_UTIL
#include "TSEUtil.h"
#endif

#ifndef INCL_TSE_DESIGN
#include "TSEDesign.h"
#endif

class CResourceDb
	{
	public:
		CResourceDb (const CString &sDefFilespec, CResourceDb *pMainDb = NULL, bool bExtension = false);
		~CResourceDb (void);

		CString GetRootTag (void);
		bool IsUsingExternalGameFile (void) const { return !m_bGameFileInDb; }
		bool IsUsingExternalResources (void) const { return !m_bResourcesInDb; }
		ALERROR LoadEntities (CString *retsError);
		ALERROR LoadGameFile (CXMLElement **retpData, CString *retsError);
//		ALERROR LoadImage (const CString &sFolder, const CString &sFilename, HBITMAP *rethImage);
		ALERROR LoadModule (const CString &sFolder, const CString &sFilename, CXMLElement **retpData, CString *retsError);
		ALERROR LoadSound (CSoundMgr &SoundMgr, const CString &sFolder, const CString &sFilename, int *retiChannel);
		ALERROR Open (DWORD dwFlags = 0);

		CString GetResourceFilespec (int iIndex);
		int GetResourceCount (void);
		inline int GetVersion (void) { return m_iVersion; }
		ALERROR ExtractMain (CString *retsData);
		ALERROR ExtractResource (const CString sFilespec, CString *retsData);

	private:
		int m_iVersion;
		bool m_bGameFileInDb;
		bool m_bResourcesInDb;

		//	If we're just using the file system
		CString m_sRoot;
		CString m_sGameFile;

		//	If we're using a TDB
		CDataFile *m_pDb;
		CSymbolTable *m_pResourceMap;
		int m_iGameFile;

		CExternalEntityTable m_Entities;			//	Main file entities (only if loading main file)
		CResourceDb *m_pMainDb;						//	Main file db (only if loading extension)
	};

//	CTileMap (for nebulosity)

class CTileMapSection;

class CTile
	{
	public:
		CTile (void) : m_dwData(0) { }

		inline DWORD GetTile (void) { return m_dwData; }
		inline CTileMapSection *GetTileMapSection (void) { return (CTileMapSection *)m_dwData; }
		inline void SetTile (DWORD dwTile) { m_dwData = dwTile; }
		inline void SetTileMapSection (CTileMapSection *pMap) { m_dwData = (DWORD)pMap; }

	private:
		DWORD m_dwData;
	};

class CTileMapSection
	{
	public:
		CTileMapSection (int iCount) { m_pMap = new CTile [iCount]; }
		~CTileMapSection (void) { delete [] m_pMap; }

		inline DWORD GetTile (int iIndex) { return m_pMap[iIndex].GetTile(); }
		inline CTileMapSection *GetTileMapSection (int iIndex) { return m_pMap[iIndex].GetTileMapSection(); }
		void ReadFromStream (int iCount, IReadStream *pStream);
		inline void SetTile (int iIndex, DWORD dwTile) { m_pMap[iIndex].SetTile(dwTile); }
		inline void SetTileMapSection (int iIndex, CTileMapSection *pMap) { m_pMap[iIndex].SetTileMapSection(pMap); }
		void WriteToStream (int iCount, IWriteStream *pStream) const;

	private:
		CTile *m_pMap;
	};

struct STileMapSectionPos
	{
	CTileMapSection *pMap;
	int iIndex;
	STileMapSectionPos *pParent;
	};

struct STileMapEnumerator
	{
	STileMapEnumerator (void) : pCurPos(NULL), bDone(false) { }

	STileMapSectionPos *pCurPos;
	bool bDone;
	};

class CTileMap
	{
	public:
		CTileMap (int iSize, int iScale);
		~CTileMap (void);

		static ALERROR CreateFromStream (IReadStream *pStream, CTileMap **retpMap);
		void GetNext (STileMapEnumerator &i, int *retx, int *rety, DWORD *retdwTile) const;
		DWORD GetTile (int x, int y) const;
		inline int GetScale (void) const { return m_iScale; }
		int GetTotalSize (void) const;
		inline int GetSize (void) const { return m_iSize; }
		bool HasMore (STileMapEnumerator &i) const;
		void SetTile (int x, int y, DWORD dwTile);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		void ComputeDenominator (void);
		void CleanUpMapSection (CTileMapSection *pMap, int iScale);
		bool SelectNext (STileMapEnumerator &i) const;
		void WriteMapSection (CTileMapSection *pMap, int iOffset, int iDenom, IWriteStream *pStream) const;

		int m_iSize;
		int m_iScale;
		int m_iFirstDenominator;
		CTileMapSection *m_pMap;
	};

//	Paint Utilities

void DrawItemTypeIcon (CG16bitImage &Dest, int x, int y, CItemType *pType);
void DrawLightning (CG16bitImage &Dest,
					int xFrom, int yFrom,
					int xTo, int yTo,
					COLORREF wColor,
					int iPoints,
					Metric rChaos);
void DrawParticle (CG16bitImage &Dest,
				   int x, int y,
				   COLORREF wColor,
				   int iSize,
				   DWORD byOpacity);

//	Helper functions

CString ComposeDamageAdjReference (int *AdjRow, int *StdRow);
CString ComposeNounPhrase (const CString &sNoun, 
						   int iCount, 
						   const CString &sModifier, 
						   DWORD dwNounFlags, 
						   DWORD dwComposeFlags);
void ComputePercentages (int iCount, int *pTable);

#define VISUAL_RANGE_INDEX		6
#define RANGE_INDEX_COUNT		9
Metric RangeIndex2Range (int iIndex);

CString ReactorPower2String (int iPower);

int NLCompare (CStringArray &Input, CStringArray &Pattern);
void NLSplit (const CString &sPhrase, CStringArray *retArray);

void ReportCrashObj (CString *retsMessage, CSpaceObject *pCrashObj = NULL);

enum ProgramTypes
	{
	progNOP,
	progShieldsDown,
	progReboot,
	progDisarm,

	progCustom,
	};

struct ProgramDesc
	{
	ProgramTypes iProgram;
	CString sProgramName;
	int iAILevel;

	//	Used for custom programs
	CEvalContext *pCtx;
	ICCItem *ProgramCode;
	};

//	Supporting structures

class CWeaponFireDesc
	{
	public:
		struct SFragmentDesc
			{
			CWeaponFireDesc *pDesc;			//	Data for fragments
			DiceRange Count;				//	Number of fragments
			bool bMIRV;						//	Fragments seek independent targets

			SFragmentDesc *pNext;
			};

		CWeaponFireDesc (void);
		CWeaponFireDesc (const CWeaponFireDesc &Desc);
		~CWeaponFireDesc (void);

		inline int GetAveParticleCount (void) const { return m_ParticleCount.GetAveValue(); }
		inline bool CanAutoTarget (void) const { return m_bAutoTarget; }
		CEffectCreator *FindEffectCreator (const CString &sUNID);
		CWeaponFireDesc *FindWeaponFireDesc (const CString &sUNID, char **retpPos = NULL);
		static CWeaponFireDesc *FindWeaponFireDesc (DWORD dwUNID, int iOrdinal = 0);
		static CWeaponFireDesc *FindWeaponFireDescFromFullUNID (const CString &sUNID);
		inline CItemType *GetAmmoType (void) const { return m_pAmmoType; }
		inline int GetInitialDelay (void) const { return m_InitialDelay.Roll(); }
		Metric GetInitialSpeed (void) const;
		inline int GetInteraction (void) const { return m_iInteraction; }
		inline SFragmentDesc *GetFirstFragment (void) const { return m_pFirstFragment; }
		inline int GetHitPoints (void) const { return m_iHitPoints; }
		inline int GetLifetime (void) const { return m_Lifetime.Roll(); }
		inline int GetMaxLifetime (void) const { return m_Lifetime.GetMaxValue(); }
		inline int GetMaxParticleCount (void) const { return m_ParticleCount.GetMaxValue(); }
		inline Metric GetMaxRadius (void) const { return m_rMaxRadius; }
		inline Metric GetMinRadius (void) const { return m_rMinRadius; }
		inline int GetParticleCount (void) const { return m_ParticleCount.Roll(); }
		inline int GetParticleEmitTime (void) const { return m_ParticleEmitTime.Roll(); }
		inline int GetParticleSpreadAngle (void) const { return m_iParticleSpread; }
		inline int GetParticleSpreadWidth (void) const { return m_iParticleSpreadWidth; }
		inline Metric GetRatedSpeed (void) const { return m_rMissileSpeed; }
		inline bool HasFragments (void) const { return m_pFirstFragment != NULL; }

		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		void LoadImages (void);
		void MarkImages (void);
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		ALERROR OverrideDesc (CXMLElement *pDesc);

	public:
		CString m_sUNID;					//	Identification. The format is
											//		uuuuu/n/fi/e
											//
											//		uuuuu = weapon UNID
											//		n = shot data ordinal
											//		fi = fragment, i is index (optional)
											//		e = enhanced (optional)

		CItemTypeRef m_pAmmoType;			//	item type for this ammo

		FireTypes m_iFireType;				//	beam or missile
		DamageDesc m_Damage;				//	Damage per shot
		Metric m_rMaxEffectiveRange;		//	Max effective range of weapon
		int m_iPassthrough;					//	Chance that the missile will continue through target
		int m_iContinuous;					//	repeat for this number of frames
		int m_iFireSound;					//	Sound when weapon is fired (-1 == no sound)

		//	Effects
		CEffectCreatorRef m_pEffect;		//	Effect for the actual bullet/missile/beam
		CEffectCreatorRef m_pHitEffect;		//	Effect when we hit/explode
		CEffectCreatorRef m_pFireEffect;	//	Effect when we fire (muzzle flash)
		int m_iVaporTrailWidth;				//	Width of vapor trail (0 = none)
		int m_iVaporTrailLength;			//	Number of segments
		int m_iVaporTrailWidthInc;			//	Width increment in 100ths of a pixel
		COLORREF m_wVaporTrailColor;			//	Color of vapor trail
		WORD m_wSpare;

		//	Beam stuff (m_iFireType == ftBeam)
		BeamTypes m_iBeamType;				//	If a beam...
		COLORREF m_wPrimaryColor;				//	Primary beam color
		COLORREF m_wSecondaryColor;				//	Secondary (usually halo) color
		int m_iIntensity;					//	Variation depends on beam type

		//	Missile stuff (m_iFireType == ftMissile)
		CObjectImageArray m_Image;			//	Image for missile
		int m_iManeuverability;				//	Tracking maneuverability (0 = none)
		int m_iAccelerationFactor;			//	% increase in speed per 10 ticks
		Metric m_rMaxMissileSpeed;			//	Max speed.
		bool m_bDirectional;				//	True if different images for each direction
		bool m_bProximityBlast;				//	Fragments when close to target
		bool m_bFragment;					//	True if this is a fragment of a proximity blast
		int m_iProximityFailsafe;			//	Min ticks before proximity is active
		bool m_bCanDamageSource;			//	TRUE if we can damage the source
		int m_iStealth;						//	Missile stealth

		//	Missile exhaust
		int m_iExhaustRate;					//	Ticks per exhaust creation (0 if no exhaust)
		CObjectImageArray m_ExhaustImage;	//	Image for exhaust
		int m_iExhaustLifetime;				//	Ticks that each exhaust particle lasts
		Metric m_rExhaustDrag;				//	Coefficient of drag for exhaust particles

		//	Area stuff (m_iFireType == ftArea)
		Metric m_rExpansionSpeed;			//	Speed of expansion

		//	Miscellaneous
		CWeaponFireDesc *m_pEnhanced;		//	Data when weapon is enhanced

		//	Flags
		DWORD m_fVariableInitialSpeed:1;	//	TRUE if initial speed is random
		DWORD m_dwSpare:31;

	private:
		Metric m_rMissileSpeed;				//	Speed of missile
		DiceRange m_MissileSpeed;			//	Speed of missile (if random)
		DiceRange m_Lifetime;				//	Lifetime of fire in seconds
		DiceRange m_InitialDelay;			//	Delay for n ticks before starting

		SFragmentDesc *m_pFirstFragment;	//	Pointer to first fragment desc (or NULL)

		//	Basic properties
		bool m_bAutoTarget;					//	TRUE if we can acquire new targets after launch

		//	Missile stuff (m_iFireType == ftMissile)
		int m_iHitPoints;					//	HP before disipating (0 = destroyed by any hit)
		int m_iInteraction;					//	Interaction opacity (0-100)

		//	Particles (m_iFireType == ftParticles)
		DiceRange m_ParticleCount;			//	Number of new particles per tick
		DiceRange m_ParticleEmitTime;		//	Emit new particles for this number of ticks (default = 1)
		int m_iParticleSpreadWidth;			//	Width of spread
		int m_iParticleSpread;				//	Angle of spread

		//	Radius stuff (m_iFireType == ftRadius)
		Metric m_rMinRadius;				//	All objects inside this radius take full damage
		Metric m_rMaxRadius;				//	Damage inside this radius is decreased by inverse square law
	};

class CDamageSource
	{
	public:
		CDamageSource (void) : m_pSource(NULL), m_pSecondarySource(NULL) { }
		CDamageSource (CSpaceObject *pSource, CSpaceObject *pSecondarySource = NULL) : m_pSource(pSource), m_pSecondarySource(pSecondarySource) { }

		CString GetDamageCauseNounPhrase (DWORD dwFlags);
		inline CSpaceObject *GetObj (void) const { return m_pSource; }
		inline CSpaceObject *GetSecondaryObj (void) const { return m_pSecondarySource; }
		inline bool IsEmpty (void) { return (m_pSource == NULL); }
		void OnObjDestroyed (CSpaceObject *pObjDestroyed);
		void ReadFromStream (SLoadCtx &Ctx);
		void SetObj (CSpaceObject *pSource) { m_pSource = pSource; }
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);
		
	private:
		CSpaceObject *m_pSource;
		CString m_sSourceName;
		DWORD m_dwSourceFlags;

		CSpaceObject *m_pSecondarySource;
	};

//	Trading

class CTradingDesc
	{
	public:
		CTradingDesc (void);
		~CTradingDesc (void);

		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CTradingDesc **retpTrade);
		void OnCreate (CSpaceObject *pObj);
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		void OnUpdate (CSpaceObject *pObj);

		bool Buys (CSpaceObject *pObj, const CItem &Item, int *retiPrice = NULL, int *retiMaxCount = NULL);
		int Charge (CSpaceObject *pObj, int iCharge);
		bool Sells (CSpaceObject *pObj, const CItem &Item, int *retiPrice = NULL);

	private:
		enum Flags
			{
			FLAG_SELLS =			0x00000001,	//	TRUE if station sells this item type
			FLAG_BUYS =				0x00000002,	//	TRUE if station buys this item type
			};

		struct SCommodityDesc
			{
			CItemTypeRef pItemType;				//	Item type
			CItemCriteria ItemCriteria;		//	If ItemType is NULL, this is the criteria

			int iPriceAdj;						//	Price adjustment

			DWORD dwFlags;						//	Flags
			};

		int ComputeMaxCurrency (CSpaceObject *pObj);
		int ComputePrice (const CItem &Item, const SCommodityDesc &Commodity);
		bool Matches (const CItem &Item, const SCommodityDesc &Commodity);

		CString m_sCurrency;
		int m_iCurrencyConversion;
		int m_iMaxCurrency;
		int m_iReplenishCurrency;

		int m_iCount;
		SCommodityDesc *m_pList;
	};

//	Image Cache

class CImageLibrary
	{
	public:
		CImageLibrary (void);
		~CImageLibrary (void);

		ALERROR AddImage (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		void ClearMarks (void);
		CObjectImage *FindImage (DWORD dwUNID);
		CG16bitImage *GetImage (DWORD dwUNID);
		void Sweep (void);

	private:
		CIDTable m_Library;
	};

//	Space environment

//	CNavigationPath

class CNavigationPath : public TListNode<CNavigationPath>
	{
	public:
		CNavigationPath (void);
		~CNavigationPath (void);

		static void Create (CSystem *pSystem, CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd, CNavigationPath **retpPath);
		static void CreateFromStream (SLoadCtx &Ctx, CNavigationPath **retpNavPath);

		Metric ComputePathLength (CSystem *pSystem) const;
		CVector ComputePointOnPath (CSystem *pSystem, Metric rDist) const;
		static CString DebugDescribe (CSpaceObject *pObj, CNavigationPath *pNavPath);
		inline DWORD GetID (void) const { return m_dwID; }
		inline int GetNavPointCount (void) const { return m_iWaypointCount; }
		CVector GetNavPoint (int iIndex) const;
		bool Matches (CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd);
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream) const;

	private:
		static int ComputePath (CSystem *pSystem, CSovereign *pSovereign, const CVector &vFrom, const CVector &vTo, int iDepth, CVector **retpPoints);
		static bool PathIsClear (CSystem *pSystem, CSovereign *pSovereign, const CVector &vFrom, const CVector &vTo, CSpaceObject **retpEnemy, CVector *retvAway);

		DWORD m_dwID;							//	ID of path
		CSovereign *m_pSovereign;				//	Sovereign that will use (NULL if all)
		DWORD m_iStartIndex;					//	Index of start object
		DWORD m_iEndIndex;						//	Index of end object

		int m_iSuccesses;						//	Count of ships that successfully reached end
		int m_iFailures;						//	Count of ships that were destroyed

		CVector m_vStart;						//	Start position
		int m_iWaypointCount;					//	Number of waypoints (excludes start)
		CVector *m_Waypoints;					//	Array of waypoints
	};

//	System

const int STARFIELD_COUNT =						1000;

enum SpaceEnvironmentAreaTypes
	{
	seaNone	=					0,
	seaNebula =					1,
	seaDarkNebula =				2,
	seaRadiationField =			3,
	seaIonStorm =				4,
	seaPlasmaStorm =			5,

	seaTileSize =				512,	//	tile size in pixels
	seaArraySize =				16,		//	each level is 16x16 tiles
	seaScale =					1,		//	2 levels (levels = scale + 1)
	};

class CSystemCreateStats
	{
	public:
		CSystemCreateStats (void);
		~CSystemCreateStats (void);

		void AddLabel (const CString &sAttributes);
		inline int GetLabelAttributesCount (void) { return m_LabelAttributeCounts.GetCount(); }
		void GetLabelAttributes (int iIndex, CString *retsAttribs, int *retiCount);
		inline int GetTotalLabelCount (void) { return m_iLabelCount; }

	private:
		struct SLabelAttributeEntry
			{
			CString sAttributes;
			int iCount;
			};

		void AddLabelExpansion (const CString &sAttributes, const CString &sPrefix = NULL_STR);

		int m_iLabelCount;
		CSymbolTable m_LabelAttributeCounts;
	};

struct SSystemCreateCtx
	{
	SSystemCreateCtx (void) : Labels(TRUE), bLabelsChecked(false), pStats(NULL), pStation(NULL) { }

	CTopologyNode *pTopologyNode;			//	Topology node
	CSystem *pSystem;						//	System that we're creating
	CXMLElement *pLocalTables;				//	Lookup tables
	CXMLElement *pGlobalTables;				//	Lookup tables
	CObjectArray Labels;					//	CObjectArray of CLabelDesc
	bool bLabelsChecked;					//	TRUE if labels checked for overlap
	CString sLocationAttribs;				//	Current location attribs

	CString sError;							//	Creation error
	CSystemCreateStats *pStats;				//	System creation stats (may be NULL)

	CSpaceObject *pStation;					//	Root station when creating satellites
	};

class CSystem : public CObject
	{
	public:
		//	Orbit definitions

		class Orbit
			{
			public:
				Orbit (void);
				Orbit (const CVector &vCenter, Metric rRadius, Metric rPos = 0.0);
				Orbit (const CVector &vCenter, Metric rSemiMajorAxis, Metric rEccentricity, Metric rRotation, Metric rPos = 0.0);

				inline const Metric &GetEccentricity (void) const { return m_rEccentricity; }
				inline const CVector &GetFocus (void) const { return m_vFocus; }
				inline const Metric &GetObjectAngle (void) const { return m_rPos; }
				inline CVector GetObjectPos (void) const { return GetPoint(m_rPos); }
				CVector GetPoint (Metric rAngle) const;
				CVector GetPointAndRadius (Metric rAngle, Metric *retrRadius) const;
				CVector GetPointCircular (Metric rAngle) const;
				inline const Metric &GetRotation (void) const { return m_rRotation; }
				inline const Metric &GetSemiMajorAxis (void) const { return m_rSemiMajorAxis; }
				void Paint (CG16bitImage &Dest, const ViewportTransform &Trans, COLORREF rgbColor);

			private:
				CVector m_vFocus;				//	Focus of orbit
				Metric m_rEccentricity;			//	Ellipse eccentricity
				Metric m_rSemiMajorAxis;		//	Semi-major axis
				Metric m_rRotation;				//	Angle of rotation (radians)

				Metric m_rPos;					//	Obj position in orbit (radians)
			};

		//	Other defines

		enum LayerEnum
			{
			layerFirst = 0,

			layerBackground = 0,
			layerSpace = 1,
			layerStations = 2,
			layerShips = 3,
			layerEffects = 4,

			layerCount = 5
			};

		//	System methods

		static ALERROR CreateEmpty (CUniverse *pUniv, CTopologyNode *pTopology, CSystem **retpSystem);
		static ALERROR CreateFromStream (CUniverse *pUniv, 
										 IReadStream *pStream, 
										 CSystem **retpSystem,
										 DWORD dwObjID = 0xffffffff,
										 CSpaceObject **retpObj = NULL);
		static ALERROR CreateFromXML (CUniverse *pUniv, 
									  CSystemType *pType, 
									  CTopologyNode *pTopology, 
									  CXMLElement *pGlobalTables,
									  CSystem **retpSystem,
									  CSystemCreateStats *pStats = NULL);
		virtual ~CSystem (void);

		ALERROR CreateMarker (CXMLElement *pDesc, const CSystem::Orbit &oOrbit);
		ALERROR CreateParticles (CXMLElement *pDesc, const CSystem::Orbit &oOrbit);
		ALERROR CreateShip (DWORD dwClassID,
				IShipController *pController,
				CSovereign *pSovereign,
				const CVector &vPos,
				const CVector &vVel,
				int iRotation,
				CSpaceObject *pExitGate,
				CShip **retpShip);
		ALERROR CreateStargate (CStationType *pType,
								CVector &vPos,
								const CString &sStargateID,
								const CString &sDestNodeID,
								const CString &sDestStargateID,
								CSpaceObject **retpStation = NULL);
		ALERROR CreateStation (CStationType *pType,
							   CVector &vPos,
							   CSpaceObject **retpStation = NULL);
		ALERROR CreateStation (SSystemCreateCtx *pCtx, 
							   CStationType *pType, 
							   const CVector &vPos,
							   const CSystem::Orbit &OrbitDesc,
							   bool bCreateSatellites,
							   CXMLElement *pExtraData,
							   CSpaceObject **retpStation = NULL);
		ALERROR CreateRandomEncounter (IShipGenerator *pTable, 
									   CSpaceObject *pBase,
									   CSovereign *pBaseSovereign,
									   CSpaceObject *pTarget,
									   CSpaceObject *pGate = NULL);
		ALERROR CreateWeaponFire (CWeaponFireDesc *pDesc,
								  int iBonus,
								  DestructionTypes iCause,
								  const CDamageSource &Source,
								  const CVector &vPos,
								  const CVector &vVel,
								  int iDirection,
								  CSpaceObject *pTarget,
								  CSpaceObject **retpShot);
		ALERROR CreateWeaponFragments (CWeaponFireDesc *pDesc,
									   int iBonus,
									   DestructionTypes iCause,
									   const CDamageSource &Source,
									   CSpaceObject *pTarget,
									   const CVector &vPos,
									   const CVector &vVel,
									   CSpaceObject *pMissileSource);

		ALERROR AddTimedEvent (CTimedEvent *pEvent);
		ALERROR AddToSystem (CSpaceObject *pObj, int *retiIndex);
		int CalculateLightIntensity (const CVector &vPos, CSpaceObject **retpStar = NULL);
		COLORREF CalculateSpaceColor (CSpaceObject *pPOV);
		void CancelTimedEvent (CSpaceObject *pSource, const CString &sEvent);
		void DestroyObject (int iIndex, DestructionTypes iCause, CSpaceObject *pCause);
		inline DWORD GetID (void) { return m_dwID; }
		int GetLevel (void);
		CSpaceObject *GetNamedObject (const CString &sName);
		inline const CString &GetName (void) { return m_sName; }
		CNavigationPath *GetNavPath (CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd);
		CNavigationPath *GetNavPathByID (DWORD dwID);
		CSpaceObject *GetObject (int iIndex) { return (CSpaceObject *)m_AllObjects.GetObject(iIndex); }
		int GetObjectCount (void) { return m_AllObjects.GetCount(); }
		inline Metric GetSpaceScale (void) const { return m_rKlicksPerPixel; }
		inline int GetTick (void) { return m_iTick; }
		inline Metric GetTimeScale (void) const { return m_rTimeScale; }
		inline CTopologyNode *GetTopology (void) { return m_pTopology; }
		CSpaceEnvironmentType *GetSpaceEnvironment (int xTile, int yTile);
		CSpaceEnvironmentType *GetSpaceEnvironment (const CVector &vPos, int *retxTile = NULL, int *retyTile = NULL);
		CTopologyNode *GetStargateDestination (const CString &sStargate, CString *retsEntryPoint);
		inline CUniverse *GetUniverse (void) const { return m_pUniv; }
		inline bool IsCreationInProgress (void) const { return (m_fInCreate ? true : false); }
		bool IsStationInSystem (CStationType *pType);
		inline bool IsTimeStopped (void) { return (m_iTimeStopped > 0); }
		void LoadImages (void);
		void MarkImages (void);
		void NameObject (const CString &sName, CSpaceObject *pObj);
		void PlayerEntered (CSpaceObject *pPlayer);
		ALERROR SaveToStream (IWriteStream *pStream);
		inline void SetID (DWORD dwID) { m_dwID = dwID; }
		void SetObjectSovereign (CSpaceObject *pObj, CSovereign *pSovereign);
		void SetPOVLRS (CSpaceObject *pCenter);
		void SetSpaceEnvironment (int xTile, int yTile, CSpaceEnvironmentType *pEnvironment);
		ALERROR StargateCreated (CSpaceObject *pGate, const CString &sStargateID, const CString &sDestNodeID, const CString &sDestEntryPoint);
		void StopTimeForAll (int iDuration, CSpaceObject *pExcept);
		void ReadObjRefFromStream (SLoadCtx &Ctx, CSpaceObject **retpObj);
		void ReadSovereignRefFromStream (SLoadCtx &Ctx, CSovereign **retpSovereign);
		void RemoveObject (int iIndex, DestructionTypes iCause, CSpaceObject *pCause, CSpaceObject *pWreck);
		void RemoveTimersForObj (CSpaceObject *pObj);
		void RestartTime (void);

		void PaintViewport (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, bool bEnhanced);
		void PaintViewportGrid (CG16bitImage &Dest, const RECT &rcView, const ViewportTransform &Trans, const CVector &vCenter, Metric rGridSize);
		void PaintViewportObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, CSpaceObject *pObj);
		void PaintViewportLRS (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, bool *retbNewEnemies);
		void PaintViewportMap (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, Metric rMapScale);
		void PaintViewportMapObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, CSpaceObject *pObj);
		void Update (Metric rSecondsPerTick);
		void UpdateExtended (void);
		void WriteObjRefToStream (CSpaceObject *pObj, IWriteStream *pStream);
		void WriteSovereignRefToStream (CSovereign *pSovereign, IWriteStream *pStream);

	private:
		struct CStar
			{
			int x;
			int y;
			COLORREF wColor;
			WORD wDistance;
			};

		CSystem (void);
		CSystem (CUniverse *pUniv, CTopologyNode *pTopology);

		void ComputeMapLabels (void);
		void ComputeRandomEncounters (int iCount);
		void ComputeStars (void);
		static void ConvertSpaceEnvironmentToPointers (CTileMap &UNIDs, CTileMap **retpPointers);
		static void ConvertSpaceEnvironmentToUNIDs (CTileMap &Pointers, CTileMap **retpUNIDs);
		ALERROR CreateStarField (int iCount);
		ALERROR CreateStation (CStationType *pType,
							   const CVector &vPos,
							   const CVector &vVel,
							   CXMLElement *pExtraData,
							   CSpaceObject **retpStation,
							   CString *retsError = NULL);
		inline int GetTimedEventCount (void) { return m_TimedEvents.GetCount(); }
		inline CTimedEvent *GetTimedEvent (int iIndex) { return (CTimedEvent *)m_TimedEvents.GetObject(iIndex); }
		void FlushEnemyObjectCache (void);
		void PaintDestinationMarker (CG16bitImage &Dest, int x, int y, CSpaceObject *pObj, CSpaceObject *pCenter);
		void PaintStarField(CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, Metric rKlicksPerPixel);
		void UpdateRandomEncounters (void);

		//	Game instance data

		DWORD m_dwID;							//	System ID
		CSystemType *m_pType;					//	System type definition
		int m_iTick;							//	Ticks since beginning of time
		CString m_sName;						//	Name of system
		CTopologyNode *m_pTopology;				//	Topology descriptor
		CObjectArray m_AllObjects;				//	Array of CSpaceObject
		CSymbolTable m_NamedObjects;			//	Indexed array of named objects (CSpaceObject *)
		CObjectArray m_TimedEvents;				//	Array of CTimedEvent
		int m_iNextEncounter;					//	Time of next random encounter
		CTileMap *m_pEnvironment;				//	Nebulas, etc.
		int m_iTimeStopped;						//	Ticks until time restarts
		TListNode<CNavigationPath> m_NavPaths;	//	List of navigation paths
		Metric m_rKlicksPerPixel;				//	Space scale
		Metric m_rTimeScale;					//	Time scale

		DWORD m_fNoRandomEncounters:1;			//	TRUE if no random encounters in this system
		DWORD m_fInCreate:1;					//	TRUE if system in being created
		DWORD m_fSpare:30;

		//	Support structures

		CUniverse *m_pUniv;						//	Universe
		CStructArray m_StarField;				//	Star field
		int m_iEncounterObjCount;				//	Number of objects that generate encounters
		CSpaceObject **m_pEncounterObj;			//	List of objects that generate encounters
		CSpaceObjectList m_BarrierObjects;		//	List of barrier objects
		CSpaceObjectList m_Stars;				//	List of stars in the system

		static const Metric g_MetersPerKlick;

	friend class CObjectClass<CSystem>;
	};

//	Miscellaneous Structures & Classes

class CParticleArray
	{
	public:
		CParticleArray (void);
		~CParticleArray (void);

		void AddParticle (const CVector &vPos, const CVector &vVel, int iLifeLeft = -1, int iRotation = -1, DWORD dwData = 0);
		const RECT &GetBounds (void) const { return m_rcBounds; }
		inline int GetCount (void) const { return m_iCount; }
		int HitTest (CSpaceObject *pObj,
					 const CVector &vOldPos,
					 const CVector &vNewPos,
					 CVector *retvHitPos);
		void Init (int iMaxCount);
		void Paint (CG16bitImage &Dest,
					int xPos,
					int yPos,
					SViewportPaintCtx &Ctx,
					SParticlePaintDesc &Desc);
		void Paint (CG16bitImage &Dest,
					int xPos,
					int yPos,
					SViewportPaintCtx &Ctx,
					IEffectPainter *pPainter);
		void ReadFromStream (SLoadCtx &Ctx);
		void UpdateMotionLinear (bool *retbAlive = NULL);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		struct SParticle
			{
			int x;								//	Offset from center of particle cloud
			int y;								//		(screen-coords, in 256ths of pixels)
			int xVel;							//	Velocity relative to particle cloud
			int yVel;							//		(screen-coords, in 256ths of pixels per tick)

			int iLifeLeft;						//	Ticks of life left
			int iDestiny;						//	Random number from 1-360
			int iRotation;						//	Particle rotation
			DWORD dwData;						//	Miscellaneous data for particle

			DWORD fAlive:1;						//	TRUE if particle is alive
			DWORD dwSpare:31;					//	Spare
			};

		void CleanUp (void);
		void PaintFireAndSmoke (CG16bitImage &Dest, 
								int xPos, 
								int yPos, 
								SViewportPaintCtx &Ctx, 
								int iLifetime, 
								int iMinWidth,
								int iMaxWidth,
								int iCore,
								int iFlame,
								int iSmoke,
								int iSmokeBrightness);
		void PaintGaseous (CG16bitImage &Dest,
						   int xPos,
						   int yPos,
						   SViewportPaintCtx &Ctx,
						   int iMaxLifetime,
						   int iMinWidth,
						   int iMaxWidth,
						   COLORREF wPrimaryColor,
						   COLORREF wSecondaryColor);
		void PaintImage (CG16bitImage &Dest, int xPos, int yPos, SViewportPaintCtx &Ctx, SParticlePaintDesc &Desc);
		void PaintLine (CG16bitImage &Dest,
						int xPos,
						int yPos,
						SViewportPaintCtx &Ctx,
						COLORREF wPrimaryColor);
		void PosToXY (const CVector &xy, int *retx, int *rety);
		CVector XYToPos (int x, int y);

		int m_iCount;
		SParticle *m_pArray;
		RECT m_rcBounds;						//	Bounding box in pixels relative to center

		int m_iLastAdded;						//	Index of last particle added
	};

//	Topology Node

#define END_GAME_SYSTEM_UNID					0x00ffffff

class CTopologyNode
	{
	public:
		CTopologyNode (const CString &sID, DWORD SystemUNID);
		~CTopologyNode (void);
		static void CreateFromStream (SUniverseLoadCtx &Ctx, CTopologyNode **retpNode);

		void AddAttributes (const CString &sAttribs);
		void AddGateInt (const CString &sName, const CString &sDestNode, const CString &sEntryPoint);
		ALERROR AddStargate (const CString &sGateID, const CString &sDestNodeID, const CString &sDestGateID);
		bool FindStargate (const CString &sName, CString *retsDestNode = NULL, CString *retsEntryPoint = NULL);
		CString FindStargateName (const CString &sDestNode, const CString &sEntryPoint);
		inline const CString &GetData (const CString &sAttrib) const { return m_Data.GetData(sAttrib); }
		inline const CString &GetEndGameReason (void) { return m_sEndGameReason; }
		inline const CString &GetEpitaph (void) { return m_sEpitaph; }
		inline const CString &GetID (void) { return m_sID; }
		CTopologyNode *GetGateDest (const CString &sName, CString *retsEntryPoint = NULL);
		inline int GetLevel (void) { return m_iLevel; }
		inline int GetStargateCount (void) { return m_NamedGates.GetCount(); }
		CString GetStargate (int iIndex);
		inline CSystem *GetSystem (void) { return m_pSystem; }
		inline DWORD GetSystemDescUNID (void) { return m_SystemUNID; }
		inline DWORD GetSystemID (void) { return m_dwID; }
		inline const CString &GetSystemName (void) { return m_sName; }
		inline bool HasAttribute (const CString &sAttrib) { return ::HasModifier(m_sAttributes, sAttrib); }
		inline void SetData (const CString &sAttrib, const CString &sData) { m_Data.SetData(sAttrib, sData); }
		inline void SetEndGameReason (const CString &sReason) { m_sEndGameReason = sReason; }
		inline void SetEpitaph (const CString &sEpitaph) { m_sEpitaph = sEpitaph; }
		inline void SetLevel (int iLevel) { m_iLevel = iLevel; }
		inline void SetName (const CString &sName) { m_sName = sName; }
		void SetStargateDest (const CString &sName, const CString &sDestNode, const CString &sEntryPoint);
		inline void SetSystem (CSystem *pSystem) { m_pSystem = pSystem; }
		inline void SetSystemID (DWORD dwID) { m_dwID = dwID; }
		void WriteToStream (IWriteStream *pStream);

		inline void AddVariantLabel (const CString &sVariant) { m_VariantLabels.AppendString(sVariant, NULL); }
		bool HasVariantLabel (const CString &sVariant);

	private:
		struct StarGateDesc
			{
			CString sDestNode;
			CString sDestEntryPoint;
			};

		CString m_sID;							//	ID of node
		DWORD m_SystemUNID;						//	UNID of system type
		CString m_sName;						//	Name of system
		CSymbolTable m_NamedGates;				//	Name to StarGateDesc
		int m_iLevel;							//	Level of system
		CString m_sAttributes;					//	Attributes
		CStringArray m_VariantLabels;			//	Variant labels
		CString m_sEpitaph;						//	Epitaph if this is endgame node
		CString m_sEndGameReason;				//	End game reason if this is endgame node

		CAttributeDataBlock m_Data;				//	Opaque data

		CSystem *m_pSystem;						//	NULL if not yet created
		DWORD m_dwID;							//	ID of system instance
	};

//	Events

class CTimedEvent
	{
	public:
		enum Classes
			{
			cTimedEncounterEvent,
			cTimedCustomEvent,
			cTimedRecurringEvent,
			};

		CTimedEvent (int iTick) : m_iTick(iTick) { }
		virtual ~CTimedEvent (void) { }
		static void CreateFromStream (SLoadCtx &Ctx, CTimedEvent **retpEvent);

		inline int GetTick (void) { return m_iTick; }
		void WriteToStream (CSystem *pSystem, IWriteStream *pStream);

		virtual CString DebugCrashInfo (void) { return NULL_STR; }
		virtual void DoEvent (CSystem *pSystem, bool *retbDeleteEvent) = 0;
		virtual CString GetEventHandlerName (void) { return NULL_STR; }
		virtual CSpaceObject *GetEventHandlerObj (void) { return NULL; }
		virtual bool InDoEvent (void) { return false; }
		virtual bool OnObjDestroyed (CSpaceObject *pObj) { return false; }

	protected:
		CTimedEvent (void) { }

		inline void SetTick (int iTick) { m_iTick = iTick; }

		virtual void OnReadFromStream (SLoadCtx &Ctx) = 0;
		virtual void OnWriteClassToStream (IWriteStream *pStream) = 0;
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream) = 0;

	private:
		int m_iTick;
	};

class CTimedEncounterEvent : public CTimedEvent
	{
	public:
		CTimedEncounterEvent (void) { } //	Used only for loading
		CTimedEncounterEvent (int iTick,
							  CSpaceObject *pTarget,
							  DWORD dwEncounterTableUNID,
							  CSpaceObject *pGate,
							  Metric rDistance);

		virtual CString DebugCrashInfo (void);
		virtual void DoEvent (CSystem *pSystem, bool *retbDeleteEvent);
		virtual bool OnObjDestroyed (CSpaceObject *pObj);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteClassToStream (IWriteStream *pStream);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		CSpaceObject *m_pTarget;
		DWORD m_dwEncounterTableUNID;
		CSpaceObject *m_pGate;
		Metric m_rDistance;
	};

class CTimedCustomEvent : public CTimedEvent
	{
	public:
		CTimedCustomEvent (void) { }	//	Used only for loading
		CTimedCustomEvent (int iTick,
						   CSpaceObject *pObj,
						   const CString &sEvent);

		virtual CString DebugCrashInfo (void);
		virtual void DoEvent (CSystem *pSystem, bool *retbDeleteEvent);
		virtual CString GetEventHandlerName (void) { return m_sEvent; }
		virtual CSpaceObject *GetEventHandlerObj (void) { return m_pObj; }
		virtual bool InDoEvent (void) { return m_bInDoEvent; }
		virtual bool OnObjDestroyed (CSpaceObject *pObj);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteClassToStream (IWriteStream *pStream);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		CSpaceObject *m_pObj;
		CString m_sEvent;
		bool m_bInDoEvent;
	};

class CTimedRecurringEvent : public CTimedEvent
	{
	public:
		CTimedRecurringEvent (void) { }	//	Used only for loading
		CTimedRecurringEvent (int iInterval,
							  CSpaceObject *pObj,
							  const CString &sEvent);

		virtual CString DebugCrashInfo (void);
		virtual void DoEvent (CSystem *pSystem, bool *retbDeleteEvent);
		virtual CString GetEventHandlerName (void) { return m_sEvent; }
		virtual CSpaceObject *GetEventHandlerObj (void) { return m_pObj; }
		virtual bool InDoEvent (void) { return m_bInDoEvent; }
		virtual bool OnObjDestroyed (CSpaceObject *pObj);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteClassToStream (IWriteStream *pStream);
		virtual void OnWriteToStream (CSystem *pSystem, IWriteStream *pStream);

	private:
		int m_iInterval;
		CSpaceObject *m_pObj;
		CString m_sEvent;
		bool m_bDestroy;
		bool m_bInDoEvent;
	};

//	Docking ports implementation

class CDockingPorts
	{
	public:
		CDockingPorts (void);
		~CDockingPorts (void);

		void DockAtRandomPort (CSpaceObject *pOwner, CSpaceObject *pObj);
		int FindNearestEmptyPort (CSpaceObject *pOwner, CSpaceObject *pRequestingObj, CVector *retvDistance = NULL);
		int FindRandomEmptyPort (CSpaceObject *pOwner);
		inline int GetPortCount (CSpaceObject *pOwner) { return m_iPortCount; }
		inline CSpaceObject *GetPortObj (CSpaceObject *pOwner, int iPort) { return m_pPort[iPort].pObj; }
		CVector GetPortPos (CSpaceObject *pOwner, int iPort);
		int GetPortsInUseCount (CSpaceObject *pOwner);
		void InitPorts (CSpaceObject *pOwner, int iCount, Metric rRadius);
		void InitPorts (CSpaceObject *pOwner, int iCount, CVector *pPos);
		void InitPortsFromXML (CSpaceObject *pOwner, CXMLElement *pElement);
		inline bool IsObjDocked (CSpaceObject *pObj) { return IsDocked(pObj); }
		void MoveAll (CSpaceObject *pOwner);
		void OnDestroyed (void);
		void OnObjDestroyed (CSpaceObject *pOwner, CSpaceObject *pObj, bool *retbDestroyed = NULL);
		void OnOwnerChanged (CSpaceObject *pOwner);
		void ReadFromStream (CSpaceObject *pOwner, SLoadCtx &Ctx);
		void RepairAll (CSpaceObject *pOwner, int iRepairRate);
		bool RequestDock (CSpaceObject *pOwner, CSpaceObject *pObj);
		void Undock (CSpaceObject *pOwner, CSpaceObject *pObj);
		void UpdateAll (CSpaceObject *pOwner);
		void WriteToStream (CSpaceObject *pOwner, IWriteStream *pStream);

	private:
		enum DockingPortStatus
			{
			psEmpty,
			psDocking,
			psInUse
			};

		struct DockingPort
			{
			DockingPortStatus iStatus;			//	Status of port
			CSpaceObject *pObj;					//	Object docked at this port
			CVector vPos;						//	Position of dock (relative coords)
			int iRotation;						//	Rotation of ship at dock
			};

		bool IsDocked (CSpaceObject *pObj);
		bool IsDockedOrDocking (CSpaceObject *pObj);
		bool ShipsNearPort (CSpaceObject *pOwner, CSpaceObject *pRequestingObj, const CVector &vPortPos);

		int m_iPortCount;						//	Number of docking ports
		DockingPort *m_pPort;					//	Array of docking ports
	};

//	Attack Detector

class CAttackDetector
	{
	public:
		CAttackDetector (void);

		inline void Blacklist (void) { m_iCounter = -1; }
		inline bool IsBlacklisted (void) { return m_iCounter == -1; }
		bool Hit (int iTick);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void Update (int iTick) { if ((iTick % DECAY_RATE) == 0) OnUpdate(); }
		void WriteToStream (IWriteStream *pStream);

	private:
		enum Constants
			{
			DECAY_RATE = 150,
			HIT_LIMIT = 3,
			MULTI_HIT_WINDOW = 20,
			};

		void OnUpdate (void);

		int m_iCounter;
		int m_iLastHit;
	};

//	Item Event Handler

enum ItemEventTypes
	{
	eventNone =							0,
	eventOnAIUpdate =					1,
	eventOnUpdate =						2,
	};

class CItemEventDispatcher
	{
	public:
		CItemEventDispatcher (void);
		~CItemEventDispatcher (void);

		inline void FireEvent (CSpaceObject *pSource, ItemEventTypes iType)	{ if (m_pFirstEntry) FireEventFull(pSource, iType); }
		void Init (CSpaceObject *pSource);

	private:
		struct SEntry
			{
			ItemEventTypes iType;
			ICCItem *pCode;
			CItem theItem;

			SEntry *pNext;
			};

		SEntry *AddEntry (void);
		void FireEventFull (CSpaceObject *pSource, ItemEventTypes iType);
		void RemoveAll (void);

		SEntry *m_pFirstEntry;
	};

//	Ship classes and types

class CAutoDefenseClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
		virtual int GetDamageType (CInstalledDevice *pDevice);
		virtual int GetPowerRating (const CItem *pItem);
		virtual bool IsAutomatedWeapon (void) { return true; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

	private:
		enum TargetTypes
			{
			trgMissiles,
			};

		CAutoDefenseClass (void);

		inline CDeviceClass *GetWeapon (void) { return m_pWeapon; }

		TargetTypes m_iTarget;
		int m_iRechargeTicks;
		CDeviceClassRef m_pWeapon;
	};

class CCargoSpaceClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual bool CanBeDamaged (void) { return false; }
		virtual bool CanBeDisabled (void) { return false; }
		virtual ItemCategories GetCategory (void) { return itemcatCargoHold; }
		virtual int GetCargoSpace (void) { return m_iCargoSpace; }

	private:
		CCargoSpaceClass (void);

		int m_iCargoSpace;
	};

class CCyberDeckClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual void Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   int iFireAngle,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL);
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource) { return 30; }
		virtual ItemCategories GetCategory (void) { return itemcatWeapon; }
		virtual int GetDamageType (CInstalledDevice *pDevice) { return -1; }
		virtual Metric GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual CString GetReference (const CItem *pItem = NULL, CSpaceObject *pInstalled = NULL);
		virtual void GetSelectedVariantInfo (CSpaceObject *pSource, 
											 CInstalledDevice *pDevice,
											 CString *retsLabel,
											 int *retiAmmoLeft,
											 CItemType **retpType = NULL);
		virtual int GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice) { return 1; }
		virtual int GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual bool IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }
		virtual bool IsWeaponAligned (CSpaceObject *pShip, CInstalledDevice *pDevice, CSpaceObject *pTarget, int *retiAimAngle, int *retiFireAngle = NULL) { return true; }
		virtual bool SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }
		virtual bool SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }
		virtual bool ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }

	private:
		CCyberDeckClass (void);

		int m_iRange;							//	in light-seconds
		int m_iAttackChance;					//	%chance of success per attack

		ProgramDesc m_Program;					//	Program description
	};

class CDriveClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDrive);

		//	CDeviceClass virtuals

		virtual bool CanBeDisabled (void) { return true; }
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) { return itemcatDrive; }
		virtual const DriveDesc *GetDriveDesc (CInstalledDevice *pDevice = NULL, CSpaceObject *pSource = NULL);
		virtual int GetPowerRating (const CItem *pItem);

	private:
		CDriveClass (void);

		DriveDesc m_DriveDesc;
		DriveDesc m_DamagedDriveDesc;
		DriveDesc m_EnhancedDriveDesc;
	};

class CEnhancerClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
		virtual int GetPowerRating (const CItem *pItem) { return m_iPowerUse; }
		virtual int GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon, CString *retsBonusType);

	private:
		CEnhancerClass (void);

		CString m_sEnhancementType;			//	Type of enhancement
		int m_iDamageAdj[damageCount];		//	Adjustment to weapons damage
		int m_iPowerUse;
	};

class CMiscellaneousClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
		virtual int GetPowerRating (const CItem *pItem);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

	private:
		CMiscellaneousClass (void);

		int GetCapacitorPowerUse (CInstalledDevice *pDevice, CSpaceObject *pSource);

		int m_iPowerRating;					//	If >0, power rating for device
											//		This value is required if
											//		m_iPowerForCapacitor is negative
		int m_iPowerUse;					//	If >0, power used per update cycle
		int m_iPowerToActivate;				//	If >0, power used per activation
		int m_iPowerForCapacitor;			//	Required if m_iPowerToActivate is >0
											//		This is power used per cycle
											//		to charge for activation. If
											//		<0, this is a percent of max
											//		reactor power.
	};

class CReactorClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual bool CanBeDisabled (void) { return false; }
		virtual ItemCategories GetCategory (void) { return itemcatReactor; }
		virtual const ReactorDesc *GetReactorDesc (CInstalledDevice *pDevice = NULL, CSpaceObject *pSource = NULL);

	private:
		CReactorClass (void);

		ReactorDesc m_Desc;
		ReactorDesc m_DamagedDesc;
		ReactorDesc m_EnhancedDesc;
	};

class CRepairerClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
		virtual int GetPowerRating (const CItem *pItem) { return 2 * m_iPowerUse; }
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

	private:
		CRepairerClass (void);

		CIntArray m_RepairCycle;				//	10-Ticks to repair 1 hp (by armor tech)
		int m_iPowerUse;						//	Power used for each hp of repair
	};

class CShieldClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpShield);

		//	CDeviceClass virtuals

		virtual bool AbsorbsWeaponFire (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon);
		virtual bool AbsorbDamage (CInstalledDevice *pDevice, CShip *pShip, SDamageCtx &Ctx);
		virtual void Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   int iFireAngle,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL);
		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual void Deplete (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual ItemCategories GetCategory (void) { return itemcatShields; }
		virtual int GetPowerRating (const CItem *pItem);
		virtual CString GetReference (const CItem *pItem = NULL, CSpaceObject *pInstalled = NULL);
		virtual void GetStatus (CInstalledDevice *pDevice, CShip *pShip, int *retiStatus, int *retiMaxStatus);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList);
		virtual void Recharge (CInstalledDevice *pDevice, CShip *pShip, int iStatus);
		virtual bool RequiresItems (void);
		virtual void Reset (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

		static int GetStdPowerByLevel (int iLevel);

	protected:
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID) { return m_pHitEffect; }
		virtual void OnLoadImages (void);
		virtual void OnMarkImages (void);

	private:
		CShieldClass (void);

		bool IsDepleted (CInstalledDevice *pDevice);
		void FireOnShieldDown (CInstalledDevice *pDevice, CSpaceObject *pSource);
		int GetDamageAdj (CItemEnhancement Mods, const DamageDesc &Damage);
		int GetHPLeft (CInstalledDevice *pDevice, CSpaceObject *pSource);
		int GetMaxHP (CInstalledDevice *pDevice, CSpaceObject *pSource);
		bool UpdateDepleted (CInstalledDevice *pDevice);
		void SetDepleted (CInstalledDevice *pDevice, CSpaceObject *pSource);
		void SetHPLeft (CInstalledDevice *pDevice, int iHP);

		int m_iHitPoints;						//	Max HP
		int m_iArmorShield;						//	If non-zero then this is the
												//		This is the number of shield
												//		HPs that are generated by
												//		10 HP of average armor
		int m_iAbsorbAdj[damageCount];			//	Absorbtion adjustments
		int m_iDamageAdj[damageCount];			//	Adjustments for damage type
		int m_iRegenRate;						//	Ticks between regen
		int m_iRegenHP;							//	HP regenerated each cycle
		int m_iDepletionDelay;					//	Ticks to recover from full depletion
		int m_iPowerUse;						//	Power used during regeneration (1/10 megawatt)
		DamageTypeSet m_WeaponSuppress;			//	Types of weapons suppressed
		DamageTypeSet m_Reflective;				//	Types of damage reflected

		int m_iMaxCharges;						//	Max charges
		int m_iExtraHPPerCharge;				//	Extra HP for each point of charge
		int m_iExtraPowerPerCharge;				//	Extra power use for each point of charge (1/10 megawatt)
		int m_iExtraRegenPerCharge;				//	Extra regen per 10 points of charge

		ICCItem *m_pOnShieldDown;				//	Code to execute when shields go down (may be NULL)

		CEffectCreatorRef m_pHitEffect;			//	Effect when shield is hit
	};

class CSolarDeviceClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice);

		//	CDeviceClass virtuals

		virtual bool CanBeDisabled (void) { return false; }
		virtual ItemCategories GetCategory (void) { return itemcatMiscDevice; }
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

	private:
		CSolarDeviceClass (void);

		int m_iRefuel;							//	Units of fuel recharged per 10 ticks
												//	at 100% intensity.
	};

class CWeaponClass : public CDeviceClass
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpWeapon);
		virtual ~CWeaponClass (void);

		inline int GetVariantCount (void) { return m_iShotVariants; }
		inline CWeaponFireDesc *GetVariant (int iIndex) { return &m_pShotData[iIndex]; }

		//	CDeviceClass virtuals

		virtual void Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   int iFireAngle,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL);
		virtual CWeaponClass *AsWeaponClass (void) { return this; }
		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource);
		virtual int GetAmmoVariant (CItemType *pItem);
		virtual ItemCategories GetCategory (void);
		virtual int GetCounter (CInstalledDevice *pDevice, CounterTypes *retiType = NULL);
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL);

		virtual bool FindDataField (const CString &sField, CString *retsValue);
		virtual bool FindDataField (int iVariant, const CString &sField, CString *retsValue);
		virtual const DamageDesc *GetDamageDesc (CInstalledDevice *pDevice);
		virtual int GetDamageType (CInstalledDevice *pDevice);
		virtual Metric GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual int GetPowerRating (const CItem *pItem);
		virtual CString GetReference (const CItem *pItem = NULL, CSpaceObject *pInstalled = NULL);
		virtual void GetSelectedVariantInfo (CSpaceObject *pSource, 
											 CInstalledDevice *pDevice,
											 CString *retsLabel,
											 int *retiAmmoLeft,
											 CItemType **retpType = NULL);
		virtual int GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual int GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget);
		virtual bool IsAreaWeapon (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool IsWeaponAligned (CSpaceObject *pShip, CInstalledDevice *pDevice, CSpaceObject *pTarget, int *retiAimAngle, int *retiFireAngle = NULL);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual bool RequiresItems (void);
		virtual bool SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice);
		virtual bool ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice);

	protected:
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);

	private:
		enum ConfigurationTypes
			{
			ctSingle,				//	single shot
			ctDual,					//	dual, parallel shots
			ctWall,					//	five parallel shots
			ctSpread2,				//	dual, spread shots
			ctSpread3,				//	three, spread shots
			ctSpread5,				//	five, spread shots
			ctDualAlternating,		//	alternate barrels
			ctCustom,				//	custom configuration
			};

		struct SConfigDesc
			{
			DiceRange Angle;		//	Offset from fire angle
			int iPosAngle;			//	Origin of shot
			Metric rPosRadius;		//	Origin of shot
			};

		CWeaponClass (void);

		int CalcConfigurationMultiplier (CWeaponFireDesc *pShot = NULL);
		bool CanRotate (CInstalledDevice *pDevice);
		void FireWeapon (CInstalledDevice *pDevice, 
						 CWeaponFireDesc *pShot, 
						 CSpaceObject *pSource, 
						 CSpaceObject *pTarget,
						 int iFireAngle,
						 bool bConsumeItems,
						 bool *retbSourceDestroyed,
						 bool *retbConsumedItems);
		CWeaponFireDesc *GetSelectedShotData (CInstalledDevice *pDevice);
		inline bool IsCapacitorEnabled (void) { return (m_Counter == cntCapacitor); }
		inline bool IsCounterEnabled (void) { return (m_Counter != cntNone); }
		bool IsDirectional (CInstalledDevice *pDevice, int *retiMinFireArc = NULL, int *retiMaxFireArc = NULL);
		bool IsOmniDirectional (CInstalledDevice *pDevice);
		inline bool IsTemperatureEnabled (void) { return (m_Counter == cntTemperature); }
		bool VariantIsValid (CSpaceObject *pSource, CWeaponFireDesc &ShotData);

		int GetAlternatingPos (CInstalledDevice *pDevice);
		int GetContinuousFire (CInstalledDevice *pDevice);
		int GetCurrentVariant (CInstalledDevice *pDevice);
		void SetAlternatingPos (CInstalledDevice *pDevice, int iAlternatingPos);
		void SetContinuousFire (CInstalledDevice *pDevice, int iContinuous);
		void SetCurrentVariant (CInstalledDevice *pDevice, int iVariant);

		int m_iFireRate;						//	Ticks between shots
		int m_iFireRateSecs;					//	Game seconds between shots
		int m_iPowerUse;						//	Power use to recharge capacitors (1/10 megawatt)
		int m_iRecoil;							//	0-7 (as per momentum damage)
		int m_iFailureChance;					//	Chance of failure

		bool m_bLauncher;						//	Generic missile launcher
		bool m_bOmnidirectional;				//	Omnidirectional
		bool m_bCharges;						//	TRUE if weapon has charges
		bool m_bMIRV;							//	Each shot seeks an independent target
		int m_iMinFireArc;						//	Min angle of fire arc (degrees)
		int m_iMaxFireArc;						//	Max angle of fire arc (degrees)

		int m_iShotVariants;					//	Number of shot variations
		CWeaponFireDesc *m_pShotData;			//	Desc for each shot variation

		ConfigurationTypes m_Configuration;		//	Shot configuration;
		int m_iConfigCount;						//	Number of shots for custom configurations
		SConfigDesc *m_pConfig;					//	Custom configuration (may be NULL)
		int m_iConfigAimTolerance;				//	Aim tolerance

		CounterTypes m_Counter;					//	Counter type
		int m_iCounterUpdateRate;				//	Ticks to update counter
		int m_iCounterUpdate;					//	Inc/dec value per update
		int m_iCounterActivate;					//	Inc/dec value per shot

	friend class CObjectClass<CWeaponClass>;
	};

class CEnergyField
	{
	public:
		CEnergyField (void);
		~CEnergyField (void);
		static void CreateFromType (CEnergyFieldType *pType, int iLifeLeft, CEnergyField **retpField);

		bool AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx);
		inline CEnergyField *GetNext (void) const { return m_pNext; }
		inline CEnergyFieldType *GetType (void) const { return m_pType; }
		void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void SetNext (CEnergyField *pNext) { m_pNext = pNext; }
		void Update (bool *retbDeleteField);
		void WriteToStream (IWriteStream *pStream);

	private:
		CEnergyFieldType *m_pType;				//	Type of field
		int m_iLifeLeft;						//	Ticks left of energy field life (-1 = permanent)
		IEffectPainter *m_pPainter;				//	Painter

		int m_iPaintHit;						//	If >0 then we paint a hit
		int m_iPaintHitTick;					//	Tick for hit painter
		IEffectPainter *m_pHitPainter;			//	Hit painter

		CEnergyField *m_pNext;					//	Next energy field associated with this object
	};

class CEnergyFieldList
	{
	public:
		CEnergyFieldList (void);
		~CEnergyFieldList (void);

		void AddField (CEnergyFieldType *pType, int iLifeLeft);
		bool AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx);
		bool AbsorbsWeaponFire (CInstalledDevice *pDevice);
		int GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource);
		void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void ReadFromStream (SLoadCtx &Ctx);
		void Update (bool *retbModified);
		void WriteToStream (IWriteStream *pStream);

	private:
		void AddField (CEnergyField *pNewField);

		CEnergyField *m_pFirst;
	};

//	Effect Desc

class CEffectGroupCreator : public CEffectCreator
	{
	public:
		CEffectGroupCreator (void);

		inline int GetCount (void) { return m_iCount; }
		inline CEffectCreator *GetCreator (int iIndex) { return m_pCreators[iIndex]; }

		//	Virtuals

		virtual ~CEffectGroupCreator (void);
		virtual ALERROR CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel);
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void);
		virtual void LoadImages (void);
		virtual void MarkImages (void);
		virtual void SetLifetime (int iLifetime);
		virtual void SetSpeed (int iSpeed);
		virtual void SetVariants (int iVariants);

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		int m_iCount;
		CEffectCreator **m_pCreators;
	};

class CBoltEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return 0; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		int m_iLength;
		int m_iWidth;
		COLORREF m_wPrimaryColor;
		COLORREF m_wSecondaryColor;
	};

class CFlareEffectCreator : public CEffectCreator
	{
	public:
		enum Styles
			{
			styleFadingBlast,							//	Starts bright and fades out
			};

		void CreateFlareSpike (int iAngle, int iLength, int iWidth, SPoint *Poly);
		inline int GetLifetime (void) const { return m_iLifetime; }
		inline COLORREF GetPrimaryColor (void) const { return m_wPrimaryColor; }
		inline int GetRadius (void) const { return m_iRadius; }
		inline Styles GetStyle (void) const { return m_iStyle; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void SetLifetime (int iLifetime) { m_iLifetime = iLifetime; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		Styles m_iStyle;
		int m_iRadius;									//	Radius in pixels
		int m_iLifetime;								//	Lifetime (ticks)
		COLORREF m_wPrimaryColor;
	};

class CImageEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void LoadImages (void) { m_Image.LoadImage(); }
		virtual void MarkImages (void) { m_Image.MarkImage(); }
		virtual void SetVariants (int iVariants);

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual bool GetParticlePaintDesc (SParticlePaintDesc *retDesc);
		virtual void GetRect (RECT *retRect) const { *retRect = m_Image.GetImageRect(); }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		CObjectImageArray m_Image;
		int m_iLifetime;
		int m_iVariants;
		bool m_bRandomStartFrame;
		bool m_bDirectional;
	};

class CImageAndTailEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void LoadImages (void) { m_Image.LoadImage(); }
		virtual void MarkImages (void) { m_Image.MarkImage(); }
		virtual void SetVariants (int iVariants) { m_iVariants = iVariants; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const { *retRect = m_Image.GetImageRect(); }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		CObjectImageArray m_Image;
		int m_iLifetime;
		int m_iVariants;
		bool m_bRandomStartFrame;

		int m_iLength;
		int m_iWidth;
		COLORREF m_wPrimaryColor;
		COLORREF m_wSecondaryColor;
	};

class CMoltenBoltEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return 0; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		void CreateBoltShape (int iAngle, int iLength, int iWidth, SPoint *Poly);

		int m_iWidth;
		int m_iLength;
		int m_iGrowth;
		COLORREF m_wPrimaryColor;
		COLORREF m_wSecondaryColor;
	};

class CShapeEffectCreator : public CEffectCreator
	{
	public:
		CShapeEffectCreator (void) : m_Points(NULL), m_TransBuffer(NULL) { }

		void CreateShapeRegion (int iAngle, int iLength, int iWidth, CG16bitRegion *pRegion);
		inline COLORREF GetColor (void) const { return m_wColor; }
		inline int GetLength (void) const { return m_iLength; }
		inline int GetLengthInc (void) const { return m_iLengthInc; }
		inline DWORD GetOpacity (void) const { return m_byOpacity; }
		inline int GetWidth (void) const { return m_iWidth; }
		inline int GetWidthInc (void) const { return m_iWidthInc; }
		inline bool IsDirectional (void) const { return m_bDirectional; }

		//	CEffectCreator virtuals
		virtual ~CShapeEffectCreator (void);
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return 0; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		int m_iWidth;								//	Scale factor: pixel width of 100 units
		int m_iLength;								//	Scale factor: pixel length of 100 units

		bool m_bDirectional;						//	If TRUE, rotate shape based on Ctx.iRotation
		int m_iWidthInc;							//	Increment width every tick
		int m_iLengthInc;							//	Increment length every tick

		COLORREF m_wColor;								//	Shape color
		DWORD m_byOpacity;							//	Shape opacity

		int m_iPointCount;
		SPoint *m_Points;
		bool m_bConvexPolygon;						//	TRUE if simple polygon

		SPoint *m_TransBuffer;						//	Buffer used for transforms
	};

class CSingleParticleEffectCreator : public CEffectCreator
	{
	public:
		inline ParticlePaintStyles GetStyle (void) const { return m_iStyle; }
		inline int GetMaxWidth (void) const { return m_MaxWidth.Roll(); }
		inline int GetMinWidth (void) const { return m_MinWidth.Roll(); }
		inline COLORREF GetPrimaryColor (void) const { return m_wPrimaryColor; }
		inline COLORREF GetSecondaryColor (void) const { return m_wSecondaryColor; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		ParticlePaintStyles m_iStyle;
		DiceRange m_MinWidth;
		DiceRange m_MaxWidth;
		COLORREF m_wPrimaryColor;
		COLORREF m_wSecondaryColor;
	};

class CSmokeTrailEffectCreator : public CEffectCreator
	{
	public:
		enum Styles
			{
			styleFlame,									//	Flame fading to dark smoke
			styleSmoke,									//	Mostly smoke
			stylePlain,									//	Use colors
			};

		inline int GetEmitLifetime (void) const { return m_iEmitLifetime; }
		Metric GetEmitSpeed (void) const;
		inline int GetNewParticleCount (void) const { return m_NewParticles.Roll(); }
		inline int GetNewParticleMax (void) const { return m_NewParticles.GetMaxValue(); }
		inline CEffectCreator *GetParticleEffect (void) const { return m_pParticleEffect; }
		inline int GetParticleLifetime (void) const { return Max(1, m_ParticleLifetime.Roll()); }
		inline int GetParticleLifetimeMax (void) const { return Max(1, m_ParticleLifetime.GetMaxValue()); }
		inline int GetSpread (void) const { return m_Spread.Roll(); }

		//	CEffectCreator virtuals
		virtual ~CSmokeTrailEffectCreator (void);
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void SetLifetime (int iLifetime) { m_iLifetime = iLifetime; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		DiceRange m_Spread;								//	Random spread as % of particle speed (per particle)
		DiceRange m_NewParticles;						//	Number of new particles per tick
		DiceRange m_InitSpeed;							//	Initial speed of each particle
		DiceRange m_ParticleLifetime;					//	Lifetime of each particle

		int m_iLifetime;								//	Lifetime of effect
		int m_iEmitLifetime;							//	% time that it emits particles

		CEffectCreator *m_pParticleEffect;				//	Effect to use to paint particles
	};

class CStarburstEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return m_iLifetime; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		enum Styles
			{
			stylePlain,									//	Random spikes from center point
			styleMorningStar,							//	Sphere with random triangular spikes
			styleLightningStar,							//	Sphere with tail of lightning
			styleFlare,									//	Constant glow plus spikes
			};

		void CreateDiamondSpike (int iAngle, int iLength, int iWidthAngle, SPoint *Poly);

		Styles m_iStyle;
		DiceRange m_SpikeCount;
		DiceRange m_SpikeLength;
		COLORREF m_wPrimaryColor;
		COLORREF m_wSecondaryColor;
		int m_iLifetime;
	};

class CPlasmaSphereEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }
		virtual int GetLifetime (void) { return 0; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		DiceRange m_SpikeCount;
		DiceRange m_SpikeLength;
		COLORREF m_wPrimaryColor;
		COLORREF m_wSecondaryColor;
	};

class CParticleCometEffectCreator : public CEffectCreator,
		public IEffectPainter
	{
	public:
		CParticleCometEffectCreator (void);
		virtual ~CParticleCometEffectCreator (void);

		CVector GetParticlePos (int iParticle, int iTick, int iDirection, int *retiAge = NULL);
		inline int GetParticleCount (void) { return m_iParticleCount; }
		inline COLORREF GetPrimaryColor (void) { return m_wPrimaryColor; }
		inline int GetMaxAge (void) { return POINT_COUNT-1; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void) { return this; }

		//	IEffectPainter virtuals
		virtual void Delete (void) { }
		virtual CEffectCreator *GetCreator (void) { return this; }
		virtual void GetRect (RECT *retRect) const;
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		enum Constants
			{
			POINT_COUNT = 100,
			};

		struct SParticle
			{
			int iPos;
			CVector vScale;
			};

		void ComputeSplinePoints (void);
		void CreateParticles (void);

		int m_iParticleCount;
		int m_iWidth;
		int m_iLength;
		COLORREF m_wPrimaryColor;

		CVector *m_vPoints;
		SParticle *m_Particles;
	};

class CParticleExplosionEffectCreator : public CEffectCreator
	{
	public:
		//	CEffectCreator virtuals
		virtual ALERROR CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel);
		virtual void LoadImages (void) { m_Image.LoadImage(); }
		virtual void MarkImages (void) { m_Image.MarkImage(); }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		DiceRange m_Lifetime;							//	Total lifetime
		DiceRange m_ParticleCount;						//	Number of particles
		Metric m_rParticleSpeed;						//	Speed of particles
		int m_iParticleLifetime;						//	Particle lifespan
		CObjectImageArray m_Image;						//	Images
	};

class CPolyflashEffectCreator : public CEffectCreator
	{
	public:
		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return 1; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
	};

class CShockwaveEffectCreator : public CEffectCreator
	{
	public:
		enum Styles
			{
			styleImage,									//	Use an image to paint shockwave
			styleGlowRing,								//	Glowing right
			};

		CShockwaveEffectCreator (void);
		~CShockwaveEffectCreator (void);

		inline COLORREF *GetColorGradient (void) { return m_wColorGradient; }
		inline int GetFadeStart (void) { return m_iFadeStart; }
		inline const CObjectImageArray &GetImage (void) { return m_Image; }
		inline BYTE *GetOpacityGradient (void) { return m_byOpacityGradient; }
		inline COLORREF GetPrimaryColor (void) const { return m_wPrimaryColor; }
		inline COLORREF GetSecondaryColor (void) const { return m_wSecondaryColor; }
		inline int GetRingThickness (void) { return m_iGradientCount; }
		inline int GetSpeed (void) { return m_iSpeed; }
		inline Styles GetStyle (void) const { return m_iStyle; }

		//	CEffectCreator virtuals
		virtual IEffectPainter *CreatePainter (void);
		virtual int GetLifetime (void) { return m_iLifetime; }
		virtual void LoadImages (void) { m_Image.LoadImage(); }
		virtual void MarkImages (void) { m_Image.MarkImage(); }
		virtual void SetLifetime (int iLifetime) { m_iLifetime = iLifetime; }
		virtual void SetSpeed (int iSpeed) { m_iSpeed = iSpeed; }

	protected:
		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx);

	private:
		void CreateGlowGradient (int iSolidWidth, int iGlowWidth, COLORREF wSolidColor, COLORREF wGlowColor);

		Styles m_iStyle;								//	Style of effect
		CObjectImageArray m_Image;						//	Shockwave image
		int m_iSpeed;									//	Expansion speed (pixels per tick)
		int m_iLifetime;								//	Lifetime (ticks)
		int m_iFadeStart;								//	% of lifetime at which we start to fade

		int m_iWidth;									//	Width of central ring
		int m_iGlowWidth;								//	Glow width

		COLORREF m_wPrimaryColor;
		COLORREF m_wSecondaryColor;

		int m_iGradientCount;
		COLORREF *m_wColorGradient;
		BYTE *m_byOpacityGradient;
	};

//	Particle Field Type

#if 0
class CParticleGroup;

class IParticleAppearance
	{
	public:
		virtual void Paint (CG16bitImage &Dest,
							const ViewportTransform &Trans,
							CParticleGroup *pGroup) = 0;
	};

class IParticleBehavior
	{
	};

class IParticleMotion
	{
	};

class CParticleGroupType : public CObject
	{
	public:
		static ALERROR CreateFromXML (CXMLElement *pDesc, CParticleGroupType **retpType);

		inline IParticleAppearance *GetAppearance (void) { return m_pAppearance; }
		inline IParticleBehavior *GetBehavior (void) { return m_pBehavior; }
		inline IParticleMotion *GetMotion (void) { return m_pMotion; }
		inline DWORD GetUNID (void) { return m_dwUNID; }

	private:
		CParticleGroupType (void);

		DWORD m_dwUNID;

		IParticleAppearance *m_pAppearance;
		IParticleBehavior *m_pBehavior;
		IParticleMotion *m_pMotion;

	friend CObjectClass<CParticleGroupType>;
	};

struct SParticle
	{
	inline bool IsAlive (void) { return iLifeLeft != -1; }

	int iDestiny;					//	Particle randomization
	int iLifeLeft;					//	Ticks left in life (-1 if particle is dead)
	CVector vPos;			//	Position of particle
	CVector vVel;			//	Velocity of particle
	};

class CParticleGroup
	{
	public:
		static ALERROR CreateFromType (CParticleGroupType *pType,
									   CSpaceObject *pAnchor,
									   CParticleGroup **retpGroup);
		~CParticleGroup (void);

		void Paint (CG16bitImage &Dest, const ViewportTransform &Trans);

	private:
		CParticleGroup (void);

		CParticleGroupType *m_pType;	//	Type of particle group

		int m_iCount;					//	Number of particles in the array
		int m_iAlive;					//	Number of particles that are alive in the group
		SParticle *m_pParticles;		//	Array of particles
	};
#endif

//	Space Objects

#define MAX_COMMS_CAPS					25

enum ObjectComponentTypes
	{
	comArmor,
	comCargo,
	comShields,
	comWeapons,
	comDrive,
	comReactor,
	comDeviceCounter,					//	One or more devices need to show a counter
	};

struct SExplosionType
	{
	CWeaponFireDesc *pDesc;				//	Explosion type
	int iBonus;							//	Bonus damage
	DestructionTypes iCause;			//	Cause
	};

class CSpaceObject : public CObject
	{
	public:
		enum Categories
			{
			catShip =			0x00000001,
			catStation =		0x00000002,
			catBeam =			0x00000004,
			catMissile =		0x00000008,
			catFractureEffect =	0x00000010,
			catOther =			0x80000000,
			};

		enum DamageResults
			{
			damageNoDamage,
			damageAbsorbedByShields,
			damageArmorHit,
			damageStructuralHit,
			damageDestroyed,
			damagePassthrough,					//	When we hit another missile (or small obj) we pass through
			};

		enum StealthAndPerceptionLevels
			{
			stealthMin =			0,
			stealthNormal =			4,
			stealthMax =			15,			//	Cloaked

			perceptMin =			0,
			perceptNormal =			4,
			perceptMax =			15,
			};

		struct Criteria
			{
			CSpaceObject *pSource;				//	Source

			DWORD dwCategories;					//	Only these object categories
			bool bActiveObjectsOnly;			//	Only active object (e.g., objects that can attack)
			bool bFriendlyObjectsOnly;			//	Only friendly to source
			bool bEnemyObjectsOnly;				//	Only enemy to source
			bool bManufacturedObjectsOnly;		//	Exclude planets, stars, etc.
			bool bStructureScaleOnly;			//	Only structure-scale objects
			bool bStargatesOnly;				//	Only stargates
			bool bNearestOnly;					//	Only objects within radius
			bool bFarthestOnly;					//	Only objects beyond radius
			bool bEscortingSource;				//	Only objects escorting source
			bool bGuardingSource;				//	Only objects guarding source
			bool bHomeBaseIsSource;				//	Only objects whose home base is the source
			bool bDockedWithSource;				//	Only objects currently docked with source
			CString sData;						//	Only objects with non-Nil data
			CString sStargateID;				//	Only objects with this stargate ID (if non blank)
			Metric rMinRadius;					//	Only objects at or beyond the given radius
			Metric rMaxRadius;					//	Only objects within the given radius

			CStringArray AttribsRequired;		//	Required attributes
			CStringArray AttribsNotAllowed;		//	Exclude objects with these attributes

			bool bSingleObj;					//	Return only the single nearest/farthest object
			};

		CSpaceObject (IObjectClass *pClass);
		virtual ~CSpaceObject (void);
		static void CreateFromStream (SLoadCtx &Ctx, CSpaceObject **retpObj);

		void Accelerate (const CVector &vPush, Metric rSeconds);
		void AccelerateStop (Metric rPush, Metric rSeconds);
		void AddEffect (IEffectPainter *pPainter, const CVector &vPos);
		ALERROR AddToSystem (CSystem *pSystem);
		inline bool Blocks (CSpaceObject *pObj) { return (m_fIsBarrier && CanBlock(pObj)); }
		void CalculateMove (void);
		inline bool CanBeControlled (void) { return m_iControlsFrozen == 0; }
		inline bool CanBeHit (void) { return !m_fCannotBeHit; }
		inline bool CanBeHitByFriends (void) { return !m_fNoFriendlyTarget; }
		bool CanDetect (int Perception, CSpaceObject *pObj);
		bool CanCommunicateWith (CSpaceObject *pSender);
		inline bool CanHitFriends (void) { return !m_fNoFriendlyFire; }
		inline void ClearPlayerTarget (void) { m_fPlayerTarget = false; }
		inline void ClearPOVLRS (void) { m_fInPOVLRS = false; }
		inline void ClearSelection (void) { m_fSelected = false; }
		void ClipSpeed (Metric rMaxSpeed);
		void CommsMessageFrom (CSpaceObject *pSender, int iIndex);
		inline DWORD Communicate (CSpaceObject *pReceiver, MessageTypes iMessage, CSpaceObject *pParam1 = NULL, DWORD dwParam2 = 0) { return pReceiver->OnCommunicate(this, iMessage, pParam1, dwParam2); }
		ALERROR CreateRandomItems (CXMLElement *pItems);
		ALERROR CreateRandomItems (IItemGenerator *pItems);
		static CString DebugDescribe (CSpaceObject *pObj);
		inline bool DebugIsValid (void) { return (DWORD)m_pSystem != 0xdddddddd; }
		void Destroy (DestructionTypes iCause, CSpaceObject *pCause);
		void EnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		void FireCustomEvent (const CString &sEvent, ICCItem **retpResult = NULL);
		void FireCustomItemEvent (const CString &sEvent, const CItem &Item, ICCItem **retpResult = NULL);
		void FireGetExplosionType (SExplosionType *retExplosion);
		void FireOnAttacked (SDamageCtx &Ctx);
		void FireOnAttackedByPlayer (void);
		void FireOnCreate (void);
		void FireOnDamage (SDamageCtx &Ctx);
		void FireOnDestroy (const SDestroyCtx &Ctx);
		void FireOnEnteredGate (CSpaceObject *pGate);
		void FireOnItemAIUpdate (void);
		void FireOnItemUpdate (void);
		void FireOnMining (const SDamageCtx &Ctx);
		void FireOnObjDestroyed (const SDestroyCtx &Ctx);
		void FireOnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget);
		void FireOnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate);
		void FireOnObjJumped (CSpaceObject *pObj);
		void FireOnObjReconned (CSpaceObject *pObj);
		void FireOnOrderChanged (void);
		void FireOnOrdersCompleted (void);
		inline void FreezeControls (void) { m_iControlsFrozen++; }
		void GetBoundingRect (CVector *retvUR, CVector *retvLL);
		int GetCommsMessageCount (void);
		inline const CString &GetData (const CString &sAttrib) const { return m_Data.GetData(sAttrib); }
		int GetDataInteger (const CString &sAttrib) const;
		inline int GetDestiny (void) const { return m_iDestiny; }
		inline Metric GetDetectionRange (int iPerception) { return RangeIndex2Range(GetDetectionRangeIndex(iPerception)); }
		Metric GetDetectionRange2 (int iPerception);
		int GetDetectionRangeIndex (int iPerception);
		CSovereign::Disposition GetDispositionTowards (CSpaceObject *pObj);
		Metric GetDistance (CSpaceObject *pObj) { return (pObj->GetPos() - GetPos()).Length(); }
		Metric GetDistance2 (CSpaceObject *pObj) { return (pObj->GetPos() - GetPos()).Length2(); }
		inline DWORD GetID (void) const { return m_dwID; }
		inline int GetIndex (void) const { return m_iIndex; }
		inline CItemList &GetItemList (void) { return m_ItemList; }
		CSpaceObject *GetNearestEnemy (Metric rMaxRange = g_InfiniteDistance, bool bIncludeStations = false);
		CSpaceObject *GetNearestEnemyStation (Metric rMaxRange = g_InfiniteDistance);
		CSpaceObject *GetNearestStargate (bool bExcludeUncharted = false);
		int GetNearestVisibleEnemies (int iMaxEnemies, Metric rMaxDist, CIntArray *pretList, CSpaceObject *pExcludeObj = NULL);
		CSpaceObject *GetNearestVisibleEnemy (Metric rMaxRange = g_InfiniteDistance, bool bIncludeStations = false, CSpaceObject *pExcludeObj = NULL);
		CString GetNounPhrase (DWORD dwFlags);
		inline CSpaceObject *GetObjRefData (const CString &sAttrib) { return m_Data.GetObjRefData(sAttrib); }
		CSpaceObject *GetOrderGiver (DestructionTypes iCause = killedNone);
		inline const CVector &GetPos (void) const { return m_vPos; }
		CSovereign *GetSovereignToDefend (void) const;
		inline CSystem *GetSystem (void) const { return m_pSystem; }
		inline CUniverse *GetUniverse (void) const { return m_pSystem->GetUniverse(); }
		inline const CVector &GetVel (void) const { return m_vVel; }
		bool HasFuelItem (void);
		inline bool HasOnAttackedEvent (void) const { return (m_fHasOnAttackedEvent ? true : false); }
		inline bool HasOnDamageEvent (void) const { return (m_fHasOnDamageEvent ? true : false); }
		inline bool HasOnObjDockedEvent (void) const { return (m_fHasOnObjDockedEvent ? true : false); }
		inline void Highlight (COLORREF wColor) { m_wHighlightColor = wColor; m_iHighlightCountdown = 200; }
		bool InBarrier (const CVector &vPos);
		inline bool InBox (const CVector &vUR, const CVector &vLL) const 
			{ return (vUR.GetX() > m_vPos.GetX() - m_rBoundsX) 
					&& (vUR.GetY() > m_vPos.GetY() - m_rBoundsY)
					&& (vLL.GetX() < m_vPos.GetX() + m_rBoundsX)
					&& (vLL.GetY() < m_vPos.GetY() + m_rBoundsY); }
		inline void InvalidateItemListAddRemove (void) { m_fItemEventsValid = false; }
		inline void InvalidateItemListState (void) { m_fItemEventsValid = false; }
		bool IsAutomatedWeapon (void) const { return (m_fAutomatedWeapon ? true : false); }
		bool IsBarrier (void) const { return (m_fIsBarrier ? true : false); }
		bool IsCommsMessageValidFrom (CSpaceObject *pSender, int iIndex, CString *retsMsg = NULL, CString *retsKey = NULL);
		bool IsCovering (CSpaceObject *pObj);
		bool IsDestinyTime (int iCycle);
		bool IsEnemy (const CSpaceObject *pObj) const;
		bool IsEnemyInRange (Metric rMaxRange, bool bIncludeStations = false);
		bool IsEscortingFriendOf (const CSpaceObject *pObj) const;
		bool IsFriend (const CSpaceObject *pObj) const;
		bool IsLineOfFireClear (const CVector &vSource, CSpaceObject *pTarget, int iAngle, Metric rDistance = (30.0 * LIGHT_SECOND), bool bAreaWeapon = false);
		inline bool IsMobile (void) { return !m_fCannotMove; }
		inline bool IsPlayerTarget (void) { return m_fPlayerTarget; }
		inline bool IsSelected (void) { return m_fSelected; }
		bool IsStargateInRange (Metric rMaxRange);
		inline bool IsTimeStopped (void) { return m_fTimeStop; }
		bool IsUnderAttack (void);
		inline void ItemsModified (void) { OnItemsModified(); }
		void Jump (const CVector &vPos);
		void Move (const CSpaceObjectList &Barriers, Metric rSeconds);
		inline bool NotifyOthersWhenDestroyed (void) { return (m_fNoObjectDestructionNotify ? false : true); }
		void OnObjDestroyed (const SDestroyCtx &Ctx);
		inline void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) 
			{
			PaintDebugVector(Dest, x, y, Ctx);
			OnPaint(Dest, x, y, Ctx); 
			}
		inline void Place (const CVector &vPos, const CVector &vVel = NullVector) { SetPos(vPos); SetVel(vVel); }
		inline bool PosInBox (const CVector &vUR, const CVector &vLL) const
			{ return (vUR.GetX() > m_vPos.GetX()) 
					&& (vUR.GetY() > m_vPos.GetY())
					&& (vLL.GetX() < m_vPos.GetX())
					&& (vLL.GetY() < m_vPos.GetY()); }
		void Reconned (void);
		void Remove (DestructionTypes iCause, CSpaceObject *pCause);
		void ReportEventError (const CString &sEvent, ICCItem *pError);
		inline void RestartTime (void) { m_fTimeStop = false; }
		inline void SetAutomatedWeapon (void) { m_fAutomatedWeapon = true; }
		inline void SetData (const CString &sAttrib, const CString &sData) { m_Data.SetData(sAttrib, sData); }
		inline void SetDataFromDataBlock (const CAttributeDataBlock &Block) { m_Data = Block; }
		inline void SetDataFromXML (CXMLElement *pData) { m_Data.SetFromXML(pData); }
		void SetDataInteger (const CString &sAttrib, int iValue);
		inline void SetHasOnAttackedEvent (bool bHasEvent) { m_fHasOnAttackedEvent = bHasEvent; }
		inline void SetHasOnDamageEvent (bool bHasEvent) { m_fHasOnDamageEvent = bHasEvent; }
		inline void SetHasOnObjDockedEvent (bool bHasEvent) { m_fHasOnObjDockedEvent = bHasEvent; }
		inline void SetObjRefData (const CString &sAttrib, CSpaceObject *pObj) { m_Data.SetObjRefData(sAttrib, pObj); }
		inline void SetPlayerTarget (void) { m_fPlayerTarget = true; }
		inline bool SetPOVLRS (void)
			{
			if (m_fInPOVLRS)
				return false;

			m_fInPOVLRS = true;
			return true;
			}
		inline void SetSelection (void) { m_fSelected = true; }
		inline void StopTime (void) { m_fTimeStop = true; }
		inline bool SupportsDocking (void) { return (GetDockScreen() != NULL); }
		inline void UnfreezeControls (void) { m_iControlsFrozen--; }
		void Update (void);
		inline void UpdateExtended (void) { OnUpdateExtended(); }
		void UseItem (CItem &Item, CString *retsError = NULL);
		void WriteToStream (IWriteStream *pStream);

		bool MatchesCriteria (const Criteria &Crit) const;
		static void ParseCriteria (CSpaceObject *pSource, const CString &sCriteria, Criteria *retCriteria);

#ifdef DEBUG_VECTOR
		void PaintDebugVector (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		inline void SetDebugVector (const CVector &vVector) { m_vDebugVector = vVector; }
#else
		inline void PaintDebugVector (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { }
		inline void SetDebugVector (const CVector &vVector) { }
#endif

		//	Virtuals to be overridden

		//	...for all objects
		virtual CMissile *AsMissile (void) { return NULL; }
		virtual CShip *AsShip (void) { return NULL; }
		virtual CStation *AsStation (void) { return NULL; }
		virtual bool CanAttack (void) const { return false; }
		virtual bool CanBeDestroyed (void) { return true; }
		virtual bool CanBeHitBy (const DamageDesc &Damage) { return true; }
		virtual bool CanMove (void) { return false; }
		virtual bool ClassCanAttack (void) { return false; }
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage) { return damageNoDamage; }
		virtual CString DebugCrashInfo (void) { return NULL_STR; }
		virtual Categories GetCategory (void) const { return catOther; }
		virtual DWORD GetClassUNID (void) { return 0; }
		virtual const CString &GetGlobalData (const CString &sAttribute) { return NULL_STR; }
		virtual const CObjectImageArray &GetImage (void);
		virtual int GetInteraction (void) { return 100; }
		virtual const CSystem::Orbit *GetMapOrbit (void) { return NULL; }
		virtual Metric GetMass (void) { return 0.0; }
		virtual Metric GetMaxSpeed (void) { return (IsMobile() ? LIGHT_SPEED : 0.0); }
		virtual CString GetName (DWORD *retdwFlags = NULL) { if (retdwFlags) *retdwFlags = 0; return LITERAL("unknown object"); }
		virtual CString GetObjClassName (void) { return CONSTLIT("unknown"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerStations; }
		virtual ScaleTypes GetScale (void) const { return scaleFlotsam; }
		virtual CSovereign *GetSovereign (void) const { return NULL; }
		virtual const CString &GetStaticData (const CString &sAttribute) { return NULL_STR; }
		virtual bool HasAttribute (const CString &sAttribute) const { return false; }
		virtual bool IsKnown (void) { return true; }
		virtual void LoadImages (void) { }
		virtual void MarkImages (void) { }
		virtual void OnSystemCreated (void) { }
		virtual void OnSystemLoaded (void) { }
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual void PaintMap (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }
		virtual bool PointInObject (CVector vPos) { return false; }
		virtual void SetGlobalData (const CString &sAttribute, const CString &sData) { }
		virtual void SetKnown (bool bKnown = true) { }
		virtual void SetName (const CString &sName, DWORD dwFlags = 0) { }
		virtual void SetSovereign (CSovereign *pSovereign) { }

		//	...for active/intelligent objects (ships, stations, etc.)
		virtual int ChargeMoney (int iValue) { return 0; }
		virtual void CreateRandomDockedShips (IShipGenerator *pGenerator) { }
		virtual int CreditMoney (int iValue) { return 0; }
		virtual void DamageDevice (CInstalledDevice *pDevice, CDeviceClass::DeviceFailureType iCause) { }
		virtual void DeactivateShields (void) { }
		virtual void Decontaminate (void) { }
		virtual void DepleteShields (void) { }
		virtual void DisableDevice (CInstalledDevice *pDevice) { }
		virtual CInstalledDevice *FindDevice (const CItem &Item) { return NULL; }
		virtual bool FindEventHandler (const CString &sEntryPoint, ICCItem **retpCode) { return false; }
		virtual int GetBalance (void) { return 0; }
		virtual int GetBuyPrice (const CItem &Item, int *retiBuyPrice = NULL) { return -1; }
		virtual Metric GetCargoSpaceLeft (void) { return 1000000.0; }
		virtual int GetCombatPower (void) { return 0; }
		virtual CCommunicationsHandler *GetCommsHandler (void) { return NULL; }
		virtual int GetCyberDefenseLevel (void) { return 1; }
		virtual DamageTypes GetDamageType (void) { return damageNone; }
		virtual CStationType *GetEncounterInfo (void) { return NULL; }
		virtual CSpaceObject *GetEscortPrincipal (void) const { return NULL; }
		virtual int GetLevel (void) const { return 1; }
		virtual int GetMaxPower (void) const { return 0; }
		virtual int GetOpenDockingPortCount (void) { return 0; }
		virtual int GetPerception (void) { return perceptNormal; }
		virtual CSpaceObject *GetTarget (void) { return NULL; }
		virtual int GetScore (void) { return 0; }
		virtual int GetSellPrice (const CItem &Item) { return 0; }
		virtual int GetShieldLevel (void) { return -1; }
		virtual COLORREF GetSpaceColor (void) { return 0; }
		virtual CString GetStargateID (void) const { return NULL_STR; }
		virtual int GetStealth (void) { return stealthNormal; }
		virtual int GetVisibleDamage (void) { return 0; }
		virtual bool HasMapLabel (void) { return false; }
		virtual bool IsAngryAt (CSpaceObject *pObj) { return IsEnemy(pObj); }
		virtual bool IsBlind (void) { return false; }
		virtual bool IsDisarmed (void) { return false; }
		virtual bool IsIdentified (void) { return true; }
		virtual bool IsObjDocked (CSpaceObject *pObj) { return false; }
		virtual bool IsOutOfFuel (void) { return false; }
		virtual bool IsParalyzed (void) { return false; }
		virtual bool IsPlayer (void) { return false; }
		virtual bool IsRadioactive (void) { return false; }
		virtual bool IsTimeStopImmune (void) { return false; }
		virtual void MakeParalyzed (int iTickCount = -1) { }
		virtual void OnComponentChanged (ObjectComponentTypes iComponent) { }
		virtual void OnDeviceFailure (CInstalledDevice *pDev, int iFailureType) { }
		virtual void OnObjLeaveGate (CSpaceObject *pObj) { }
		virtual void OnPlayerObj (CSpaceObject *pPlayer) { }
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx) { }
		virtual void ProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program) { }
		virtual void RegisterObjectForEvents (CSpaceObject *pObj) { }
		virtual void SendMessage (CSpaceObject *pSender, const CString &sMsg) { }
		virtual void SetIdentified (bool bIdentified = true) { }
		virtual void SetMapLabelPos (int x, int y) { }
		virtual void UnregisterObjectForEvents (CSpaceObject *pObj) { }
		virtual void UpdateArmorItems (void) { }

		//	...for objects that can bounce
		virtual bool ImageInObject (const CObjectImageArray &Image, int iTick, int iRotation, CVector vPos) { return PointInObject(vPos); }
		virtual bool ObjectInObject (CSpaceObject *pObj) { return PointInObject(pObj->GetPos()); }
		virtual void OnObjBounce (CSpaceObject *pObj, const CVector &vPos) { }

		//	...for objects with weapons
		virtual bool AbsorbWeaponFire (CInstalledDevice *pWeapon) { return false; }
		virtual CString GetInstallationPhrase (const CItem &Item) const { return NULL_STR; }
		virtual CVector GetWeaponPos (void) const { return m_vPos; }

		//	...for objects with docking ports
		virtual DWORD GetDefaultBkgnd (void) { return 0; }
		virtual CXMLElement *GetDockScreen (void) { return NULL; }
		virtual CVector GetNearestDockVector (CSpaceObject *pRequestingObj) { return CVector(g_InfiniteDistance, g_InfiniteDistance); }
		virtual CXMLElement *GetScreen (const CString &sName);
		virtual void OnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget) { }
		virtual void PlaceAtRandomDockPort (CSpaceObject *pObj) { }
		virtual bool RequestDock (CSpaceObject *pObj) { return false; }
		virtual void Undock (CSpaceObject *pObj) { }

		//	...for beams, missiles, etc.
		virtual void CreateReflection (const CVector &vPos, int iDirection) { }
		virtual void DetonateNow (CSpaceObject *pHit) { }
		virtual CSpaceObject *GetDamageCause (void) { return this; }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return GetNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return killedByDamage; }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return NULL; }
		virtual CSpaceObject *GetSource (void) { return NULL; }
		virtual CSpaceObject *GetSecondarySource (void) { return NULL; }

		//	...for ships
		virtual void Behavior (void) { }
		virtual void ConsumeFuel (int iFuel) { }
		virtual bool FollowsObjThroughGate (CSpaceObject *pLeader) { return false; }
		virtual CSpaceObject *GetBase (void) const { return NULL; }
		virtual int GetRotation (void) const { return 0; }
		virtual void OnDocked (CSpaceObject *pObj) { }
		virtual void OnDockedObjChanged (void) { }
		virtual void Refuel (int iFuel) { }
		virtual void Refuel (const CItem &Fuel) { }
		virtual void RepairDamage (int iHitPoints) { }
		virtual void UpdateDockingManeuver(const CVector &vDest, const CVector &vDestVel, int iDestFacing) { }

		//	...for stations
		virtual void AddSubordinate (CSpaceObject *pSubordinate) { }
		virtual IShipGenerator *GetRandomEncounterTable (int *retiFrequency = NULL) { return NULL; }
		virtual bool IsAbandoned (void) const { return false; }
		virtual bool IsStargate (void) const { return false; }
		virtual bool RequestGate (CSpaceObject *pObj);
		virtual bool SupportsGating (void) { return false; }

		//	...for particle effects
		virtual void SetAttractor (CSpaceObject *pObj) { }

		//	...miscellaneous
		virtual void OnLosePOV (void) { }

	protected:

		//	Virtuals to be overridden
		virtual bool CanBlock (CSpaceObject *pObj) { return true; }
		virtual bool CanHit (CSpaceObject *pObj) { return true; }
		virtual bool CanFireOn (CSpaceObject *pObj) { return true; }
		virtual void GateHook (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate) { Destroy(removedFromSystem, NULL); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx) { }
		virtual void ObjectEnteredGateHook (CSpaceObject *pObjEnteredGate) { }
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2) { return resNoAnswer; }
		virtual void OnDestroyed (DestructionTypes iCause, CSpaceObject *pCause, bool *ioResurrectPending, CSpaceObject **retpWreck) { }
		virtual CSpaceObject *OnGetOrderGiver (void) { return this; }
		virtual void OnItemsModified (void) { }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds) { }
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate) { }
		virtual void OnObjJumped (CSpaceObject *pObj) { }
		virtual void OnObjReconned (CSpaceObject *pObj) { }
		virtual void OnUpdate (Metric rSecondsPerTick) { }
		virtual void OnUpdateExtended (void) { }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { }
		virtual void OnReadFromStream (SLoadCtx &Ctx) { }
		virtual void OnWriteToStream (IWriteStream *pStream) { }
		virtual bool OrientationChanged (void) { return false; }
		virtual void RevertOrientationChange (void) { }

		//	Helper functions
		void AddEffect (IEffectPainter *pPainter, int xOffset, int yOffset, int iTick = 0);
		Metric CalculateItemMass (Metric *retrCargoMass);
		bool CanFireOnObjHelper (CSpaceObject *pObj);
		inline void ClearCannotBeHit (void) { m_fCannotBeHit = false; }
		inline void DisableObjectDestructionNotify (void) { m_fNoObjectDestructionNotify = true; }
		inline const Metric &GetBounds (void) { return m_rBoundsX; }
		CSpaceObject *HitTest (const CVector &vStart, Metric rThreshold, const DamageDesc &Damage, CVector *retvHitPos, int *retiHitDir);
		bool ImagesIntersect (const CObjectImageArray &Image1, int iTick1, int iRotation1, const CVector &vPos1,
				const CObjectImageArray &Image2, int iTick2, int iRotation2, const CVector &vPos2);
		inline bool IsHighlighted (void) { return ((m_iHighlightCountdown != 0) || m_fSelected); }
		inline bool IsObjectDestructionHooked (void) { return (m_fHookObjectDestruction ? true : false); }
		bool MissileCanHitObj (CSpaceObject *pObj, CSpaceObject *pSource, bool bCanDamageSource);
		void PaintEffects (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void PaintHighlight (CG16bitImage &Dest, const RECT &rcRect);
		inline void SetObjectDestructionHook (void) { m_fHookObjectDestruction = true; }
		inline void SetCannotBeHit (void) { m_fCannotBeHit = true; }
		inline void SetCannotMove (void) { m_fCannotMove = true; }
		inline void SetCanBounce (void) { m_fCanBounce = true; }
		inline void SetBounds (Metric rBounds) { m_rBoundsX = rBounds; m_rBoundsY = rBounds; }
		inline void SetBounds (const RECT &rcRect)
			{ 
			m_rBoundsX = g_KlicksPerPixel * (RectWidth(rcRect) / 2);
			m_rBoundsY = g_KlicksPerPixel * (RectHeight(rcRect) / 2);
			}
		inline void SetIsBarrier (void) { m_fIsBarrier = true; }
		inline void SetNoFriendlyFire (void) { m_fNoFriendlyFire = true; }
		inline void SetNoFriendlyTarget (void) { m_fNoFriendlyTarget = true; }
		inline void SetPos (const CVector &vPos) { m_vPos = vPos; }
		inline void SetVel (const CVector &vVel) { m_vVel = vVel; }

	private:

		struct SEffectNode
			{
			IEffectPainter *pPainter;
			int xOffset;
			int yOffset;
			int iTick;

			SEffectNode *pNext;
			};

		CSpaceObject (void);

		inline void InitItemEvents (void) { m_ItemEvents.Init(this); m_fItemEventsValid = true; }

		CSystem *m_pSystem;						//	Current system
		int m_iIndex;							//	Index in system
		DWORD m_dwID;							//	Universal ID
		int m_iDestiny;							//	Random number 0..DestinyRange-1
		CVector m_vPos;							//	Position of object in system
		CVector m_vVel;							//	Velocity of object
		CVector m_vNewPos;						//	New position of object (after CalculateMove)
		Metric m_rBoundsX;						//	Object bounds
		Metric m_rBoundsY;						//	Object bounds

		COLORREF m_wHighlightColor;					//	Highlight color
		int m_iHighlightCountdown:16;			//	Frames left to highlight

		CItemList m_ItemList;					//	List of items
		CAttributeDataBlock m_Data;				//	Opaque data
		SEffectNode *m_pFirstEffect;			//	List of effects
		CItemEventDispatcher m_ItemEvents;		//	Item event dispatcher

		int m_iControlsFrozen:8;				//	Object will not respond to controls
		int m_iSpare:24;

		DWORD m_fHookObjectDestruction:1;		//	Call the object when another object is destroyed
		DWORD m_fNoObjectDestructionNotify:1;	//	Do not call other objects when this one is destroyed
		DWORD m_fCannotBeHit:1;					//	TRUE if it cannot be hit by other objects
		DWORD m_fSelected:1;					//	TRUE if selected
		DWORD m_fInPOVLRS:1;					//	TRUE if object appears in POV's LRS
		DWORD m_fCanBounce:1;					//	TRUE if object can bounce off others
		DWORD m_fIsBarrier:1;					//	TRUE if objects bounce off this object
		DWORD m_fCannotMove:1;					//	TRUE if object cannot move
		DWORD m_fNoFriendlyFire:1;				//	TRUE if object cannot hit friendly objects
		DWORD m_fTimeStop:1;					//	TRUE if time has stopped for this object
		DWORD m_fPlayerTarget:1;				//	TRUE if this is a target for the player
		DWORD m_fAutomatedWeapon:1;				//	TRUE if this is a shot from automated weapon
		DWORD m_fHasOnObjDockedEvent:1;			//	TRUE if this object has an OnObjDocked event
		DWORD m_fOnCreateCalled:1;				//	TURE if OnCreate event has already been called
		DWORD m_fNoFriendlyTarget:1;			//	TRUE if object cannot be hit by friends
		DWORD m_fItemEventsValid:1;				//	TRUE if item event dispatcher is up to date
		DWORD m_fHasOnDamageEvent:1;			//	TRUE if object has OnDamage event
		DWORD m_fHasOnAttackedEvent:1;			//	TRUE if object has OnAttacked event
		DWORD m_fSpare:13;

#ifdef DEBUG_VECTOR
		CVector m_vDebugVector;			//	Draw a vector
#endif

	friend class CObjectClass<CSpaceObject>;
	};

class CAreaDamage : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				int iBonus,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				CAreaDamage **retpObj);
		~CAreaDamage (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage) { return damagePassthrough; }
		virtual CSpaceObject *GetDamageCause (void) { return (m_Source.IsEmpty() ? this : m_Source.GetObj()); }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CAreaDamage"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void OnSystemLoaded (void);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual bool PointInObject (CVector vPos);

	protected:
		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CAreaDamage (void);
		bool AlreadyHit (CSpaceObject *pObj);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		int m_iBonus;							//	Bonus
		DestructionTypes m_iCause;				//	Cause of damage
		IEffectPainter *m_pPainter;				//	Effect painter
		int m_iInitialDelay;					//	Delay before start
		int m_iTick;							//	Counter
		int m_iLifeLeft;						//	Ticks left
		CDamageSource m_Source;					//	Object that fired the beam
		CSovereign *m_pSovereign;				//	Sovereign
		CIDTable m_Hit;							//	List of objects that we hit

		int m_iSavedCount;						//	Temporary storage while loading
		CSpaceObject **m_pSavedObj;
		DWORD *m_pSavedValue;

	friend class CObjectClass<CAreaDamage>;
	};

class CBeam : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				int iBonus,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				int iRotation,
				CBeam **retpBeam);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual void CreateReflection (const CVector &vPos, int iDirection);
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage) { return damagePassthrough; }
		virtual Categories GetCategory (void) const { return catBeam; }
		virtual CSpaceObject *GetDamageCause (void) { return (m_Source.IsEmpty() ? this : m_Source.GetObj()); }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual int GetInteraction (void) { return 0; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CBeam"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerStations; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);

	protected:
		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CBeam (void);
		void ComputeOffsets (void);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		int m_iBonus;							//	Bonus damage
		DestructionTypes m_iCause;				//	Cause of damage
		int m_iRotation;						//	Direction
		CVector m_vPaintTo;						//	Paint from old position to this location
		int m_iTick;							//	Ticks that we have been alive
		int m_iLifeLeft;						//	Ticks left
		CDamageSource m_Source;					//	Object that fired the beam
		CSovereign *m_pSovereign;				//	Sovereign
		CSpaceObject *m_pHit;					//	Object that was hit
		int m_iHitDir;							//	Direction from which object was hit

		int m_xFromOffset;						//	Offsets for painting (volatile)
		int m_yFromOffset;
		int m_xToOffset;
		int m_yToOffset;

		DWORD m_fReflection:1;					//	TRUE if this is a reflection
		DWORD m_dwSpareFlags:31;				//	Flags

	friend class CObjectClass<CBeam>;
	};

class CDisintegrationEffect : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				const CVector &vPos,
				const CVector &vVel,
				const CObjectImageArray &MaskImage,
				int iMaskTick,
				int iMaskRotation,
				CDisintegrationEffect **retpEffect);
		virtual ~CDisintegrationEffect (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage) { return damagePassthrough; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CDisintegrationEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }

	protected:

		//	CSpaceObject virtuals
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnUpdateExtended (void);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);

	private:
		struct SParticle
			{
			int x;							//	Position in 256th of a pixel
			int y;
			int xV;							//	Velocity in 256th of a pixel
			int yV;							//	per tick
			int iTicksLeft;
			COLORREF wColor;
			WORD wSpare;
			};

		CDisintegrationEffect (void);
		void InitParticle (SParticle *pParticle);

		int m_iTick;
		CObjectImageArray m_MaskImage;
		int m_iMaskTick;
		int m_iMaskRotation;

		//	particles
		int m_iParticleCount;
		SParticle *m_pParticles;

	friend class CObjectClass<CDisintegrationEffect>;
	};

class CEffect : public CSpaceObject
	{
	public:
		static ALERROR Create (CEffectCreator *pType,
				CSystem *pSystem,
				CSpaceObject *pAnchor,
				const CVector &vPos,
				const CVector &vVel,
				CEffect **retpEffect);
		virtual ~CEffect (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage) { return damagePassthrough; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }

	protected:

		//	CSpaceObject virtuals
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnUpdateExtended (void);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);

	private:
		CEffect (void);

		IEffectPainter *m_pPainter;
		CSpaceObject *m_pAnchor;
		int m_iLifetime;
		int m_iTick;

	friend class CObjectClass<CEffect>;
	};

class CFractureEffect : public CSpaceObject
	{
	public:
		enum Style
			{
			styleExplosion,
			};

		static ALERROR Create (CSystem *pSystem,
				const CVector &vPos,
				const CVector &vVel,
				const CObjectImageArray &Image,
				int iImageTick,
				int iImageRotation,
				int iStyle,
				CFractureEffect **retpEffect);
		virtual ~CFractureEffect (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage) { return damagePassthrough; }
		virtual Categories GetCategory (void) const { return catFractureEffect; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CFractureEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual void SetAttractor (CSpaceObject *pObj);

	protected:

		//	CSpaceObject virtuals
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnUpdateExtended (void);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);

	private:
		struct SParticle
			{
			int x;							//	Position in 256th of a pixel
			int y;
			int xV;							//	Velocity in 256th of a pixel
			int yV;							//	per tick
			int iTicks;

			int xSrc;						//	Position in source
			int ySrc;
			int iShape;						//	Index of shape
			};

		CFractureEffect (void);
		void InitParticleArray (void);

		int m_iTick;
		int m_iLifeTime;
		CObjectImageArray m_Image;
		int m_iImageTick;
		int m_iImageRotation;
		int m_iStyle;
		int m_iCellSize;

		int m_iParticleCount;
		SParticle *m_pParticles;

		CSpaceObject *m_pAttractor;

	friend class CObjectClass<CFractureEffect>;
	};

class CMarker : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
							   CSovereign *pSovereign,
							   const CVector &vPos,
							   const CVector &vVel,
							   const CString &sName,
							   CMarker **retpMarker);

		//	CSpaceObject virtuals
		virtual CString GetName (DWORD *retdwFlags = NULL) { if (retdwFlags) *retdwFlags = 0; return m_sName; }
		virtual void OnObjLeaveGate (CSpaceObject *pObj);

	protected:
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual CSovereign *GetSovereign (void) const;
		virtual CString GetObjClassName (void) { return CONSTLIT("CMarker"); }
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }

	private:
		CMarker (void);

		CString m_sName;						//	Name
		CSovereign *m_pSovereign;				//	Sovereign

	friend class CObjectClass<CMarker>;
	};

class CMissile : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				int iBonus,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				int iRotation,
				CSpaceObject *pTarget,
				CMissile **retpMissile);
		~CMissile (void);

		//	CSpaceObject virtuals
		virtual CMissile *AsMissile (void) { return this; }
		virtual bool CanMove (void) { return true; }
		virtual void CreateReflection (const CVector &vPos, int iDirection);
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage);
		virtual CString DebugCrashInfo (void);
		virtual void DetonateNow (CSpaceObject *pHit);
		virtual Categories GetCategory (void) const { return catMissile; }
		virtual CSpaceObject *GetDamageCause (void) { return (m_Source.IsEmpty() ? this : m_Source.GetObj()); }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual CString GetName (DWORD *retdwFlags = NULL) { if (retdwFlags) *retdwFlags = 0; return CONSTLIT("missile"); }
		virtual CString GetObjClassName (void) { return CONSTLIT("CMissile"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerStations; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual int GetStealth (void);
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual bool PointInObject (CVector vPos);

	protected:

		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		struct SExhaustParticle
			{
			CVector vPos;				//	Position of particle
			CVector vVel;				//	Velocity of particle
			};

		CMissile (void);

		int ComputeVaporTrail (void);
		void CreateFragments (const CVector &vPos);
		void CreateHitEffect (const CVector &vPos);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		int m_iBonus;							//	Bonus damage
		DestructionTypes m_iCause;				//	Cause of damage
		int m_iLifeLeft;						//	Ticks left
		int m_iHitPoints;						//	HP left
		IEffectPainter *m_pPainter;				//	Effect painter
		CDamageSource m_Source;					//	Object that fired missile
		CSovereign *m_pSovereign;				//	Sovereign
		CSpaceObject *m_pHit;					//	Object hit
		CVector m_vHitPos;						//	Position hit
		int m_iHitDir;							//	Direction hit
		int m_iRotation;						//	Current rotation (degrees)
		CSpaceObject *m_pTarget;				//	Target
		int m_iTick;							//	Number of ticks of life so far
		TQueue<SExhaustParticle> *m_pExhaust;	//	Array of exhaust particles
		CG16bitRegion *m_pVaporTrailRegions;	//	Array of vapor trail regions
		int m_iSavedRotationsCount;				//	Number of saved rotations
		int *m_pSavedRotations;					//	Array of saved rotation angles

		DWORD m_fDestroyed:1;					//	TRUE if destroyed (only around to paint effect)
		DWORD m_fReflection:1;					//	TRUE if this is a reflection
		DWORD m_fDetonate:1;					//	TRUE if we should detonate on next update
		DWORD m_dwSpareFlags:29;				//	Flags

	friend class CObjectClass<CMissile>;
	};

class CParticleDamage : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				int iBonus,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				int iDirection,
				CParticleDamage **retpObj);
		~CParticleDamage (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage) { return damagePassthrough; }
		virtual CSpaceObject *GetDamageCause (void) { return (m_Source.IsEmpty() ? this : m_Source.GetObj()); }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CParticleDamage"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual bool PointInObject (CVector vPos);

	protected:
		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CParticleDamage (void);

		void InitParticles (int iCount, const CVector &vPos, int iDirection);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		int m_iBonus;							//	Bonus
		DestructionTypes m_iCause;				//	Cause of damage
		int m_iTick;							//	Counter
		int m_iLifeLeft;						//	Ticks left
		int m_iEmitTime;						//	Ticks to emit new particles
		CDamageSource m_Source;					//	Object that fired the beam
		CSovereign *m_pSovereign;				//	Sovereign
		int m_iDamage;							//	Damage in points
		int m_iParticleCount;					//	Number of particles generated
		CVector m_vEmitSourcePos;				//	Emit source position
		CVector m_vEmitSourceVel;				//	Emit source velocity
		int m_iEmitDirection;					//	Emit direction

		CVector m_vOldPos;						//	Previous position

		IEffectPainter *m_pPainter;				//	Painter to use for each particle
		CParticleArray m_Particles;

	friend class CObjectClass<CParticleDamage>;
	friend struct SParticle;
	};

class CParticleEffect : public CSpaceObject
	{
	public:
		enum ParticlePainting
			{
			paintDot,
			paintImage,
			paintSmoke,
			paintFlame,
			};

		struct SParticleType
			{
			int iPaintStyle;			//	Painting style
			CObjectImageArray Image;	//	Image for each particle
			COLORREF wColor;				//	Color of particle

			int iRegenerationTimer;		//	Max lifespan of entire group
			int iLifespan;				//	Max lifespan (in ticks)

			Metric rAveSpeed;			//	Average speed of particles
			int iDirection;				//	Direction (-1 == all directions)
			int iDirRange;				//	Span to either side (in degrees)

			Metric rRadius;				//	Radius of particle field
			Metric rHoleRadius;			//	Keep particles out of the center
			Metric rDampening;			//	Velocity dampening constant

			DamageDesc Damage;			//	Damage caused to objects in field

			DWORD m_fMaxRadius:1;		//	TRUE if we should keep particles in radius
			DWORD m_fLifespan:1;		//	TRUE if particles have a lifespan
			DWORD m_fWake:1;			//	TRUE if particles are affected by passing objects
			DWORD m_fRegenerate:1;		//	TRUE if particles regenerate when they die
			DWORD m_fDrag:1;			//	TRUE if particles are subject to drag
			DWORD m_fDamage:1;			//	TRUE if particles cause damage
			};

		static ALERROR Create (CSystem *pSystem,
							   CXMLElement *pDesc,
							   const CVector &vPos,
							   const CVector &vVel,
							   CParticleEffect **retpEffect);
		static ALERROR CreateEmpty (CSystem *pSystem,
									const CVector &vPos,
									const CVector &vVel,
									CParticleEffect **retpEffect);
		static ALERROR CreateExplosion (CSystem *pSystem,
										CSpaceObject *pAnchor,
										const CVector &vPos,
										const CVector &vVel,
										int iCount,
										Metric rAveSpeed,
										int iTotalLifespan,
										int iParticleLifespan,
										const CObjectImageArray &Image,
										CParticleEffect **retpEffect);
		static ALERROR CreateGeyser (CSystem *pSystem,
									 CSpaceObject *pAnchor,
									 const CVector &vPos,
									 const CVector &vVel,
									 int iTotalLifespan,
									 int iParticleCount,
									 ParticlePainting iParticleStyle,
									 int iParticleLifespan,
									 Metric rAveSpeed,
									 int iDirection,
									 int iAngleWidth,
									 CParticleEffect **retpEffect);

		~CParticleEffect (void);

		void AddGroup (SParticleType *pType, int iCount);

		//	CSpaceObject virtuals
		virtual bool CanBeHitBy (const DamageDesc &Damage);
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage);
		virtual CString GetName (DWORD *retdwFlags = NULL) { if (retdwFlags) *retdwFlags = 0; return m_sName; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CParticleEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerSpace; }
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);

	protected:

		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual bool PointInObject (CVector vPos);

	private:

		enum Constants
			{
			ctMaxParticleGroups =	8,
			ctMaxObjsInField =		16,
			};

		struct SParticle
			{
			inline bool IsValid (void) { return iLifeLeft != -1; }

			int iDestiny;
			int iLifeLeft;
			CVector vPos;
			CVector vVel;
			};

		struct SParticleArray
			{
			~SParticleArray (void)
				{
				if (pType)
					delete pType;

				if (pParticles)
					delete [] pParticles;
				}

			SParticleType *pType;
			int iAlive;
			int iCount;
			SParticle *pParticles;

			SParticleArray *pNext;
			};

		CParticleEffect (void);

		void CreateGroup (SParticleType *pType, int iCount, SParticleArray **retpGroup);
		void PaintFlameParticles (SParticleArray *pGroup, CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		void PaintSmokeParticles (SParticleArray *pGroup, CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

		CString m_sName;
		SParticleArray *m_pFirstGroup;
		CSpaceObject *m_pAnchor;

	friend class CObjectClass<CParticleEffect>;
	};

class CPOVMarker : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
							   const CVector &vPos,
							   const CVector &vVel,
							   CPOVMarker **retpMarker);

	protected:
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual CSovereign *GetSovereign (void) const;
		virtual void OnLosePOV (void);
		virtual CString GetObjClassName (void) { return CONSTLIT("CPOVMarker"); }
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }

	private:
		CPOVMarker (void);

	friend class CObjectClass<CPOVMarker>;
	};

class CRadiusDamage : public CSpaceObject
	{
	public:
		static ALERROR Create (CSystem *pSystem,
				CWeaponFireDesc *pDesc,
				int iBonus,
				DestructionTypes iCause,
				const CDamageSource &Source,
				const CVector &vPos,
				const CVector &vVel,
				CSpaceObject *pTarget,
				CRadiusDamage **retpObj);
		~CRadiusDamage (void);

		//	CSpaceObject virtuals
		virtual bool CanMove (void) { return true; }
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage) { return damagePassthrough; }
		virtual CSpaceObject *GetDamageCause (void) { return (m_Source.IsEmpty() ? this : m_Source.GetObj()); }
		virtual CString GetDamageCauseNounPhrase (DWORD dwFlags) { return m_Source.GetDamageCauseNounPhrase(dwFlags); }
		virtual DestructionTypes GetDamageCauseType (void) { return m_iCause; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CString GetObjClassName (void) { return CONSTLIT("CRadiusDamage"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual CSpaceObject *GetSecondarySource (void) { return m_Source.GetSecondaryObj(); }
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual CSpaceObject *GetSource (void) { return m_Source.GetObj(); }
		virtual CWeaponFireDesc *GetWeaponFireDesc (void) { return m_pDesc; }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void OnSystemLoaded (void);
		virtual bool PointInObject (CVector vPos);

	protected:
		//	Virtuals to be overridden
		virtual bool CanHit (CSpaceObject *pObj) { return MissileCanHitObj(pObj, m_Source.GetObj(), m_pDesc->m_bCanDamageSource); }
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CRadiusDamage (void);

		CWeaponFireDesc *m_pDesc;				//	Weapon descriptor
		int m_iBonus;							//	Bonus
		DestructionTypes m_iCause;				//	Cause of damage
		IEffectPainter *m_pPainter;				//	Effect painter
		int m_iTick;							//	Counter
		int m_iLifeLeft;						//	Ticks left
		CDamageSource m_Source;					//	Object that fired the shot
		CSpaceObject *m_pTarget;				//	Target
		CSovereign *m_pSovereign;				//	Sovereign

	friend class CObjectClass<CRadiusDamage>;
	};

class CShip : public CSpaceObject
	{
	public:
		enum InstallArmorStatus
			{
			insArmorOK				= 0,
			insArmorTooHeavy		= 1,	//	Armor is too heavy for ship class
			};

		enum InstallDeviceStatus
			{
			insOK					= 0,	//	Can install this device
			insNotADevice			= 1,	//	Item is not a device
			insNoSlots				= 2,	//	No slots available for any device
			insAlreadyInstalled		= 3,	//	Device is already installed
			insShieldsInstalled		= 4,	//	Shield generator is already installed
			insDriveInstalled		= 5,	//	Drive is already installed
			insLauncherInstalled	= 6,	//	Missile launcher already installed
			insEngineCoreTooWeak	= 7,	//	Engine core is too weak
			insCargoInstalled		= 8,	//	Cargo expansion is already installed
			insReactorInstalled		= 9,	//	Reactor upgrade is already installed
			insCargoMaxExpansion	= 10,	//	Cargo expansion does not fit on ship
			insReactorMaxPower		= 11,	//	Reactor is too powerful for ship
			insNoWeaponSlots		= 12,	//	No slots available for more weapons
			insNoGeneralSlots		= 13,	//	No slots available for non-weapons
			};

		enum RemoveDeviceStatus
			{
			remOK					= 0,	//	Can remove this device
			remTooMuchCargo			= 1,	//	Can't remove cargo expansion (because too much cargo)
			remNotInstalled			= 2,	//	Device is not installed
			};

		static ALERROR CreateFromClass (CSystem *pSystem, 
				CShipClass *pClass, 
				IShipController *pController,
				CSovereign *pSovereign,
				const CVector &vPos, 
				const CVector &vVel, 
				int iRotation, 
				CShip **retpShip);
		virtual ~CShip (void);

		//	Armor methods
		inline CInstalledArmor *GetArmorSection (int iSect) { return (CInstalledArmor *)m_Armor.GetStruct(iSect); }
		inline int GetArmorSectionCount (void) { return m_Armor.GetCount(); }
		void DamageArmor (int iSect, DamageTypes Type, int iDamage);
		void InstallItemAsArmor (CItemListManipulator &ItemList, int iSect);
		bool IsArmorDamaged (int iSect);
		bool IsArmorRepairable (int iSect);
		void RepairAllArmor (void);
		void RepairArmor (int iSect, int iHitPoints);
		void SetCursorAtArmor (CItemListManipulator &ItemList, int iSect);
		void UninstallArmor (CItemListManipulator &ItemList);

		//	Device methods
		InstallArmorStatus CanInstallArmor (const CItem &Item) const;
		InstallArmorStatus CanInstallArmor (CItemListManipulator &ItemList) const;
		InstallDeviceStatus CanInstallDevice (const CItem &Item, bool bReplace = false);
		InstallDeviceStatus CanInstallDevice (CItemListManipulator &ItemList);
		RemoveDeviceStatus CanRemoveDevice (const CItem &Item);
		void DamageExternalDevice (int iDev, SDamageCtx &Ctx);
		void DamageItem (CItemListManipulator &ItemList);
		void EnableDevice (int iDev, bool bEnable = true);
		EnhanceItemStatus EnhanceItem (CItemListManipulator &ItemList, DWORD dwMods);
		void EnhanceItem (CItemListManipulator &ItemList);
		DeviceNames GetDeviceNameForCategory (ItemCategories iCategory);
		int GetItemDeviceName (const CItem &Item) const;
		CItem GetNamedDeviceItem (DeviceNames iDev);
		bool HasNamedDevice (DeviceNames iDev) const;
		inline bool HasSecondaryWeapons (void) { return m_fHasSecondaryWeapons; }
		void InstallItemAsDevice (CItemListManipulator &ItemList, int iDeviceSlot = -1);
		bool IsDeviceSlotAvailable (void);
		void ReadyFirstWeapon (void);
		void ReadyNextWeapon (void);
		void ReadyFirstMissile (void);
		void ReadyNextMissile (void);
		void RechargeItem (CItemListManipulator &ItemList, int iCharges);
		int GetMissileCount (void);
		void RemoveItemAsDevice (CItemListManipulator &ItemList);
		DeviceNames SelectWeapon (int iDev, int iVariant);
		void SetCursorAtDevice (CItemListManipulator &ItemList, int iDev);
		void SetCursorAtNamedDevice (CItemListManipulator &ItemList, DeviceNames iDev);
		CInstalledDevice *GetNamedDevice (DeviceNames iDev);
		CDeviceClass *GetNamedDeviceClass (DeviceNames iDev);
		inline int GetDeviceCount (void) const { return m_iDeviceCount; }
		CInstalledDevice *GetDevice (int iDev) const { return &m_Devices[iDev]; }
		CString GetReactorName (void);
		inline int GetWeaponAimTolerance (DeviceNames iDev) { return 3; }
		bool GetWeaponIsReady (DeviceNames iDev);
		Metric GetWeaponRange (DeviceNames iDev) { return LIGHT_SPEED * 60.0; }
		Metric GetWeaponSpeed (DeviceNames iDev) { return LIGHT_SPEED; }
		bool IsWeaponAligned (DeviceNames iDev, CSpaceObject *pTarget, int *retiAimAngle, int *retiFireAngle = NULL);
		void SetFireDelay (CInstalledDevice *pWeapon);

		//	Settings
		inline bool HasAutopilot (void) { return m_fHasAutopilot; }
		inline void InstallAutopilot (void) { m_fHasAutopilot = true; }

		void ClearBlindness (void);
		void MakeBlind (int iTickCount = -1);

		void ClearDisarmed (void);
		void MakeDisarmed (int iTickCount = -1);

		void ClearLRSBlindness (void);
		inline bool IsLRSBlind (void) { return m_iLRSBlindnessTimer != 0; }
		void MakeLRSBlind (int iTickCount = -1);

		void ClearParalyzed (void);

		bool IsRadiationImmune (void);
		void MakeRadioactive (void);

		inline bool IsInGate (void) const { return m_iExitGateTimer > 0; }
		void SetInGate (CSpaceObject *pGate, int iTickCount);

		inline bool HasTargetingComputer (void) { return m_fHasTargetingComputer; }
		inline void InstallTargetingComputer (void) { m_fHasTargetingComputer = true; }

		inline void ClearSRSEnhanced (void) { m_fSRSEnhanced = false; }
		inline bool IsSRSEnhanced (void) { return (m_fSRSEnhanced ? true : false); }
		inline void SetSRSEnhanced (void) { m_fSRSEnhanced = true; }

		//	Reactor methods
		inline int GetFuelLeft (void) { return (m_fOutOfFuel ? 0 : m_iFuelLeft); }
		int GetMaxFuel (void);
		inline const ReactorDesc *GetReactorDesc (void) { return m_pReactorDesc; }
		inline void TrackFuel (void) { m_fTrackFuel = true; }
		inline void TrackMass (void) { m_fTrackMass = true; }
		int GetPowerConsumption (void);
		bool IsFuelCompatible (const CItem &Item);

		//	Drive methods
		inline int Angle2Direction (int iAngle) const { return m_pClass->Angle2Direction(iAngle); }
		inline int AlignToRotationAngle (int iAngle) const { return m_pClass->AlignToRotationAngle(iAngle); }
		int GetManeuverDelay (void);
		inline int GetRotationAngle (void) { return m_pClass->GetRotationAngle(); }
		inline int GetRotationRange (void) { return m_pClass->GetRotationRange(); }
		inline Metric GetThrust (void) { return m_iThrust; }
		inline bool IsInertialess (void) { return (m_pDriveDesc->fInertialess ? true : false); }
		inline void SetMaxSpeedHalf (void) { m_fHalfSpeed = true; }
		inline void ResetMaxSpeed (void) { m_fHalfSpeed = false; }

		//	Energy field methods
		void AddEnergyField (CEnergyFieldType *pType, int iLifeLeft);

		//	Miscellaneous
		inline CSpaceObject *GetDockedObj (void) { return m_pDocked; }
		inline IShipController *GetController (void) { return m_pController; }
		inline CShipClass *GetClass (void) { return m_pClass; }
		void SetController (IShipController *pController, bool bFreeOldController = true);
		void SetCommandCode (ICCItem *pCode);
		inline void SetDestroyInGate (void) { m_fDestroyInGate = true; }
		inline void SetEncounterInfo (CStationType *pEncounterInfo) { m_pEncounterInfo = pEncounterInfo; }
		inline void SetFollowPlayerThroughGate (void) { m_fFollowPlayerThroughGate = true; }
		inline void SetRotation (int iAngle) { m_iRotation = AlignToRotationAngle(iAngle); }
		void Undock (void);

		//	CSpaceObject virtuals
		virtual bool AbsorbWeaponFire (CInstalledDevice *pWeapon);
		virtual CShip *AsShip (void) { return this; }
		virtual void Behavior (void);
		virtual bool CanAttack (void) const;
		virtual bool CanMove (void) { return true; }
		virtual bool ClassCanAttack (void) { return true; }
		virtual void ConsumeFuel (int iFuel);
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage);
		virtual void DamageDevice (CInstalledDevice *pDevice, CDeviceClass::DeviceFailureType iCause);
		virtual void DeactivateShields (void);
		virtual CString DebugCrashInfo (void);
		virtual void Decontaminate (void);
		virtual void DepleteShields (void);
		virtual void DisableDevice (CInstalledDevice *pDevice);
		virtual CInstalledDevice *FindDevice (const CItem &Item);
		virtual bool FindEventHandler (const CString &sEntryPoint, ICCItem **retpCode) { return m_pClass->FindEventHandler(sEntryPoint, retpCode); }
		virtual CSpaceObject *GetBase (void) const;
		virtual Metric GetCargoSpaceLeft (void);
		virtual Categories GetCategory (void) const { return catShip; }
		virtual DWORD GetClassUNID (void) { return m_pClass->GetUNID(); }
		virtual int GetCombatPower (void);
		virtual CCommunicationsHandler *GetCommsHandler (void) { return m_pClass->GetCommsHandler(); }
		virtual int GetCyberDefenseLevel (void) { return m_pClass->GetCyberDefenseLevel(); }
		virtual DamageTypes GetDamageType (void);
		virtual DWORD GetDefaultBkgnd (void) { return m_pClass->GetDefaultBkgnd(); }
		virtual CXMLElement *GetDockScreen (void);
		virtual CStationType *GetEncounterInfo (void) { return m_pEncounterInfo; }
		virtual CSpaceObject *GetEscortPrincipal (void) const;
		virtual const CString &GetGlobalData (const CString &sAttribute) { return m_pClass->GetGlobalData(sAttribute); }
		virtual const CObjectImageArray &GetImage (void) { return m_pClass->GetImage(); }
		virtual CString GetInstallationPhrase (const CItem &Item) const;
		virtual bool FollowsObjThroughGate (CSpaceObject *pLeader);
		virtual int GetLevel (void) const { return m_pClass->GetLevel(); }
		virtual Metric GetMass (void);
		virtual int GetMaxPower (void) const;
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CVector GetNearestDockVector (CSpaceObject *pRequestingObj);
		virtual CString GetObjClassName (void) { return CONSTLIT("CShip"); }
		virtual int GetOpenDockingPortCount (void) { return m_DockingPorts.GetPortCount(this) - m_DockingPorts.GetPortsInUseCount(this); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return (m_pDocked ? CSystem::layerStations : CSystem::layerShips); }
		virtual int GetPerception (void);
		virtual int GetRotation (void) const { return AlignToRotationAngle(m_iRotation); }
		virtual ScaleTypes GetScale (void) const { return scaleShip; }
		virtual int GetScore (void) { return m_pClass->GetScore(); }
		virtual CXMLElement *GetScreen (const CString &sName) { return m_pClass->GetScreen(sName); }
		virtual int GetShieldLevel (void);
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual const CString &GetStaticData (const CString &sAttribute) { return m_pClass->GetStaticData(sAttribute); }
		virtual int GetStealth (void);
		virtual Metric GetMaxSpeed (void) { return (m_fHalfSpeed ? (m_rMaxSpeed / 2.0) : m_rMaxSpeed); }
		virtual CSpaceObject *GetTarget (void);
		virtual int GetVisibleDamage (void);
		virtual CVector GetWeaponPos (void) const;
		virtual bool HasAttribute (const CString &sAttribute) const;
		virtual bool ImageInObject (const CObjectImageArray &Image, int iTick, int iRotation, CVector vPos);
		virtual bool IsAngryAt (CSpaceObject *pObj);
		virtual bool IsBlind (void) { return m_iBlindnessTimer != 0; }
		virtual bool IsDisarmed (void) { return m_iDisarmedTimer != 0; }
		virtual bool IsIdentified (void) { return m_fIdentified; }
		virtual bool IsObjDocked (CSpaceObject *pObj) { return m_DockingPorts.IsObjDocked(pObj); }
		virtual bool IsOutOfFuel (void) { return m_fOutOfFuel; }
		virtual bool IsParalyzed (void) { return m_iParalysisTimer != 0; }
		virtual bool IsPlayer (void);
		virtual bool IsRadioactive (void) { return (m_fRadioactive ? true : false); }
		virtual bool IsTimeStopImmune (void) { return m_pClass->IsTimeStopImmune(); }
		virtual void LoadImages (void) { m_pClass->LoadImages(); }
		virtual void MarkImages (void) { m_pClass->MarkImages(); }
		virtual bool ObjectInObject (CSpaceObject *pObj);
		virtual void OnComponentChanged (ObjectComponentTypes iComponent);
		virtual void OnDeviceFailure (CInstalledDevice *pDev, int iFailureType);
		virtual void OnDocked (CSpaceObject *pObj);
		virtual void OnDockedObjChanged (void);
		virtual void OnItemsModified (void);
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void OnPlayerObj (CSpaceObject *pPlayer);
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx);
		virtual void OnSystemCreated (void);
		virtual void OnSystemLoaded (void);
		virtual void MakeParalyzed (int iTickCount = -1);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual void PaintMap (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual bool PointInObject (CVector vPos);
		virtual void ProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program);
		virtual void Refuel (int iFuel);
		virtual void Refuel (const CItem &Fuel);
		virtual void RegisterObjectForEvents (CSpaceObject *pObj) { m_RegisteredObjects.Add(pObj); }
		virtual void RepairDamage (int iHitPoints);
		virtual bool RequestDock (CSpaceObject *pObj);
		virtual void SendMessage (CSpaceObject *pSender, const CString &sMsg);
		virtual void SetGlobalData (const CString &sAttribute, const CString &sData) { m_pClass->SetGlobalData(sAttribute, sData); }
		virtual void SetIdentified (bool bIdentified = true) { m_fIdentified = bIdentified; }
		virtual void SetKnown (bool bKnown = true) { m_fKnown = bKnown; }
		virtual void SetName (const CString &sName, DWORD dwFlags = 0) { m_sName = sName; m_dwNameFlags = dwFlags; }
		virtual void SetSovereign (CSovereign *pSovereign) { m_pSovereign = pSovereign; }
		virtual void Undock (CSpaceObject *pObj);
		virtual void UnregisterObjectForEvents (CSpaceObject *pObj) { m_RegisteredObjects.Remove(pObj); }
		virtual void UpdateArmorItems (void);
		virtual void UpdateDockingManeuver(const CVector &vDest, const CVector &vDestVel, int iDestFacing);

	protected:

		//	CSpaceObject virtuals
		virtual bool CanFireOn (CSpaceObject *pObj) { return CanFireOnObjHelper(pObj); }
		virtual void GateHook (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual void OnDestroyed (DestructionTypes iCause, CSpaceObject *pCause, bool *ioResurrectPending, CSpaceObject **retpWreck);
		virtual CSpaceObject *OnGetOrderGiver (void);
		virtual void OnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget);
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate);
		virtual void OnObjJumped (CSpaceObject *pObj);
		virtual void OnObjReconned (CSpaceObject *pObj);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);
		virtual bool OrientationChanged (void);
		virtual void RevertOrientationChange (void);

	private:
		CShip (void);

		void DamageRandomDevice (void);
		bool OnDestroyCheck (DestructionTypes iCause, CSpaceObject *pCause);
		void CalcArmorBonus (void);
		int CalcMaxCargoSpace (void) const;
		void CalcDeviceBonus (void);
		void CalcReactorStats (void);
		int FindDeviceIndex (CInstalledDevice *pDevice) const;
		int FindFreeDeviceSlot (void);
		int FindNextDevice (int iStart, ItemCategories Category);
		int FindRandomDevice (bool bEnabledOnly = false);
		ItemCategories GetCategoryForNamedDevice (DeviceNames iDev);
		void ReactorOverload (void);
		bool ShieldsAbsorbFire (CInstalledDevice *pWeapon);
		void SetDriveDesc (const DriveDesc *pDesc);

		CShipClass *m_pClass;					//	Ship class
		IShipController *m_pController;			//	Controller
		CSovereign *m_pSovereign;				//	Allegiance
		CString m_sName;						//	Ship's name
		DWORD m_dwNameFlags;					//	Name flags

		int m_iRotation:16;						//	Current rotation (in degrees)
		int m_iPrevRotation:16;					//	Previous rotation

		CStructArray m_Armor;					//	Array of CInstalledArmor
		int m_iDeviceCount;						//	Number of devices
		CInstalledDevice *m_Devices;			//	Array of devices
		int m_NamedDevices[devNamesCount];
		const DriveDesc *m_pDriveDesc;			//	Drive descriptor
		const ReactorDesc *m_pReactorDesc;		//	Reactor descriptor
		CEnergyFieldList m_EnergyFields;		//	List of energy fields
		CDockingPorts m_DockingPorts;			//	Docking ports (optionally)
		CSpaceObjectList m_RegisteredObjects;	//	Objects that we fire events for
		CStationType *m_pEncounterInfo;			//	Pointer back to encounter type (generally NULL)

		int m_iFireDelay:16;					//	Ticks until next fire
		int m_iMissileFireDelay:16;				//	Ticks until next missile fire
		int m_iManeuverDelay:16;				//	Ticks until next rotation
		int m_iContaminationTimer:16;			//	Ticks left to live
		int m_iBlindnessTimer:16;				//	Ticks until blindness wears off
												//	(-1 = permanent)
		int m_iParalysisTimer:16;				//	Ticks until paralysis wears off
												//	(-1 = permanent)
		int m_iExitGateTimer:16;				//	Ticks until ship exits gate (in gate until then)
		int m_iDisarmedTimer:16;				//	Ticks until ship can use weapons
												//	(-1 = permanent)
		int m_iLRSBlindnessTimer:16;			//	Ticks until LRS blindness wears off
												//	(-1 = permanent)
		int m_iSpare:16;

		int m_iFuelLeft;						//	Fuel left (kilos)
		Metric m_rItemMass;						//	Total mass of all items (including installed)
		Metric m_rCargoMass;					//	Mass of cargo items (not including installed)
		int m_iPowerDrain;						//	(temp) power consumed (1/10 megawatt)
		int m_iMaxPower;						//	(temp) max power (1/10 megawatt)
		int m_iThrust;							//	Computed thrust
		Metric m_rMaxSpeed;						//	Computed max speed
		int m_iStealth;							//	Computed stealth

		CSpaceObject *m_pDocked;				//	If not NULL, object we are docked to.
		CSpaceObject *m_pExitGate;				//	If not NULL, gate we are about to exit.

		DWORD m_fOutOfFuel:1;					//	TRUE if ship is out of fuel
		DWORD m_fRadioactive:1;					//	TRUE if radioactive
		DWORD m_fHasAutopilot:1;				//	TRUE if ship has autopilot
		DWORD m_fDestroyInGate:1;				//	TRUE if ship has entered a gate
		DWORD m_fHalfSpeed:1;					//	TRUE if ship is at half speed
		DWORD m_fHasTargetingComputer:1;		//	TRUE if ship has targeting computer
		DWORD m_fTrackFuel:1;					//	TRUE if ship keeps track of fuel (only player ship does)
		DWORD m_fHasSecondaryWeapons:1;			//	TRUE if ship has multiple weapons
		DWORD m_fSRSEnhanced:1;					//	TRUE if ship's SRS is enhanced
		DWORD m_fFollowPlayerThroughGate:1;		//	TRUE if ship is following player through gate
		DWORD m_fKnown:1;						//	TRUE if we know about this ship
		DWORD m_fHiddenByNebula:1;				//	TRUE if ship is hidden by nebula
		DWORD m_fTrackMass:1;					//	TRUE if ship keeps track of mass to compute performance
		DWORD m_fIdentified:1;					//	TRUE if player can see ship class, etc.
		DWORD m_dwSpare:20;

	friend class CObjectClass<CShip>;
	};

class CStaticEffect : public CSpaceObject
	{
	public:
		static ALERROR Create (CEffectCreator *pType,
							   CSystem *pSystem,
							   const CVector &vPos,
							   CStaticEffect **retpEffect);
		virtual ~CStaticEffect (void);

		//	CSpaceObject virtuals
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage) { return damagePassthrough; }
		virtual CString GetObjClassName (void) { return CONSTLIT("CStaticEffect"); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return CSystem::layerEffects; }
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans) { }

	protected:

		//	CSpaceObject virtuals
		virtual bool CanHit (CSpaceObject *pObj) { return false; }
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CStaticEffect (void);

		IEffectPainter *m_pPainter;

	friend class CObjectClass<CStaticEffect>;
	};

class CStation : public CSpaceObject
	{
	public:
		static ALERROR CreateFromType (CSystem *pSystem,
				CStationType *pType,
				const CVector &vPos,
				const CVector &vVel,
				CXMLElement *pExtraData,
				CStation **retpStation);
		virtual ~CStation (void);

		inline void ClearFireReconEvent (void) { m_fFireReconEvent = false; }
		inline void ClearReconned (void) { m_fReconned = false; }
		inline int GetDockingPortCount (void) { return m_DockingPorts.GetPortCount(this); }
		inline CSpaceObject *GetShipAtDockingPort (int iPort) { return m_DockingPorts.GetPortObj(this, iPort); }
		inline int GetImageVariant (void) { return m_iVariant; }
		inline int GetMaxStructuralHitPoints (void) { return m_iMaxStructuralHP; }
		inline int GetStructuralHitPoints (void) { return m_iStructuralHP; }
		inline int GetSubordinateCount (void) { return m_Subordinates.GetCount(); }
		inline CSpaceObject *GetSubordinate (int iIndex) { return m_Subordinates.GetObj(iIndex); }
		inline CStationType *GetType (void) { return m_pType; }
		inline bool IsReconned (void) { return (m_fReconned ? true : false); }
		inline bool IsWreck (void) { return (m_dwWreckUNID != 0); }
		inline void MakeRadioactive (void) { m_fRadioactive = true; }
		inline void SetActive (void) { m_fActive = true; }
		inline void SetFireReconEvent (void) { m_fFireReconEvent = true; }
		inline void SetImageVariant (int iVariant) { m_iVariant = iVariant; }
		inline void SetInactive (void) { m_fActive = false; }
		void SetMapOrbit (const CSystem::Orbit &oOrbit);
		inline void SetMass (Metric rMass) { m_rMass = rMass; }
		inline void SetMaxStructuralHitPoints (int iHP) { m_iMaxStructuralHP = iHP; }
		inline void SetNoMapLabel (void) { m_fNoMapLabel = true; }
		inline void SetNoReinforcements (void) { m_fNoReinforcements = true; }
		inline void SetReconned (void) { m_fReconned = true; }
		void SetStargate (const CString &sDestNode, const CString &sDestEntryPoint);
		inline void SetStructuralHitPoints (int iHP) { m_iStructuralHP = iHP; }
		void SetWreckImage (DWORD dwWreckUNID, const CObjectImageArray &Image, int iVariants = 1);

		//	CSpaceObject virtuals
		virtual void AddSubordinate (CSpaceObject *pSubordinate);
		virtual CStation *AsStation (void) { return this; }
		virtual bool CanAttack (void) const;
		virtual bool CanBeDestroyed (void) { return (m_iStructuralHP > 0); }
		virtual bool CanBlock (CSpaceObject *pObj);
		virtual bool CanMove (void) { return IsMobile(); }
		virtual int ChargeMoney (int iValue);
		virtual bool ClassCanAttack (void);
		virtual void CreateRandomDockedShips (IShipGenerator *pGenerator);
		virtual int CreditMoney (int iValue);
		virtual DamageResults Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage);
		virtual CString DebugCrashInfo (void);
		virtual void Decontaminate (void) { m_fRadioactive = false; }
		virtual bool FindEventHandler (const CString &sEntryPoint, ICCItem **retpCode) { return m_pType->FindEventHandler(sEntryPoint, retpCode); }
		virtual int GetBalance (void);
		virtual Categories GetCategory (void) const { return catStation; }
		virtual DWORD GetDefaultBkgnd (void) { return m_pType->GetDefaultBkgnd(); }
		virtual int GetBuyPrice (const CItem &Item, int *retiMaxCount = NULL);
		virtual CXMLElement *GetDockScreen (void);
		virtual CStationType *GetEncounterInfo (void) { return m_pType; }
		virtual const CString &GetGlobalData (const CString &sAttribute) { return m_pType->GetGlobalData(sAttribute); }
		virtual const CObjectImageArray &GetImage (void) { return m_pType->GetImage(GetImageVariant()); }
		virtual int GetLevel (void) const { return m_pType->GetLevel(); }
		virtual const CSystem::Orbit *GetMapOrbit (void) { return m_pMapOrbit; }
		virtual Metric GetMass (void) { return m_rMass; }
		virtual CString GetName (DWORD *retdwFlags = NULL);
		virtual CVector GetNearestDockVector (CSpaceObject *pRequestingObj);
		virtual CString GetObjClassName (void) { return CONSTLIT("CStation"); }
		virtual int GetOpenDockingPortCount (void) { return m_DockingPorts.GetPortCount(this) - m_DockingPorts.GetPortsInUseCount(this); }
		virtual CSystem::LayerEnum GetPaintLayer (void) { return (m_pType->IsBackgroundObject() ? CSystem::layerBackground : CSystem::layerStations); }
		virtual IShipGenerator *GetRandomEncounterTable (int *retiFrequency = NULL);
		virtual ScaleTypes GetScale (void) const { return m_Scale; }
		virtual CXMLElement *GetScreen (const CString &sName) { return m_pType->GetScreen(sName); }
		virtual int GetSellPrice (const CItem &Item);
		virtual CSovereign *GetSovereign (void) const { return m_pSovereign; }
		virtual COLORREF GetSpaceColor (void) { return m_pType->GetSpaceColor(); }
		virtual CString GetStargateID (void) const;
		virtual const CString &GetStaticData (const CString &sAttribute) { return m_pType->GetStaticData(sAttribute); }
		virtual int GetVisibleDamage (void);
		virtual bool HasAttribute (const CString &sAttribute) const;
		virtual bool HasMapLabel (void);
		virtual bool ImageInObject (const CObjectImageArray &Image, int iTick, int iRotation, CVector vPos);
		virtual bool IsAbandoned (void) const { return m_iHitPoints == 0; }
		virtual bool IsAngryAt (CSpaceObject *pObj) { return (IsEnemy(pObj) || IsBlacklisted(pObj)); }
		virtual bool IsKnown (void) { return m_fKnown; }
		virtual bool IsObjDocked (CSpaceObject *pObj) { return m_DockingPorts.IsObjDocked(pObj); }
		virtual bool IsRadioactive (void) { return (m_fRadioactive ? true : false); }
		virtual bool IsStargate (void) const { return !m_sStargateDestNode.IsBlank() && m_fActive; }
		virtual bool IsTimeStopImmune (void) { return m_pType->IsTimeStopImmune(); }
		virtual void LoadImages (void) { m_pType->LoadImages(m_iVariant); }
		virtual void MarkImages (void) { m_pType->MarkImages(m_iVariant); }
		virtual bool ObjectInObject (CSpaceObject *pObj);
		virtual void OnDestroyed (DestructionTypes iCause, CSpaceObject *pCause, bool *ioResurrectPending, CSpaceObject **retpWreck);
		virtual void OnObjBounce (CSpaceObject *pObj, const CVector &vPos);
		virtual void OnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget);
		virtual void OnObjLeaveGate (CSpaceObject *pObj);
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx);
		virtual void OnSystemCreated (void);
		virtual void PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual void PaintMap (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans);
		virtual void PlaceAtRandomDockPort (CSpaceObject *pObj) { m_DockingPorts.DockAtRandomPort(this, pObj); }
		virtual bool PointInObject (CVector vPos);
		virtual void RegisterObjectForEvents (CSpaceObject *pObj) { m_RegisteredObjects.Add(pObj); }
		virtual bool RequestDock (CSpaceObject *pObj);
		virtual bool RequestGate (CSpaceObject *pObj);
		virtual void SetGlobalData (const CString &sAttribute, const CString &sData) { m_pType->SetGlobalData(sAttribute, sData); }
		virtual void SetKnown (bool bKnown = true) { m_fKnown = bKnown; }
		virtual void SetMapLabelPos (int x, int y) { m_xMapLabel = x; m_yMapLabel = y; }
		virtual void SetName (const CString &sName, DWORD dwFlags = 0);
		virtual void SetSovereign (CSovereign *pSovereign) { m_pSovereign = pSovereign; }
		virtual bool SupportsGating (void) { return IsStargate(); }
		virtual void Undock (CSpaceObject *pObj);
		virtual void UnregisterObjectForEvents (CSpaceObject *pObj) { m_RegisteredObjects.Remove(pObj); }

	protected:

		//	CSpaceObject virtuals
		virtual bool CanFireOn (CSpaceObject *pObj) { return CanFireOnObjHelper(pObj); }
		virtual void OnMove (const CVector &vOldPos, Metric rSeconds);
		virtual void ObjectDestroyedHook (const SDestroyCtx &Ctx);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual void OnItemsModified (void);
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate);
		virtual void OnObjJumped (CSpaceObject *pObj);
		virtual void OnObjReconned (CSpaceObject *pObj);
		virtual void OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnUpdate (Metric rSecondsPerTick);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum Constants
			{
			maxRegisteredObjects	= 12,
			maxDevices				= 8,
			};

		CStation (void);

		int CalcNumberOfShips (void);
		void CreateDestructionEffect (void);
		void CreateEjectaFromDamage (int iDamage, const CVector &vHitPos, int iDirection, const DamageDesc &Damage);
		void CreateStructuralDestructionEffect (void);
		ALERROR CreateMapImage (void);

		const CObjectImageArray &GetImage (int *retiTick, int *retiRotation);

		void Blacklist (CSpaceObject *pObj);
		bool IsBlacklisted (CSpaceObject *pObj);

		void FriendlyFire (CSpaceObject *pAttacker);
		void RaiseAlert (CSpaceObject *pTarget);
		void SetAngry (void);

		void UpdateAttacking (int iTick);
		void UpdateReinforcements (int iTick);

		CStationType *m_pType;					//	Station type
		CString m_sName;						//	Station name
		CSovereign *m_pSovereign;				//	Allegiance
		ScaleTypes m_Scale;						//	Scale of station
		Metric m_rMass;							//	Mass of station

		int m_iVariant;							//	Image variant to display
		int m_iDestroyedAnimation;				//	Frames left of destroyed animation
		CSystem::Orbit *m_pMapOrbit;			//	Orbit to draw on map
		int m_xMapLabel;						//	Name label in map view
		int m_yMapLabel;

		CString m_sStargateDestNode;			//	Destination node
		CString m_sStargateDestEntryPoint;		//	Destination entry point

		CArmorClass *m_pArmorClass;				//	Armor class
		int m_iHitPoints;						//	Total hit points (0 = station abandoned)
		int m_iStructuralHP;					//	Structural hp (0 = station cannot be destroyed)
		int m_iMaxStructuralHP;					//	Max structural hp

		CInstalledDevice *m_pDevices;			//	Array of devices

		CDockingPorts m_DockingPorts;

		CSpaceObject *m_pTarget;				//	Target to hit (by our weapons)
		CSpaceObjectList m_Subordinates;		//	List of subordinates
		CSpaceObjectList m_RegisteredObjects;	//	Objects that we fire events for
		CSpaceObjectList m_Targets;				//	Targets to destroy (by our ships)

		CAttackDetector m_Blacklist;			//	Player blacklisted
		int m_iAngryCounter;					//	Attack cycles until station is not angry
		int m_iReinforceRequestCount;			//	Number of times we've requested reinforcements
		int m_iBalance;							//	Money left

		DWORD m_fArmed:1;						//	TRUE if station has weapons
		DWORD m_fKnown:1;						//	TRUE if known to the player
		DWORD m_fNoMapLabel:1;					//	Do not show map label
		DWORD m_fCustomImage:1;					//	TRUE if we should use station image instead of type
		DWORD m_fActive:1;						//	TRUE if stargate is active
		DWORD m_fNoReinforcements:1;			//	Do not send reinforcements
		DWORD m_fRadioactive:1;					//	TRUE if radioactive
		DWORD m_fReconned:1;					//	TRUE if reconned by player
		DWORD m_fFireReconEvent:1;				//	If TRUE, fire OnReconned
		DWORD m_fNoArticle:1;					//	Do not use "a" or "the"
		DWORD m_dwSpare:22;

		//	Wreck image
		DWORD m_dwWreckUNID;					//	UNID of wreck class (0 if none)
		CObjectImageArray m_Image;				//	Image of station

		CG16bitImage m_MapImage;				//	Image for the map (if star or world)

	friend class CObjectClass<CStation>;
	};

//	AIs

class IShipController
	{
	public:
		enum OrderTypes
			{
			orderNone,					//	Nothing to do

			orderGuard,					//	pTarget = target to guard
			orderDock,					//	pTarget = station to dock with
			orderDestroyTarget,			//	pTarget = target to destroy
			orderWait,					//	dwData = seconds to wait (0 == indefinitely)
			orderGate,					//	Gate out of the system

			orderGateOnThreat,			//	Gate out of system if threatened
			orderGateOnStationDestroyed,//	Gate out if station is destroyed
			orderPatrol,				//	pTarget = target to orbit; dwData = radius (light-seconds)
			orderEscort,				//	pTarget = target to escort
			orderScavenge,				//	Scavenge for scraps

			orderFollowPlayerThroughGate,//	Follow the player through the nearest gate
			orderAttackNearestEnemy,	//	Attack the nearest enemy
			orderTradeRoute,			//	Moves from station to station
			orderWander,				//	Wander around system, avoiding enemies
			orderLoot,					//	pTarget = target to loot

			orderHold,					//	Stay in place indefinitely
			orderMine,					//	Mine asteroids; pTarget = base
			orderWaitForPlayer,			//	Wait for player to return to the system
			orderDestroyPlayerOnReturn,	//	Wait to attack player when she returns
			orderFollow,				//	pTarget = target to follow (like escort, but no defense)

			orderNavPath,				//	dwData = nav path ID to follow
			orderGoTo,					//	Go to the given object (generally a marker)
			orderWaitForTarget,			//	Hold until pTarget is in LRS range
			orderWaitForEnemy,			//	Hold until any enemy is in LRS range
			orderBombard,				//	Hold and attack target from here; pTarget = target

			orderApproach,				//	Get within dwData ls of pTarget
			orderAimAtTarget,			//	Same as orderDestroyTarget, but we never fire
			};

		enum ManeuverTypes
			{
			NoRotation,
			RotateLeft,
			RotateRight,

			ThrustLeft,
			ThrustRight
			};

		virtual ~IShipController (void) { }

		virtual void Behavior (void) { }
		virtual CString DebugCrashInfo (void) { return NULL_STR; }
		virtual bool FollowsObjThroughGate (CSpaceObject *pLeader) { return false; }
		virtual int GetCombatPower (void) = 0;
		virtual CSpaceObject *GetEscortPrincipal (void) const { return NULL; }
		virtual ManeuverTypes GetManeuver (void) = 0;
		virtual bool GetThrust (void) = 0;
		virtual bool GetReverseThrust (void) = 0;
		virtual bool GetStopThrust (void) = 0;
		virtual bool GetMainFire (void) = 0;
		virtual bool GetMissileFire (void) = 0;
		virtual bool GetDeviceActivate (void) = 0;
		virtual int GetFireDelay (void) { return 0; }
		virtual int GetFireRateAdj (void) { return 10; }
		virtual CSpaceObject *GetBase (void) const { return NULL; }
		virtual CSpaceObject *GetOrderGiver (void) = 0;
		virtual CSpaceObject *GetTarget (void) { return NULL; }
		virtual bool GetWeaponFire (int iDev) = 0;
		virtual CSpaceObject *GetWeaponTarget (int iDev) { return GetTarget(); }
		virtual bool IsPlayer (void) { return false; }
		virtual void ReadFromStream (SLoadCtx &Ctx) { ASSERT(false); }
		virtual void SetCommandCode (ICCItem *pCode) { }
		virtual void SetManeuver (ManeuverTypes iManeuver) { }
		virtual void SetShipToControl (CShip *pShip) { }
		virtual void SetThrust (bool bThrust) { }
		virtual void WriteToStream (IWriteStream *pStream) { ASSERT(false); }

		virtual void AddOrder(OrderTypes Order, CSpaceObject *pTarget, DWORD dwData, bool bAddBefore = false) { }
		virtual void CancelAllOrders (void) { }
		virtual void CancelCurrentOrder (void) { }
		virtual OrderTypes GetCurrentOrderEx (CSpaceObject **retpTarget = NULL, DWORD *retdwData = NULL) { return orderNone; }

		//	Events

		virtual void OnArmorRepaired (int iSection) { }
		virtual void OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage) { }
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2) { return resNoAnswer; }
		virtual void OnComponentChanged (ObjectComponentTypes iComponent) { }
		virtual void OnDamaged (CSpaceObject *pCause, int iSection, const DamageDesc &Damage) { }
		virtual void OnDeviceEnabledDisabled (int iDev, bool bEnabled) { }
		virtual bool OnDestroyCheck (DestructionTypes iCause, CSpaceObject *pCause) { return true; }
		virtual void OnDestroyed (DestructionTypes iCause, CSpaceObject *pCause, bool *ioResurrectPending) { }
		virtual void OnDocked (CSpaceObject *pObj) { }
		virtual void OnDockedObjChanged (void) { }
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate) { }
		virtual void OnFuelLowWarning (int iSeq) { }
		virtual void OnLifeSupportWarning (int iSecondsLeft) { }
		virtual void OnMessage (CSpaceObject *pSender, const CString &sMsg) { }
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate) { }
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx) { }
		virtual void OnPlayerObj (CSpaceObject *pPlayer) { }
		virtual void OnProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program) { }
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx) { }
		virtual void OnSystemLoaded (void) { }
		virtual void OnRadiationWarning (int iSecondsLeft) { }
		virtual void OnRadiationCleared (void) { }
		virtual void OnReactorOverloadWarning (int iSeq) { }
		virtual void OnWeaponStatusChanged (void) { }
		virtual void OnWreckCreated (CSpaceObject *pWreck) { }
		virtual void OnBlindnessChanged (bool bBlind) { }
		virtual void OnDeviceFailure (CInstalledDevice *pDev, int iType) { }
	};

extern SAISettings g_DefaultAISettings;

class CBaseShipAI : public CObject, public IShipController
	{
	public:
		CBaseShipAI (IObjectClass *pClass);
		virtual ~CBaseShipAI (void);

		//	IShipController virtuals
		virtual bool FollowsObjThroughGate (CSpaceObject *pLeader);
		virtual int GetCombatPower (void);
		virtual bool GetDeviceActivate (void) { return m_fDeviceActivate; }
		virtual CSpaceObject *GetEscortPrincipal (void) const;
		virtual int GetFireRateAdj (void) { return m_pAISettings->iFireRateAdj; }
		virtual bool GetMainFire (void) { return IsFireWeaponFlagSet(devPrimaryWeapon); }
		virtual ManeuverTypes GetManeuver (void) { return m_iManeuver; }
		virtual bool GetMissileFire (void) { return IsFireWeaponFlagSet(devMissileWeapon); }
		virtual CSpaceObject *GetOrderGiver (void) { return m_pShip; }
		virtual bool GetReverseThrust (void) { return false; }
		virtual bool GetStopThrust (void) { return false; }
		virtual bool GetThrust (void) { return (m_iThrustDir == constAlwaysThrust || m_iThrustDir == m_pShip->GetRotation()); }
		virtual bool GetWeaponFire (int iDev) { return IsFireWeaponFlagSet(iDev); }
		virtual CSpaceObject *GetWeaponTarget (int iDev);
		virtual void OnDocked (CSpaceObject *pObj);
		virtual void OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate);
		virtual void OnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate);
		virtual void OnObjDestroyed (const SDestroyCtx &Ctx);
		virtual void OnPlayerObj (CSpaceObject *pPlayer);
		virtual void OnStationDestroyed (const SDestroyCtx &Ctx);
		virtual void OnSystemLoaded (void) { CalcInvariants(); OnSystemLoadedNotify(); }
		virtual void SetManeuver (ManeuverTypes iManeuver) { m_iManeuver = iManeuver; }
		virtual void SetShipToControl (CShip *pShip);
		virtual void SetThrust (bool bThrust) { m_iThrustDir = (bThrust ? constAlwaysThrust : constNeverThrust); }
		virtual void ReadFromStream (SLoadCtx &Ctx);
		virtual void WriteToStream (IWriteStream *pStream);

		virtual void AddOrder(IShipController::OrderTypes Order, CSpaceObject *pTarget, DWORD dwData, bool bAddBefore = false);
		virtual void CancelAllOrders (void);
		virtual void CancelCurrentOrder (void);
		virtual OrderTypes GetCurrentOrderEx (CSpaceObject **retpTarget = NULL, DWORD *retdwData = NULL);

	protected:
		enum Constants
			{
			constAlwaysThrust = -1,
			constNeverThrust = 1000,
			};

		struct OrderEntry
			{
			IShipController::OrderTypes Order;
			CSpaceObject *pTarget;
			DWORD dwData;
			};

		void CalcBestWeapon (CSpaceObject *pTarget, Metric rTargetDist2);
		CSpaceObject *CalcEnemyShipInRange (CSpaceObject *pCenter, Metric rRange, CSpaceObject *pExcludeObj = NULL);
		bool CalcFlockingFormation (CSpaceObject *pLeader,
									Metric rFOVRange,
									Metric rSeparationRange,
									CVector *retvPos, 
									CVector *retvVel, 
									int *retiFacing);
		void CalcInvariants (void);
		bool CalcNavPath (CSpaceObject *pTo);
		void CalcNavPath (CSpaceObject *pFrom, CSpaceObject *pTo);
		void CalcNavPath (CNavigationPath *pPath);
		void CalcShieldState (void);
		Metric CalcShipIntercept (const CVector &vRelPos, const CVector &vAbsVel, Metric rMaxSpeed);
		int CalcWeaponScore (CSpaceObject *pTarget, CInstalledDevice *pWeapon, Metric rTargetDist2);
		void CancelDocking (CSpaceObject *pTarget);
		bool CheckForEnemiesInRange (CSpaceObject *pCenter, Metric rRange, int iInterval, CSpaceObject **retpTarget);
		bool CheckOutOfRange (CSpaceObject *pTarget, Metric rRange, int iInterval);
		bool CheckOutOfZone (CSpaceObject *pBase, Metric rInnerRadius, Metric rOuterRadius, int iInterval);
		inline void ClearBestWeapon (void) { m_fRecalcBestWeapon = true; }
		inline void ClearFireWeaponFlags (void) { m_dwFireWeapon = 0; }
		inline void ClearNavPath (void) { m_pNavPath = NULL; m_iNavPathPos = -1; }
		void FireOnOrderChanged (void);
		void FireOnOrdersCompleted (void);
		IShipController::OrderTypes GetCurrentOrder (void) const;
		inline DWORD GetCurrentOrderData (void) const { return ((OrderEntry *)m_Orders.GetStruct(0))->dwData; }
		inline CSpaceObject *GetCurrentOrderTarget (void) const { return ((OrderEntry *)m_Orders.GetStruct(0))->pTarget; }
		inline Metric GetDistance (CSpaceObject *pObj) const { return (pObj->GetPos() - m_pShip->GetPos()).Length(); }
		inline Metric GetDistance2 (CSpaceObject *pObj) const { return (pObj->GetPos() - m_pShip->GetPos()).Length2(); }
		inline int GetOrderCount (void) const { return m_Orders.GetCount(); }
		inline bool IsImmobile (void) const { return m_fImmobile; }
		void HandleFriendlyFire (CSpaceObject *pOrderGiver);
		void ImplementAttackNearestTarget (Metric rMaxRange, CSpaceObject **iopTarget, CSpaceObject *pExcludeObj = NULL);
		void ImplementAttackTarget (CSpaceObject *pTarget, bool bMaintainCourse = false, bool bDoNotShoot = false);
		void ImplementCloseOnImmobileTarget (CSpaceObject *pTarget, 
											 const CVector &vTarget, 
											 Metric rTargetDist2);
		void ImplementCloseOnTarget (CSpaceObject *pTarget, 
									 const CVector &vTarget, 
									 Metric rTargetDist2, 
									 bool bFlank = false);
		void ImplementDocking (CSpaceObject *pTarget);
		void ImplementEscort (CSpaceObject *pBase, CSpaceObject **iopTarget);
		void ImplementEscortManeuvers (CSpaceObject *pTarget, const CVector &vTarget);
		void ImplementEvasiveManeuvers (CSpaceObject *pTarget);
		void ImplementFireOnTarget (CSpaceObject *pTarget, bool *retbOutOfRange = NULL);
		void ImplementFireOnTargetsOfOpportunity (CSpaceObject *pTarget = NULL, CSpaceObject *pExcludeObj = NULL);
		void ImplementFireWeapon (DeviceNames iDev = devNone);
		void ImplementFireWeaponOnTarget (int iWeapon,
										  int iWeaponVariant,
										  CSpaceObject *pTarget,
										  const CVector &vTarget,
										  Metric rTargetDist2,
										  int *retiFireDir = NULL,
										  bool bDoNotShoot = false);
		void ImplementFollowNavPath (bool *retbAtDestination = NULL);
		void ImplementFormationManeuver (const CVector vDest, 
										 const CVector vDestVel, 
										 int iDestFacing,
										 bool *retbInFormation = NULL);
		void ImplementGating (CSpaceObject *pTarget);
		void ImplementHold (bool *retbInPlace = NULL);
		void ImplementManeuver (int iDir, bool bThrust, bool bNoThrustThroughTurn = false);
		void ImplementOrbit (CSpaceObject *pBase, Metric rDistance);
		void ImplementSpiralIn (const CVector &vTarget);
		void ImplementSpiralOut (const CVector &vTarget, int iTrajectory = 30);
		void ImplementStop (void);
		void ImplementTurnTo (int iRotation);
		inline bool IsDockingRequested (void) { return (m_fDockingRequested ? true : false); }
		inline bool IsFireWeaponFlagSet (int iDev) { return ((m_dwFireWeapon & (1 << iDev)) ? true : false); }
		void ResetBehavior (void);
		void SetCurrentOrderData (DWORD dwData);
		inline void SetFireWeaponFlag (int iDev) { m_dwFireWeapon |= (1 << iDev); }
		void UpgradeShieldBehavior (void);
		void UpgradeWeaponBehavior (void);
		void UseItemsBehavior (void);

		//	CBaseShipAI virtuals
		virtual void OnDockedEvent (CSpaceObject *pObj) { }
		virtual void OnOrderChanged (void) { }
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx) { }
		virtual void OnReadFromStream (SLoadCtx &Ctx) { }
		virtual void OnSystemLoadedNotify (void) { }
		virtual void OnWriteToStream (IWriteStream *pStream) { }

		CShip *m_pShip;							//	Ship that we're controlling
		const SAISettings *m_pAISettings;		//	Settings
		CStructArray m_Orders;					//	Ordered list of orders

		ManeuverTypes m_iManeuver;
		int m_iThrustDir;
		int m_iManeuverCounter;					//	Counter used by maneuvers
		DWORD m_dwFireWeapon;
		ManeuverTypes m_iLastTurn;				//	Last turn direction
		int m_iLastTurnCount;					//	Number of updates turning
		CAttackDetector m_Blacklist;			//	Player blacklisted
		CNavigationPath *m_pNavPath;			//	Current navigation path
		int m_iNavPathPos;						//	-1 = not in nav path

		//	Some cached calculations
		Metric m_rPrimaryAimRange2;				//	If further than this, close on target
		Metric m_rFlankDist;					//	Flank distance
		int m_iMaxTurnCount;					//	Max ticks turning in the same direction
		DeviceNames m_iBestWeapon;
		CInstalledDevice *m_pBestWeapon;
		Metric m_rBestWeaponRange;				//	Range of best weapon
		Metric m_rMaxWeaponRange;				//	Range of longest range primary weapon
		int m_iBestNonLauncherWeaponLevel;		//	Level of best non-launcher weapon
		CInstalledDevice *m_pShields;			//	Shields (NULL if none)

		//	Flags
		DWORD m_fThrustThroughTurn:1;
		DWORD m_fRecalcBestWeapon:1;
		DWORD m_fDeviceActivate:1;
		DWORD m_fDockingRequested:1;
		DWORD m_fSuperconductingShields:1;
		DWORD m_fInOnOrderChanged:1;
		DWORD m_fInOnOrdersCompleted:1;
		DWORD m_fWaitForShieldsToRegen:1;
		DWORD m_fHasMultipleWeapons:1;
		DWORD m_fAvoidExplodingStations:1;
		DWORD m_fImmobile:1;
		DWORD m_fSpare:21;
	};

class CAutonAI : public CBaseShipAI
	{
	public:
		CAutonAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetOrderGiver (void);
		virtual CSpaceObject *GetTarget (void);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual void OnPlayerObj (CSpaceObject *pPlayer);

	protected:
		//	CBaseShipAI overrides
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateEscorting,
			stateAttackingTarget,
			stateOnCourseForStargate,
			stateFollowing,
			stateWaiting,
			};

		void BehaviorStart (void);
		void SetState (StateTypes State);

		StateTypes m_State;
		CSpaceObject *m_pDest;
		CSpaceObject *m_pTarget;

	friend class CObjectClass<CAutonAI>;
	};

class CFerianShipAI : public CBaseShipAI
	{
	public:
		CFerianShipAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetBase (void) const { return m_pBase; }
		virtual CSpaceObject *GetTarget (void) { return m_pTarget; }
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateOnCourseForMine,
			stateMining,
			stateAttackingThreat,
			stateOnCourseForStargate,
			stateAvoidThreat,
			stateAttackingTarget,
			stateWaitForPlayerAtGate,
			};

		void BehaviorStart (void);
		CSpaceObject *FindRandomAsteroid (void);
		bool InRangeOfThreat (CSpaceObject **retpThreat);
		void SetState (StateTypes State);

		StateTypes m_State;						//	Ship's current state
		CSpaceObject *m_pBase;
		CSpaceObject *m_pTarget;

	friend class CObjectClass<CFerianShipAI>;
	};

class CFleetCommandAI : public CBaseShipAI
	{
	public:
		CFleetCommandAI (void);
		virtual ~CFleetCommandAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetTarget (void) { return m_pTarget; }
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnDockedEvent (CSpaceObject *pObj);
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateAttackAtWill,
			stateChargeInFormation,
			stateFormAtRallyPoint,
			stateAttackFromRallyPoint,
			stateOnCourseForStargate,
			stateWaiting,
			stateWaitingForThreat,
			};

		struct STarget
			{
			CSpaceObject *pTarget;
			int iAssignedTo;					//	Power points assigned to destroy this target
			int iKilled;						//	Power points killed by this asset
			};

		struct SAsset
			{
			CSpaceObject *pAsset;
			int iFormationPos;
			CSpaceObject *pTarget;				//	Assigned target (NULL = none)
			};

		SAsset *AddAsset (CSpaceObject *pAsset);
		void AddTarget (CSpaceObject *pTarget);
		void BehaviorStart (void);
		void ComputeCombatPower (int *retiAssetPower, int *retiTargetPower);
		bool ComputeFormationReady (void);
		CVector ComputeRallyPoint (CSpaceObject *pTarget, int *retiFacing = NULL);
		CVector ComputeRallyPointEx (int iBearing, CSpaceObject *pTarget, int *retiFacing = NULL);
		SAsset *FindAsset (CSpaceObject *pAsset, int *retiIndex = NULL);
		STarget *FindTarget (CSpaceObject *pTarget, int *retiIndex = NULL);
		void ImplementAttackAtWill (void);
		void ImplementAttackFromRallyPoint (void);
		void ImplementChargeInFormation (void);
		void ImplementFormAtRallyPoint (void);
		void ImplementWaitAtPosition (void);
		void OrderAllFormUp (bool bResetFormation = false);
		void OrderAllOpenFire (void);
		void OrderAttackTarget (CSpaceObject *pTarget);
		void RemoveAsset (int iIndex);
		bool RemoveAssetObj (CSpaceObject *pObj);
		void RemoveTarget (int iIndex);
		void SetState (StateTypes State);
		void UpdateAttackTargets (void);
		void UpdateTargetList (void);
		void UpdateTargetListAndPotentials (void);

		StateTypes m_State;

		CSpaceObject *m_pTarget;
		CSpaceObject *m_pObjective;
		int m_iCounter;

		int m_iStartingAssetCount;
		int m_iAssetCount;
		int m_iAssetAlloc;
		SAsset *m_pAssets;

		int m_iTargetCount;
		int m_iTargetAlloc;
		STarget *m_pTargets;

		CVector m_vThreatPotential;
		CVector m_vRallyPoint;
		int m_iRallyFacing;

		DWORD m_fOpenFireOrdered:1;
		DWORD m_dwSpare:31;

	friend class CObjectClass<CFleetCommandAI>;
	};

class CFleetShipAI : public CBaseShipAI
	{
	public:
		CFleetShipAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetTarget (void);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnDockedEvent (CSpaceObject *pObj);
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateKeepFormation,
			stateAttackAtWill,
			stateAttackInFormation,
			stateAttackTarget,
			stateOnCourseForStargate,				//	Heading for m_pDest to gate out
			stateOnCourseForDocking,				//	Heading for m_pDest to dock
			stateWaitForPlayerAtGate,
			stateOnPatrolOrbit,
			stateAttackOnPatrol,
			};

		void BehaviorStart (void);
		CVector ComputeAttackPos (CSpaceObject *pTarget, Metric rDist, int *retiFacing);
		void ImplementFireOnNearestTarget (void);
		void ImplementKeepFormation (bool *retbInFormation = NULL);
		void SetState (StateTypes State);

		StateTypes m_State;						//	Ship's current state
		CSpaceObject *m_pLeader;				//	Group leader
		int m_iFormation;						//	Formation to use
		int m_iPlace;							//	Place in formation
		int m_iCounter;							//	Misc counter
		int m_iAngle;							//	Misc angle
		CVector m_vVector;				//	Misc vector

		CSpaceObject *m_pTarget;
		CSpaceObject *m_pDest;

	friend class CObjectClass<CFleetShipAI>;
	};

class CGaianProcessorAI : public CBaseShipAI
	{
	public:
		CGaianProcessorAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetTarget (void);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnSystemLoadedNotify (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateLookingForWreck,
			statePulverizingWreck,
			stateConsumingWreck
			};

		void BehaviorStart (void);
		void CalcDevices (void);
		void SetState (StateTypes State);

		StateTypes m_State;						//	Ship's current state
		Metric m_rOrbitRadius;					//	Current orbital radius
		CSpaceObject *m_pTarget;
		CSpaceObject *m_pDest;					//	Scrap to eat
		int m_iStartTime;						//	Start time for various actions

		//	Volatiles
		int m_iDestructorDev;					//	Destructor weapon
		int m_iDisablerDev;						//	Disabling weapon
		DWORD m_dwAmmo;							//	Ammo to manufacture

	friend class CObjectClass<CGaianProcessorAI>;
	};

class CGladiatorAI : public CBaseShipAI
	{
	public:
		CGladiatorAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetTarget (void);

	protected:
		//	CBaseShipAI overrides
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateAttackingTarget,
			stateOnCourseForDocking,
			stateOnCourseForStargate,
			};

		void BehaviorStart (void);
		CVector ComputePotential (void);
		void SetState (StateTypes State);

		StateTypes m_State;
		CSpaceObject *m_pDest;
		CSpaceObject *m_pTarget;
		CVector m_vPotential;

	friend class CObjectClass<CGladiatorAI>;
	};

class CStandardShipAI : public CBaseShipAI
	{
	public:
		CStandardShipAI (void);
		virtual ~CStandardShipAI (void);

		static void SetDebugShip (CShip *pShip);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetBase (void) const;
		virtual CSpaceObject *GetTarget (void);
		virtual CSpaceObject *GetWeaponTarget (int iDev);
		virtual void SetCommandCode (ICCItem *pCode);

		virtual void OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);
		virtual void OnWeaponStatusChanged (void) { ClearBestWeapon(); }

	protected:
		//	CBaseShipAI overrides
		virtual void OnDockedEvent (CSpaceObject *pObj);
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateAttackingOnPatrol,					//	Attacking m_pTarget while on patrol
			stateAttackingTarget,					//	Attacking m_pTarget
			stateAttackingThreat,					//	Attacking m_pTarget to protect principal
			stateDeterTarget,						//	Attack m_pTarget, but stop if target doesn't attack back
			stateDeterTargetNoChase,				//	Attack m_pTarget, but don't chase it
			stateEscorting,							//	Escorting m_pDest
			stateHolding,							//	Doing nothing indefinitely
			stateLookingForLoot,					//	Looking for loot in the system
			stateLooting,							//	Looting stuff from station
			stateLootingOnce,						//	Looting stuff from station
			stateMaintainBearing,					//	Move in m_rDistance (angle) direction
													//		until we run into enemies.
			stateNone,								//	Ship needs to consult orders
			stateOnCourseForDocking,				//	Heading for m_pDest to dock
			stateOnCourseForStargate,				//	Heading for m_pDest to gate out
			stateOnPatrolOrbit,						//	Orbiting m_pDest at m_rDistance
			stateReturningFromThreat,				//	Dock with m_pDest
			stateWaiting,							//	Waiting for countdown (m_iCountdown)
			stateWaitingForThreat,					//	Waiting for a threat to m_pDest
			stateWaitingUnarmed,					//	If threat shows up, we gate out
			stateWaitForPlayerAtGate,				//	Waiting for player at gate
			stateFollowing,
			stateOnCourseForLootDocking,			//	Going to dock with loot
			stateDeterTargetWhileLootDocking,		//	Attack the target, but continue towards loot
			stateFollowNavPath,						//	Follow current nav path
			stateReturningViaNavPath,				//	Follow current nav path
			stateOnCourseForDockingViaNavPath,		//	Follow current nav path
			stateOnCourseForPatrolViaNavPath,		//	Follow current nav path
			stateOnCourseForPoint,					//	Going to a given point
			stateWaitingForTarget,					//	Waiting for m_pTarget
			stateWaitingForEnemy,					//	Waiting for an enemy
			stateAttackingTargetAndAvoiding,		//	Attacking m_pTarget and avoiding enemy stations
			stateAvoidingEnemyStation,				//	Attacking m_pTarget and avoiding m_pDest
			stateOnCourseForPointViaNavPath,		//	Follow current nav path
			stateBombarding,						//	Attacking m_pTarget while holding
			stateApproaching,						//	Get within m_rDistance of m_pDest
			stateAimingAtTarget,					//	Aiming at m_pTarget
			};

		struct TacticsSet
			{
			DWORD fCloseOnTarget:1;					//	Header for target
			DWORD fFireAtTarget:1;					//	Fire at target
			DWORD fFireOnOpportunity:1;				//	Fire at any target of opportunity
			DWORD fFlankTarget:1;					//	When closing on a target, aim a little to the side
			DWORD fSpiralToTarget:1;				//	Spiral towards target
			DWORD fSpiralAwayFromTarget:1;			//	Spiral away from target
			DWORD fEscortTarget:1;					//	Follow target
			};

		void AttackShipInRange (CSpaceObject *pCenter, Metric rRange);
		void BehaviorStart (void);
		void CommunicateWithEscorts (MessageTypes iMessage, CSpaceObject *pParam1 = NULL, DWORD dwParam2 = 0);
		void ImplementTactics (TacticsSet &Tactics, CVector &vTarget, Metric rTargetDist2);
		void SetState (StateTypes State);

		//	Orders and state
		StateTypes m_State;						//	Ship's current state
		CSpaceObject *m_pDest;					//	Destination
		CSpaceObject *m_pTarget;				//	Ship's target
		Metric m_rDistance;						//	Distance
		int m_iCountdown;						//	Timer
		ICCItem *m_pCommandCode;				//	Code to generate orders

		int m_iLastAttack;						//	Tick of last attack on us

		//	Flags
		bool m_bHasEscorts;

	friend class CObjectClass<CStandardShipAI>;
	};

class CZoanthropeAI : public CBaseShipAI
	{
	public:
		CZoanthropeAI (void);

		//	IShipController virtuals
		virtual void Behavior (void);
		virtual CString DebugCrashInfo (void);
		virtual CSpaceObject *GetBase (void) const;
		virtual CSpaceObject *GetOrderGiver (void);
		virtual CSpaceObject *GetTarget (void);
		virtual void OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage);
		virtual DWORD OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2);

	protected:
		//	CBaseShipAI overrides
		virtual void OnDockedEvent (CSpaceObject *pObj);
		virtual void OnObjDestroyedNotify (const SDestroyCtx &Ctx);
		virtual void OnOrderChanged (void);
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum StateTypes
			{
			stateNone,
			stateAttackingThreat,
			stateOnCourseForStargate,
			stateReturningFromThreat,
			stateWaitingForThreat,
			stateOnPatrolOrbit,
			stateAttackingOnPatrol,
			stateOnEscortCourse,
			stateWaiting,
			};

		void BehaviorStart (void);
		void ImplementCombatManeuvers (CSpaceObject *pTarget);
		void SetState (StateTypes State, CSpaceObject *pBase = NULL, CSpaceObject *pTarget = NULL);

		StateTypes m_State;						//	Ship's current state
		CSpaceObject *m_pBase;
		CSpaceObject *m_pTarget;

		CVector m_vPotential;

	friend class CObjectClass<CZoanthropeAI>;
	};

IShipController *CreateShipController (const CString &sAI);

//	The Universe

enum EInitFlags
	{
	flagNoResources			= 0x00000001,
	flagNoVersionCheck		= 0x00000002,
	};

class CUniverse : public CObject
	{
	public:
		class IDebug
			{
			public:
				virtual ~IDebug() { }
				virtual void DebugOutput (const CString &sLine) = 0;
				virtual void GameOutput (const CString &sLine) { }
			};

		CUniverse (void);
		virtual ~CUniverse (void);

		ALERROR AddStarSystem (CTopologyNode *pTopology, CSystem *pSystem);
		ALERROR CreateEmptyStarSystem (CSystem **retpSystem);
		ALERROR CreateFirstStarSystem (const CString &sStartNode, CSystem **retpSystem, CString *retsError);
		inline DWORD CreateGlobalID (void) { return m_dwNextID++; }
		ALERROR CreateRandomItem (const CItemCriteria &Crit, 
								  const CString &sLevelFrequency,
								  CItem *retItem);
		ALERROR CreateStarSystem (CTopologyNode *pTopology, CSystem **retpSystem, CSystemCreateStats *pStats = NULL);
		void DestroySystem (CSystem *pSystem);
		CSpaceObject *FindObject (DWORD dwID);
		void FlushStarSystem (CTopologyNode *pTopology);
		inline CAdventureDesc *GetCurrentAdventureDesc (void) { return m_pAdventure; }
		CTopologyNode *GetFirstTopologyNode (void);
		CXMLElement *GetGlobalSystemTables (void) { return m_pSystemTables; }
		void GetRandomLevelEncounter (int iLevel, IShipGenerator **retpTable, CSovereign **retpBaseSovereign);
		inline CString GetResourceDb (void) { return m_sResourceDb; }
		inline bool InDebugMode (void) { return m_bDebugMode; }
		ALERROR Init (const CString &sFilespec, CString *retsError, DWORD dwFlags = 0);
		ALERROR InitAdventure (DWORD dwAdventureUNID, CString *retsError);
		ALERROR LoadFromStream (IReadStream *pStream, DWORD *retdwSystemID, DWORD *retdwPlayerID, CString *retsError);
		inline bool NoImages (void) { return m_bNoImages; }
		void PlaySound (CSpaceObject *pSource, int iChannel);
		ALERROR Reinit (void);
		ALERROR SaveToStream (IWriteStream *pStream);
		void SetCurrentSystem (CSystem *pSystem);
		inline void SetCurrentAdventureDesc (CAdventureDesc *pAdventure) { m_pAdventure = pAdventure; }
		inline void SetDebugMode (bool bDebug = true) { m_bDebugMode = bDebug; }
		inline void SetDebugOutput (IDebug *pDebug) { m_pDebug = pDebug; }
		void SetPOV (CSpaceObject *pPOV);
		inline void SetSound (bool bSound = true) { m_bNoSound = !bSound; }
		inline void SetSoundMgr (CSoundMgr *pSoundMgr) { m_pSoundMgr = pSoundMgr; }
		void StartGameTime (void);
		CTimeSpan StopGameTime (void);

		inline CAdventureDesc *FindAdventureDesc (DWORD dwUNID) { return CAdventureDesc::AsType(m_Design.FindEntry(dwUNID)); }
		inline CDesignType *FindDesignType (DWORD dwUNID) { return m_Design.FindEntry(dwUNID); }
		CArmorClass *FindArmor (DWORD dwUNID);
		CDeviceClass *FindDeviceClass (DWORD dwUNID);
		inline CEffectCreator *FindEffectType (DWORD dwUNID) { return CEffectCreator::AsType(m_Design.FindEntry(dwUNID)); }
		inline CShipTable *FindEncounterTable (DWORD dwUNID) { return CShipTable::AsType(m_Design.FindEntry(dwUNID)); }
		inline CItemTable *FindItemTable (DWORD dwUNID) { return CItemTable::AsType(m_Design.FindEntry(dwUNID)); }
		inline CItemType *FindItemType (DWORD dwUNID) { return CItemType::AsType(m_Design.FindEntry(dwUNID)); }
		inline CPower *FindPower (DWORD dwUNID) { return CPower::AsType(m_Design.FindEntry(dwUNID)); }
		inline CDockScreenType *FindSharedDockScreen (DWORD dwUNID) { return CDockScreenType::AsType(m_Design.FindEntry(dwUNID)); }
		inline CShipClass *FindShipClass (DWORD dwUNID) { return CShipClass::AsType(m_Design.FindEntry(dwUNID)); }
		inline CEnergyFieldType *FindShipEnergyFieldType (DWORD dwUNID) { return CEnergyFieldType::AsType(m_Design.FindEntry(dwUNID)); }
		inline int FindSound (DWORD dwUNID) { return (int)FindByUNID(m_Sounds, dwUNID); }
		inline CSovereign *FindSovereign (DWORD dwUNID) const { return CSovereign::AsType(m_Design.FindEntry(dwUNID)); }
		inline CSpaceEnvironmentType *FindSpaceEnvironment (DWORD dwUNID) { return CSpaceEnvironmentType::AsType(m_Design.FindEntry(dwUNID)); }
		inline CStationType *FindStationType (DWORD dwUNID) { return CStationType::AsType(m_Design.FindEntry(dwUNID)); }
		inline CTopologyNode *FindTopologyNode (const CString &sName) { CTopologyNode *pNode; if (m_Topology.Lookup(sName, (CObject **)&pNode) == NOERROR) return pNode; else return NULL; }
		inline CSystemType *FindSystemType (DWORD dwUNID) { return CSystemType::AsType(m_Design.FindEntry(dwUNID)); }
		CWeaponFireDesc *FindWeaponFireDesc (const CString &sName);
		CWeaponFireDesc *FindWeaponFireDesc (DWORD dwUNID);
		inline CCodeChain &GetCC (void) { return m_CC; }
		inline CSystem *GetCurrentSystem (void) { return m_pCurrentSystem; }
		inline CSpaceObject *GetPOV (void) { return m_pPOV; }
		inline CSpaceObject *GetPlayer (void) { return (m_pPOV ? m_pPOV->AsShip() : NULL); }
		inline int GetTicks (void) { return m_iTick; }

		void GarbageCollectLibraryBitmaps (void);
		inline CObjectImage *FindLibraryImage (DWORD dwUNID) { return m_BitmapLibrary.FindImage(dwUNID); }
		inline CG16bitImage *GetLibraryBitmap (DWORD dwUNID) { return m_BitmapLibrary.GetImage(dwUNID); }
		void LoadLibraryBitmaps (void);
		inline void ReleaseLibraryBitmap (CG16bitImage *pBitmap) { }

		inline CDesignType *GetDesignType (int iIndex) { return m_Design.GetEntry(iIndex); }
		inline int GetDesignTypeCount (void) { return m_Design.GetCount(); }
		inline CAdventureDesc *GetAdventureDesc (int iIndex) { return (CAdventureDesc *)m_Design.GetEntry(designAdventureDesc, iIndex); }
		inline int GetAdventureDescCount (void) { return m_Design.GetCount(designAdventureDesc); }
		inline CItemType *GetItemType (int iIndex) { return (CItemType *)m_Design.GetEntry(designItemType, iIndex); }
		inline int GetItemTypeCount (void) { return m_Design.GetCount(designItemType); }
		inline CPower *GetPower (int iIndex) { return (CPower *)m_Design.GetEntry(designPower, iIndex); }
		inline int GetPowerCount (void) { return m_Design.GetCount(designPower); }
		inline CShipClass *GetShipClass (int iIndex) { return (CShipClass *)m_Design.GetEntry(designShipClass, iIndex); }
		inline int GetShipClassCount (void) { return m_Design.GetCount(designShipClass); }
		inline CSovereign *GetSovereign (int iIndex) const { return (CSovereign *)m_Design.GetEntry(designSovereign, iIndex); }
		inline int GetSovereignCount (void) { return m_Design.GetCount(designSovereign); }
		inline CStationType *GetStationType (int iIndex) { return (CStationType *)m_Design.GetEntry(designStationType, iIndex); }
		inline int GetStationTypeCount (void) { return m_Design.GetCount(designStationType); }
		inline CTopologyNode *GetTopologyNode (int iIndex) { return (CTopologyNode *)m_Topology.GetValue(iIndex); }
		inline int GetTopologyNodeCount (void) { CString sError; InitTopology(&sError); return m_Topology.GetCount(); }

		inline CG16bitFont &GetMapLabelFont (void) { return m_MapLabelFont; }
		inline CG16bitFont &GetSignFont (void) { return m_SignFont; }

		void PaintPOV (CG16bitImage &Dest, const RECT &rcView, bool bEnhanced);
		void PaintPOVLRS (CG16bitImage &Dest, const RECT &rcView, bool *retbNewEnemies);
		void PaintPOVMap (CG16bitImage &Dest, const RECT &rcView, Metric rMapScale);
		void PaintObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pObj);
		void PaintObjectMap (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pObj);
		void Update (Metric rSecondsPerTick);

		void DebugOutput (char *pszLine, ...);

	private:
		struct SLevelEncounter
			{
			int iWeight;
			CSovereign *pBaseSovereign;
			IShipGenerator *pTable;
			};

		struct STransSystemObject
			{
			CSpaceObject *pObj;
			CTopologyNode *pDest;
			int iArrivalTime;

			STransSystemObject *pNext;
			};

		ALERROR AddTopologyNode (CXMLElement *pNode, 
								 CTopologyDescTable *pNodeMap, 
								 CTopologyNode *pPrevNode, 
								 CTopologyNode **retpNewNode,
								 CString *retsError);
		ALERROR BindDesign (SDesignLoadCtx &Ctx);
		CObject *FindByUNID (CIDTable &Table, DWORD dwUNID);
		ALERROR InitCodeChainPrimitives (void);
		ALERROR InitExtensions (SDesignLoadCtx &Ctx, const CString &sFilespec);
		ALERROR InitImages (SDesignLoadCtx &Ctx, CXMLElement *pImages, CResourceDb &Resources);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx,
							 CXMLElement *pElement, 
							 CResourceDb &Resources);
		ALERROR InitLevelEncounterTables (void);
		ALERROR InitSounds (SDesignLoadCtx &Ctx, CXMLElement *pSounds, CResourceDb &Resources);
		ALERROR InitStarSystemTypes (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR InitStationTypeResources (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR InitTopology (CString *retsError);
		ALERROR LoadAdventure (SDesignLoadCtx &Ctx, CAdventureDesc *pAdventure);
		ALERROR LoadDesignElement (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR LoadEncounterTable (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR LoadExtension (SDesignLoadCtx &Ctx, CXMLElement *pExtension);
		ALERROR LoadGlobals (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR LoadImage (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR LoadModule (SDesignLoadCtx &Ctx, CXMLElement *pModule);
		ALERROR LoadModules (SDesignLoadCtx &Ctx, CXMLElement *pModules);
		ALERROR LoadSound (SDesignLoadCtx &Ctx, CXMLElement *pElement);
		ALERROR LoadSystemNode (SDesignLoadCtx &Ctx, CXMLElement *pElement);

		//	Design data

		CDesignCollection m_Design;				//	Design collection

		CXMLElement *m_pSystemTables;			//	Global lookup table for systems types
		CString m_sResourceDb;					//	Resource database

		CImageLibrary m_BitmapLibrary;			//	Cached bitmaps
		CIDTable m_Sounds;						//	Array of sound channels (int)
		CObjectArray m_LevelEncounterTables;	//	Array of SLevelEncounter arrays

		//	Game instance data

		int m_iTick;							//	Ticks since beginning of time
		CAdventureDesc *m_pAdventure;			//	Current adventure
		CSpaceObject *m_pPOV;					//	Point of view
		CSpaceObject *m_pPlayer;				//	Player ship
		CSystem *m_pCurrentSystem;				//	Current star system (used by code)
		CIDTable m_StarSystems;					//	Array of CSystem (indexed by ID)
		CTimeDate m_StartTime;					//	Time when we started the game
		DWORD m_dwNextID;						//	Next universal ID
		CSymbolTable m_Topology;				//	Array of CTopologyNode

		//	Support structures

		CCodeChain m_CC;
		CSoundMgr *m_pSoundMgr;
		CG16bitFont m_MapLabelFont;
		CG16bitFont m_SignFont;

		//	Debugging structures

		IDebug *m_pDebug;
		bool m_bNoImages;
		bool m_bDebugMode;
		bool m_bNoSound;
	};

//	Helpers

DestructionTypes GetDestructionCause (const CString &sString);
CString GetDestructionName (DestructionTypes iCause);
CString GetOrderName (IShipController::OrderTypes iOrder);
IShipController::OrderTypes GetOrderType (const CString &sString);
IShipController::ManeuverTypes CalcTurnManeuver (int iDesired, int iCurrent, int iRotationAngle);

//	CodeChain helper functions

ICCItem *CreateListFromImage (CCodeChain &CC, const CObjectImageArray &Image, int iRotation = 0);
ICCItem *CreateListFromItem (CCodeChain &CC, const CItem &Item);
CItem CreateItemFromList (CCodeChain &CC, ICCItem *pList);
void DefineGlobalSpaceObject (CCodeChain &CC, const CString &sVar, CSpaceObject *pObj);
void DefineGlobalVector (CCodeChain &CC, const CString &sVar, const CVector &vVector);
void GetImageDescFromList (CCodeChain &CC, ICCItem *pList, CG16bitImage **retpBitmap, RECT *retrcRect);
ALERROR LoadCodeBlock (const CString &sCode, ICCItem **retpCode, CString *retsError = NULL);

#include "TSEDesign.i"

#endif

