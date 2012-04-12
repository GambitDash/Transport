//	CItem.cpp
//
//	CItem class

#include "PreComp.h"

#include "Kernel.h"

#define DEBUG_ITEM_NAME_MATCH

CItem::CItem (void) : 
		m_pItemType(NULL), 
		m_dwCount(0),
		m_dwFlags(0),
		m_dwInstalled(0xff),
		m_pExtra(NULL)

//	CItem constructor

	{
	}

CItem::CItem (const CItem &Copy)

//	CItem constructor

	{
	m_pItemType = Copy.m_pItemType;
	m_dwCount = Copy.m_dwCount;
	m_dwFlags = Copy.m_dwFlags;
	m_dwInstalled = Copy.m_dwInstalled;

	if (Copy.m_pExtra)
		m_pExtra = new SExtra(*Copy.m_pExtra);
	else
		m_pExtra = NULL;
	}

CItem::CItem (CItemType *pItemType, int iCount)

//	CItem constructor

	{
	ASSERT(pItemType);

	m_pItemType = pItemType;
	m_dwCount = (DWORD)iCount;
	m_dwFlags = 0;
	m_dwInstalled = 0xff;
	m_pExtra = NULL;

	int iCharges = pItemType->GetCharges();
	if (iCharges)
		SetCharges(iCharges);
	}

CItem::~CItem (void)

//	CItem destructor

	{
	if (m_pExtra)
		delete m_pExtra;
	}

CItem &CItem::operator= (const CItem &Copy)

//	CItem equals operator

	{
	if (m_pExtra)
		delete m_pExtra;

	m_pItemType = Copy.m_pItemType;
	m_dwCount = Copy.m_dwCount;
	m_dwFlags = Copy.m_dwFlags;
	m_dwInstalled = Copy.m_dwInstalled;

	if (Copy.m_pExtra)
		m_pExtra = new SExtra(*Copy.m_pExtra);
	else
		m_pExtra = NULL;

	return *this;
	}

CItem CItem::CreateItemByName (const CString &sName, const CItemCriteria &Criteria, bool bActualName)

//	CreateItemByName
//
//	Generates an item structure based on natural language
//	name. If there is ambiguity, a random item is chosen.

	{
	int i;

	CIntArray List;

	//	Prepare the input name

	CStringArray Input;
	NLSplit(sName, &Input);

	//	If we've got nothing, then nothing matches

	if (Input.GetCount() == 0)
		return CItem();

	//	Loop over all items

	int iBestMatch = -1000;
	int iBestNonCriteriaMatch = -1000;
	for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		CItem Item(pType, 1);

		//	Compare names

		CStringArray ItemName;
		NLSplit(pType->GetNounPhrase(bActualName ? nounActual : 0), &ItemName);
		int iMatch = NLCompare(Input, ItemName);

		//	If this is not in the criteria, then continue

		if (!Item.MatchesCriteria(Criteria))
			{
			if (iMatch > iBestNonCriteriaMatch)
				iBestNonCriteriaMatch = iMatch;
			continue;
			}

		//	If we have a match, add to the list

		if (iMatch >= iBestMatch)
			{
			if (iMatch > iBestMatch)
				List.RemoveAll();

			List.AppendElement((int)pType, NULL);
			iBestMatch = iMatch;
			}
		}

#ifdef DEBUG_ITEM_NAME_MATCH
	{
	for (i = 0; i < List.GetCount(); i++)
		{
		CItemType *pItem = (CItemType *)List.GetElement(i);
		char szBuffer[1024];
		sprintf(szBuffer, "%s\n", pItem->GetNounPhrase().GetASCIIZPointer());
		kernelDebugLogMessage(szBuffer);
		}
	}
#endif

	//	If we don't have a good match and there is a better match not in the criteria, then
	//	we don't match anything

	if (iBestMatch == 0 && List.GetCount() > 1)
		List.RemoveAll();
	else if (iBestNonCriteriaMatch > iBestMatch && List.GetCount() > 1)
		List.RemoveAll();

	//	Pick a random item

	if (List.GetCount())
		{
		CItemType *pBest = (CItemType *)List.GetElement(mathRandom(0, List.GetCount()-1));
		return CItem(pBest, pBest->GetNumberAppearing().Roll());
		}
	else
		return CItem();
	}

