//	CEnergyField.cpp
//
//	CEnergyField class

#include "PreComp.h"

#include "Kernel.h"

CEnergyField::CEnergyField (void) : 
		m_pType(NULL),
		m_pPainter(NULL),
		m_pHitPainter(NULL),
		m_pNext(NULL)

//	CEnergyField constructor

	{
	}

CEnergyField::~CEnergyField (void)

//	CEnergyField destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();

	if (m_pHitPainter)
		m_pHitPainter->Delete();
	}

bool CEnergyField::AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx)

//	AbsorbDamage
//
//	Absorbs damage and returns TRUE if all damage is absorbed

	{
	int iAbsorb = m_pType->GetDamageAbsorbed(pSource, Ctx);
	if (iAbsorb > 0)
		{
		//	Absorb damage

		Ctx.iDamage -= iAbsorb;

		//	Set hit effect, if we've got a painter

		if (m_pHitPainter)
			{
			m_iPaintHit = m_pHitPainter->GetInitialLifetime();
			m_iPaintHitTick = 0;

			m_pHitPainter->PlaySound(pSource);
			}

		//	Done

		return (Ctx.iDamage == 0);
		}
	else
		return false;
	}

void CEnergyField::CreateFromType (CEnergyFieldType *pType, int iLifeLeft, CEnergyField **retpField)

//	CreateFromType
//
//	Create field from type

	{
	ASSERT(pType);

	CEnergyField *pField = new CEnergyField;

	pField->m_pType = pType;
	pField->m_iLifeLeft = iLifeLeft;
	pField->m_iPaintHit = 0;

	//	Create painters

	CEffectCreator *pCreator = pType->GetEffectCreator();
	if (pCreator)
		pField->m_pPainter = pCreator->CreatePainter();

	pCreator = pType->GetHitEffectCreator();
	if (pCreator)
		pField->m_pHitPainter = pCreator->CreatePainter();

	*retpField = pField;
	}

void CEnergyField::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint the field

	{
	if (m_iPaintHit > 0 && m_pHitPainter)
		{
		int iSavedTick = Ctx.iTick;
		Ctx.iTick = m_iPaintHitTick;

		m_pHitPainter->Paint(Dest, x, y, Ctx);

		Ctx.iTick = iSavedTick;
		}
	else if (m_pPainter)
		m_pPainter->Paint(Dest, x, y, Ctx);
	}

void CEnergyField::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	DWORD	UNID of energy field type
//	DWORD	Life left
//	Painter

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pType = g_pUniverse->FindShipEnergyFieldType(dwLoad);
	if (m_pType == NULL)
		{
		kernelDebugLogMessage("Unable to find energy field type: %x", dwLoad);
		throw;
		}

	Ctx.pStream->Read((char *)&m_iLifeLeft, sizeof(DWORD));

	CEffectCreator *pCreator = m_pType->GetEffectCreator();
	if (pCreator)
		m_pPainter = pCreator->CreatePainterFromStreamAndCreator(Ctx);

	pCreator = m_pType->GetHitEffectCreator();
	if (pCreator)
		m_pHitPainter = pCreator->CreatePainterFromStreamAndCreator(Ctx);

	m_iPaintHit = 0;
	}

void CEnergyField::Update (bool *retbDeleteField)

//	Update
//
//	Update the field

	{
	if (m_iLifeLeft != -1 && --m_iLifeLeft == 0)
		{
		*retbDeleteField = true;
		return;
		}

	if (m_iPaintHit > 0)
		{
		m_iPaintHit--;
		m_iPaintHitTick++;
		}

	*retbDeleteField = false;
	}

void CEnergyField::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	DWORD	UNID of energy field type
//	DWORD	Life left
//	Painter

	{
	DWORD dwSave = m_pType->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(DWORD));

	if (m_pPainter)
		m_pPainter->WriteToStream(pStream);

	if (m_pHitPainter)
		m_pHitPainter->WriteToStream(pStream);
	}

//	CEnergyFieldList ----------------------------------------------------------

CEnergyFieldList::CEnergyFieldList (void) :
		m_pFirst(NULL)

//	CEnergyFieldList constructor

	{
	}

CEnergyFieldList::~CEnergyFieldList (void)

//	CEnergyFieldList destructor

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		CEnergyField *pDelete = pField;
		pField = pField->GetNext();
		delete pDelete;
		}
	}

bool CEnergyFieldList::AbsorbDamage (CSpaceObject *pSource, SDamageCtx &Ctx)

//	AbsorbDamage
//
//	Absorbs damage (returns TRUE if damage was completely absorbed)

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->AbsorbDamage(pSource, Ctx))
			return true;

		pField = pField->GetNext();
		}

	return false;
	}

bool CEnergyFieldList::AbsorbsWeaponFire (CInstalledDevice *pDevice)

//	AbsorbWeaponFire
//
//	Returns TRUE if the field prevents the ship from firing the given weapon

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		if (pField->GetType()->AbsorbsWeaponFire(pDevice))
			return true;

		pField = pField->GetNext();
		}

	return false;
	}

void CEnergyFieldList::AddField (CEnergyFieldType *pType, int iLifeLeft)

//	AddField
//
//	Adds a field of the given type to the head of the list

	{
	CEnergyField *pField;
	CEnergyField::CreateFromType(pType, iLifeLeft, &pField);

	AddField(pField);
	}

void CEnergyFieldList::AddField (CEnergyField *pNewField)

//	AddField
//
//	Add a new field to the head of the list

	{
	pNewField->SetNext(m_pFirst);
	m_pFirst = pNewField;
	}

int CEnergyFieldList::GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetWeaponBonus
//
//	Returns the weapon bonus confered by the fields

	{
	int iBonus = 0;

	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		iBonus += pField->GetType()->GetWeaponBonus(pDevice, pSource);
		pField = pField->GetNext();
		}

	return iBonus;
	}

void CEnergyFieldList::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paints all fields

	{
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		pField->Paint(Dest, x, y, Ctx);
		pField = pField->GetNext();
		}
	}

void CEnergyFieldList::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	DWORD		no of fields
//	CEnergyField array

	{
	DWORD dwCount;

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	CEnergyField *pPrevField = NULL;
	while (dwCount)
		{
		CEnergyField *pField = new CEnergyField;
		if (pPrevField)
			pPrevField->SetNext(pField);
		else
			m_pFirst = pField;

		pField->ReadFromStream(Ctx);

		dwCount--;
		}
	}

void CEnergyFieldList::Update (bool *retbModified)

//	Update
//
//	Update fields. Returns bModified = TRUE if any field changed such that the object
//	need to be recalculated.

	{
	bool bModified = false;

	CEnergyField *pField = m_pFirst;
	CEnergyField *pPrevField = NULL;
	while (pField)
		{
		CEnergyField *pNext = pField->GetNext();

		//	Update

		bool bDeleteField;
		pField->Update(&bDeleteField);
		if (bDeleteField)
			{
			delete pField;

			if (pPrevField)
				pPrevField->SetNext(pNext);
			else
				m_pFirst = pNext;

			bModified = true;
			}
		else
			pPrevField = pField;

		//	Next energy field

		pField = pNext;
		}

	*retbModified = bModified;
	}

void CEnergyFieldList::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//	ReadFromStream
//
//	DWORD		no of fields
//	CEnergyField array

	{
	DWORD dwSave = 0;
	CEnergyField *pField = m_pFirst;
	while (pField)
		{
		dwSave++;
		pField = pField->GetNext();
		}

	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pField = m_pFirst;
	while (pField)
		{
		pField->WriteToStream(pStream);
		pField = pField->GetNext();
		}
	}
