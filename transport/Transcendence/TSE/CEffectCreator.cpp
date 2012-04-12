//	CEffectCreator.cpp
//
//	CEffectCreator class

#include "PreComp.h"

#include "Kernel.h"

#define BOLT_TAG								CONSTLIT("Bolt")
#define FLARE_TAG								CONSTLIT("Flare")
#define IMAGE_TAG								CONSTLIT("Image")
#define IMAGE_AND_TAIL_TAG						CONSTLIT("ImageAndTail")
#define MOLTEN_BOLT_TAG							CONSTLIT("MoltenBolt")
#define PARTICLE_TAG							CONSTLIT("Particle")
#define PARTICLE_COMET_TAG						CONSTLIT("ParticleComet")
#define PARTICLE_EXPLOSION_TAG					CONSTLIT("ParticleExplosion")
#define PLASMA_SPHERE_TAG						CONSTLIT("PlasmaSphere")
#define POLYFLASH_TAG							CONSTLIT("Polyflash")
#define SHAPE_TAG								CONSTLIT("Shape")
#define SHOCKWAVE_TAG							CONSTLIT("Shockwave")
#define SMOKE_TRAIL_TAG							CONSTLIT("SmokeTrail")
#define STARBURST_TAG							CONSTLIT("Starburst")

#define UNID_ATTRIB								CONSTLIT("UNID")
#define SOUND_ATTRIB							CONSTLIT("sound")
#define VARIANTS_ATTRIB							CONSTLIT("imageVariants")
#define RANDOM_START_FRAME_ATTRIB				CONSTLIT("randomStartFrame")
#define ROTATION_COUNT_ATTRIB					CONSTLIT("rotationCount")

#define STR_NO_UNID								CONSTLIT("(no UNID)")

ALERROR CEffectCreator::CreateEffect (CSystem *pSystem,
									  CSpaceObject *pAnchor,
									  const CVector &vPos,
									  const CVector &vVel)

//	CreateEffect
//
//	Default creation of effect (using CEffect)

	{
	return CEffect::Create(this,
			pSystem,
			pAnchor,
			vPos,
			vVel,
			NULL);
	}

ALERROR CEffectCreator::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID, CEffectCreator **retpCreator)

//	CreateFromXML
//
//	Creates the creator from an XML element

	{
	ALERROR error;
	CEffectCreator *pCreator;

	//	Create the effect based on the child tag

	if (pDesc->GetContentElementCount() == 0)
		return ERR_FAIL;
	else if (pDesc->GetContentElementCount() == 1)
		{
		if (error = CreateSimpleFromXML(Ctx, pDesc->GetContentElement(0), &pCreator))
			return error;
		}
	else
		{
		pCreator = new CEffectGroupCreator;
		if (pCreator == NULL)
			return ERR_MEMORY;

		//	Type-specific creation

		if (error = pCreator->OnEffectCreateFromXML(Ctx, pDesc))
			return error;
		}

	//	Basic info

	pCreator->m_sUNID = sUNID;
	if (sUNID.IsBlank())
		{
		DWORD dwUNID = pDesc->GetAttributeInteger(UNID_ATTRIB);
		if (dwUNID)
			pCreator->m_sUNID = strFromInt(dwUNID, FALSE);
		else
			pCreator->m_sUNID = STR_NO_UNID;
		}

	//	Sound Effect (resolved later)

	pCreator->m_dwSoundUNID = pDesc->GetAttributeInteger(SOUND_ATTRIB);
	pCreator->m_iSound = -1;
	
	//	Done

	*retpCreator = pCreator;

	return NOERROR;
	}

ALERROR CEffectCreator::CreateSimpleFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CEffectCreator **retpCreator)

