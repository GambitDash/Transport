//	CCompositeImageDesc.cpp
//
//	CCompositeImageDesc class

#include "PreComp.h"

#define IMAGE_TAG								CONSTLIT("Image")
#define COMPOSITE_TAG							CONSTLIT("Composite")
#define IMAGE_VARIANTS_TAG						CONSTLIT("ImageVariants")
#define EFFECT_TAG								CONSTLIT("Effect")

#define LIGHTEN_BLENDING						CONSTLIT("brighten")

CCompositeImageDesc::CCompositeImageDesc (void) :
		m_Variants(NULL),
		m_iVariantCount(0)

//	CCompositeImageDesc constructor

	{
	}

CCompositeImageDesc::~CCompositeImageDesc (void)

//	CCompositeImageDesc destructor

	{
	if (m_Variants)
		{
		for (int i = 0; i < m_iVariantCount; i++)
			if (m_Variants[i].pEffect)
				delete m_Variants[i].pEffect;

		delete [] m_Variants;
		}
	}

CObjectImageArray &CCompositeImageDesc::GetImage (int iVariant, int *retiRotation)

//	GetImage
//
//	Returns the image

	{
	if (m_iVariantCount > 1)
		{
		if (retiRotation)
			*retiRotation = 0;

		return m_Variants[iVariant % m_iVariantCount].Image;
		}
	else
		{
		ASSERT(m_Variants);

		if (retiRotation)
			*retiRotation = iVariant;

		return m_Variants[0].Image;
		}
	}

ALERROR CCompositeImageDesc::InitEntryFromImageXML (SEntry &Entry, SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitEntryFromImageXML
//
//	Initializes an entry from <Image> tag

	{
	ALERROR error;

	//	Initialize the image

	if (error = Entry.Image.InitFromXML(Ctx, pDesc))
		return error;

	//	Done

	Entry.pEffect = NULL;

	return NOERROR;
	}

ALERROR CCompositeImageDesc::InitEntryFromCompositeXML (SEntry &Entry, SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitEntryFromCompositeXML
//
//	Initializes an entry from <Composite> tag

	{
	ALERROR error;

	//	Initialize the main image

	CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
	if (pImage == NULL)
		{
		Ctx.sError = CONSTLIT("Unable to find <Image> tag in <Composite> element");
		return ERR_FAIL;
		}

	if (error = Entry.Image.InitFromXML(Ctx, pImage))
		return error;

	//	Initialize the effect

	CXMLElement *pEffect = pDesc->GetContentElementByTag(EFFECT_TAG);
	if (pEffect)
		{
		if (error = CEffectCreator::CreateFromXML(Ctx, pEffect, NULL_STR, &Entry.pEffect))
			return error;
		}
	else
		Entry.pEffect = NULL;

	//	Done

	return NOERROR;
	}

ALERROR CCompositeImageDesc::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initializes the desc from XML

	{
	ALERROR error;

	ASSERT(m_Variants == NULL);

	if (strEquals(pDesc->GetTag(), IMAGE_TAG))
		{
		m_iVariantCount = 1;
		m_Variants = new SEntry [1];

		if (error = InitEntryFromImageXML(m_Variants[0], Ctx, pDesc))
			return error;
		}
	else if (strEquals(pDesc->GetTag(), COMPOSITE_TAG))
		{
		m_iVariantCount = 1;
		m_Variants = new SEntry [1];

		if (error = InitEntryFromCompositeXML(m_Variants[0], Ctx, pDesc))
			return error;
		}
	else if (strEquals(pDesc->GetTag(), IMAGE_VARIANTS_TAG))
		{
		m_iVariantCount = pDesc->GetContentElementCount();
		if (m_iVariantCount < 1)
			{
			Ctx.sError = CONSTLIT("<ImageVariants> element must have at least one image");
			return ERR_FAIL;
			}

		m_Variants = new SEntry [m_iVariantCount];

		for (int i = 0; i < m_iVariantCount; i++)
			{
			CXMLElement *pVar = pDesc->GetContentElement(i);
			if (strEquals(pVar->GetTag(), IMAGE_TAG))
				{
				if (error = InitEntryFromImageXML(m_Variants[i], Ctx, pVar))
					return error;
				}
			else if (strEquals(pVar->GetTag(), COMPOSITE_TAG))
				{
				if (error = InitEntryFromCompositeXML(m_Variants[i], Ctx, pVar))
					return error;
				}
			else
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Unknown image tag: %s"), pVar->GetTag().GetASCIIZPointer());
				return ERR_FAIL;
				}
			}
		}
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown image tag: %s"), pDesc->GetTag().GetASCIIZPointer());
		return ERR_FAIL;
		}

	return NOERROR;
	}

void CCompositeImageDesc::LoadImage (int iVariant)

//	LoadImage
//
//	Loads the image

	{
	if (m_Variants)
		GetImage(iVariant).LoadImage();
	}

void CCompositeImageDesc::MarkImage (int iVariant)

//	MarkImage
//
//	Marks the image in use

	{
	if (m_Variants)
		GetImage(iVariant).MarkImage();
	}

ALERROR CCompositeImageDesc::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Handle deferred loading

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_iVariantCount; i++)
		{
		if (error = m_Variants[i].Image.OnDesignLoadComplete(Ctx))
			return error;

		if (m_Variants[i].pEffect)
			if (error = m_Variants[i].pEffect->BindDesign(Ctx))
				return error;
		}

	return NOERROR;
	}
