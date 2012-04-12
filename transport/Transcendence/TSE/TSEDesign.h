//	TSEDesign.h
//
//	Transcendence design classes

#ifndef INCL_TSE_DESIGN
#define INCL_TSE_DESIGN

struct SExtensionDesc;
struct SDesignLoadCtx;

//	Constants & Enums

const int MAX_ITEM_LEVEL =				25;		//	Max level for items

enum ItemCategories
	{
	itemcatMisc =			0x00000001,		//	General item with no object UNID
	itemcatArmor =			0x00000002,		//	Armor items
	itemcatWeapon =			0x00000004,		//	Primary weapons
	itemcatMiscDevice =		0x00000008,		//	Misc device
	itemcatLauncher =		0x00000010,		//	Launcher
	itemcatNano =			0x00000020,
	itemcatReactor =		0x00000040,		//	Reactor device
	itemcatShields =		0x00000080,		//	Shield device
	itemcatCargoHold =		0x00000100,		//	Cargo expansion device
	itemcatFuel =			0x00000200,		//	Fuel items
	itemcatMissile =		0x00000400,		//	Missiles
	itemcatDrive =			0x00000800,		//	Drive device
	itemcatUseful =			0x00001000,		//	Usable item
	};

//	CEventHandler

class CEventHandler
	{
	public:
		CEventHandler (void);
		~CEventHandler (void);

		void AddEvent (const CString &sEvent, ICCItem *pCode);
		ALERROR AddEvent (const CString &sEvent, const CString &sCode, CString *retsError = NULL);
		bool FindEvent (const CString &sEvent, ICCItem **retpCode) const;
		inline int GetCount (void) const { return m_Handlers.GetCount(); }
		CString GetEvent (int iIndex, ICCItem **retpCode = NULL) const;
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		inline bool IsEmpty (void) const { return m_Handlers.GetCount() == 0; }

	private:
		CSymbolTable m_Handlers;
	};

//	Base Design Type ----------------------------------------------------------

enum DesignTypes
	{
	designItemType =					0,
	designItemTable =					1,
	designShipClass =					2,
	designEnergyFieldType =				3,
	designSystemType =					4,
	designStationType =					5,
	designSovereign =					6,
	designDockScreen =					7,
	designEffectType =					8,
	designPower =						9,
	designSpaceEnvironmentType =		10,
	designShipTable =					11,
	designAdventureDesc =				12,
	designGlobals =						13,
	designImage =						14,
	designSound =						15,
	designSystemNode =					16,

	designCount	=						17, 
	};

// Prototypes used in the inline code. 
DWORD LoadUNID (SDesignLoadCtx &Ctx, const CString &sString);

class CDesignType
	{
	public:
		CDesignType (void) : m_dwUNID(0) { }
		virtual ~CDesignType (void) { }
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CDesignType **retpType);

		inline ALERROR BindDesign (SDesignLoadCtx &Ctx) { return OnBindDesign(Ctx); }
		void ReadFromStream (SUniverseLoadCtx &Ctx);
		void Reinit (void);
		void WriteToStream (IWriteStream *pStream);

		ALERROR AddEventHandler (const CString &sEvent, const CString &sCode, CString *retsError = NULL) { return m_Events.AddEvent(sEvent, sCode, retsError); }
		inline CEffectCreator *FindEffectCreator (const CString &sUNID) { return OnFindEffectCreator(sUNID); }
		inline bool FindEventHandler (const CString &sEvent, ICCItem **retpCode = NULL) const { return m_Events.FindEvent(sEvent, retpCode); }
		ALERROR FireOnGlobalSystemCreated (CString *retsError = NULL);
		ALERROR FireOnGlobalTopologyCreated (CString *retsError = NULL);
		ICCItem *GetEventHandler (const CString &sEvent) const;
		const CEventHandler &GetEventHandlers (void) { return m_Events; }
		inline const CString &GetGlobalData (const CString &sAttrib) { return m_GlobalData.GetData(sAttrib); }
		inline const CString &GetStaticData (const CString &sAttrib) { return m_StaticData.GetData(sAttrib); }
		inline DWORD GetUNID (void) const { return m_dwUNID; }
		inline bool HasEvents (void) const { return !m_Events.IsEmpty(); }
		inline void SetGlobalData (const CString &sAttrib, const CString &sData) { m_GlobalData.SetData(sAttrib, sData); }

		//	CDesignType overrides
		virtual DesignTypes GetType (void) = 0;

	protected:
		void ReadGlobalData (SUniverseLoadCtx &Ctx);

		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID) { return NULL; }
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx) { }
		virtual void OnReinit (void) { }
		virtual void OnWriteToStream (IWriteStream *pStream) { }
		
	private:
		DWORD m_dwUNID;

		CAttributeDataBlock m_StaticData;		//	Static data
		CAttributeDataBlock m_GlobalData;		//	Global (variable) data
		CAttributeDataBlock m_InitGlobalData;	//	Initial global data
		CEventHandler m_Events;					//	Event handlers
	};

template <class CLASS> class CDesignTypeRef
	{
	public:
		CDesignTypeRef (void) : m_pType(NULL), m_dwUNID(0) { }

		inline operator CLASS *() const { return m_pType; }
		inline CLASS * operator->() const { return m_pType; }

		ALERROR Bind (SDesignLoadCtx &Ctx);

		inline DWORD GetUNID (void) const { return m_dwUNID; }
		void LoadUNID (SDesignLoadCtx &Ctx, const CString &sUNID) { m_dwUNID = ::LoadUNID(Ctx, sUNID); }

	protected:
		CLASS *m_pType;
		DWORD m_dwUNID;
	};

//	Design Type References ----------------------------------------------------

class CItemTypeRef : public CDesignTypeRef<CItemType>
	{
	public:
		inline ALERROR Bind (SDesignLoadCtx &Ctx) { return CDesignTypeRef<CItemType>::Bind(Ctx); }
		ALERROR Bind (SDesignLoadCtx &Ctx, ItemCategories iCategory);
	};

class CArmorClassRef : public CDesignTypeRef<CArmorClass>
	{
	public:
		ALERROR Bind (SDesignLoadCtx &Ctx);
	};

class CDeviceClassRef : public CDesignTypeRef<CDeviceClass>
	{
	public:
		ALERROR Bind (SDesignLoadCtx &Ctx);
		void Set (CDeviceClass *pDevice);
	};

class CWeaponFireDescRef : public CDesignTypeRef<CWeaponFireDesc>
	{
	public:
		ALERROR Bind (SDesignLoadCtx &Ctx);
	};

class CDockScreenTypeRef
	{
	public:
		CDockScreenTypeRef (void) : m_pType(NULL), m_pLocal(NULL) { }

		inline operator CDockScreenType *() const { return m_pType; }
		inline CDockScreenType * operator->() const { return m_pType; }

		ALERROR Bind (SDesignLoadCtx &Ctx, CXMLElement *pLocalScreens = NULL);
		CXMLElement *GetDesc (void) const;
		inline const CString &GetUNID (void) const { return m_sUNID; }
		void LoadUNID (SDesignLoadCtx &Ctx, const CString &sUNID);

		ALERROR Bind (CXMLElement *pLocalScreens = NULL);
		inline void LoadUNID (const CString &sUNID) { m_sUNID = sUNID; }

	private:
		CString m_sUNID;
		CDockScreenType *m_pType;
		CXMLElement *m_pLocal;
	};

class CEffectCreatorRef : public CDesignTypeRef<CEffectCreator>
	{
	public:
		CEffectCreatorRef (void) : m_bDelete(false) { }
		~CEffectCreatorRef (void);

		ALERROR Bind (SDesignLoadCtx &Ctx);
		ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID);
		ALERROR LoadEffect (SDesignLoadCtx &Ctx, const CString &sUNID, CXMLElement *pDesc, const CString &sAttrib);
		void Set (CEffectCreator *pEffect);

	private:
		bool m_bDelete;
	};

class CItemTableRef : public CDesignTypeRef<CItemTable>
	{
	};

class CShipClassRef : public CDesignTypeRef<CShipClass>
	{
	};

class CSovereignRef : public CDesignTypeRef<CSovereign>
	{
	};

//	Classes and structs

struct SViewportPaintCtx
	{
	SViewportPaintCtx (void)
		{
		pObj = NULL;
		wSpaceColor = 0;
		fNoSelection = false;
		iTick = 0;
		iVariant = 0;
		iDestiny = 0;
		iRotation = 0;
		};

	CSpaceObject *pObj;
	ViewportTransform XForm;
	COLORREF wSpaceColor;
	WORD fNoSelection:1;
	WORD wSpare:15;

	//	May be modified by callers
	int iTick;
	int iVariant;
	int iDestiny;
	int iRotation;
	};

class CObjectImage
	{
	public:
		CObjectImage (CG16bitImage *pBitmap);
		~CObjectImage (void);
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CObjectImage **retpImage);

		CG16bitImage *GetImage (CString *retsError = NULL);
		inline DWORD GetUNID (void) { return m_dwUNID; }

		inline void ClearMark (void) { m_bMarked = false; }
		ALERROR Lock (SDesignLoadCtx &Ctx);
		inline void Mark (void) { m_bMarked = true; }
		void Sweep (void);

	private:
		CObjectImage (void);

		DWORD m_dwUNID;					//	UNID of image
		CString m_sResourceDb;			//	Resource db
		CString m_sBitmap;				//	Bitmap resource within db
		CString m_sBitmask;				//	Bitmask resource within db
		bool m_bTransColor;				//	If TRUE, m_wTransColor is valid
		COLORREF m_wTransColor;			//	Transparent color
		bool m_bSprite;					//	If TRUE, convert to sprite after loading
		bool m_bPreMult;				//	If TRUE, image is premultiplied with alpha

		CG16bitImage *m_pBitmap;		//	Loaded image (NULL if not loaded)
		bool m_bMarked;					//	Marked
		bool m_bLocked;					//	Image is never unloaded
	};

class CObjectImageArray : public CObject
	{
	public:
		CObjectImageArray (void);
		CObjectImageArray (const CObjectImageArray &Source);
		~CObjectImageArray (void);
		CObjectImageArray &operator= (const CObjectImageArray &Source);

		ALERROR Init (CG16bitImage *pBitmap, const RECT &rcImage, int iFrameCount, int iTicksPerFrame);
		ALERROR Init (DWORD dwBitmapUNID, const RECT &rcImage, int iFrameCount, int iTicksPerFrame);
		ALERROR InitFromXML (CXMLElement *pDesc);
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bResolveNow = false);
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

		void CopyImage (CG16bitImage &Dest, int x, int y, int iFrame, int iRotation) const;
		inline DWORD GetBitmapUNID (void) const { return m_dwBitmapUNID; }
		inline int GetFrameCount (void) const { return m_iFrameCount; }
		inline CG16bitImage &GetImage (void) const { return *(m_pImage->GetImage()); }
		inline const RECT &GetImageRect (void) const { return m_rcImage; }
		RECT GetImageRect (int iTick, int iRotation, int *retxCenter = NULL, int *retyCenter = NULL) const;
		RECT GetImageRectAtPoint (int x, int y) const;
		inline int GetTicksPerFrame (void) const { return m_iTicksPerFrame; }
		bool ImagesIntersect (int iTick, int iRotation, int x, int y, const CObjectImageArray &Image2, int iTick2, int iRotation2) const;
		inline bool IsEmpty (void) const { return m_pImage == NULL; }
		void LoadImage (void);
		void MarkImage (void);
		void PaintImage (CG16bitImage &Dest, int x, int y, int iTick, int iRotation, bool srcAlpha = true) const;
		void PaintImageUL (CG16bitImage &Dest, int x, int y, int iTick, int iRotation, bool srcAlpha = true) const;
		void PaintImageWithGlow (CG16bitImage &Dest,
								 int x,
								 int y,
								 int iTick,
								 int iRotation,
								 COLORREF rgbGlowColor) const;
		void PaintScaledImage (CG16bitImage &Dest,
							   int x,
							   int y,
							   int iTick,
							   int iRotation,
							   int cxWidth,
							   int cyHeight) const;
		void PaintSilhoutte (CG16bitImage &Dest,
							 int x,
							 int y,
							 int iTick,
							 int iRotation,
							 COLORREF wColor) const;
		bool PointInImage (int x, int y, int iTick, int iRotation) const;
		void ReadFromStream (SLoadCtx &Ctx);
		void SetRotationCount (int iRotationCount);
		void WriteToStream (IWriteStream *pStream) const;

	private:
		enum BlendingTypes
			{
			blendNormal,
			blendLighten,
			};

		struct OffsetStruct
			{
			int x;
			int y;
			};

		void ComputeRotationOffsets (void);
		void ComputeRotationOffsets (int xOffset, int yOffset);
		int ComputeSourceX (int iTick) const;
		void CopyFrom (const CObjectImageArray &Source);
		void GenerateGlowImage (int iRotation) const;
		void GenerateScaledImages (int iRotation, int cxWidth, int cyHeight) const;

		DWORD m_dwBitmapUNID;				//	UNID of bitmap (0 if none)
		CObjectImage *m_pImage;				//	Image (if UNID is 0, we own this structure)
		RECT m_rcImage;
		int m_iFrameCount;
		int m_iTicksPerFrame;
		int m_iFlashTicks;
		int m_iRotationCount;
		int m_iRotationOffset;
		OffsetStruct *m_pRotationOffset;
		int m_iBlending;

		//	Glow effect
		mutable CG16bitImage *m_pGlowImages;
		mutable CG16bitImage *m_pScaledImages;

	friend class CObjectClass<CObjectImageArray>;
	};

class CCompositeImageDesc
	{
	public:
		CCompositeImageDesc (void);
		~CCompositeImageDesc (void);

		CObjectImageArray &GetImage (int iVariant, int *retiRotation = NULL);
		inline int GetVariantCount (void) { return m_iVariantCount; }
		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		inline bool IsEmpty (void) { return (m_Variants == NULL); }
		void LoadImage (int iVariant);
		void MarkImage (int iVariant);
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			CObjectImageArray Image;
			CEffectCreator *pEffect;
			};

		ALERROR InitEntryFromImageXML (SEntry &Entry, SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR InitEntryFromCompositeXML (SEntry &Entry, SDesignLoadCtx &Ctx, CXMLElement *pDesc);

		int m_iVariantCount;
		SEntry *m_Variants;
	};

//	Sovereigns

enum AlignmentTypes
	{
	alignNone =					-1,

	alignConstructiveChaos =	0,
	alignConstructiveOrder =	1,
	alignNeutral =				2,
	alignDestructiveOrder =		3,
	alignDestructiveChaos =		4,
	};

//	Damage

enum BeamTypes
	{
	beamHeavyBlaster,
	beamLaser,
	beamLightning,
	beamStarBlaster,

	beamGreenParticle,
	beamBlueParticle,
	beamElectron,
	beamPositron,
	beamGraviton,
	beamBlaster,
	beamGreenLightning,
	};

enum DamageTypes
	{
	damageNone			= -1,

	damageLaser			= 0,					//	standard lasers
	damageKinetic		= 1,					//	mass drivers
	damageParticle		= 2,					//	charged particle beam
	damageBlast			= 3,					//	chemical explosives
	damageIonRadiation	= 4,					//	ionizing radiation
	damageThermonuclear	= 5,					//	hydrogen bomb
	damagePositron		= 6,					//	anti-matter charged particles
	damagePlasma		= 7,					//	fusion weapons
	damageAntiMatter	= 8,					//	anti-matter torpedo
	damageNano			= 9,					//	nano-machines
	damageGravitonBeam	= 10,					//	graviton beam
	damageSingularity	= 11,					//	spacetime weapons
	damageDarkAcid		= 12,					//	exotics
	damageDarkSteel		= 13,					//	exotics
	damageDarkLightning	= 14,					//	exotics
	damageDarkFire		= 15,					//	exotics

	damageCount			= 16
	};