//	CreateSimpleFromXML
//
//	Creates the creator from an XML element

	{
	ALERROR error;
	CEffectCreator *pCreator;

	//	Create the effect based on the tag

	if (strEquals(pDesc->GetTag(), IMAGE_TAG))
		pCreator = new CImageEffectCreator;
	else if (strEquals(pDesc->GetTag(), POLYFLASH_TAG))
		pCreator = new CPolyflashEffectCreator;
	else if (strEquals(pDesc->GetTag(), SHOCKWAVE_TAG))
		pCreator = new CShockwaveEffectCreator;
	else if (strEquals(pDesc->GetTag(), STARBURST_TAG))
		pCreator = new CStarburstEffectCreator;
	else if (strEquals(pDesc->GetTag(), PLASMA_SPHERE_TAG))
		pCreator = new CPlasmaSphereEffectCreator;
	else if (strEquals(pDesc->GetTag(), BOLT_TAG))
		pCreator = new CBoltEffectCreator;
	else if (strEquals(pDesc->GetTag(), MOLTEN_BOLT_TAG))
		pCreator = new CMoltenBoltEffectCreator;
	else if (strEquals(pDesc->GetTag(), PARTICLE_EXPLOSION_TAG))
		pCreator = new CParticleExplosionEffectCreator;
	else if (strEquals(pDesc->GetTag(), IMAGE_AND_TAIL_TAG))
		pCreator = new CImageAndTailEffectCreator;
	else if (strEquals(pDesc->GetTag(), FLARE_TAG))
		pCreator = new CFlareEffectCreator;
	else if (strEquals(pDesc->GetTag(), PARTICLE_COMET_TAG))
		pCreator = new CParticleCometEffectCreator;
	else if (strEquals(pDesc->GetTag(), SMOKE_TRAIL_TAG))
		pCreator = new CSmokeTrailEffectCreator;
	else if (strEquals(pDesc->GetTag(), PARTICLE_TAG))
		pCreator = new CSingleParticleEffectCreator;
	else if (strEquals(pDesc->GetTag(), SHAPE_TAG))
		pCreator = new CShapeEffectCreator;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Invalid painter tag: %s"), pDesc->GetTag().GetASCIIZPointer());
		return ERR_FAIL;
		}

	if (pCreator == NULL)
		return ERR_MEMORY;

	//	Type-specific creation

	if (error = pCreator->OnEffectCreateFromXML(Ctx, pDesc))
		return error;

	//	Done

	*retpCreator = pCreator;

	return NOERROR;
	}

IEffectPainter *CEffectCreator::CreatePainterFromStream (SLoadCtx &Ctx)

//	CreatePainterFromStream
//
//	Load a painter from a stream

	{
	CEffectCreator *pCreator;

	//	At version 15 we started saving versions as string UNIDs. We need to do this
	//	because sometimes the effect creator is inside a weapon fire desc
	//	structure (also identified by string UNIDs).

	if (Ctx.dwVersion >= 15)
		{
		CString sUNID;
		sUNID.ReadFromStream(Ctx.pStream);
		if (sUNID.IsBlank())
			return NULL;

		pCreator = CEffectCreator::FindEffectCreator(sUNID);

		//	Error

		if (pCreator == NULL)
			{
			kernelDebugLogMessage("Invalid painter creator: %s", sUNID.GetASCIIZPointer());
			return NULL;
			}
		}

	//	Old style uses DWORD UNIDs

	else
		{
		//	The first DWORD is the UNID of the creator

		DWORD dwUNID;
		Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
		if (dwUNID == 0)
			return NULL;

		pCreator = g_pUniverse->FindEffectType(dwUNID);

		//	Error

		if (pCreator == NULL)
			{
			kernelDebugLogMessage("Invalid painter creator: %x", dwUNID);
			return NULL;
			}
		}

	//	Let the creator create the object

	IEffectPainter *pPainter = pCreator->CreatePainter();

	//	Load it

	pPainter->ReadFromStream(Ctx);

	//	Done

	return pPainter;
	}

IEffectPainter *CEffectCreator::CreatePainterFromStreamAndCreator (SLoadCtx &Ctx)

//	CreatePainterFromStreamAndCreator
//
//	Load a painter from a stream given a creator

	{
	//	Let the creator create the object

	IEffectPainter *pPainter = CreatePainter();

	//	The UNID is ignored (because it is the UNID of the creator)

	pPainter->ReadUNID(Ctx);

	//	Load it

	pPainter->ReadFromStream(Ctx);

	//	Done

	return pPainter;
	}

CString IEffectPainter::ReadUNID (SLoadCtx &Ctx)

//	ReadUNID
//
//	Returns the UNID saved to a stream

	{
	CString sUNID;

	if (Ctx.dwVersion >= 15)
		sUNID.ReadFromStream(Ctx.pStream);
	else
		{
		DWORD dwUNID;
		Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
		sUNID = strFromInt(dwUNID, FALSE);
		}

	return sUNID;
	}