void CItem::Extra (void)

//	Extra
//
//	Make sure that Extra data structure is allocated

	{
	if (m_pExtra == NULL)
		{
		m_pExtra = new SExtra;
		m_pExtra->m_dwCharges = 0;
		m_pExtra->m_dwCondition = 0;
		m_pExtra->m_dwMods = 0;
		}
	}

CString CItem::GetEnhancedDesc (CSpaceObject *pInstalled) const

//	GetEnhancedDesc
//
//	Returns a string that describes the kind of enhancement
//	for this item.

	{
	//	Deal with complex enhancements

	DWORD dwMods = GetMods();
	if (dwMods)
		return CItemEnhancement(dwMods).GetEnhancedDesc(*this, pInstalled);

	//	Otherwise, generic enhancement

	else if (IsEnhanced())
		return CONSTLIT("+Enhanced");

	//	Otherwise, not enhanced

	else
		return NULL_STR;
	}

CString CItem::GetModsDesc (CSpaceObject *pSource) const

//	GetModsDesc
//
//	Returns the description of the mods

	{
	CItemEnhancement Mods(GetMods());
	return Mods.GetEnhancedDesc(*this, pSource);
	}

CString CItem::GetReference (CSpaceObject *pInstalled) const

//	GetReference
//
//	Returns a string that describes the basic stats for this
//	item

	{
	CArmorClass *pArmor;
	CDeviceClass *pDevice;

	//	Return armor reference, if this is armor

	if (pArmor = m_pItemType->GetArmorClass())
		return pArmor->GetReference(this, pInstalled);

	//	Return device reference, if this is a device

	else if (pDevice = m_pItemType->GetDeviceClass())
		return pDevice->GetReference(this, pInstalled);

	//	Otherwise, nothing

	else
		return NULL_STR;
	}

CString CItem::GetReferencePower (CSpaceObject *pInstalled) const

//	GetReferencePower
//
//	Returns the reactor type required for this item

	{
	CDeviceClass *pDevice;

	if (m_pItemType->GetCategory() == itemcatFuel)
		{
		if (m_pItemType->HasModifier(CONSTLIT("100MW")))
			return CONSTLIT("100MW");
		else if (m_pItemType->HasModifier(CONSTLIT("1GW")))
			return CONSTLIT("1GW");
		else if (m_pItemType->HasModifier(CONSTLIT("10GW")))
			return CONSTLIT("10GW");
		else
			return CONSTLIT("10MW");
		}
	else if (pDevice = m_pItemType->GetDeviceClass())
		return pDevice->GetReferencePower(this);
	else
		return NULL_STR;
	}

CString CItem::GetNounPhrase (DWORD dwFlags) const

//	GetNounPhrase
//
//	Returns a noun phrase that represents this item

	{
	DWORD dwNounFlags;
	CString sName;
	if (dwFlags & nounShort)
		sName = m_pItemType->GetShortName(&dwNounFlags, ((dwFlags & nounActual) ? true : false));
	else
		sName = m_pItemType->GetName(&dwNounFlags, ((dwFlags & nounActual) ? true : false));

	//	Modifiers

	CString sModifier;
	if (!(dwFlags & nounNoModifiers))
		{
		if (IsDamaged() || (GetMods() & etDisadvantage))
			sModifier = CONSTLIT("damaged ");
		else if (IsEnhanced() || GetMods())
			sModifier = CONSTLIT("enhanced ");
		}

	//	Compose phrase

	return ComposeNounPhrase(sName, (int)m_dwCount, sModifier, dwNounFlags, dwFlags);
	}

int CItem::GetValue (bool bActual) const