enum SpecialDamageTypes
	{
	specialRadiation	= 0,
	specialBlinding		= 1,
	specialEMP			= 2,
	specialDeviceDamage	= 3,
	specialDisintegration = 4,
	specialMomentum		= 5,
	specialShieldDisrupt = 6,
	specialWMD			= 7,
	specialMining		= 8,

	specialMemetic		= 9,
	specialWormhole		= 10,
	specialFuel			= 11,
	specialShatter		= 12,
	//	max 15
	};

enum DestructionTypes
	{
	removedFromSystem				= 0,

	killedByDamage					= 1,	//	Ship destroyed
	killedByRunningOutOfFuel		= 2,	//	Ran out of fuel
	killedByRadiationPoisoning		= 3,	//	Radiation poisoning
	killedBySelf					= 4,	//	Self-destruct
	killedByDisintegration			= 5,	//	Ship disintegrated
	killedByWeaponMalfunction		= 6,	//	Weapon malfunction
	killedByEjecta					= 7,	//	Killed by ejecta
	killedByExplosion				= 8,	//	Killed when a station (or ship) exploded
	killedByShatter					= 9,	//	Killed by shatter effect
	killedByPlayerCreatedExplosion	= 10,	//	Killed by explosion created by the player

	killedNone						= -1,
	killedCount						= 11
	};

class DamageDesc
	{
	public:
		DamageDesc (void) { }
		DamageDesc (DamageTypes iType, const DiceRange &Damage) : m_iType(iType),
				m_Damage(Damage),
				m_iBonus(0),
				m_iCause(killedByDamage),
				m_EMPDamage(0),
				m_MomentumDamage(0),
				m_RadiationDamage(0),
				m_MemeticDamage(0),
				m_BlindingDamage(0),
				m_SensorDamage(0),
				m_ShieldDamage(0),
				m_WormholeDamage(0),
				m_FuelDamage(0),
				m_DisintegrationDamage(0),
				m_fSpare1(0),
				m_fAutomatedWeapon(0),
				m_DeviceDamage(0),
				m_MassDestructionAdj(0),
				m_MiningAdj(0),
				m_ShatterDamage(0),
				m_dwSpare2(0)
			{ }

		inline void AddBonus (int iBonus) { m_iBonus += iBonus; }
		Metric GetAverageDamage (void);
		inline DestructionTypes GetCause (void) const { return m_iCause; }
		inline DamageTypes GetDamageType (void) const { return m_iType; }
		CString GetDesc (void);
		int GetMinDamage (void);
		int GetMaxDamage (void);
		bool IsAutomatedWeapon (void) const { return (m_fAutomatedWeapon ? true : false); }
		bool IsEnergyDamage (void) const;
		bool IsMatterDamage (void) const;
		ALERROR LoadFromXML (const CString &sAttrib);
		void ReadFromStream (SLoadCtx &Ctx);
		int RollDamage (void) const;
		void SetDamage (int iDamage);
		inline void SetAutomatedWeapon (void) { m_fAutomatedWeapon = true; }
		inline void SetCause (DestructionTypes iCause) { m_iCause = iCause; }
		void WriteToStream (IWriteStream *pStream) const;

		inline int GetBlindingDamage (void) const { return (int)m_BlindingDamage; }
		inline int GetDeviceDamage (void) const { return (int)m_DeviceDamage; }
		inline int GetDisintegrationDamage (void) const { return (int)m_DisintegrationDamage; }
		inline int GetEMPDamage (void) const { return (int)m_EMPDamage; }
		inline int GetMemeticDamage (void) const { return (int)m_MemeticDamage; }
		inline int GetMomentumDamage (void) const { return (int)m_MomentumDamage; }
		inline int GetRadiationDamage (void) const { return (int)m_RadiationDamage; }
		inline int GetShatterDamage (void) const { return (int)m_ShatterDamage; }

		inline int GetMassDestructionAdj (void) const { return (int)(m_MassDestructionAdj ? (2 * (m_MassDestructionAdj * m_MassDestructionAdj) + 2) : 0); }
		inline int GetMiningAdj (void) const { return (int)(m_MiningAdj ? (2 * (m_MiningAdj * m_MiningAdj) + 2) : 0); }

	private:
		DamageTypes m_iType;					//	Type of damage
		DiceRange m_Damage;						//	Amount of damage
		int m_iBonus;							//	Bonus to damage (%)
		DestructionTypes m_iCause;				//	Cause of damage

		//	Extra damage
		DWORD m_EMPDamage:3;					//	Ion (paralysis) damage
		DWORD m_MomentumDamage:3;				//	Momentum damage
		DWORD m_RadiationDamage:3;				//	Radiation damage
		DWORD m_MemeticDamage:3;				//	Memetic (computer) damage
		DWORD m_BlindingDamage:3;				//	Optical sensor damage
		DWORD m_SensorDamage:3;					//	Long-range sensor damage
		DWORD m_ShieldDamage:3;					//	Disrupt shield damage
		DWORD m_WormholeDamage:3;				//	Teleport
		DWORD m_FuelDamage:3;					//	Drain fuel
		DWORD m_DisintegrationDamage:3;			//	Disintegration damage

		DWORD m_fSpare1:1;						//	(used to be weapon misfire)
		DWORD m_fAutomatedWeapon:1;				//	TRUE if this damage is caused by automated weapon

		DWORD m_DeviceDamage:3;					//	Damage to devices
		DWORD m_MassDestructionAdj:3;			//	Adj for mass destruction
		DWORD m_MiningAdj:3;					//	Adj for mining capability
		DWORD m_ShatterDamage:3;				//	Shatter damage

		DWORD m_dwSpare2:20;
	};

struct SDamageCtx
	{
	CSpaceObject *pObj;							//	Object hit
	DamageDesc Damage;							//	Damage
	int iDirection;								//	Direction that hit came from
	CVector vHitPos;							//	Hit at this position
	CSpaceObject *pCause;						//	Object that directly caused the damage
	CSpaceObject *pAttacker;					//	Ultimate attacker (pCause->GetDamageCause())

	int iDamage;								//	Damage hp
	};

struct SDestroyCtx
	{
	CSpaceObject *pObj;							//	Object destroyed
	CSpaceObject *pDestroyer;					//	Ultimate destroyer (pCause->GetDamageCause())
	CSpaceObject *pWreck;						//	Wreck left behind
	DestructionTypes iCause;					//	Cause of damage
	};

class DamageTypeSet
	{
	public:
		DamageTypeSet (void) : m_dwSet(0) { }

		ALERROR InitFromXML (const CString &sAttrib);
		void Add (int iType) { m_dwSet |= (1 << iType); }
		bool InSet (int iType) { return ((m_dwSet & (1 << iType)) ? true : false); }
		void Remove (int iType) { m_dwSet &= ~(1 << iType); }

	private:
		DWORD m_dwSet;
	};

//	WeaponFireDesc

enum FireTypes
	{
	ftArea,
	ftBeam,
	ftMissile,
	ftParticles,
	ftRadius,
	};

enum ParticleTypes
	{
	particleImage,
	particlePlasma,
	particleBolt,
	};

//	Communications

//	DO NOT CHANGE NUMBERS
//	NOTE: These numbers must match-up with CSovereign.cpp and with the
//	definitions in the <Sovereign> elements.

enum MessageTypes
	{
	msgAttack =						1,	//	Order to attack (pParam1 is target)
	msgDestroyBroadcast =			2,	//	Broadcast order to destroy target (pParam1 is target)
	msgHitByFriendlyFire =			3,	//	Hit by friendly fire (pParam1 is attacker)
	msgQueryEscortStatus =			4,	//	Replies with Ack if is escorting pParam1
	msgQueryFleetStatus =			5,	//	Replies with Ack if is in formation with pParam1
	msgEscortAttacked =				6,	//	pParam1 attack escort ship
	msgEscortReportingIn =			7,	//	pParam1 is now escorting the ship
	msgWatchTargets =				8,	//	"watch your targets!"
	msgNiceShooting =				9,	//	"nice shooting!"
	msgFormUp =						10,	//	Enter formation (dwParam2 is formation place)
	msgBreakAndAttack =				11,	//	Break formation and attack
	msgQueryCommunications =		12,	//	Returns comms capabilities
	msgAbort =						13,	//	Cancel attack order
	msgWait =						14,	//	Wait there for a while
	msgQueryWaitStatus =			15,	//	Replies with Ack if waiting
	msgAttackInFormation =			16,	//	Fire weapon while staying in formation
	};

enum ResponseTypes
	{
	resNoAnswer =					0,
	resAck =						1,

	resCanBeInFormation =			0x0010,
	resCanAttack =					0x0020,
	resCanBreakAndAttack =			0x0040,
	resCanAbortAttack =				0x0080,
	resCanWait =					0x0100,
	resCanFormUp =					0x0200,
	resCanAttackInFormation =		0x0400,
	};

class CCommunicationsHandler
	{
	public:
		struct SMessage
			{
			CString sMessage;
			CString sShortcut;
			ICCItem *pCode;
			ICCItem *pOnShow;
			};

		CCommunicationsHandler (void);
		~CCommunicationsHandler (void);

		ALERROR InitFromXML (CXMLElement *pDesc, CString *retsError);

		inline int GetCount (void) { return m_iCount; }
		inline const SMessage &GetMessage (int iIndex) { return m_pMessages[iIndex]; }

	private:
		int m_iCount;
		SMessage *m_pMessages;
	};

//	Item Types

struct CItemCriteria
	{
	CItemCriteria (void);
	CItemCriteria (const CItemCriteria &Copy);
	~CItemCriteria (void);
	CItemCriteria &operator= (const CItemCriteria &Copy);

	DWORD dwItemCategories;			//	Set of ItemCategories to match on

	WORD wFlagsMustBeSet;			//	These flags must be set
	WORD wFlagsMustBeCleared;		//	These flags must be cleared

	bool bUsableItemsOnly;			//	Item must be usable
	bool bExcludeVirtual;			//	Exclude virtual items
	bool bInstalledOnly;			//	Item must be installed
	bool bNotInstalledOnly;			//	Item must not be installed

	CStringArray ModifiersRequired;		//	Required modifiers
	CStringArray ModifiersNotAllowed;	//	Exclude these modifiers
	CString Frequency;				//	If not blank, only items with these frequencies

	int iEqualToLevel;				//	If not -1, only items of this level
	int iGreaterThanLevel;			//	If not -1, only items greater than this level
	int iLessThanLevel;				//	If not -1, only items less than this level
	int iEqualToPrice;				//	If not -1, only items at this price
	int iGreaterThanPrice;			//	If not -1, only items greater than this price
	int iLessThanPrice;				//	If not -1, only items less than this price
	int iEqualToMass;				//	If not -1, only items of this mass (in kg)
	int iGreaterThanMass;			//	If not -1, only items greater than this mass (in kg)
	int iLessThanMass;				//	If not -1, only items less than this mass (in kg)

	ICCItem *pFilter;				//	Filter returns Nil for excluded items
	};

enum ItemEnhancementTypes
	{
	etNone =							0x0000,
	etBinaryEnhancement =				0x0001,
	etLoseEnhancement =					0x0002,	//	Lose enhancements

	etStrengthen =						0x0100,	//	+hp, data1 = %increase (10% increments)
	etRegenerate =						0x0200,	//	data1 = rate
	etReflect =							0x0300,	//	data2 = damage type reflected
	etRepairOnHit =						0x0400,	//	repair damage on hit, data2 = damage type of hit
	etResist =							0x0500,	//	-damage, data1 = %damage adj
	etResistEnergy =					0x0600,	//	-energy damage, data1 = %damage adj (90%, 80%, etc)
	etResistMatter =					0x0700,	//	-matter damage, data1 = %damage adj (90%, 80%, etc)
	etResistByLevel =					0x0800,	//	-damage, data1 = %damage adj, data2 = damage type
	etResistByDamage =					0x0900,	//	-damage, data1 = %damage adj, data2 = damage type
	etResistByDamage2 =					0x0a00,	//	-damage, data1 = %damage adj, data2 = damage type
	etSpecialDamage =					0x0b00,	//	Immunity to damage effects:
												//		data2 = 0: immune to radiation
												//		data2 = 1: immune to blinding
												//		data2 = 2: immune to EMP
												//		data2 = 3: immune to device damage
												//		data2 = 4: immune to disintegration
	etImmunityIonEffects =				0x0c00,	//	Immunity to ion effects (blinding, EMP, etc.)
												//		(if disadvantage, interferes with shields)
	etPhotoRegenerate =					0x0d00,	//	regen near sun
	etPhotoRecharge =					0x0e00,	//	refuel near sun
	etPowerEfficiency =					0x0f00,	//	power usage decrease, 01 = 90%/110%, 02 = 80%/120%
	etSpeed =							0x1000,	//	decrease cycle time
	etTurret =							0x1100,	//	weapon turret, data1 is angle
	etMultiShot =						0x1200,	//	multiple shots, data2 = count, data1 = %weakening

	etData1Mask =						0x000f,	//	4-bits of data (generally for damage adj)
	etData2Mask =						0x00f0,	//	4-bits of data (generally for damage type)
	etTypeMask =						0x7f00,	//	Type
	etDisadvantage =					0x8000,	//	If set, this is a disadvantage
	};

enum EnhanceItemStatus
	{
	eisOK,										//	Enhancement OK
	eisNoEffect,								//	Nothing happens
	eisAlreadyEnhanced,							//	Already has this exact enhancement
	eisWorse,									//	A disadvantage was made worse
	eisRepaired,								//	Disadvantage was repaired
	eisEnhancementRemoved,						//	Enhancement removed
	eisEnhancementReplaced,						//	Enhancement replaced by another enhancement
	eisBetter,									//	Enhancement made better
	};

const int MAX_ITEM_CHARGES =			0x7fff;