bool CEffectCreator::IsValidUNID (void)

//	IsValidUNID
//
//	Returns TRUE if UNID is valid. If not valid, the creator must be known
//	before loading.

	{
	return !strEquals(m_sUNID, STR_NO_UNID);
	}

CEffectCreator *CEffectCreator::FindEffectCreator (const CString &sUNID)

//	FindEffectCreator
//
//	Finds the effect creator from a complex UNID (or NULL if not found)

	{
	//	A complex UNID looks like this:
	//
	//	For Effects:
	//
	//	{unid}
	//
	//	"12345"					= Effect UNID 12345
	//
	//	For weapons:
	//
	//	{unid}/{var}[/f{frag}]:(e | h | f)
	//
	//	"12345/0:e"				= Weapon UNID 12345; variant 0; Bullet effect
	//	"12345/0/f0:h"			= Weapon UNID 12345; variant 0; fragment 0; Hit Effect
	//
	//	For shields:
	//
	//	{unid}:h
	//	"12345:h"				= Shield UNID 12345; Hit effect

	//	First we parse the UNID

	char *pPos = sUNID.GetASCIIZPointer();
	DWORD dwUNID = strParseInt(pPos, 0, &pPos, NULL);

	//	Look for the design type

	CDesignType *pDesign = g_pUniverse->FindDesignType(dwUNID);
	if (pDesign == NULL)
		return NULL;

	//	Allow the design type to parse the remainder of the UNID

	return pDesign->FindEffectCreator(CString(pPos));
	}

ALERROR CEffectCreator::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind the design

	{
	ALERROR error;

	//	Load sounds

	if (m_dwSoundUNID)
		m_iSound = g_pUniverse->FindSound(m_dwSoundUNID);
	else
		m_iSound = -1;

	//	Load our descendants

	if (error = OnEffectBindDesign(Ctx))
		return error;

	return NOERROR;
	}

ALERROR CEffectCreator::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Load from XML

	{
	return NOERROR;
	}

void CEffectCreator::PlaySound (CSpaceObject *pSource)

//	PlaySound
//
//	Play the sound effect

	{
	if (m_iSound != -1)
		g_pUniverse->PlaySound(pSource, m_iSound);
	}

//	IEffectPainter object

int IEffectPainter::GetInitialLifetime (void)
	{
	return GetCreator()->GetLifetime();
	}

Metric IEffectPainter::GetRadius (void) const
	{
	RECT rcRect;
	GetRect(&rcRect);

	int cxWidth = RectWidth(rcRect);
	int cyHeight = RectHeight(rcRect);

	return g_KlicksPerPixel * Max(cxWidth / 2, cyHeight / 2);
	}

void IEffectPainter::GetRect (RECT *retRect) const
	{
	retRect->left = 0; 
	retRect->top = 0;
	retRect->bottom = 0;
	retRect->right = 0;
	}

void IEffectPainter::WriteToStream (IWriteStream *pStream)
	{
	CString sUNID = GetCreator()->GetUNIDString();
	sUNID.WriteToStream(pStream);
	OnWriteToStream(pStream);
	}

//	CImageEffectCreator object

bool CImageEffectCreator::GetParticlePaintDesc (SParticlePaintDesc *retDesc)

//	GetParticlePaintDesc
//
//	Returns particle painting descriptor for optimized painting

	{
	retDesc->iStyle = paintImage;
	retDesc->pImage = &m_Image;
	retDesc->iVariants = m_iVariants;
	retDesc->bDirectional = m_bDirectional;
	retDesc->bRandomStartFrame = m_bRandomStartFrame;

	return true;
	}

ALERROR CImageEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnEffectCreateFromXML
//
//	Initialize type-specific data

	{
	ALERROR error;

	if (error = m_Image.InitFromXML(Ctx, pDesc))
		return error;

	m_iLifetime = m_Image.GetFrameCount() * m_Image.GetTicksPerFrame();
	m_bRandomStartFrame = pDesc->GetAttributeBool(RANDOM_START_FRAME_ATTRIB);

	//	Variants & Rotation

	m_iVariants = pDesc->GetAttributeInteger(ROTATION_COUNT_ATTRIB);
	m_bDirectional = (m_iVariants > 1);

	if (m_iVariants == 0)
		m_iVariants = pDesc->GetAttributeInteger(VARIANTS_ATTRIB);

	if (m_iVariants <= 0)
		m_iVariants = 1;

	return NOERROR;
	}