//	GetValue
//
//	Returns the value of a single item in this group. Caller must multiply by
//	GetCount() to find the value of the whole item group. The value takes into
//	account damage, enhancements, and charges.

	{
	int iValue = m_pItemType->GetValue(bActual);

	//	Adjust value based on enhancements

	int iAdj;

	//	Deal with enhancements

	DWORD dwMods = GetMods();
	if (dwMods)
		iAdj = CItemEnhancement(dwMods).GetValueAdj(*this);

	//	Otherwise, generic enhancement

	else
		iAdj = (IsEnhanced() ? 75 : 0);

	//	Increase value

	iValue += iValue * iAdj / 100;

	//	If the item is damaged, value is only 20% of full value

	if (IsDamaged())
		iValue = iValue * 20 / 100;

	//	If the item has charges, decrease value proportional to the
	//	number of charges that have been used up.

	if (m_pItemType->AreChargesValued())
		{
		int iMaxCharges = m_pItemType->GetMaxCharges();
		if (!IsInstalled() && iMaxCharges > 0)
			iValue = (iValue * (1 + GetCharges())) / (1 + iMaxCharges);
		}

	return iValue;
	}

bool CItem::IsEqual (const CItem &Item) const

//	IsEqual
//
//	Returns TRUE if this item is the same as the given
//	item except for the count

	{
	return (m_pItemType == Item.m_pItemType
			&& m_dwFlags == Item.m_dwFlags
			&& m_dwInstalled == Item.m_dwInstalled
			&& IsExtraEqual(Item.m_pExtra));
	}

bool CItem::IsExtraEqual (SExtra *pSrc) const

//	IsExtraEqual
//
//	Returns TRUE if this item's Extra struct is the same as the source

	{
	//	Both have extra struct

	if (m_pExtra && pSrc)
		{
		return (m_pExtra->m_dwCharges == pSrc->m_dwCharges
				&& m_pExtra->m_dwCondition == pSrc->m_dwCondition
				&& m_pExtra->m_dwMods == pSrc->m_dwMods
				&& m_pExtra->m_Data.IsEqual(pSrc->m_Data));
		}

	//	Neither has extra struct

	else if (m_pExtra == pSrc)
		return true;

	//	One has extra struct, other doesn't

	else
		return false;
	}

bool CItem::MatchesCriteria (const CItemCriteria &Criteria) const