class CItem
	{
	public:
		CItem (void);
		CItem (const CItem &Copy);
		CItem (CItemType *pItemType, int iCount);
		~CItem (void);
		CItem &operator= (const CItem &Copy);

		inline void ClearDamaged (void) { m_dwFlags &= ~flagDamaged; }
		inline void ClearEnhanced (void) { m_dwFlags &= ~flagEnhanced; }
		inline void ClearModFlag (int iMod) { m_dwFlags &= ~Mod2Flags(iMod); }
		static CItem CreateItemByName (const CString &sName, const CItemCriteria &Criteria, bool bActualName = false);
		bool IsEqual (const CItem &Item) const;
		inline int GetCharges (void) const { return (m_pExtra ? (int)m_pExtra->m_dwCharges : 0); }
		inline int GetCount (void) const { return (int)m_dwCount; }
		inline CString GetData (const CString &sAttrib) const { return (m_pExtra ? m_pExtra->m_Data.GetData(sAttrib) : NULL_STR); }
		inline const CString &GetDesc (void) const;
		CString GetEnhancedDesc (CSpaceObject *pInstalled = NULL) const;
		inline int GetInstalled (void) const { return (int)(char)m_dwInstalled; }
		inline DWORD GetMods (void) const { return (m_pExtra ? m_pExtra->m_dwMods : 0); }
		CString GetModsDesc (CSpaceObject *pSource) const;
		CString GetReference (CSpaceObject *pInstalled = NULL) const;
		CString GetReferencePower (CSpaceObject *pInstalled = NULL) const;
		CString GetNounPhrase (DWORD dwFlags) const;
		inline CItemType *GetType (void) const { return m_pItemType; }
		int GetValue (bool bActual = false) const;
		inline bool IsDamaged (void) const { return (m_dwFlags & flagDamaged ? true : false); }
		inline bool IsEnhanced (void) const { return (m_dwFlags & flagEnhanced ? true : false); }
		inline bool IsInstalled (void) const { return (m_dwInstalled != 0xff); }
		inline bool IsMarkedForDelete (void) { return (m_dwCount == 0xffff); }
		inline void MarkForDelete (void) { m_dwCount = 0xffff; }
		inline void SetCharges (int iCharges) { Extra(); m_pExtra->m_dwCharges = iCharges; }
		inline void SetCount (int iCount) { m_dwCount = (DWORD)iCount; }
		inline void SetDamaged (void) { m_dwFlags |= flagDamaged; }
		inline void SetDamaged (bool bDamaged) { ClearDamaged(); if (bDamaged) SetDamaged(); }
		inline void SetData (const CString &sAttrib, const CString &sData) { Extra(); m_pExtra->m_Data.SetData(sAttrib, sData); }
		inline void SetEnhanced (void) { m_dwFlags |= flagEnhanced; }
		inline void SetEnhanced (bool bEnhanced) { ClearEnhanced(); if (bEnhanced) SetEnhanced(); }
		inline void SetInstalled (int iInstalled) { m_dwInstalled = (BYTE)(char)iInstalled; }
		inline void SetMods (DWORD dwMods) { Extra(); m_pExtra->m_dwMods = dwMods; }

		static void InitCriteriaAll (CItemCriteria *retCriteria);
		static void ParseCriteria (const CString &sCriteria, CItemCriteria *retCriteria);
		bool MatchesCriteria (const CItemCriteria &Criteria) const;

		void ReadFromStream (SLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream);

		void ReadFromCCItem (CCodeChain &CC, ICCItem *pBuffer);
		ICCItem *WriteToCCItem (CCodeChain &CC) const;

	private:
		enum Flags
			{
			flagDamaged =		0x01,			//	Item is damaged

			flagEnhanced =		0x10,			//	Item is enhanced (Mod 1)
			flagMod2 =			0x20,			//	Mod 2
			flagMod3 =			0x40,			//	Mod 3
			flagMod4 =			0x80,			//	Mod 4
			};

		struct SExtra
			{
			DWORD m_dwCharges:16;				//	Charges for items
			DWORD m_dwCondition:16;				//	Wear and tear on item
			DWORD m_dwMods;						//	Class-specific modifications (e.g., armor enhancements)

			CAttributeDataBlock m_Data;			//	Opaque data
			};

		void Extra (void);
		bool IsExtraEqual (SExtra *pSrc) const;
		DWORD Mod2Flags (int iMod) const;

		CItemType *m_pItemType;

		DWORD m_dwCount:16;						//	Number of items
		DWORD m_dwFlags:8;						//	Miscellaneous flags
		DWORD m_dwInstalled:8;					//	Location where item is installed

		SExtra *m_pExtra;						//	Extra data (may be NULL)
	};

class CItemList
	{
	public:
		CItemList (void);
		~CItemList (void);
		CItemList &operator= (const CItemList &Copy);

		void AddItem (const CItem &Item);
		void DeleteAll (void);
		void DeleteItem (int iIndex);
		int GetCount (void) const;
		CItem &GetItem (int iIndex);
		const CItem &GetItem (int iIndex) const;
		void ReadFromStream (SLoadCtx &Ctx);
		void SortItems (void);
		void WriteToStream (IWriteStream *pStream);

	private:
		struct SHeader
			{
			int iCount;							//	Number of items
			int iAlloc;							//	Number of allocations
			};

		CItem *GetItems (void) { return (CItem *)(&m_pList[sizeof(SHeader)]); }
		const CItem *GetItems (void) const { return (const CItem *)(&m_pList[sizeof(SHeader)]); }

		char *m_pList;							//	List of items (first 32-bits is count)
	};

class CItemListManipulator
	{
	public:
		CItemListManipulator (CItemList &ItemList);
		~CItemListManipulator (void);

		void AddItem (const CItem &Item);
		void AddItems (const CItemList &ItemList);

		inline int GetCount (void) { return m_ViewMap.GetCount(); }
		inline int GetCursor (void) { return m_iCursor; }
		inline void SetCursor (int iCursor) { m_iCursor = iCursor; }
		bool SetCursorAtItem (const CItem &Item);
		void SetFilter (const CItemCriteria &Filter);
		void Refresh (const CItem &Item);

		inline bool IsCursorValid (void) { return m_iCursor != -1; }
		bool MoveCursorBack (void);
		bool MoveCursorForward (void);
		void ResetCursor (void);

		void DeleteAtCursor (int iCount);
		void DeleteMarkedItems (void);
		const CItem &GetItemAtCursor (void);
		void MarkDeleteAtCursor (int iCount);
		void SetChargesAtCursor (int iCharges);
		void SetCountAtCursor (int iCount);
		void SetDamagedAtCursor (bool bDamaged);
		void SetDataAtCursor (const CString &sAttrib, const CString &sData);
		void SetEnhancedAtCursor (bool bEnhanced);
		void SetInstalledAtCursor (int iInstalled);
		void SetModsAtCursor (DWORD dwMods);
		void TransferAtCursor (int iCount, CItemList &DestList);

	private:
		int FindItem (const CItem &Item);
		void GenerateViewMap (const CItemCriteria *pFilter = NULL);
		void MoveItemTo (const CItem &NewItem, const CItem &OldItem);

		CItemList &m_ItemList;
		int m_iCursor;
		CIntArray m_ViewMap;
		CItemCriteria m_Filter;
	};

class CItemEnhancement
	{
	public:
		CItemEnhancement (void) : m_dwMods(0) { }
		CItemEnhancement (DWORD dwMods) : m_dwMods(dwMods) { }

		operator DWORD() { return m_dwMods; }
		CItemEnhancement &operator= (DWORD dwMods) { m_dwMods = dwMods; return *this; }

		EnhanceItemStatus Combine (CItemEnhancement Enhancement);
		int GetAbsorbAdj (const DamageDesc &Damage);
		int GetDamageAdj (const DamageDesc &Damage);
		int GetDamageBonus (void) const;
		inline DamageTypes GetDamageType (void) const { return (DamageTypes)(DWORD)((m_dwMods & etData2Mask) >> 4); }
		CString GetEnhancedDesc (const CItem &Item, CSpaceObject *pInstalled);
		int GetEnhancedRate (int iRate) const;
		int GetHPAdj (void) const;
		inline int GetLevel (void) const { return (int)(DWORD)(m_dwMods & etData1Mask); }
		inline int GetLevel2 (void) const { return (int)(DWORD)((m_dwMods & etData2Mask) >> 4); }
		int GetPowerAdj (void) const;
		inline ItemEnhancementTypes GetType (void) const { return (ItemEnhancementTypes)(m_dwMods & etTypeMask); }
		int GetValueAdj (const CItem &Item) const;
		inline bool IsBlindingImmune (void) const { return IsIonEffectImmune() || ((GetType() == etSpecialDamage) && GetLevel2() == specialBlinding && !IsDisadvantage()); }
		inline bool IsDecaying (void) const { return ((GetType() == etRegenerate) && IsDisadvantage()); }
		inline bool IsDeviceDamageImmune (void) const { return IsIonEffectImmune() || ((GetType() == etSpecialDamage) && GetLevel2() == specialDeviceDamage && !IsDisadvantage()); }
		inline bool IsDisadvantage (void) const { return ((m_dwMods & etDisadvantage) ? true : false); }
		inline bool IsDisintegrationImmune (void) const { return ((GetType() == etSpecialDamage) && GetLevel2() == specialDisintegration && !IsDisadvantage()); }
		inline bool IsEMPImmune (void) const { return IsIonEffectImmune() || ((GetType() == etSpecialDamage) && GetLevel2() == specialEMP && !IsDisadvantage()); }
		inline bool IsEnhancement (void) const { return (m_dwMods && !IsDisadvantage()); }
		inline bool IsPhotoRecharge (void) const { return ((GetType() == etPhotoRecharge) && !IsDisadvantage()); }
		inline bool IsPhotoRegenerating (void) const { return ((GetType() == etPhotoRegenerate) && !IsDisadvantage()); }
		inline bool IsRadiationImmune (void) const { return ((GetType() == etSpecialDamage) && GetLevel2() == 0 && !IsDisadvantage()); }
		inline bool IsRegenerating (void) const { return ((GetType() == etRegenerate) && !IsDisadvantage()); }
		inline bool IsReflective (void) const { return ((GetType() == etReflect) && !IsDisadvantage()); }
		bool IsReflective (const DamageDesc &Damage, int *retiReflectChance = NULL) const;
		inline bool IsShieldInterfering (void) const { return ((GetType() == etImmunityIonEffects) && IsDisadvantage()); }

	private:
		inline bool IsIonEffectImmune (void) const { return ((GetType() == etImmunityIonEffects) && !IsDisadvantage()); }
		static int Level2Bonus (int iLevel, bool bDisadvantage = false);
		static int Level2DamageAdj (int iLevel, bool bDisadvantage = false);

		DWORD m_dwMods;
	};

enum DeviceNames
	{
	devNone = -1,

	devFirstName = 0,

	devPrimaryWeapon = 0,
	devMissileWeapon = 1,
	devTurretWeapon1 = 2,
	devTurretWeapon2 = 3,
	devTurretWeapon3 = 4,
	devTurretWeapon4 = 5,
	devTurretWeapon5 = 6,
	devTurretWeapon6 = 7,

	devShields = 8,
	devDrive = 9,
	devCargo = 10,
	devReactor = 11,

	devTurretWeaponCount = 6,
	devNamesCount = 12
	};

struct DriveDesc
	{
	DWORD dwUNID;								//	UNID source (either ship class or device)
	Metric rMaxSpeed;							//	Max speed (Km/sec)
	int iThrust;								//	Thrust (GigaNewtons--gasp!)
	int iPowerUse;								//	Power used while thrusting (1/10 megawatt)

	DWORD fInertialess:1;						//	Inertialess drive
	DWORD dwSpare:31;
	};

struct ReactorDesc
	{
	int iMaxPower;								//	Maximum power output
	int iMaxFuel;								//	Maximum fuel space
	int iMinFuelLevel;							//	Min tech level of fuel
	int iMaxFuelLevel;							//	Max tech level of fuel
	int iPowerPerFuelUnit;						//	MW/10-tick per fuel unit

	DWORD fDamaged:1;							//	TRUE if damaged
	DWORD fEnhanced:1;							//	TRUE if enhanced
	DWORD dwSpare:30;
	};

struct CInstalledArmor
	{
	CArmorClass *pArmorClass;					//	Armor class used
	int iHitPoints;								//	Hit points left

	CItemEnhancement m_Mods;					//	Enhancements from CItem

	DWORD m_fComplete:1;						//	All armor segments the same
	DWORD m_fSpare:31;
	};

enum ArmorCompositionTypes
	{
	compUnknown = -1,

	compMetallic = 0,
	compCeramic = 1,
	compCarbide = 2,
	compNanoScale = 3,
	compQuantum = 4,
	compGravitic = 5,
	compDarkMatter = 6,
	};

ArmorCompositionTypes LoadArmorComposition (const CString &sString);

class CArmorClass : public CObject
	{
	public:
		void AddArmorComplete (CInstalledArmor *pArmor);
		int CalcAdjustedDamage (CInstalledArmor *pArmor, const DamageDesc &Damage, int iDamage);
		int CalcBalance (void);
		int CalcPowerUsed (CInstalledArmor *pArmor);
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CArmorClass **retpArmor);
		bool FindDataField (const CString &sField, CString *retsValue);
		inline int GetBlindingDamageAdj (void) { return m_iBlindingDamageAdj; }
		inline int GetDamageAdj (DamageTypes iDamage) { return m_iDamageAdj[iDamage]; }
		inline int GetDeviceDamageAdj (void) { return m_iDeviceDamageAdj; }
		inline int GetEMPDamageAdj (void) { return m_iEMPDamageAdj; }
		inline CItemType *GetItemType (void) { return m_pItemType; }
		inline Metric GetMass (void) const;
		inline CString GetName (void);
		int GetHitPoints (CItemEnhancement Mods);
		int GetHitPoints (CInstalledArmor *pArmor);
		inline int GetInstallCost (void) { return m_iInstallCost; }
		CString GetReference (const CItem *pItem = NULL, CSpaceObject *pInstalled = NULL);
		inline int GetRepairCost (void) { return m_iRepairCost; }
		inline int GetRepairTech (void) { return m_iRepairTech; }
		CString GetShortModifierDesc (const CItem &Item, CSpaceObject *pInstalled = NULL);
		CString GetShortName (void);
		inline int GetStealth (void) { return m_iStealth; }
		inline DWORD GetUNID (void);
		inline bool IsBlindingDamageImmune (CInstalledArmor *pArmor) { return (m_iBlindingDamageAdj == 0) || ((pArmor && pArmor->m_Mods.IsBlindingImmune())); }
		inline bool IsDeviceDamageImmune (CInstalledArmor *pArmor) { return (m_iDeviceDamageAdj == 0) || ((pArmor && pArmor->m_Mods.IsDeviceDamageImmune())); }
		inline bool IsDisintegrationImmune (CInstalledArmor *pArmor) { return (m_fDisintegrationImmune || (pArmor && pArmor->m_Mods.IsDisintegrationImmune())); }
		inline bool IsEMPDamageImmune (CInstalledArmor *pArmor) { return (m_iEMPDamageAdj == 0) || ((pArmor && pArmor->m_Mods.IsEMPImmune())); }
		inline bool IsRadiationImmune (CInstalledArmor *pArmor) { return (m_fRadiationImmune || (pArmor && pArmor->m_Mods.IsRadiationImmune())); }
		bool IsReflective (CInstalledArmor *pArmor, const DamageDesc &Damage);
		inline bool IsShieldInterfering (CInstalledArmor *pArmor) { return (m_fShieldInterference || (pArmor && pArmor->m_Mods.IsShieldInterfering())); }
		void Update (CInstalledArmor *pArmor, CSpaceObject *pObj, int iTick, bool *retbModified);

	private:
		CArmorClass (void);

		int GetDamageAdj (CItemEnhancement Mods, const DamageDesc &Damage);

		int m_iHitPoints;						//	Hit points for this armor class
		int m_iRepairCost;						//	Cost per HP to repair
		int m_iInstallCost;						//	Cost to install
		int m_iSelfRepairCycle;					//	Number of ticks to repair 1 hp
		int m_iDecayCycle;						//	Number of ticks to decay 1 hp
		int m_iRepairTech;						//	Tech required to repair
		int m_iArmorCompleteBonus;				//	Extra HP if armor is complete
		int m_iStealth;							//	Stealth level
		int m_iPowerUse;						//	Power consumed (1/10 MWs)
		
		int m_iDamageAdj[damageCount];			//	Adjustments for damage type
		DamageTypeSet m_Reflective;				//	Types of damage reflected
		int m_iEMPDamageAdj;					//	Adjust for EMP damage
		int m_iBlindingDamageAdj;				//	Adjust for blinding damage
		int m_iDeviceDamageAdj;					//	Adjust for device damage

		DWORD m_fPhotoRepair:1;					//	TRUE if repairs when near a star
		DWORD m_fRadiationImmune:1;				//	TRUE if immune to radiation
		DWORD m_fPhotoRecharge:1;				//	TRUE if refuels when near a star
		DWORD m_fShieldInterference:1;			//	TRUE if armor interferes with shields
		DWORD m_fDisintegrationImmune:1;		//	TRUE if immune to disintegration
		DWORD m_fSpare:27;

		CItemType *m_pItemType;					//	Item for this armor

	friend class CObjectClass<CArmorClass>;
	};