ALERROR CImageEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		return error;

	return NOERROR;
	}

void CImageEffectCreator::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the effect

	{
	int iTick = Ctx.iTick;
	if (m_bRandomStartFrame)
		iTick += Ctx.iDestiny;

	int iFrame = (m_bDirectional ? Angle2Direction(Ctx.iRotation, m_iVariants) : (Ctx.iVariant % m_iVariants));

	m_Image.PaintImage(Dest, x, y, iTick, iFrame);
	}

bool CImageEffectCreator::PointInImage (int x, int y, int iTick, int iVariant) const

//	PointInImage
//
//	Returns TRUE if the given point is in the image

	{
	return m_Image.PointInImage(x, y, iTick, (iVariant % m_iVariants));
	}

void CImageEffectCreator::SetVariants (int iVariants)

//	SetVariants
//
//	Sets the number of variants
//	This function is only called because directionally used to be set at the
//	object level. We only honor this if we are not already directional

	{
	if (!m_bDirectional)
		{
		m_bDirectional = true;
		m_iVariants = iVariants;
		}
	}

//	CEffectGroupPainter -------------------------------------------------------

class CEffectGroupPainter : public IEffectPainter
	{
	public:
		CEffectGroupPainter (CEffectGroupCreator *pCreator);

		virtual ~CEffectGroupPainter (void);
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual int GetFadeLifetime (void);
		virtual void GetRect (RECT *retRect) const;
		virtual void OnBeginFade (void);
		virtual void OnUpdate (void);
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual void PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);
		virtual bool PointInImage (int x, int y, int iTick, int iVariant = 0) const;
		virtual void SetDirection (int iDirection);
		virtual void SetVariants (int iVariants);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CEffectGroupCreator *m_pCreator;
		IEffectPainter **m_pPainters;
	};

CEffectGroupPainter::CEffectGroupPainter (CEffectGroupCreator *pCreator) : m_pCreator(pCreator)

//	CEffectGroupPainter constructor

	{
	m_pPainters = new IEffectPainter * [m_pCreator->GetCount()];

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i] = m_pCreator->GetCreator(i)->CreatePainter();
	}

CEffectGroupPainter::~CEffectGroupPainter (void)

//	CEffectGroupPainter destructor

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i]->Delete();

	delete [] m_pPainters;
	}

int CEffectGroupPainter::GetFadeLifetime (void)

//	GetFadeLifetime
//
//	Returns the lifetime to paint fading state

	{
	int iMaxLifetime = 0;

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		iMaxLifetime = Max(iMaxLifetime, m_pPainters[i]->GetFadeLifetime());

	return iMaxLifetime;
	}

void CEffectGroupPainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Return the bounding rect

	{
	retRect->left = 0;
	retRect->top = 0;
	retRect->bottom = 0;
	retRect->right = 0;

	for (int i = 0; i < m_pCreator->GetCount(); i++)
		{
		RECT rcRect;
		m_pPainters[i]->GetRect(&rcRect);

		::UnionRect(retRect, retRect, &rcRect);
		}
	}

void CEffectGroupPainter::OnBeginFade (void)

//	OnBeginFade

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i]->OnBeginFade();
	}

void CEffectGroupPainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		{
		//	The UNID is ignored (because it is the UNID of the creator)

		ReadUNID(Ctx);

		//	Read the painter

		m_pPainters[i]->ReadFromStream(Ctx);
		}
	}

void CEffectGroupPainter::OnUpdate (void)

//	OnUpdate
//
//	Update painters

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i]->OnUpdate();
	}

void CEffectGroupPainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i]->WriteToStream(pStream);
	}

void CEffectGroupPainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i]->Paint(Dest, x, y, Ctx);
	}

void CEffectGroupPainter::PaintFade (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintFade
//
//	Paint

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i]->PaintFade(Dest, x, y, Ctx);
	}

bool CEffectGroupPainter::PointInImage (int x, int y, int iTick, int iVariant) const