//	MatchesCriteria
//
//	Return TRUE if this item matches the given criteria

	{
	int i;

	//	If we've got a filter, then use that

	if (Criteria.pFilter)
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		//	Create a list representing this item

		ICCItem *pItem = ::CreateListFromItem(CC, *this);

		//	Create an argument list consisting of the item

		ICCItem *pArgs = CC.CreateLinkedList();
		if (pArgs->IsError())
			{
			ASSERT(false);
			return false;
			}

		CCLinkedList *pList = (CCLinkedList *)pArgs;
		pList->Append(&CC, pItem, NULL);

		//	Apply the function to the arg list

		ICCItem *pResult = CC.Apply(Criteria.pFilter, pArgs, &g_pUniverse);
		bool bMatches = !pResult->IsNil();

		//	Done

		pResult->Discard(&CC);
		pArgs->Discard(&CC);
		pItem->Discard(&CC);

		return bMatches;
		}

	//	If we're looking for anything, then continue

	if (Criteria.dwItemCategories == 0xFFFFFFFF)
		{ }

	//	If we're looking for fuel and this item is fuel, then
	//	we continue

	else if ((Criteria.dwItemCategories & itemcatFuel)
			&& m_pItemType->IsFuel())
		{ }

	//	If we're looking for missiles and this item is a
	//	missile, then we continue.

	else if ((Criteria.dwItemCategories & itemcatMissile)
			&& m_pItemType->IsMissile())
		{ }

	//	If we're looking for usable items and this item is
	//	isable, then we continue

	else if ((Criteria.dwItemCategories & itemcatUseful)
			&& m_pItemType->IsUsable())
		{ }

	//	Otherwise, if this is not one of the required categories, bail out

	else if (!(Criteria.dwItemCategories & m_pItemType->GetCategory()))
		return false;

	//	If any of the flags that must be set is not set, then
	//	we do not match.

	if ((Criteria.wFlagsMustBeSet & m_dwFlags) != Criteria.wFlagsMustBeSet)
		return false;

	//	If any of the flags that must be cleared is set, then
	//	we do not match.

	if ((Criteria.wFlagsMustBeCleared & m_dwFlags) != 0)
		return false;

	//	Check installed flags

	if (Criteria.bInstalledOnly && !IsInstalled())
		return false;

	if (Criteria.bNotInstalledOnly && IsInstalled())
		return false;

	//	Check miscellaneous flags

	if (Criteria.bUsableItemsOnly && m_pItemType->GetUseScreen() == NULL)
		return false;

	if (Criteria.bExcludeVirtual && m_pItemType->IsVirtual())
		return false;

	//	Check required modifiers

	for (i = 0; i < Criteria.ModifiersRequired.GetCount(); i++)
		if (!m_pItemType->HasModifier(Criteria.ModifiersRequired.GetStringValue(i)))
			return false;

	//	Check modifiers not allowed

	for (i = 0; i < Criteria.ModifiersNotAllowed.GetCount(); i++)
		if (m_pItemType->HasModifier(Criteria.ModifiersNotAllowed.GetStringValue(i)))
			return false;

	//	Check frequency range

	if (!Criteria.Frequency.IsBlank())
		{
		int iFreq = m_pItemType->GetFrequency();
		char *pPos = Criteria.Frequency.GetASCIIZPointer();
		bool bMatch = false;
		while (*pPos != '\0' && !bMatch)
			{
			switch (*pPos)
				{
				case 'c':
				case 'C':
					if (iFreq == ftCommon)
						bMatch = true;
					break;

				case 'u':
				case 'U':
					if (iFreq == ftUncommon)
						bMatch = true;
					break;

				case 'r':
				case 'R':
					if (iFreq == ftRare)
						bMatch = true;
					break;

				case 'v':
				case 'V':
					if (iFreq == ftVeryRare)
						bMatch = true;
					break;

				case '-':
				case 'n':
				case 'N':
					if (iFreq == ftNotRandom)
						bMatch = true;
					break;
				}

			pPos++;
			}

		if (!bMatch)
			return false;
		}

	//	Check for level modifiers

	if (Criteria.iEqualToLevel != -1 && m_pItemType->GetLevel() != Criteria.iEqualToLevel)
		return false;

	if (Criteria.iGreaterThanLevel != -1 && m_pItemType->GetLevel() <= Criteria.iGreaterThanLevel)
		return false;

	if (Criteria.iLessThanLevel != -1 && m_pItemType->GetLevel() >= Criteria.iLessThanLevel)
		return false;

	//	Check for price modifiers

	if (Criteria.iEqualToPrice != -1 && GetValue() != Criteria.iEqualToPrice)
		return false;

	if (Criteria.iGreaterThanPrice != -1 && GetValue() <= Criteria.iGreaterThanPrice)
		return false;

	if (Criteria.iLessThanPrice != -1 && GetValue() >= Criteria.iLessThanPrice)
		return false;

	//	Check for mass modifiers

	if (Criteria.iEqualToMass != -1 && m_pItemType->GetMassKg() != Criteria.iEqualToMass)
		return false;

	if (Criteria.iGreaterThanMass != -1 && m_pItemType->GetMassKg() <= Criteria.iGreaterThanMass)
		return false;

	if (Criteria.iLessThanMass != -1 && m_pItemType->GetMassKg() >= Criteria.iLessThanMass)
		return false;

	//	Otherwise, we match completely

	return true;
	}

void CItem::InitCriteriaAll (CItemCriteria *retCriteria)

//	InitCriteriaAll
//
//	Initializes criteria set to all

	{
	retCriteria->dwItemCategories = 0xffffffff;
	retCriteria->wFlagsMustBeSet = 0;
	retCriteria->wFlagsMustBeCleared = 0;

	retCriteria->bUsableItemsOnly = false;
	retCriteria->bExcludeVirtual = true;
	retCriteria->bInstalledOnly = false;
	retCriteria->bNotInstalledOnly = false;
	retCriteria->ModifiersRequired.RemoveAll();
	retCriteria->ModifiersNotAllowed.RemoveAll();
	retCriteria->Frequency = NULL_STR;

	retCriteria->iEqualToLevel = -1;
	retCriteria->iGreaterThanLevel = -1;
	retCriteria->iLessThanLevel = -1;

	retCriteria->iEqualToPrice = -1;
	retCriteria->iGreaterThanPrice = -1;
	retCriteria->iLessThanPrice = -1;

	retCriteria->iEqualToMass = -1;
	retCriteria->iGreaterThanMass = -1;
	retCriteria->iLessThanMass = -1;

	retCriteria->pFilter = NULL;
	}