class CDeviceClass : public CObject
	{
	public:
		enum DeviceFailureType
			{
			failWeaponJammed,
			failWeaponMisfire,
			failWeaponExplosion,
			failShieldFailure,
			failDeviceHitByDamage,
			failDeviceOverheat,
			};

		enum CounterTypes
			{
			cntNone,							//	No counter
			cntTemperature,						//	Current device temperature (0-100)
			cntRecharge,						//	Current recharge level (0-100)
			cntCapacitor,						//	Current capacitor level (0-100)
			};

		CDeviceClass (IObjectClass *pClass) : CObject(pClass), m_pItemType(NULL) { }
		virtual ~CDeviceClass (void) { }

		static bool FindAmmoDataField (CItemType *pItem, const CString &sField, CString *retsValue);
		inline CEffectCreator *FindEffectCreator (const CString &sUNID) { return OnFindEffectCreator(sUNID); }
		inline CString GetDataField (const CString &sField) { CString sValue; FindDataField(sField, &sValue); return sValue; }
		inline int GetDataFieldInteger (const CString &sField) { CString sValue; if (FindDataField(sField, &sValue)) return strToInt(sValue, 0, NULL); else return 0; }
		inline CItemType *GetItemType (void) { return m_pItemType; }
		inline int GetLevel (void);
		inline Metric GetMass (void);
		inline CString GetName (void);
		CString GetReferencePower (const CItem *pItem);
		inline int GetSlotsRequired (void) { return m_iSlots; }
		inline DWORD GetUNID (void);
		inline void LoadImages (void) { OnLoadImages(); }
		inline void MarkImages (void) { OnMarkImages(); }

		virtual bool AbsorbDamage (CInstalledDevice *pDevice, CShip *pShip, SDamageCtx &Ctx) { return false; }
		virtual bool AbsorbsWeaponFire (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon) { return false; }
		virtual void Activate (CInstalledDevice *pDevice, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   int iFireAngle,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems = NULL) { }
		virtual CWeaponClass *AsWeaponClass (void) { return NULL; }
		virtual int CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource) { return 0; }
		virtual bool CanBeDamaged (void) { return true; }
		virtual bool CanBeDisabled (void) { return true; }
		virtual void Deplete (CInstalledDevice *pDevice, CSpaceObject *pSource) { }
		virtual bool FindDataField (const CString &sField, CString *retsValue) { return false; }
		virtual bool FindDataField (int iVariant, const CString &sField, CString *retsValue) { return false; }
		virtual int GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource) { return 0; }
		virtual int GetAmmoVariant (CItemType *pItem) { return -1; }
		virtual int GetCargoSpace (void) { return 0; }
		virtual ItemCategories GetCategory (void) = 0;
		virtual int GetCounter (CInstalledDevice *pDevice, CounterTypes *retiType = NULL) { return 0; }
		virtual const DamageDesc *GetDamageDesc (CInstalledDevice *pDevice) { return NULL; }
		virtual const DriveDesc *GetDriveDesc (CInstalledDevice *pDevice = NULL, CSpaceObject *pSource = NULL) { return NULL; }
		virtual CString GetEnhancedDesc (const CItem &Item, CSpaceObject *pInstalled);
		virtual int GetPowerRating (const CItem *pItem) { return 0; }
		virtual const ReactorDesc *GetReactorDesc (CInstalledDevice *pDevice = NULL, CSpaceObject *pSource = NULL) { return NULL; }
		virtual CString GetReference (const CItem *pItem = NULL, CSpaceObject *pInstalled = NULL);
		virtual void GetStatus (CInstalledDevice *pDevice, CShip *pShip, int *retiStatus, int *retiMaxStatus) { *retiStatus = 0; *retiMaxStatus = 0; }
		virtual int GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon, CString *retsBonusType) { return 0; }
		virtual void Update (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 int iTick,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems = NULL) { }

		virtual int GetDamageType (CInstalledDevice *pDevice) { return -1; }
		virtual void GetSelectedVariantInfo (CSpaceObject *pSource, 
											 CInstalledDevice *pDevice,
											 CString *retsLabel,
											 int *retiAmmoLeft,
											 CItemType **retpType = NULL) { }
		virtual Metric GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget) { return 0.0; }
		virtual int GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice) { return 0; }
		virtual int GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget) { return 0; }
		virtual bool IsAreaWeapon (CSpaceObject *pSource, CInstalledDevice *pDevice) { return false; }
		virtual bool IsAutomatedWeapon (void) { return false; }
		virtual bool IsExternal (void) { return (m_fExternal ? true : false); }
		virtual bool IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice) { return true; }
		virtual bool IsWeaponAligned (CSpaceObject *pShip, CInstalledDevice *pDevice, CSpaceObject *pTarget, int *retiAimAngle, int *retiFireAngle = NULL) { return false; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual void OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList) { }
		virtual void OnUninstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList) { }
		virtual void Recharge (CInstalledDevice *pDevice, CShip *pShip, int iStatus) { }
		virtual bool RequiresItems (void) { return false; }
		virtual void Reset (CInstalledDevice *pDevice, CSpaceObject *pSource) { }
		virtual bool SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return false; }
		virtual bool SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return false; }
		virtual bool ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice) { return false; }

	protected:
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID) { return NULL; }
		virtual void OnLoadImages (void) { }
		virtual void OnMarkImages (void) { }

		void InitDeviceFromXML (CXMLElement *pDesc, CItemType *pType);

	private:
		CItemType *m_pItemType;					//	Item for device
		int m_iSlots;							//	Number of device slots required

		DWORD m_fExternal:1;					//	Device is external
		DWORD m_dwSpare:31;
	};

class CRandomEnhancementGenerator
	{
	public:
		CRandomEnhancementGenerator (void) : m_iChance(0), m_dwMods(0), m_pCode(NULL) { }
		~CRandomEnhancementGenerator (void);

		ALERROR InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

		void EnhanceItem (CItem &Item);
		inline int GetChance (void) const { return m_iChance; }

	private:
		int m_iChance;
		DWORD m_dwMods;
		ICCItem *m_pCode;
	};

//	IItemGenerator

class IItemGenerator
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IItemGenerator **retpGenerator);
		static ALERROR CreateRandomItemTable (const CItemCriteria &Crit, 
											  const CString &sLevelFrequency,
											  IItemGenerator **retpGenerator);

		virtual ~IItemGenerator (void) { }
		virtual void AddItems (CItemListManipulator &ItemList) { }
		virtual IItemGenerator *GetGenerator (int iIndex) { return NULL; }
		virtual int GetGeneratorCount (void) { return 0; }
		virtual CItemType *GetItemType (int iIndex) { return NULL; }
		virtual int GetItemTypeCount (void) { return 0; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return NOERROR; }
	};

//	IDeviceGenerator

struct SDeviceDesc
	{
	CItem Item;

	int iPosAngle;
	int iPosRadius;

	bool bOmnidirectional;
	int iMinFireArc;
	int iMaxFireArc;
	bool bSecondary;

	CItemList ExtraItems;
	};

class CDeviceDescList
	{
	public:
		CDeviceDescList (void);
		~CDeviceDescList (void);

		void AddDeviceDesc (const SDeviceDesc &Desc);
		inline int GetCount (void) const { return m_iCount; }
		inline CDeviceClass *GetDeviceClass (int iIndex) const;
		inline const SDeviceDesc &GetDeviceDesc (int iIndex) const { return m_pDesc[iIndex]; }
		CDeviceClass *GetNamedDevice (DeviceNames iDev) const;
		void RemoveAll (void);

	private:
		SDeviceDesc *m_pDesc;
		int m_iCount;
		int m_iAlloc;
	};

class IDeviceGenerator
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IDeviceGenerator **retpGenerator);

		virtual ~IDeviceGenerator (void) { }
		virtual void AddDevices (CDeviceDescList &Result) { }
		virtual IDeviceGenerator *GetGenerator (int iIndex) { return NULL; }
		virtual int GetGeneratorCount (void) { return 0; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return NOERROR; }
	};

//	Object properties

enum ScaleTypes
	{
	scaleStar,
	scaleWorld,
	scaleStructure,
	scaleShip,
	scaleFlotsam
	};

//	IShipGenerator

struct SShipCreateCtx
	{
	SShipCreateCtx (void) :
			pSystem(NULL),
			pGate(NULL),
			pBase(NULL),
			pBaseSovereign(NULL),
			pTarget(NULL),
			pEncounterInfo(NULL),
			dwFlags(0)
		{ }

	CSystem *pSystem;							//	System to create ship in
	CSpaceObject *pGate;						//	Gate where ship will appear (may be NULL)
	CVector vPos;								//	Position where ship will appear (only if pGate is NULL)
	DiceRange PosSpread;						//	Distance from vPos (in light-seconds)
	CSpaceObject *pBase;						//	Base for this ship (may be NULL)
	CSovereign *pBaseSovereign;					//	Only if pBase is NULL
	CSpaceObject *pTarget;						//	Target for ship orders (may be NULL)
	CStationType *pEncounterInfo;				//	Encounter info (may be NULL)

	DWORD dwFlags;								//	Flags

	CSpaceObjectList Result;					//	List of ships created

	enum Flags
		{
		SHIPS_FOR_STATION =			0x00000001,	//	Create ships at station creation time
		ATTACK_NEAREST_ENEMY =		0x00000002,	//	After all other orders, attack nearest enemy
		RETURN_RESULT =				0x00000004,	//	Add created ships to Result
		};
	};

class IShipGenerator
	{
	public:
		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IShipGenerator **retpGenerator);
		static ALERROR CreateFromXMLAsGroup (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IShipGenerator **retpGenerator);

		virtual ~IShipGenerator (void) { }
		virtual void CreateShips (SShipCreateCtx &Ctx) { }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ALERROR ValidateShipTable (SDesignLoadCtx &Ctx) { return NOERROR; }
	};

//	Ship properties

struct SArmorImageDesc
	{
	CString sName;								//	Name of segment

	int xDest;									//	Coordinates of destination
	int yDest;
	int xHP;									//	Coordinates of HP display
	int yHP;
	int yName;									//	Coordinates of armor name
	int cxNameBreak;							//	Line break
	int xNameDestOffset;
	int yNameDestOffset;

	CObjectImageArray Image;					//	Image for armor damage
	};

struct SPlayerSettings
	{
	CString m_sDesc;							//	Description
	DWORD m_dwLargeImage;						//	UNID of large image
	DWORD m_fInitialClass:1;					//	Use ship class at game start
	DWORD m_fDebug:1;							//	Debug only
	DWORD m_dwSpare:30;

	//	Miscellaneous
	DiceRange m_StartingCredits;				//	Starting credits
	CString m_sStartNode;						//	Starting node (may be blank)
	CString m_sStartPos;						//	Label of starting position (may be blank)
	CDockScreenTypeRef m_pShipScreen;			//	Ship screen

	//	Armor
	int m_iArmorDescCount;
	SArmorImageDesc *m_pArmorDesc;

	//	Shields
	CObjectImageArray m_ShieldImage;

	//	Reactor
	CObjectImageArray m_ReactorImage;
	CObjectImageArray m_PowerLevelImage;
	int m_xPowerLevelImage;
	int m_yPowerLevelImage;
	CObjectImageArray m_FuelLevelImage;
	int m_xFuelLevelImage;
	int m_yFuelLevelImage;
	CObjectImageArray m_FuelLowLevelImage;
	RECT m_rcReactorText;
	RECT m_rcPowerLevelText;
	RECT m_rcFuelLevelText;
	};

enum AISettingsFlags
	{
	aiflagNoShieldRetreat		= 0x00000001,	//	Ship does not retreat when shields go down
	aiflagNoDogfights			= 0x00000002,	//	[NOTE: Not currently used]
	aiflagNonCombatant			= 0x00000004,	//	Tries to stay out of trouble
	aiflagNoFriendlyFire		= 0x00000008,	//	Cannot hit friends
	};

enum AICombatStyles
	{
	aicombatStandard,							//	Normal dogfighting
	aicombatStandOff,							//	Missile ship combat
	aicombatFlyby,								//	Maximize relative speed wrt target
	aicombatNoRetreat,							//	Do not turn away from target
	};

struct SAISettings
	{
	int iFireRateAdj;							//	Adjustment to weapon's fire rate (10 = normal; 20 = double delay)
	int iFireRangeAdj;							//	Adjustment to range (100 = normal; 50 = half range)
	int iFireAccuracy;							//	Percent chance of hitting
	int iPerception;							//	Perception
	AICombatStyles iCombatStyle;				//	Combat style
	DWORD dwFlags;								//	Flags
	};

//	Effects

enum ParticlePaintStyles
	{
	paintPlain,
	paintFlame,
	paintSmoke,
	paintImage,
	paintLine,
	};

struct SParticlePaintDesc
	{
	SParticlePaintDesc (void)
		{
		iStyle = paintPlain;

		pImage = NULL;
		iVariants = 0;
		bRandomStartFrame = false;
		bDirectional = false;

		iMaxLifetime = -1;
		iMinWidth = 4;
		iMaxWidth = 4;

		wPrimaryColor = CG16bitImage::RGBValue(255, 255, 255);
		wSecondaryColor = CG16bitImage::RGBValue(0, 0, 0);
		}

	ParticlePaintStyles iStyle;

	CObjectImageArray *pImage;					//	Image for each particle
	int iVariants;								//	No of image variants
	bool bRandomStartFrame;						//	Start at a random animation
	bool bDirectional;							//	Different images for each direction

	int iMaxLifetime;							//	Max lifetime for particles
	int iMinWidth;								//	Min width of particle
	int iMaxWidth;								//	Max width of particle

	COLORREF wPrimaryColor;							//	Primary color 
	COLORREF wSecondaryColor;						//	Secondary color
	};

class IEffectPainter
	{
	public:
		int GetInitialLifetime (void);
		inline void PlaySound (CSpaceObject *pSource);
		inline void ReadFromStream (SLoadCtx &Ctx) { OnReadFromStream(Ctx); }
		CString ReadUNID (SLoadCtx &Ctx);
		void WriteToStream (IWriteStream *pStream);

		virtual ~IEffectPainter (void) { }
		virtual void Delete (void) { delete this; }
		virtual CEffectCreator *GetCreator (void) = 0;
		virtual int GetFadeLifetime (void) { return 0; }
		virtual bool GetParticlePaintDesc (SParticlePaintDesc *retDesc) { return false; }
		virtual Metric GetRadius (void) const;
		virtual void GetRect (RECT *retRect) const;
		virtual void OnBeginFade (void) { }
		virtual void OnUpdate (void) { }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) = 0;
		virtual void PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx) { }
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const { return false; }
		virtual void SetDirection (int iDirection) { }
		virtual void SetVariants (int iVariants) { }

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx) { }
		virtual void OnWriteToStream (IWriteStream *pStream) { }
	};