//	PointInImage
//
//	Returns TRUE if the point is in the image

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		if (m_pPainters[i]->PointInImage(x, y, iTick, iVariant))
			return true;

	return false;
	}

void CEffectGroupPainter::SetDirection (int iDirection)

//	SetDirection
//
//	Sets the direction

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i]->SetDirection(iDirection);
	}

void CEffectGroupPainter::SetVariants (int iVariants)

//	SetVariants
//
//	Sets the variants

	{
	for (int i = 0; i < m_pCreator->GetCount(); i++)
		m_pPainters[i]->SetVariants(iVariants);
	}

//	CEffectGroupCreator --------------------------------------------------------

CEffectGroupCreator::CEffectGroupCreator (void) : m_pCreators(NULL), m_iCount(0)

//	CEffectGroupCreator constructor

	{
	}

CEffectGroupCreator::~CEffectGroupCreator (void)

//	CEffectGroupCreator destructor

	{
	int i;

	if (m_pCreators)
		{
		for (i = 0; i < m_iCount; i++)
			delete m_pCreators[i];

		delete [] m_pCreators;
		}
	}

ALERROR CEffectGroupCreator::CreateEffect (CSystem *pSystem,
										   CSpaceObject *pAnchor,
										   const CVector &vPos,
										   const CVector &vVel)

//	CreateEffect
//
//	Creates an effect object

	{
	ALERROR error;

	for (int i = 0; i < m_iCount; i++)
		{
		if (error = m_pCreators[i]->CreateEffect(pSystem, pAnchor, vPos, vVel))
			return error;
		}

	return NOERROR;
	}

IEffectPainter *CEffectGroupCreator::CreatePainter (void)

//	CreatePainter
//
//	Creates a painter

	{
	return new CEffectGroupPainter(this);
	}

int CEffectGroupCreator::GetLifetime (void)

//	GetLifetime
//
//	Returns the lifetime of the effect
	
	{
	int iTotalLifetime = 0;
	for (int i = 0; i < m_iCount; i++)
		{
		int iLifetime = m_pCreators[i]->GetLifetime();
		if (iLifetime > iTotalLifetime)
			iTotalLifetime = iLifetime;
		}

	return iTotalLifetime;
	}

void CEffectGroupCreator::LoadImages (void)

//	LoadImages
//
//	Load images used by this effect

	{
	for (int i = 0; i < m_iCount; i++)
		m_pCreators[i]->LoadImages();
	}

void CEffectGroupCreator::MarkImages (void)

//	MarkImages
//
//	Mark images used by this effect

	{
	for (int i = 0; i < m_iCount; i++)
		m_pCreators[i]->MarkImages();
	}

ALERROR CEffectGroupCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnEffectCreateFromXML
//
//	Creates from XML

	{
	ALERROR error;
	int i;

	ASSERT(m_pCreators == NULL);

	//	Allocate the creator array

	m_iCount = pDesc->GetContentElementCount();
	if (m_iCount == 0)
		return ERR_FAIL;

	m_pCreators = new CEffectCreator * [m_iCount];

	for (i = 0; i < m_iCount; i++)
		{
		CXMLElement *pCreatorDesc = pDesc->GetContentElement(i);
		if (error = CEffectCreator::CreateSimpleFromXML(Ctx, pCreatorDesc, &m_pCreators[i]))
			return error;
		}

	return NOERROR;
	}

ALERROR CEffectGroupCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	for (int i = 0; i < m_iCount; i++)
		if (error = m_pCreators[i]->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

void CEffectGroupCreator::SetLifetime (int iLifetime)

//	SetLifetime
//
//	Sets the lifetime

	{
	for (int i = 0; i < m_iCount; i++)
		m_pCreators[i]->SetLifetime(iLifetime);
	}

void CEffectGroupCreator::SetSpeed (int iSpeed)

//	SetSpeed
//
//	Sets the speed

	{
	for (int i = 0; i < m_iCount; i++)
		m_pCreators[i]->SetSpeed(iSpeed);
	}

void CEffectGroupCreator::SetVariants (int iVariants)

//	SetVariants
//
//	Sets the variants

	{
	for (int i = 0; i < m_iCount; i++)
		m_pCreators[i]->SetVariants(iVariants);
	}