void CItem::ParseCriteria (const CString &sCriteria, CItemCriteria *retCriteria)

//	ParseCriteria
//
//	Parses a criteria string and initializes the criteria structure

	{
	//	Initialize

	retCriteria->dwItemCategories = 0;
	retCriteria->wFlagsMustBeSet = 0;
	retCriteria->wFlagsMustBeCleared = 0;

	retCriteria->bUsableItemsOnly = false;
	retCriteria->bExcludeVirtual = true;
	retCriteria->bInstalledOnly = false;
	retCriteria->bNotInstalledOnly = false;
	retCriteria->ModifiersRequired.RemoveAll();
	retCriteria->ModifiersNotAllowed.RemoveAll();
	retCriteria->Frequency = NULL_STR;

	retCriteria->iEqualToLevel = -1;
	retCriteria->iGreaterThanLevel = -1;
	retCriteria->iLessThanLevel = -1;

	retCriteria->iEqualToPrice = -1;
	retCriteria->iGreaterThanPrice = -1;
	retCriteria->iLessThanPrice = -1;

	retCriteria->iEqualToMass = -1;
	retCriteria->iGreaterThanMass = -1;
	retCriteria->iLessThanMass = -1;

	retCriteria->pFilter = NULL;

	bool bExclude = false;

	//	Parse string

	char *pPos = sCriteria.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case '*':
				retCriteria->dwItemCategories = 0xFFFFFFFF;
				break;

			case 'a':
				if (bExclude)
					retCriteria->dwItemCategories &= ~itemcatArmor;
				else
					retCriteria->dwItemCategories |= itemcatArmor;
				break;

			case 'd':
				if (bExclude)
					retCriteria->dwItemCategories &= ~(itemcatMiscDevice
							| itemcatWeapon
							| itemcatLauncher
							| itemcatReactor
							| itemcatShields
							| itemcatCargoHold
							| itemcatDrive);
				else
					retCriteria->dwItemCategories |= itemcatMiscDevice
							| itemcatWeapon
							| itemcatLauncher
							| itemcatReactor
							| itemcatShields
							| itemcatCargoHold
							| itemcatDrive;
				break;

			case 'f':
				if (bExclude)
					retCriteria->dwItemCategories &= ~itemcatFuel;
				else
					retCriteria->dwItemCategories |= itemcatFuel;
				break;

			case 'l':
				if (bExclude)
					retCriteria->dwItemCategories &= ~itemcatLauncher;
				else
					retCriteria->dwItemCategories |= itemcatLauncher;
				break;

			case 'm':
				if (bExclude)
					retCriteria->dwItemCategories &= ~itemcatMissile;
				else
					retCriteria->dwItemCategories |= itemcatMissile;
				break;

			case 'r':
				if (bExclude)
					retCriteria->dwItemCategories &= ~itemcatReactor;
				else
					retCriteria->dwItemCategories |= itemcatReactor;
				break;

			case 's':
				if (bExclude)
					retCriteria->dwItemCategories &= ~itemcatShields;
				else
					retCriteria->dwItemCategories |= itemcatShields;
				break;

			case 't':
				if (bExclude)
					retCriteria->dwItemCategories &= ~itemcatMisc;
				else
					retCriteria->dwItemCategories |= itemcatMisc;
				break;

			case 'u':
				if (bExclude)
					retCriteria->dwItemCategories &= ~itemcatUseful;
				else
					retCriteria->dwItemCategories |= itemcatUseful;
				break;

			case 'v':
				if (bExclude)
					retCriteria->dwItemCategories &= ~itemcatDrive;
				else
					retCriteria->dwItemCategories |= itemcatDrive;
				break;

			case 'w':
				if (bExclude)
					retCriteria->dwItemCategories &= ~(itemcatWeapon | itemcatLauncher);
				else
					retCriteria->dwItemCategories |= itemcatWeapon | itemcatLauncher;
				break;

			case 'I':
				retCriteria->bInstalledOnly = true;
				break;

			case 'D':
				retCriteria->wFlagsMustBeSet |= flagDamaged;
				break;

			case 'F':
				{
				pPos++;
				if (*pPos == ':')
					{
					pPos++;
					char *pStart = pPos;
					while (*pPos != '\0' && *pPos != ';')
						pPos++;

					retCriteria->Frequency = CString(pStart, pPos - pStart);
					}
				break;
				}

			case 'N':
				retCriteria->wFlagsMustBeCleared |= flagDamaged;
				break;

			case 'S':
				retCriteria->bUsableItemsOnly = true;
				break;

			case 'U':
				retCriteria->bNotInstalledOnly = true;
				break;

			case '+':
			case '-':
				{
				char chChar = *pPos;
				pPos++;

				//	Deal with category exclusion

				//	Get the modifier

				char *pStart = pPos;
				while (*pPos != '\0' && *pPos != ';')
					pPos++;

				CString sModifier = CString(pStart, pPos - pStart);

				//	Required or Not Allowed

				if (chChar == '+')
					retCriteria->ModifiersRequired.AppendString(sModifier, NULL);
				else
					retCriteria->ModifiersNotAllowed.AppendString(sModifier, NULL);

				//	No trailing semi

				if (*pPos == '\0')
					pPos--;

				break;
				}

			case '~':
				bExclude = true;
				break;

			case '=':
			case '>':
			case '<':
				{
				char chChar = *pPos;
				pPos++;

				//	<= or >=

				int iEqualAdj;
				if (*pPos == '=')
					{
					pPos++;
					iEqualAdj = 1;
					}
				else
					iEqualAdj = 0;

				//	Is this price?

				char comparison;
				if (*pPos == '$' || *pPos == '#')
					comparison = *pPos++;
				else
					comparison = '\0';

				//	Get the number

				char *pNewPos;
				int iValue = strParseInt(pPos, 0, &pNewPos, NULL);

				//	Back up one because we will increment at the bottom
				//	of the loop.

				if (pPos != pNewPos)
					pPos = pNewPos - 1;

				//	Price limits

				if (comparison == '$')
					{
					if (chChar == '=')
						retCriteria->iEqualToPrice = iValue;
					else if (chChar == '>')
						retCriteria->iGreaterThanPrice = iValue - iEqualAdj;
					else if (chChar == '<')
						retCriteria->iLessThanPrice = iValue + iEqualAdj;
					}

				//	Mass limits

				else if (comparison == '#')
					{
					if (chChar == '=')
						retCriteria->iEqualToMass = iValue;
					else if (chChar == '>')
						retCriteria->iGreaterThanMass = iValue - iEqualAdj;
					else if (chChar == '<')
						retCriteria->iLessThanMass = iValue + iEqualAdj;
					}

				//	Level limits

				else
					{
					if (iValue < 1 || iValue > 25)
						retCriteria->iEqualToLevel = -1;
					else if (chChar == '=')
						retCriteria->iEqualToLevel = iValue;
					else if (chChar == '>')
						retCriteria->iGreaterThanLevel = iValue - iEqualAdj;
					else if (chChar == '<')
						retCriteria->iLessThanLevel = iValue + iEqualAdj;
					}

				break;
				}
			}

		pPos++;
		}
	}