//	Topology Descriptors

class CTopologyDescTable
	{
	public:
		CTopologyDescTable (void);
		~CTopologyDescTable (void);

		CXMLElement *FindNode (const CString &sNodeID);
		inline const CString &GetFirstNodeID (void) { return m_sFirstNode; }
		inline int GetRootNodeCount (void) { return m_RootNodes.size(); }
		inline CXMLElement *GetRootNode (int iIndex) { return m_RootNodes[iIndex]; }
		ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CSymbolTable m_Table;
		CString m_sFirstNode;

		std::vector<CXMLElement *> m_RootNodes;
	};

//	Game Types

struct SGameStats
	{
	SGameStats (void) :
			iGenome(genomeUnknown),
			bDebugGame(false),
			dwAdventure(0),
			iScore(0),
			iResurrectCount(0),
			iSystemsVisited(0),
			iEnemiesDestroyed(0),
			pBestEnemyDestroyed(NULL),
			iBestEnemyDestroyedCount(0),
			bEscapedHumanQuarantineZone(false)
		{ }

	//	Basic
	CString sName;
	CString sShipClass;
	int iGenome;
	bool bDebugGame;
	DWORD dwAdventure;
	CString sAdventureName;

	//	Stats
	int iScore;
	int iResurrectCount;
	int iSystemsVisited;
	int iEnemiesDestroyed;
	CShipClass *pBestEnemyDestroyed;
	int iBestEnemyDestroyedCount;

	//	End Game
	CString sEndGameReason;
	CString sEpitaph;
	CString sTime;
	bool bEscapedHumanQuarantineZone;
	};

//	CItemType -----------------------------------------------------------------

class CItemType : public CDesignType
	{
	public:
		CItemType (void);
		virtual ~CItemType (void);

		inline bool AreChargesValued (void) const { return (m_fValueCharges ? true : false); }
		inline void ClearKnown (void) { m_fKnown = false; }
		inline void ClearShowReference (void) { m_fReference = false; }
		bool FindDataField (const CString &sField, CString *retsValue);
		int GetApparentLevel (void) const;
		inline CArmorClass *GetArmorClass (void) const { return m_pArmor; }
		ItemCategories GetCategory (void) const;
		inline int GetCharges (void) const { return (m_fInstanceData ? m_InitDataValue.Roll() : 0); }
		inline const CString &GetData (void) const { return m_sData; }
		inline CString GetDataField (const CString &sField) { CString sValue; FindDataField(sField, &sValue); return sValue; }
		inline int GetDataFieldInteger (const CString &sField) { CString sValue; if (FindDataField(sField, &sValue)) return strToInt(sValue, 0, NULL); else return 0; }
		const CString &GetDesc (void) const;
		inline CDeviceClass *GetDeviceClass (void) const { return m_pDevice; }
		inline int GetFrequency (void) const { return m_Frequency; }
		inline const CObjectImageArray &GetImage (void) { return m_Image; }
		int GetInstallCost (void) const;
		inline const DiceRange &GetNumberAppearing (void) const { return m_NumberAppearing; }
		inline int GetLevel (void) const { return m_iLevel; }
		inline Metric GetMass (void) { return m_iMass / 1000.0; }
		inline int GetMassKg (void) { return m_iMass; }
		inline int GetMaxCharges (void) const { return (m_fInstanceData ? m_InitDataValue.GetMaxValue() : 0); }
		CString GetName (DWORD *retdwFlags, bool bActualName = false) const;
		CString GetNounPhrase (DWORD dwFlags = 0) const;
		inline ICCItem *GetOnRefuelCode (void) const { return GetEventHandler(CONSTLIT("OnRefuel")); }
		CString GetReference (void) const;
		CString GetShortName (DWORD *retdwFlags, bool bActualName = false) const;
		CString GetSortName (void) const;
		inline CItemType *GetUnknownType (void) { return m_pUnknownType; }
		inline ICCItem *GetUseCode (void) const { return m_pUseCode; }
		inline const CString &GetUseKey (void) const { return m_sUseKey; }
		inline CXMLElement *GetUseScreen (void) const;
		int GetValue (bool bActual = false) const;
		inline bool HasModifier (const CString &sModifier) const { return ::HasModifier(m_sModifiers, sModifier); }
		inline bool HasOnRefuelCode (void) const { return FindEventHandler(CONSTLIT("OnRefuel")); }
		void InitRandomNames (void);
		inline bool IsArmor (void) const { return (m_pArmor != NULL); }
		inline bool IsDevice (void) const { return (m_pDevice != NULL); }
		inline bool IsKnown (void) const { return (m_fKnown ? true : false); }
		bool IsFuel (void) const;
		bool IsMissile (void) const;
		inline bool IsUsable (void) const { return ((m_pUseCode != NULL) || (m_pUseScreen != NULL)); }
		inline bool IsUsableInCockpit (void) const { return (m_pUseCode != NULL); }
		inline bool IsUsableOnlyIfInstalled (void) const { return (m_fUseInstalled ? true : false); }
		inline bool IsUsableOnlyIfUninstalled (void) const { return (m_fUseUninstalled ? true : false); }
		inline bool IsVirtual (void) const { return (m_fVirtual ? true : false); }
		inline void SetKnown (void) { m_fKnown = true; }
		inline void SetShowReference (void) { m_fReference = true; }
		inline bool ShowReference (void) const { return (m_fReference ? true : false); }

		//	CDesignType overrides
		static CItemType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designItemType) ? (CItemType *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designItemType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID);
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		static ALERROR ComposeError (const CString &sName, const CString &sError, CString *retsError);
		CString GetUnknownName (int iIndex, DWORD *retdwFlags);

		CString m_sName;						//	Full name of item
		int m_iUnknownName;						//	Unknown name index
		CString m_sPluralName;					//	Used only if an exception
		CString m_sSortName;					//	Name to sort by
		int m_iLevel;							//	Level of item
		int m_iValue;							//	Value in credits
		int m_iMass;							//	Mass in kilograms
		FrequencyTypes m_Frequency;				//	Frequency
		DiceRange m_NumberAppearing;			//	Number appearing

		CObjectImageArray m_Image;				//	Image of item
		CString m_sDescription;					//	Description of item
		CItemTypeRef m_pUnknownType;			//	Type to show if not known
		CStringArray m_UnknownNames;			//	List of unknown names (if we are the unknown item placeholder)
		DiceRange m_InitDataValue;				//	Initial data value

		//	Usable items
		CDockScreenTypeRef m_pUseScreen;		//	Screen shown when used (may be NULL)
		ICCItem *m_pUseCode;					//	Code when using item from cockpit
		CString m_sUseKey;						//	Hotkey to invoke usage

		//	Armor
		CArmorClass *m_pArmor;					//	Armor properties (may be NULL)

		//	Devices
		CDeviceClass *m_pDevice;				//	Device properties (may be NULL)

		DWORD m_fFirstPlural:1;					//	Pluralize first word
		DWORD m_fSecondPlural:1;				//	Pluralize second word
		DWORD m_fESPlural:1;					//	Pluralize by adding "es"
		DWORD m_fRandomDamaged:1;				//	Randomly damaged when found
		DWORD m_fKnown:1;						//	Is this type known?
		DWORD m_fReference:1;					//	Does this type show reference info?
		DWORD m_fDefaultReference:1;			//	TRUE if this shows reference by default
		DWORD m_fInstanceData:1;				//	TRUE if we need to set instance data at create time
		DWORD m_fReverseArticle:1;				//	"a" instead of "an" or vice versa
		DWORD m_fVirtual:1;						//	TRUE if this is a virtual item needed for a weapon that invokes
		DWORD m_fUseInstalled:1;				//	If TRUE, item can only be used when installed
		DWORD m_fValueCharges:1;				//	TRUE if value should be adjusted based on charges
		DWORD m_fUseUninstalled:1;				//	If TRUE, item can only be used when uninstalled
		DWORD m_dwSpare:19;

		CString m_sModifiers;					//	Modifiers (separated by semicolons)
		CString m_sData;						//	Category-specific data
	};

//	CItemTable ----------------------------------------------------------------

class CItemTable : public CDesignType
	{
	public:
		CItemTable (void);
		virtual ~CItemTable (void);

		inline void AddItems (CItemListManipulator &ItemList) { if (m_pGenerator) m_pGenerator->AddItems(ItemList); }
		inline IItemGenerator *GetGenerator (void) { return m_pGenerator; }

		//	CDesignType overrides
		static CItemTable *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designItemTable) ? (CItemTable *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designItemTable; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		IItemGenerator *m_pGenerator;
	};

//	CShipClass ----------------------------------------------------------------

class CShipClass : public CDesignType
	{
	public:
		enum VitalSections
			{
			sectNonCritical	= 0x00000000,		//	Ship is not automatically
												//	destroyed (chance of being
												//	destroyed based on the power
												//	of the blast relative to the
												//	original armor HPs)

			//	NOTE: These must match the order in CShipClass.cpp (ParseNonCritical)

			sectDevice0		= 0x00000001,
			sectDevice1		= 0x00000002,
			sectDevice2		= 0x00000004,
			sectDevice3		= 0x00000008,
			sectDevice4		= 0x00000010,
			sectDevice5		= 0x00000020,
			sectDevice6		= 0x00000040,
			sectDevice7		= 0x00000080,
			sectDeviceMask	= 0x000000ff,

			sectManeuver	= 0x00000100,		//	Maneuvering damaged (quarter speed turn)
			sectDrive		= 0x00000200,		//	Drive damaged (half-speed)
			sectScanners	= 0x00000400,		//	Unable to target
			sectTactical	= 0x00000800,		//	Unable to fire weapons
			sectCargo		= 0x00001000,		//	Random cargo destroyed

			sectCritical	= 0x00010000,		//	Ship destroyed
			};

		struct HullSection
			{
			int iStartAt;						//	Start of section in degrees
			int iSpan;							//	Size of section in degrees
			CArmorClassRef pArmor;				//	Type of armor for hull
			DWORD dwAreaSet;					//	Areas that this section protects
			};

		CShipClass (void);
		virtual ~CShipClass (void);

		inline int Angle2Direction (int iAngle) const
			{ return ((m_iRotationRange - (iAngle / m_iRotationAngle)) + (m_iRotationRange / 4)) % m_iRotationRange; }
		inline int AlignToRotationAngle (int iAngle) const
			{ return (((m_iRotationRange - Angle2Direction(iAngle)) + (m_iRotationRange / 4)) * m_iRotationAngle) % 360; }
		Metric CalcMass (const CDeviceDescList &Devices);
		int CalcScore (void);
		void CreateEmptyWreck (CSystem *pSystem, CShip *pShip, const CVector &vPos, const CVector &vVel, CSovereign *pSovereign, CStation **retpWreck);
		void CreateExplosion (CShip *pShip, CSpaceObject *pWreck = NULL);
		void CreateWreck (CShip *pShip, CSpaceObject **retpWreck = NULL);
		bool FindDataField (const CString &sField, CString *retsValue);
		inline void GenerateDevices (CDeviceDescList &Devices) { Devices.RemoveAll(); if (m_pDevices) m_pDevices->AddDevices(Devices); }
		CString GenerateShipName (DWORD *retdwFlags);
		inline const SAISettings &GetAISettings (void) { return m_AISettings; }
		inline const CString &GetAttributes (void) { return m_sAttributes; }
		inline int GetCargoSpace (void) { return m_iCargoSpace; }
		inline CCommunicationsHandler *GetCommsHandler (void) { return (m_CommsHandler.GetCount() ? &m_CommsHandler : NULL); }
		inline int GetCyberDefenseLevel (void) { return m_iCyberDefenseLevel; }
		inline CString GetDataField (const CString &sField) { CString sValue; FindDataField(sField, &sValue); return sValue; }
		inline DWORD GetDefaultBkgnd (void) { return m_dwDefaultBkgnd; }
		inline int GetDockingPortCount (void) { return m_iDockingPortsCount; }
		inline CVector *GetDockingPortPositions (void) { return m_DockingPorts; }
		void GetDriveDesc (DriveDesc *retDriveDesc);
		CWeaponFireDesc *GetExplosionType (void) { return m_pExplosionType; }
		inline const DriveDesc *GetHullDriveDesc (void) { return &m_DriveDesc; }
		inline int GetHullMass (void) { return m_iMass; }
		inline int GetHullSectionCount (void) { return m_Hull.GetCount(); }
		inline HullSection *GetHullSection (int iIndex) { return (HullSection *)m_Hull.GetStruct(iIndex); }
		int GetHullSectionAtAngle (int iAngle);
		inline const CObjectImageArray &GetImage (void) { return m_Image; }
		inline const CObjectImageArray &GetImageSmall (void) { return m_Image; }
		inline int GetLevel (void) { return m_iLevel; }
		inline int GetManeuverability (void) { return m_iManeuverability; }
		inline int GetManeuverDelay (void) { return m_iManeuverDelay; }
		inline int GetMaxArmorMass (void) const { return m_iMaxArmorMass; }
		inline int GetMaxCargoSpace (void) const { return m_iMaxCargoSpace; }
		inline int GetMaxDevices (void) const { return m_iMaxDevices; }
		inline int GetMaxNonWeapons (void) const { return m_iMaxNonWeapons; }
		inline int GetMaxReactorPower (void) const { return m_iMaxReactorPower; }
		inline int GetMaxStructuralHitPoints (void) { return m_iStructuralHP; }
		inline int GetMaxWeapons (void) const { return m_iMaxWeapons; }
		CString GetName (DWORD *retdwFlags = NULL);
		CString GetNounPhrase (DWORD dwFlags);
		inline const SPlayerSettings *GetPlayerSettings (void) { return m_pPlayerSettings; }
		inline IItemGenerator *GetRandomItemTable (void) { return m_pItems; }
		inline const ReactorDesc *GetReactorDesc (void) { return &m_ReactorDesc; }
		inline int GetRotationAngle (void) { return m_iRotationAngle; }
		inline int GetRotationRange (void) { return m_iRotationRange; }
		inline int GetScore (void) { return m_iScore; }
		CXMLElement *GetScreen (const CString &sName);
		inline DWORD GetShipNameFlags (void) { return m_dwShipNameFlags; }
		CString GetShortName (void);
		inline const CString &GetClassName (void) { return m_sName; }
		inline const CString &GetManufacturerName (void) { return m_sManufacturer; }
		inline const CString &GetTypeName (void) { return m_sTypeName; }
		inline int GetWreckChance (void) { return m_iLeavesWreck; }
		void GetWreckImage (CObjectImageArray *retWreckImage);
		int GetWreckImageVariants (void);
		inline bool HasAttribute (const CString &sAttrib) { return ::HasModifier(m_sAttributes, sAttrib); }
		inline bool HasDockingPorts (void) { return (m_fHasDockingPorts ? true : false); }
		inline bool HasOnAttackedByPlayerEvent (void) const { return (m_fHasOnAttackedByPlayerEvent ? true : false); }
		inline bool HasOnObjDockedEvent (void) const { return (m_fHasOnObjDockedEvent ? true : false); }
		inline bool HasOnOrderChangedEvent (void) const { return (m_fHasOnOrderChangedEvent ? true : false); }
		inline bool HasOnOrdersCompletedEvent (void) const { return (m_fHasOnOrdersCompletedEvent ? true : false); }
		inline bool IsDebugOnly (void) { return (m_pPlayerSettings && m_pPlayerSettings->m_fDebug); }
		inline bool IsPlayerShip (void) { return (m_pPlayerSettings != NULL); }
		inline bool IsShownAtNewGame (void) { return (m_pPlayerSettings && m_pPlayerSettings->m_fInitialClass); }
		inline bool IsTimeStopImmune (void) { return (m_fTimeStopImmune ? true : false); }
		void LoadImages (void);
		void MarkImages (void);
		void Paint (CG16bitImage &Dest, 
					int x, 
					int y, 
					const ViewportTransform &Trans, 
					int iDirection, 
					int iTick, 
					bool bThrusting,
					bool bRadioactive);
		void PaintMap (CG16bitImage &Dest, 
					int x, 
					int y, 
					const ViewportTransform &Trans, 
					int iDirection, 
					int iTick, 
					bool bThrusting,
					bool bRadioactive);
		void SetShipwreckImage (CStation *pStation);
		void SetShipwreckParams (CStation *pStation, CShip *pShip);

		//	CDesignType overrides
		static CShipClass *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designShipClass) ? (CShipClass *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designShipClass; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		enum PrivateConstants
			{
			maxExhaustImages = 5
			};

		int ComputeDeviceLevel (CDeviceClass *pDevice);
		CString ComposeLoadError (const CString &sError);
		static int ComputeLevel (int iScore);
		void ComputeMovementStats (CDeviceDescList &Devices, int *retiSpeed, int *retiThrust, int *retiManeuver);
		int ComputeScore (const CDeviceDescList &Devices,
						  int iArmorLevel,
						  int iPrimaryWeapon,
						  int iSpeed,
						  int iThrust,
						  int iManeuver,
						  bool bPrimaryIsLauncher);
		void CreateWreckImage (void);
		void FindBestMissile (IItemGenerator *pItems, int *ioLevel);
		CStationType *GetWreckDesc (void);

		CString m_sManufacturer;				//	Name of manufacturer
		CString m_sName;						//	Class name
		CString m_sTypeName;					//	Name of type
		DWORD m_dwClassNameFlags;				//	Flags for class name
		CString m_sShipNames;					//	Names to use for individual ship
		DWORD m_dwShipNameFlags;				//	Flags for ship name
		int m_iScore;							//	Score when destroyed
		int m_iLevel;							//	Ship class level
		CString m_sAttributes;					//	Attributes of class

		int m_iMass;							//	Empty mass (tons)
		int m_iCargoSpace;						//	Available cargo space (tons)
		int m_iRotationRange;					//	Number of rotation positions
		int m_iRotationAngle;					//	Angler per rotation
		int m_iManeuverability;					//	Ticks per turn angle
		int m_iManeuverDelay;					//	Ticks per turn angle
		DriveDesc m_DriveDesc;					//	Drive descriptor
		ReactorDesc m_ReactorDesc;				//	Reactor descriptor
		int m_iCyberDefenseLevel;				//	Cyber defense level

		int m_iMaxArmorMass;					//	Max mass of single armor segment
		int m_iMaxCargoSpace;					//	Max amount of cargo space with expansion (tons)
		int m_iMaxReactorPower;					//	Max compatible reactor power
		int m_iMaxDevices;						//	Max number of devices
		int m_iMaxWeapons;						//	Max number of weapon devices (including launchers)
		int m_iMaxNonWeapons;					//	Max number of non-weapon devices

		int m_iLeavesWreck;						//	Chance that it leaves a wreck
		int m_iStructuralHP;					//	Structual hp of wreck

		CStructArray m_Hull;					//	Array of HullSections
		IDeviceGenerator *m_pDevices;			//	Generator of devices
		CDeviceDescList m_AverageDevices;		//	Average complement of devices (only for stats)

		SAISettings m_AISettings;				//	AI controller data
		SPlayerSettings *m_pPlayerSettings;		//	Player settings data
		IItemGenerator *m_pItems;				//	Random items

		//	Docking
		int m_iDockingPortsCount;				//	Number of docking ports
		CVector *m_DockingPorts;			//	Position of docking ports
		CXMLElement *m_pDockScreens;			//	All dock screens
		CDockScreenTypeRef m_pDefaultScreen;	//	Default screen
		DWORD m_dwDefaultBkgnd;					//	Default background screen

		CCommunicationsHandler m_CommsHandler;	//	Communications handler

		//	Image
		CObjectImageArray m_Image;				//	Image of ship

		//	Wreck image
		CG16bitImage m_WreckImage;				//	Image to use when ship is wrecked

		//	Explosion
		CWeaponFireDescRef m_pExplosionType;	//	Explosion to create when ship is destroyed

		//	Exhaust
		CObjectImageArray m_ExhaustImage;		//	Image of drive exhaust
		int m_iExhaustImageCount;				//	Number of exhaust jets
		POINT *m_ptExhaustOffset[maxExhaustImages];
		DWORD m_dwExhaustExclusion[maxExhaustImages];

		//	Misc
		DWORD m_fRadioactiveWreck:1;			//	TRUE if wreck is always radioactive
		DWORD m_fHasDockingPorts:1;				//	TRUE if ship has docking ports
		DWORD m_fTimeStopImmune:1;				//	TRUE if ship is immune to stop-time
		DWORD m_fHasOnOrderChangedEvent:1;		//	TRUE if ship has an event on each new order
		DWORD m_fHasOnAttackedByPlayerEvent:1;	//	TRUE if ship has an event when attacked by player
		DWORD m_fHasOnObjDockedEvent:1;			//	TRUE if ship has an event when other ships dock
		DWORD m_fHasOnOrdersCompletedEvent:1;	//	TRUE if ship has an event when orders complete
		DWORD m_fSpare:24;
	};