void CItem::ReadFromCCItem (CCodeChain &CC, ICCItem *pBuffer)

//	ReadFromCCItem
//
//	Reads an item from ICCItem

	{
	*this = CItem();

	if (!pBuffer->IsNil())
		{
		//	Load the item type

		DWORD dwUNID = (DWORD)pBuffer->GetElement(0)->GetIntegerValue();
		m_pItemType = g_pUniverse->FindItemType(dwUNID);
		if (m_pItemType == NULL)
			return;

		//	Next is the count, flags, and installed

		DWORD *pDest = (DWORD *)this;
		pDest[1] = (DWORD)pBuffer->GetElement(1)->GetIntegerValue();

		//	Previous version

		if (pBuffer->GetCount() == 3)
			{
			DWORD dwLoad = (DWORD)pBuffer->GetElement(2)->GetIntegerValue();
			if (dwLoad)
				{
				Extra();
				m_pExtra->m_dwCharges = LOBYTE(LOWORD(dwLoad));
				m_pExtra->m_dwCondition = HIBYTE(LOWORD(dwLoad));
				m_pExtra->m_dwMods = HIWORD(dwLoad);
				}
			}

		//	Otherwise

		else if (pBuffer->GetCount() > 3)
			{
			Extra();

			//	Charges

			m_pExtra->m_dwCharges = (DWORD)pBuffer->GetElement(2)->GetIntegerValue();
			m_pExtra->m_dwCondition = (DWORD)pBuffer->GetElement(3)->GetIntegerValue();
			m_pExtra->m_dwMods = (DWORD)pBuffer->GetElement(4)->GetIntegerValue();

			//	Attribute data block

			CString sData = pBuffer->GetElement(5)->GetStringValue();
			CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());
			Stream.Open();
			m_pExtra->m_Data.ReadFromStream(&Stream);
			Stream.Close();
			}
		}
	}