//	CEffectCreator ------------------------------------------------------------

class CEffectCreator : public CDesignType
	{
	public:
		CEffectCreator (void) : m_dwSoundUNID(0), m_iSound(-1) { }

		static ALERROR CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, CEffectCreator **retpCreator);
		static ALERROR CreateSimpleFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CEffectCreator **retpCreator);
		static IEffectPainter *CreatePainterFromStream (SLoadCtx &Ctx);
		IEffectPainter *CreatePainterFromStreamAndCreator (SLoadCtx &Ctx);
		static CEffectCreator *FindEffectCreator (const CString &sUNID);

		inline int GetSound (void) { return m_iSound; }
		inline const CString &GetUNIDString (void) { return m_sUNID; }
		bool IsValidUNID (void);
		void PlaySound (CSpaceObject *pSource = NULL);

		//	Virtuals

		virtual ~CEffectCreator (void) { }
		virtual ALERROR CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel);
		virtual IEffectPainter *CreatePainter (void) { ASSERT(false); return NULL; }
		virtual int GetLifetime (void) { return 0; }
		virtual void LoadImages (void) { }
		virtual void MarkImages (void) { }
		virtual void SetLifetime (int iLifetime) { }
		virtual void SetSpeed (int iSpeed) { }
		virtual void SetVariants (int iVariants) { }

		//	CDesignType overrides
		static CEffectCreator *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designEffectType) ? (CEffectCreator *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designEffectType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual CEffectCreator *OnFindEffectCreator (const CString &sUNID) { return this; }

		//	CEffectCreator overrides

		virtual ALERROR OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc) { return NOERROR; }
		virtual ALERROR OnEffectBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }

	private:
		CString m_sUNID;
		DWORD m_dwSoundUNID;
		int m_iSound;
	};

//	CEnergyFieldType ----------------------------------------------------------

class CEnergyFieldType : public CDesignType
	{
	public:
		CEnergyFieldType (void);
		virtual ~CEnergyFieldType (void);

//		int AbsorbDamage (const DamageDesc &Damage, int iDamage);
		bool AbsorbsWeaponFire (CInstalledDevice *pWeapon);
		int GetDamageAbsorbed (CSpaceObject *pSource, SDamageCtx &Ctx);
		inline CEffectCreator *GetEffectCreator (void) const { return m_pEffect; }
		inline CEffectCreator *GetHitEffectCreator (void) const { return m_pHitEffect; }
		int GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource);

		//	CDesignType overrides
		static CEnergyFieldType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designEnergyFieldType) ? (CEnergyFieldType *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designEnergyFieldType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		int m_iAbsorbAdj[damageCount];			//	Damage absorbed by the field
		DamageTypeSet m_WeaponSuppress;			//	Types of weapons suppressed
		int m_iBonusAdj[damageCount];			//	Adjustment to weapons damage

		CEffectCreator *m_pEffect;				//	Effect for field
		CEffectCreator *m_pHitEffect;			//	Effect when field is hit by damage
	};

//	CSystemType ---------------------------------------------------------------

class CSystemType : public CDesignType
	{
	public:
		CSystemType (void);
		virtual ~CSystemType (void);

		ALERROR FireOnCreate (CString *retsError = NULL);
		inline CXMLElement *GetDesc (void) { return m_pDesc; }
		CXMLElement *GetLocalSystemTables (void);

		//	CDesignType overrides
		static CSystemType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designSystemType) ? (CSystemType *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designSystemType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CXMLElement *m_pDesc;
	};

//	CDockScreenType -----------------------------------------------------------

class CDockScreenType : public CDesignType
	{
	public:
		CDockScreenType (void);
		virtual ~CDockScreenType (void);

		inline CXMLElement *GetDesc (void) { return m_pDesc; }

		//	CDesignType overrides
		static CDockScreenType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designDockScreen) ? (CDockScreenType *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designDockScreen; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CXMLElement *m_pDesc;
	};

//	CStationType --------------------------------------------------------------