void CItem::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pItemType = g_pUniverse->FindItemType(dwLoad);

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_dwCount = LOWORD(dwLoad);
	m_dwFlags = LOBYTE(HIWORD(dwLoad));
	m_dwInstalled = HIBYTE(HIWORD(dwLoad));

	//	Load SExtra struct

	if (Ctx.dwVersion >= 23)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad)
			{
			m_pExtra = new SExtra;

			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			m_pExtra->m_dwCharges = LOWORD(dwLoad);
			m_pExtra->m_dwCondition = HIWORD(dwLoad);

			Ctx.pStream->Read((char *)&m_pExtra->m_dwMods, sizeof(DWORD));

			m_pExtra->m_Data.ReadFromStream(Ctx);
			}
		else
			m_pExtra = NULL;
		}
	else
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		DWORD dwCharges = LOBYTE(LOWORD(dwLoad));
		DWORD dwCondition = HIBYTE(LOWORD(dwLoad));
		DWORD dwMods = HIWORD(dwLoad);

		if (dwCharges || dwCondition || dwMods)
			{
			m_pExtra = new SExtra;
			m_pExtra->m_dwCharges = dwCharges;
			m_pExtra->m_dwCondition = dwCondition;
			m_pExtra->m_dwMods = dwMods;
			}
		else
			m_pExtra = NULL;
		}
	}

ICCItem *CItem::WriteToCCItem (CCodeChain &CC) const

//	WriteToCCItem
//
//	Converts item to a ICCItem

	{
	ICCItem *pResult = CC.CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	First integer is the item UNID

	ICCItem *pInt;

	pInt = CC.CreateInteger(GetType()->GetUNID());
	pList->Append(&CC, pInt, NULL);
	pInt->Discard(&CC);

	//	Next is the count, flags, and installed

	DWORD *pSource = (DWORD *)this;
	pInt = CC.CreateInteger(pSource[1]);
	pList->Append(&CC, pInt, NULL);
	pInt->Discard(&CC);

	//	Save extra

	if (m_pExtra)
		{
		//	Charges

		pInt = CC.CreateInteger(m_pExtra->m_dwCharges);
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);

		//	Condition

		pInt = CC.CreateInteger(m_pExtra->m_dwCondition);
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);

		//	Mods

		pInt = CC.CreateInteger(m_pExtra->m_dwMods);
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);

		//	Attribute data block

		CMemoryWriteStream Stream;
		Stream.Create();
		m_pExtra->m_Data.WriteToStream(&Stream);
		Stream.Close();

		pInt = CC.CreateString(CString(Stream.GetPointer(), Stream.GetLength()));
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);
		}

	return pResult;
	}