class CStationType : public CDesignType
	{
	public:
		CStationType (void);
		static ALERROR InitFromXML (CXMLElement *pDesc);
		virtual ~CStationType (void);

		inline bool AlertWhenAttacked (void) { return (mathRandom(1, 100) <= m_iAlertWhenAttacked); }
		inline bool AlertWhenDestroyed (void) { return (mathRandom(1, 100) <= m_iAlertWhenDestroyed); }
		inline bool CanAttack (void) const { return (m_fCanAttack ? true : false); }
		inline bool CanBeEncountered (void) { return (!IsUnique() || !m_fEncountered); }
		bool CanBeEncountered (CSystem *pSystem);
		inline bool CanBeHitByFriends (void) { return (m_fNoFriendlyTarget ? false : true); }
		inline bool CanHitFriends (void) { return (m_fNoFriendlyFire ? false : true); }
		bool FindDataField (const CString &sField, CString *retsValue);
		inline CXMLElement *GetAbandonedScreen (void) { return m_pAbandonedDockScreen.GetDesc(); }
		inline CArmorClass *GetArmorClass (void) { return (m_pArmor ? m_pArmor->GetArmorClass() : NULL); }
		inline const CString &GetAttributes (void) { return m_sAttributes; }
		inline CEffectCreator *GetBarrierEffect (void) { return m_pBarrierEffect; }
		inline IShipGenerator *GetConstructionTable (void) { return m_pConstruction; }
		CSovereign *GetControllingSovereign (void);
		inline CString GetDataField (const CString &sField) { CString sValue; FindDataField(sField, &sValue); return sValue; }
		inline DWORD GetDefaultBkgnd (void) { return m_dwDefaultBkgnd; }
		inline CXMLElement *GetDesc (void) { return m_pDesc; }
		inline CString GetDestNodeID (void) { return m_sStargateDestNode; }
		inline CString GetDestEntryPoint (void) { return m_sStargateDestEntryPoint; }
		inline int GetEjectaAdj (void) { return m_iEjectaAdj; }
		CWeaponFireDesc *GetEjectaType (void) { return m_pEjectaType; }
		CWeaponFireDesc *GetExplosionType (void) { return m_pExplosionType; }
		inline int GetEncounterFrequency (void) { return m_iEncounterFrequency; }
		inline IShipGenerator *GetEncountersTable (void) { return m_pEncounters; }
		inline int GetFireRateAdj (void) { return m_iFireRateAdj; }
		inline CXMLElement *GetFirstDockScreen (void) { return m_pFirstDockScreen.GetDesc(); }
		int GetFrequencyByLevel (int iLevel);
		int GetFrequencyForSystem (CSystem *pSystem);
		inline CEffectCreator *GetGateEffect (void) { return m_pGateEffect; }
		inline const CObjectImageArray &GetImage (int iVariant, int *retiRotation = NULL) { return m_Image.GetImage(iVariant, retiRotation); }
		inline int GetImageVariants (void) { return m_iImageVariants; }
		inline int GetInitialHitPoints (void) { return m_iHitPoints; }
		inline IShipGenerator *GetInitialShips (void) { return m_pInitialShips; }
		int GetLevel (void);
		inline const CString &GetLocationCriteria (void) { return m_sLocationCriteria; }
		inline Metric GetMass (void) { return m_rMass; }
		inline int GetMinShips (void) { return m_iMinShips; }
		inline Metric GetMaxEffectiveRange (void) { return m_rMaxAttackDistance; }
		inline int GetMaxHitPoints (void) { return m_iMaxHitPoints; }
		inline int GetMaxShipConstruction (void) { return m_iMaxConstruction; }
		inline int GetMaxStructuralHitPoints (void) { return m_iStructuralHP; }
		inline const CString &GetName (void) { return m_sName; }
		inline IItemGenerator *GetRandomItemTable (void) { return m_pItems; }
		inline IShipGenerator *GetReinforcementsTable (void) { return m_pReinforcements; }
		inline int GetRepairRate (void) { return m_iRepairRate; }
		inline CXMLElement *GetSatellitesDesc (void) { return m_pSatellitesDesc; }
		inline ScaleTypes GetScale (void) const { return m_iScale; }
		CXMLElement *GetScreen (const CString &sName);
		inline int GetShipConstructionRate (void) { return m_iShipConstructionRate; }
		inline int GetShipRepairRate (void) { return m_iShipRepairRate; }
		inline CSovereign *GetSovereign (void) const { return m_pSovereign; }
		inline COLORREF GetSpaceColor (void) { return m_rgbSpaceColor; }
		inline CTradingDesc *GetTradingDesc (void) { return m_pTrade; }
		inline const CObjectImageArray &GetWreckImage (int iVariant) { return m_WreckImage[iVariant]; }
		inline bool HasAnimations (void) const { return (m_pAnimations != NULL); }
		inline bool HasAttribute (const CString &sAttribute) const { return ::HasModifier(m_sAttributes, sAttribute); }
		inline bool HasOnObjDockedEvent (void) const { return (m_fHasOnObjDockedEvent ? true : false); }
		inline bool HasWreckImage (void) { return m_WreckImage != NULL; }
		inline bool IsActive (void) { return (m_fInactive ? false : true); }
		inline bool IsBackgroundObject (void) { return (m_fBackground ? true : false); }
		inline bool IsBeacon (void) { return (m_fBeacon ? true : false); }
		inline bool IsBlacklistEnabled (void) { return (m_fNoBlacklist ? false : true); }
		inline bool IsDestroyWhenEmpty (void) { return (m_fDestroyWhenEmpty ? true : false); }
		inline bool IsEnemyDockingAllowed (void) { return (m_fAllowEnemyDocking ? true : false); }
		inline bool IsImmutable (void) { return (m_fImmutable ? true : false); }
		inline bool IsMultiHull (void) { return (m_fMultiHull ? true : false); }
		inline bool IsMobile (void) { return (m_fMobile ? true : false); }
		inline bool IsRadioactive (void) { return (m_fRadioactive ? true : false); }
		inline bool IsSign (void) { return (m_fSign ? true : false); }
		inline bool IsShipEncounter (void) { return (m_fShipEncounter ? true : false); }
		inline bool IsStatic (void) { return (m_fStatic ? true : false); }
		inline bool IsTimeStopImmune (void) { return (m_fTimeStopImmune ? true : false); }
		inline bool IsUnique (void) { return (m_fUnique ? true : false); }
		inline bool IsUniqueInSystem (void) { return (m_fUniqueInSystem ? true : false); }
		inline bool IsWall (void) { return (m_fWall ? true : false); }
		void LoadImages (int iVariant);
		void MarkImages (int iVariant);
		void PaintAnimations (CG16bitImage &Dest, int x, int y, int iTick);
		void SetCustomImage (CStation *pStation);
		inline void SetEncountered (bool bEncountered = true) { m_fEncountered = bEncountered; }
		inline bool ShowsMapIcon (void) { return (m_fNoMapIcon ? false : true); }
		inline bool UsesDefiniteArticle (void) { return (m_fDefiniteArticle ? true : false); }
		inline bool UsesNoArticle (void) { return (m_fNoArticle ? true : false); }
		inline bool UsesReverseArticle (void) { return (m_fReverseArticle ? true : false); }

		//	CDesignType overrides
		static CStationType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designStationType) ? (CStationType *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designStationType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		struct SAnimationSection
			{
			int m_x;
			int m_y;
			CObjectImageArray m_Image;
			};

		CString ComposeLoadError (const CString &sError);
		void CreateWreckImage (void);

		CXMLElement *m_pDesc;

		//	Basic station descriptors
		CString m_sName;								//	Name
		CSovereignRef m_pSovereign;						//	Sovereign
		ScaleTypes m_iScale;							//	Scale
		int m_iLevel;
		Metric m_rMass;									//	Mass of station
		int m_iFireRateAdj;								//	Fire rate adjustment

		//	Armor & HP
		CItemTypeRef m_pArmor;							//	Armor class
		int m_iHitPoints;								//	Hit points at creation time
		int m_iMaxHitPoints;							//	Max hit points
		int m_iRepairRate;								//	HP repaired every 10 ticks

		int m_iStructuralHP;							//	Structural hit points (0 = station is permanent)

		//	Devices
		int m_iDevicesCount;							//	Number of devices in array
		CInstalledDevice *m_Devices;					//	Device array

		//	Items
		IItemGenerator *m_pItems;						//	Random item table
		CTradingDesc *m_pTrade;							//	Trading structure

		DWORD m_fMobile:1;								//	Station moves
		DWORD m_fWall:1;								//	Station is a wall
		DWORD m_fInactive:1;							//	Station starts inactive
		DWORD m_fDestroyWhenEmpty:1;					//	Station is destroyed when last item removed
		DWORD m_fAllowEnemyDocking:1;					//	Station allows enemies to dock
		DWORD m_fNoFriendlyFire:1;						//	Station cannot hit friends
		DWORD m_fSign:1;								//	Station is a text sign
		DWORD m_fBeacon:1;								//	Station is a nav beacon
		DWORD m_fRadioactive:1;							//	Station is radioactive
		DWORD m_fUnique:1;								//	Station is unique
		DWORD m_fEncountered:1;							//	Unique station has already been encountered
		DWORD m_fCanAttack:1;							//	Station is active (i.e., will react if attacked)
		DWORD m_fDefiniteArticle:1;						//	Use "the" instead of "a" for name
		DWORD m_fNoArticle:1;							//	Don't use an article for name
		DWORD m_fShipEncounter:1;						//	This is a ship encounter
		DWORD m_fImmutable:1;							//	Station can not take damage or become radioactive, etc.
		DWORD m_fNoMapIcon:1;							//	Do not show on map
		DWORD m_fMultiHull:1;							//	Only harmed by WMD damage
		DWORD m_fTimeStopImmune:1;						//	TRUE if station is immune to time-stop
		DWORD m_fNoBlacklist:1;							//	Does not blacklist player if attacked
		DWORD m_fReverseArticle:1;						//	Use "a" instead of "an" and vice versa
		DWORD m_fHasOnObjDockedEvent:1;					//	Station has OnObjDocked event
		DWORD m_fStatic:1;								//	Use CStatic instead of CStation
		DWORD m_fBackground:1;							//	Background object
		DWORD m_fUniqueInSystem:1;						//	Unique in system
		DWORD m_fNoFriendlyTarget:1;					//	Station cannot be hit by friends
		DWORD m_fSpare:6;

		//	Images
		CCompositeImageDesc m_Image;
		int m_iImageVariants;							//	Number of variants
		CIntArray m_ShipWrecks;							//	Class IDs to use as image (for shipwrecks)
		int m_iAnimationsCount;							//	Number of animation sections
		SAnimationSection *m_pAnimations;				//	Animation sections (may be NULL)

		CObjectImageArray *m_WreckImage;				//	Image to use when wrecked
		CG16bitImage *m_WreckImageCache;				//	Image to use when station is wrecked

		//	Docking
		CDockScreenTypeRef m_pFirstDockScreen;			//	First screen (may be NULL)
		CDockScreenTypeRef m_pAbandonedDockScreen;		//	Screen to use when abandoned (may be NULL)
		CXMLElement *m_pLocalDockScreens;				//	Local screens (may be NULL)
		DWORD m_dwDefaultBkgnd;							//	Default background screen

		//	Behaviors
		int m_iAlertWhenAttacked;						//	Chance that station will warn others when attacked
		int m_iAlertWhenDestroyed;						//	Chance that station will warn others when destroyed
		Metric m_rMaxAttackDistance;					//	Max range at which station guns attack

		//	Random population
		CString m_sLevelFrequency;						//	String array of frequency distribution by level
		CString m_sAttributes;							//	Attributes of type
		CString m_sLocationCriteria;					//	Criteria for location

		//	Ships
		IShipGenerator *m_pInitialShips;				//	Ships at creation time
		IShipGenerator *m_pReinforcements;				//	Reinforcements table
		int m_iMinShips;								//	Min ships at station
		IShipGenerator *m_pEncounters;					//	Random encounters table
		int m_iEncounterFrequency;						//	Frequency of random encounter
		int m_iShipRepairRate;							//	HP repaired every 10 ticks (per docked ship)
		IShipGenerator *m_pConstruction;				//	Ships built by station
		int m_iShipConstructionRate;					//	Ticks between each construction
		int m_iMaxConstruction;							//	Stop building when we get this many ships

		//	Satellites
		CXMLElement *m_pSatellitesDesc;

		//	Explosion
		CWeaponFireDescRef m_pExplosionType;			//	Explosion to create when station is destroyed

		//	Ejecta
		int m_iEjectaAdj;								//	Adjustment to probability for ejecta when hit by weapon
														//		0 = no chance of ejecta
														//		100 = normal chance
														//		>100 = greater than normal chance
		CWeaponFireDescRef m_pEjectaType;				//	Type of ejecta generated

		//	Miscellaneous
		COLORREF m_rgbSpaceColor;						//	Space color
		CEffectCreatorRef m_pBarrierEffect;				//	Effect when object hits station
		CSovereignRef m_pControllingSovereign;			//	If controlled by different sovereign
														//	(e.g., centauri occupation)
		CString m_sStargateDestNode;					//	Dest node
		CString m_sStargateDestEntryPoint;				//	Dest entry point
		CEffectCreatorRef m_pGateEffect;				//	Effect when object gates in/out of station
	};

//	CSovereign ----------------------------------------------------------------

class CSovereign : public CDesignType
	{
	public:
		enum Disposition
			{
			dispEnemy = 0,
			dispNeutral = 1,
			dispFriend = 2,
			};

		CSovereign (void);
		~CSovereign (void);

		void DeleteRelationships (void);
		inline void FlushEnemyObjectCache (void) { m_pEnemyObjectsSystem = NULL; }
		inline AlignmentTypes GetAlignment (void) { return m_iAlignment; }
		Disposition GetDispositionTowards (CSovereign *pSovereign);
		inline const CSpaceObjectList &GetEnemyObjectList (CSystem *pSystem) { InitEnemyObjectList(pSystem); return m_EnemyObjects; }
		const CString &GetText (MessageTypes iMsg);
		inline bool IsEnemy (CSovereign *pSovereign) { return (GetDispositionTowards(pSovereign) == dispEnemy); }
		inline bool IsFriend (CSovereign *pSovereign) { return (GetDispositionTowards(pSovereign) == dispFriend); }
		void SetDispositionTowards (CSovereign *pSovereign, Disposition iDisp);

		//	CDesignType overrides
		static CSovereign *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designSovereign) ? (CSovereign *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designSovereign; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual void OnReadFromStream (SUniverseLoadCtx &Ctx);
		virtual void OnReinit (void);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		struct SRelationship
			{
			CSovereign *pSovereign;
			Disposition iDisp;

			SRelationship *pNext;
			};

		SRelationship *FindRelationship (CSovereign *pSovereign);
		void InitEnemyObjectList (CSystem *pSystem);
		void InitRelationships (void);

		CString m_sName;
		AlignmentTypes m_iAlignment;
		CIDTable m_Language;
		CXMLElement *m_pInitialRelationships;

		SRelationship *m_pFirstRelationship;	//	List of individual relationships

		CSystem *m_pEnemyObjectsSystem;			//	System that we've cached enemy objects
		CSpaceObjectList m_EnemyObjects;		//	List of enemy objects that can attack
	};

//	CPower --------------------------------------------------------------------

class CPower : public CDesignType
	{
	public:
		CPower (void);
		virtual ~CPower (void);

		inline ICCItem *GetCode (void) { return m_pCode; }
		inline int GetInvokeCost (void) { return m_iInvokeCost; }
		inline const CString &GetInvokeKey (void) { return m_sInvokeKey; }
		inline const CString &GetName (void) { return m_sName; }
		inline ICCItem *GetOnInvokedByPlayer (void) { return m_pOnInvokedByPlayer; }
		inline ICCItem *GetOnShow (void) { return m_pOnShow; }
		inline bool HasModifier (const CString &sModifier) { return ::HasModifier(m_sModifiers, sModifier); }
		void Invoke (CSpaceObject *pSource, CSpaceObject *pTarget, CString *retsError = NULL);
		void InvokeByPlayer (CSpaceObject *pSource, CSpaceObject *pTarget, CString *retsError = NULL);
		bool OnShow (CSpaceObject *pSource, CSpaceObject *pTarget, CString *retsError = NULL);
		bool OnDestroyCheck (CSpaceObject *pSource, DestructionTypes iCause, CSpaceObject *pCause);

		//	CDesignType overrides
		static CPower *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designPower) ? (CPower *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designPower; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx) { return NOERROR; }
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CString m_sName;
		int m_iInvokeCost;
		CString m_sInvokeKey;
		CString m_sModifiers;

		ICCItem *m_pCode;
		ICCItem *m_pOnShow;
		ICCItem *m_pOnInvokedByPlayer;
		ICCItem *m_pOnDestroyCheck;
	};

//	CSpaceEnvironmentType -----------------------------------------------------

class CSpaceEnvironmentType : public CDesignType
	{
	public:
		CSpaceEnvironmentType (void) { }

		inline Metric GetDragFactor (void) { return m_rDragFactor; }
		inline bool HasAttribute (const CString &sAttribute) const { return ::HasModifier(m_sAttributes, sAttribute); }
		inline bool IsLRSJammer (void) { return m_bLRSJammer; }
		inline bool IsShieldJammer (void) { return m_bShieldJammer; }
		inline bool IsSRSJammer (void) { return m_bSRSJammer; }
		void Paint (CG16bitImage &Dest, int x, int y);
		void PaintLRS (CG16bitImage &Dest, int x, int y);

		//	CDesignType overrides
		static CSpaceEnvironmentType *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designSpaceEnvironmentType) ? (CSpaceEnvironmentType *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designSpaceEnvironmentType; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CObjectImageArray m_Image;

		CString m_sAttributes;			//	Attributes of environment
		bool m_bLRSJammer;				//	If TRUE, LRS is disabled
		bool m_bShieldJammer;			//	If TRUE, shields are disabled
		bool m_bSRSJammer;				//	If TRUE, SRS is disabled
		Metric m_rDragFactor;			//	Coefficient of drag (1.0 = no drag)

		//	We check for damage every 20 ticks
		int m_iDamageChance;			//	Chance of damage
		DamageDesc m_Damage;			//	Damage caused to objects in environment
	};

//	CShipTable ----------------------------------------------------------------

class CShipTable : public CDesignType
	{
	public:
		CShipTable (void);
		virtual ~CShipTable (void);

		inline void CreateShips (SShipCreateCtx &Ctx) { if (m_pGenerator) m_pGenerator->CreateShips(Ctx); }

		//	CDesignType overrides
		static CShipTable *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designShipTable) ? (CShipTable *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designShipTable; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		IShipGenerator *m_pGenerator;
	};

//	Adventures and Extensions -------------------------------------------------

class CAdventureDesc : public CDesignType
	{
	public:
		void FireOnGameEnd (const SGameStats &Game);
		void FireOnGameStart (void);
		inline DWORD GetExtensionUNID (void) { return m_dwExtensionUNID; }
		inline DWORD GetBackgroundUNID (void) { return m_dwBackgroundUNID; }
		inline const CString &GetDesc (void) { return m_sDesc; }
		inline const CString &GetFilespec (void) { return m_sFilespec; }
		inline const CString &GetName (void) { return m_sName; }

		//	CDesignType overrides
		static CAdventureDesc *AsType (CDesignType *pType) { return ((pType && pType->GetType() == designAdventureDesc) ? (CAdventureDesc *)pType : NULL); }
		virtual DesignTypes GetType (void) { return designAdventureDesc; }

	protected:
		//	CDesignType overrides
		virtual ALERROR OnBindDesign (SDesignLoadCtx &Ctx);
		virtual ALERROR OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);

	private:
		CString m_sFilespec;
		DWORD m_dwExtensionUNID;

		CString m_sName;
		DWORD m_dwBackgroundUNID;
		CString m_sDesc;
	};

//	Associated Structures -----------------------------------------------------