void CItem::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes to a stream
//
//	DWORD		item type UNID
//	DWORD		low = m_dwCount; high = (low = m_dwFlags; high = m_dwInstalled)
//	DWORD		non-zero if SExtra follows
//
//	SExtra
//	DWORD		low = m_dwCharges; high = m_dwCondition
//	DWORD		m_dwMods
//	CAttributeDataBlock

	{
	DWORD dwSave = m_pItemType->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = MAKELONG(m_dwCount, MAKEWORD(m_dwFlags, m_dwInstalled));
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Save SExtra

	dwSave = (m_pExtra ? 0xffffffff : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	if (m_pExtra)
		{
		dwSave = MAKELONG(m_pExtra->m_dwCharges, m_pExtra->m_dwCondition);
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		pStream->Write((char *)&m_pExtra->m_dwMods, sizeof(DWORD));

		//	Note: Currently does not support saving object references

		m_pExtra->m_Data.WriteToStream(pStream);
		}
	}

//	CItemCriteria ------------------------------------------------------------

CItemCriteria::CItemCriteria (void) : pFilter(NULL)
	{ 
	}

CItemCriteria::CItemCriteria (const CItemCriteria &Copy)

//	Criteria copy constructor

	{
	dwItemCategories = Copy.dwItemCategories;
	wFlagsMustBeSet = Copy.wFlagsMustBeSet;
	wFlagsMustBeCleared = Copy.wFlagsMustBeCleared;

	bUsableItemsOnly = Copy.bUsableItemsOnly;
	bExcludeVirtual = Copy.bExcludeVirtual;
	bInstalledOnly = Copy.bInstalledOnly;
	bNotInstalledOnly = Copy.bNotInstalledOnly;

	ModifiersRequired = Copy.ModifiersRequired;
	ModifiersNotAllowed = Copy.ModifiersNotAllowed;
	Frequency = Copy.Frequency;

	iEqualToLevel = Copy.iEqualToLevel;
	iGreaterThanLevel = Copy.iGreaterThanLevel;
	iLessThanLevel = Copy.iLessThanLevel;

	iEqualToPrice = Copy.iEqualToPrice;
	iGreaterThanPrice = Copy.iGreaterThanPrice;
	iLessThanPrice = Copy.iLessThanPrice;

	iEqualToMass = Copy.iEqualToMass;
	iGreaterThanMass = Copy.iGreaterThanMass;
	iLessThanMass = Copy.iLessThanMass;

	pFilter = Copy.pFilter;
	if (pFilter)
		pFilter->Reference();
	}

CItemCriteria &CItemCriteria::operator= (const CItemCriteria &Copy)

//	Criteria equals operator

	{
	if (pFilter)
		pFilter->Discard(&g_pUniverse->GetCC());

	dwItemCategories = Copy.dwItemCategories;
	wFlagsMustBeSet = Copy.wFlagsMustBeSet;
	wFlagsMustBeCleared = Copy.wFlagsMustBeCleared;

	bUsableItemsOnly = Copy.bUsableItemsOnly;
	bExcludeVirtual = Copy.bExcludeVirtual;
	bInstalledOnly = Copy.bInstalledOnly;
	bNotInstalledOnly = Copy.bNotInstalledOnly;

	ModifiersRequired = Copy.ModifiersRequired;
	ModifiersNotAllowed = Copy.ModifiersNotAllowed;
	Frequency = Copy.Frequency;

	iEqualToLevel = Copy.iEqualToLevel;
	iGreaterThanLevel = Copy.iGreaterThanLevel;
	iLessThanLevel = Copy.iLessThanLevel;

	iEqualToPrice = Copy.iEqualToPrice;
	iGreaterThanPrice = Copy.iGreaterThanPrice;
	iLessThanPrice = Copy.iLessThanPrice;

	iEqualToMass = Copy.iEqualToMass;
	iGreaterThanMass = Copy.iGreaterThanMass;
	iLessThanMass = Copy.iLessThanMass;

	pFilter = Copy.pFilter;
	if (pFilter)
		pFilter->Reference();

	return *this;
	}

CItemCriteria::~CItemCriteria (void)

	{
	if (pFilter)
		pFilter->Discard(&g_pUniverse->GetCC());
	}