class CInstalledDevice
	{
	public:
		CInstalledDevice (void);

		inline bool AbsorbDamage (CShip *pShip, SDamageCtx &Ctx) { if (!IsEmpty()) return m_pClass->AbsorbDamage(this, pShip, Ctx); else return false; }
		inline void Activate (CSpaceObject *pSource, 
							  CSpaceObject *pTarget,
							  int iFireAngle,
							  bool *retbSourceDestroyed,
							  bool *retbConsumedItems = NULL)
			{ m_pClass->Activate(this, pSource, pTarget, iFireAngle, retbSourceDestroyed, retbConsumedItems); }
		int CalcPowerUsed (CSpaceObject *pSource);
		inline bool CanBeDamaged (void) { return m_pClass->CanBeDamaged(); }
		inline bool CanBeDisabled (void) { return m_pClass->CanBeDisabled(); }
		inline void Deplete (CSpaceObject *pSource) { m_pClass->Deplete(this, pSource); }
		inline int GetActivateDelay (CSpaceObject *pSource) { return m_pClass->GetActivateDelay(this, pSource); }
		inline int GetBonus (void) const { return m_iBonus; }
		inline ItemCategories GetCategory (void) { return m_pClass->GetCategory(); }
		inline int GetCharges (CSpaceObject *pSource) { return m_iCharges; }
		inline CDeviceClass *GetClass (void) const { return m_pClass; }
		inline int GetCounter (CDeviceClass::CounterTypes *retiCounter = NULL) { return m_pClass->GetCounter(this, retiCounter); }
		inline const DamageDesc *GetDamageDesc (void) { return m_pClass->GetDamageDesc(this); }
		inline int GetDamageType (void) { return m_pClass->GetDamageType(this); }
		inline DWORD GetData (void) const { return m_dwData; }
		inline int GetDeviceSlot (void) const { return m_iDeviceSlot; }
		inline const DriveDesc *GetDriveDesc (CSpaceObject *pSource) { return m_pClass->GetDriveDesc(this, pSource); }
		inline const ReactorDesc *GetReactorDesc (CSpaceObject *pSource) { return m_pClass->GetReactorDesc(this, pSource); }
		inline int GetFireArc (void) const { return (IsOmniDirectional() ? 360 : AngleRange(m_iMinFireArc, m_iMaxFireArc)); }
		CVector GetPos (CSpaceObject *pSource);
		inline int GetFireAngle (void) const { return m_iFireAngle; }
		inline int GetMinFireAngle (void) const { return m_iMinFireArc; }
		inline Metric GetMaxEffectiveRange (CSpaceObject *pSource, CSpaceObject *pTarget = NULL) { return m_pClass->GetMaxEffectiveRange(pSource, this, pTarget); }
		inline int GetMaxFireAngle (void) const { return m_iMaxFireArc; }
		inline CItemEnhancement GetMods (void) const { return m_Mods; }
		inline CString GetName (void) { return m_pClass->GetName(); }
		inline int GetPosAngle (void) const { return m_iPosAngle; }
		inline int GetPowerRating (const CItem *pItem) { return m_pClass->GetPowerRating(pItem); }
		inline int GetRotation (void) const { return m_iMinFireArc; }
		inline void GetSelectedVariantInfo (CSpaceObject *pSource, 
											CString *retsLabel,
											int *retiAmmoLeft,
											CItemType **retpType = NULL)
			{ m_pClass->GetSelectedVariantInfo(pSource, this, retsLabel, retiAmmoLeft, retpType); }
		inline void GetStatus (CShip *pShip, int *retiStatus, int *retiMaxStatus) { m_pClass->GetStatus(this, pShip, retiStatus, retiMaxStatus); }
		inline int GetTemperature (void) const { return m_iTemperature; }
		inline int GetTimeUntilReady (void) const { return m_iActivationDelay; }
		inline int GetValidVariantCount (CSpaceObject *pSource) { return m_pClass->GetValidVariantCount(pSource, this); }
		inline int GetWeaponBonus (CSpaceObject *pSource, CInstalledDevice *pWeapon, CString *retsBonusType) { return m_pClass->GetWeaponBonus(this, pSource, pWeapon, retsBonusType); }
		inline int GetWeaponEffectiveness (CSpaceObject *pSource, CSpaceObject *pTarget) { return m_pClass->GetWeaponEffectiveness(pSource, this, pTarget); }
		int IncCharges (CSpaceObject *pSource, int iChange);
		inline void IncTemperature (int iChange) { m_iTemperature += iChange; }
		void InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		void InitFromDesc (const SDeviceDesc &Desc);
		void Install (CSpaceObject *pObj, CItemListManipulator &ItemList, int iDeviceSlot);
		inline bool IsAutomatedWeapon (void) { return m_pClass->IsAutomatedWeapon(); }
		inline bool IsAreaWeapon (CSpaceObject *pSource) { return m_pClass->IsAreaWeapon(pSource, this); }
		inline bool IsDamaged (void) const { return m_fDamaged; }
		inline bool IsDirectional (void) const { return (m_iMinFireArc != m_iMaxFireArc); }
		inline bool IsEmpty (void) const { return m_pClass == NULL; }
		inline bool IsEnabled (void) const { return m_fEnabled; }
		inline bool IsEnhanced (void) const { return m_fEnhanced; }
		inline bool IsExternal (void) const { return m_fExternal; }
		inline bool IsReady (void) const { return (m_iActivationDelay == 0); }
		inline bool IsOmniDirectional (void) const { return (m_fOmniDirectional ? true : false); }
		inline bool IsOptimized (void) const { return m_fOptimized; }
		inline bool IsOverdrive (void) const { return m_fOverdrive; }
		inline bool IsSecondaryWeapon (void) const { return (m_fSecondaryWeapon ? true : false); }
		inline bool IsVariantSelected (CSpaceObject *pSource) { return m_pClass->IsVariantSelected(pSource, this); }
		inline bool IsWaiting (void) const { return (m_fWaiting ? true : false); }
		inline bool IsWeaponAligned (CSpaceObject *pShip, CSpaceObject *pTarget, int *retiAimAngle, int *retiFireAngle = NULL) { return m_pClass->IsWeaponAligned(pShip, this, pTarget, retiAimAngle, retiFireAngle); }
		ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		void ReadFromStream (SLoadCtx &Ctx);
		inline void Recharge (CShip *pShip, int iStatus) { m_pClass->Recharge(this, pShip, iStatus); }
		inline void Reset (CSpaceObject *pShip) { m_pClass->Reset(this, pShip); }
		inline void SelectFirstVariant (CSpaceObject *pSource) { m_pClass->SelectFirstVariant(pSource, this); }
		inline void SelectNextVariant (CSpaceObject *pSource) { m_pClass->SelectNextVariant(pSource, this); }
		inline void SetActivationDelay (int iDelay) { m_iActivationDelay = iDelay; }
		inline void SetBonus (int iBonus) { m_iBonus = iBonus; }
		inline void SetClass (CDeviceClass *pClass) { m_pClass.Set(pClass); }
		inline void SetChargesCache (int iCharges) { m_iCharges = iCharges; }
		inline void SetDamaged (bool bDamaged) { m_fDamaged = bDamaged; }
		inline void SetData (DWORD dwData) { m_dwData = dwData; }
		inline void SetDeviceSlot (int iDev) { m_iDeviceSlot = iDev; }
		inline void SetEnhanced (bool bEnhanced) { m_fEnhanced = bEnhanced; }
		inline void SetEnabled (bool bEnabled) { m_fEnabled = bEnabled; }
		inline void SetFireAngle (int iAngle) { m_iFireAngle = iAngle; }
		inline void SetMods (DWORD dwMods) { m_Mods = dwMods; }
		inline void SetOmniDirectional (void) { m_fOmniDirectional = true; }
		inline void SetOptimized (bool bOptimized) { m_fOptimized = bOptimized; }
		inline void SetOverdrive (bool bOverdrive) { m_fOverdrive = bOverdrive; }
		inline void SetTemperature (int iTemperature) { m_iTemperature = iTemperature; }
		inline void SetWaiting (bool bWaiting) { m_fWaiting = bWaiting; }
		void Uninstall (CSpaceObject *pObj, CItemListManipulator &ItemList);
		void Update (CSpaceObject *pSource, 
					 int iTick, 
					 bool *retbSourceDestroyed,
					 bool *retbConsumedItems = NULL);
		void WriteToStream (IWriteStream *pStream);

	private:
		CDeviceClassRef m_pClass;				//	The device class that is installed here
		DWORD m_dwData;							//	Data specific to device class

		int m_iPosAngle:16;						//	Position of installation (degrees)
		int m_iPosRadius:16;					//	Position of installation (pixels)
		int m_iMinFireArc:16;					//	Min angle of fire arc (degrees)
		int m_iMaxFireArc:16;					//	Max angle of fire arc (degrees)
		int m_iActivationDelay:16;				//	Ticks between shots
		int m_iFireAngle:16;					//	Last fire angle
		int m_iBonus:16;						//	Bonus for weapons (+1, etc.)
		int m_iTemperature:16;					//	Temperature for weapons
		int m_iCharges:16;						//	Charges
		int m_iDeviceSlot:16;					//	Device slot

		CItemEnhancement m_Mods;				//	Item enhancement

		DWORD m_fOmniDirectional:1;				//	Installed on turret
		DWORD m_fOffset:1;						//	Device is not installed at center of object
		DWORD m_fEnhanced:1;					//	Device is enhanced
		DWORD m_fOverdrive:1;					//	Device has overdrive installed
		DWORD m_fOptimized:1;					//	Device is optimized by alien engineers
		DWORD m_fSecondaryWeapon:1;				//	Secondary weapon
		DWORD m_fDamaged:1;						//	Device is damaged
		DWORD m_fEnabled:1;						//	Device is enabled
		DWORD m_fExternal:1;					//	Device is external to hull
		DWORD m_fWaiting:1;						//	Waiting for cooldown, etc.
		DWORD m_dwSpare:22;						//	Spare flags
	};

//	CDesignCollection

class CDesignList
	{
	public:
		CDesignList (void);
		~CDesignList (void);

		void AddEntry (CDesignType *pType);
		void DeleteAll (void);
		inline int GetCount (void) const { return m_iCount; }
		inline CDesignType *GetEntry (int iIndex) const { return m_pList[iIndex]; }
		void RemoveAll (void);

	private:
		int m_iAlloc;
		int m_iCount;
		CDesignType **m_pList;
	};

class CDesignTable
	{
	public:
		CDesignTable (void) : m_Table(FALSE, TRUE) { }

		ALERROR AddEntry (CDesignType *pEntry);
		ALERROR AddOrReplaceEntry (CDesignType *pEntry, CDesignType **retpOldEntry = NULL);
		void DeleteAll (void);
		CDesignType *FindByUNID (DWORD dwUNID) const;
		inline int GetCount (void) const { return m_Table.GetCount(); }
		inline CDesignType *GetEntry (int iIndex) const { return (CDesignType *)m_Table.GetValue(iIndex); }
		inline void RemoveAll (void) { m_Table.RemoveAll(); }

	private:
		CIDTable m_Table;
	};

enum EExtensionTypes
	{
	extExtension,
	extAdventure,
	};

struct SExtensionDesc
	{
	EExtensionTypes iType;

	DWORD dwUNID;
	DWORD dwVersion;
	CDesignTable Table;
	CTopologyDescTable Topology;

	bool bLoaded;
	bool bEnabled;
	};

struct SDesignLoadCtx
	{
	SDesignLoadCtx (void) :
			pResDb(NULL),
			pExtension(NULL),
			bLoadAdventureDesc(false),
			bNoResources(false),
			bNoVersionCheck(false)
		{ }

	//	Context
	CString sResDb;
	CResourceDb *pResDb;
	CString sFolder;
	SExtensionDesc *pExtension;
	bool bLoadAdventureDesc;

	//	Options
	bool bNoResources;
	bool bNoVersionCheck;

	//	Output
	CString sError;
	};

class CDesignCollection
	{
	public:
		CDesignCollection (void);
		~CDesignCollection (void);

		ALERROR BeginLoadAdventure (SDesignLoadCtx &Ctx, CAdventureDesc *pAdventure);
		ALERROR BeginLoadAdventureDesc (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR BeginLoadExtension (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR BindDesign (SDesignLoadCtx &Ctx);
		void EndLoadAdventure (SDesignLoadCtx &Ctx) { Ctx.pExtension = NULL; }
		void EndLoadAdventureDesc (SDesignLoadCtx &Ctx) { Ctx.pExtension = NULL; Ctx.bLoadAdventureDesc = false; }
		void EndLoadExtension (SDesignLoadCtx &Ctx) { Ctx.pExtension = NULL; }
		inline CDesignType *FindEntry (DWORD dwUNID) const { return m_AllTypes.FindByUNID(dwUNID); }
		inline int GetCount (void) const { return m_AllTypes.GetCount(); }
		inline int GetCount (DesignTypes iType) const { return m_ByType[iType].GetCount(); }
		inline CDesignType *GetEntry (int iIndex) const { return m_AllTypes.GetEntry(iIndex); }
		inline CDesignType *GetEntry (DesignTypes iType, int iIndex) const { return m_ByType[iType].GetEntry(iIndex); }
		CTopologyDescTable *GetTopologyDesc (void) const { return m_pTopology; }
		bool IsAdventureExtensionBound (DWORD dwUNID);
		bool IsAdventureExtensionLoaded (DWORD dwUNID);
		ALERROR LoadEntryFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		void RemoveAll (void);
		void SelectAdventure (DWORD dwUNID);

	private:
		ALERROR AddEntry (SDesignLoadCtx &Ctx, CDesignType *pEntry);
		SExtensionDesc *FindExtension (DWORD dwUNID);
		SExtensionDesc *GetExtension (int iIndex) { return (SExtensionDesc *)m_Extensions.GetValue(iIndex); }
		int GetExtensionCount (void) { return m_Extensions.GetCount(); }
		ALERROR LoadDesignType (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		ALERROR LoadExtensionDesc (SDesignLoadCtx &Ctx, CXMLElement *pDesc, SExtensionDesc **retpExtension);

		//	These are generated at bind time
		CDesignTable m_AllTypes;
		CDesignList m_ByType[designCount];
		CTopologyDescTable *m_pTopology;
		SExtensionDesc *m_pAdventureExtension;

		CDesignTable m_Base;
		CTopologyDescTable m_BaseTopology;
		CIDTable m_Extensions;

		DWORD m_dwNextAnonymousUNID;
	};

//	Utility functions

IShipController *CreateShipController (const CString &sAI);
CString GenerateRandomName (const CString &sList, const CString &sSubst);
CString GetDamageName (DamageTypes iType);
CString GetDamageShortName (DamageTypes iType);
int GetDiceCountFromAttribute(const CString &sValue);
int GetFrequency (const CString &sValue);
int GetFrequencyByLevel (const CString &sLevelFrequency, int iLevel);
CString GetItemCategoryName (ItemCategories iCategory);
bool IsEnergyDamage (DamageTypes iType);
bool IsMatterDamage (DamageTypes iType);
COLORREF LoadCOLORREF (const CString &sString);
ALERROR LoadDamageAdj (CXMLElement *pItem, const CString &sAttrib, int *retiAdj);
ALERROR LoadDamageAdj (CXMLElement *pDesc, int *pDefAdj, int *retiAdj);
void LoadDamageAdjFromXML (const CString &sAttrib, int *pAdjTable);
DamageTypes LoadDamageTypeFromXML (const CString &sAttrib);
DWORD LoadExtensionVersion (const CString &sVersion);
DWORD LoadNameFlags (CXMLElement *pDesc);
COLORREF LoadRGBColor (const CString &sString);

//	Inline implementations

inline const CString &CItem::GetDesc (void) const { return m_pItemType->GetDesc(); }

inline CDeviceClass *CDeviceDescList::GetDeviceClass (int iIndex) const { return m_pDesc[iIndex].Item.GetType()->GetDeviceClass(); }

inline Metric CArmorClass::GetMass (void) const { return m_pItemType->GetMass(); }
inline CString CArmorClass::GetName (void) { return m_pItemType->GetName(NULL); }
inline DWORD CArmorClass::GetUNID (void) { return m_pItemType->GetUNID(); }

inline int CDeviceClass::GetLevel (void) { return m_pItemType->GetLevel(); }
inline Metric CDeviceClass::GetMass (void) { return m_pItemType->GetMass(); }
inline CString CDeviceClass::GetName (void) { return m_pItemType->GetName(NULL); }
inline DWORD CDeviceClass::GetUNID (void) { return m_pItemType->GetUNID(); }

inline CXMLElement *CItemType::GetUseScreen (void) const { return m_pUseScreen.GetDesc(); }

inline bool DamageDesc::IsEnergyDamage (void) const { return ::IsEnergyDamage(m_iType); }
inline bool DamageDesc::IsMatterDamage (void) const { return ::IsEnergyDamage(m_iType); }

inline void IEffectPainter::PlaySound (CSpaceObject *pSource) { GetCreator()->PlaySound(pSource); }

#endif

