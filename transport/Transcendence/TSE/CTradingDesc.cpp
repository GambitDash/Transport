//	CTradingDesc.cpp
//
//	CTradingDesc class

#include "PreComp.h"


#define SELL_TAG								CONSTLIT("Sell")
#define BUY_TAG									CONSTLIT("Buy")

#define CURRENCY_ATTRIB							CONSTLIT("currency")
#define CREDIT_CONVERSION_ATTRIB				CONSTLIT("creditConversion")
#define MAX_ATTRIB								CONSTLIT("max")
#define REPLENISH_ATTRIB						CONSTLIT("replenish")
#define ITEM_ATTRIB								CONSTLIT("item")
#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define PRICE_ADJ_ATTRIB						CONSTLIT("priceAdj")

CTradingDesc::CTradingDesc (void) : m_pList(NULL), m_iCount(0)

//	CTradingDesc constructor

	{
	}

CTradingDesc::~CTradingDesc (void)

//	CTradingDesc destructor

	{
	if (m_pList)
		delete [] m_pList;
	}

int CTradingDesc::ComputeMaxCurrency (CSpaceObject *pObj)

//	ComputeMaxCurrency
//
//	Computes max balance

	{
	return m_iMaxCurrency * (90 + ((pObj->GetDestiny() + 9) / 18)) / 100;
	}

int CTradingDesc::ComputePrice (const CItem &Item, const SCommodityDesc &Commodity)

//	ComputePrice
//
//	Computes the price of the item

	{
	return (Item.GetValue() * Commodity.iPriceAdj) / 100;
	}

ALERROR CTradingDesc::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CTradingDesc **retpTrade)

//	InitFromXML
//
//	Initialize from an XML element

	{
	int i;

	//	Allocate the trade structure

	CTradingDesc *pTrade = new CTradingDesc;
	pTrade->m_sCurrency = pDesc->GetAttribute(CURRENCY_ATTRIB);
	pTrade->m_iCurrencyConversion = pDesc->GetAttributeInteger(CREDIT_CONVERSION_ATTRIB);
	pTrade->m_iMaxCurrency = pDesc->GetAttributeInteger(MAX_ATTRIB);
	pTrade->m_iReplenishCurrency = pDesc->GetAttributeInteger(REPLENISH_ATTRIB);

	if (pTrade->m_sCurrency.IsBlank())
		{
		pTrade->m_sCurrency = CONSTLIT("credit");
		pTrade->m_iCurrencyConversion = 100;
		}
	else if (pTrade->m_iCurrencyConversion <= 0)
		pTrade->m_iCurrencyConversion = 100;

	pTrade->m_iMaxCurrency = Max(0, pTrade->m_iMaxCurrency);
	pTrade->m_iReplenishCurrency = Max(0, pTrade->m_iReplenishCurrency);

	//	Allocate the array

	pTrade->m_iCount = pDesc->GetContentElementCount();
	if (pTrade->m_iCount)
		{
		pTrade->m_pList = new SCommodityDesc [pTrade->m_iCount];
		if (pTrade->m_pList == NULL)
			{
			delete pTrade;
			return ERR_MEMORY;
			}

		//	Load

		for (i = 0; i < pTrade->m_iCount; i++)
			{
			CXMLElement *pLine = pDesc->GetContentElement(i);
			SCommodityDesc *pCommodity = &pTrade->m_pList[i];

			//	Parse criteria

			CString sCriteria = pLine->GetAttribute(CRITERIA_ATTRIB);
			if (!sCriteria.IsBlank())
				CItem::ParseCriteria(sCriteria, &pCommodity->ItemCriteria);
			else
				CItem::InitCriteriaAll(&pCommodity->ItemCriteria);

			//	Item

			pCommodity->pItemType.LoadUNID(Ctx, pLine->GetAttribute(ITEM_ATTRIB));

			//	Other

			pCommodity->iPriceAdj = pLine->GetAttributeInteger(PRICE_ADJ_ATTRIB);

			//	Flags

			pCommodity->dwFlags = 0;
			if (strEquals(pLine->GetTag(), BUY_TAG))
				pCommodity->dwFlags |= FLAG_BUYS;
			else if (strEquals(pLine->GetTag(), SELL_TAG))
				pCommodity->dwFlags |= FLAG_SELLS;
			}
		}

	//	Done

	*retpTrade = pTrade;

	return NOERROR;
	}

bool CTradingDesc::Buys (CSpaceObject *pObj, const CItem &Item, int *retiPrice, int *retiMaxCount)

//	Buys
//
//	Returns TRUE if the given object buys items of the given type.
//	Optionally returns a price and a max number.
//
//	Note that we always return a price for items we are willing to buy, even if we
//	don't currently have enough to buy it.

	{
	int i;

	//	Loop over the commodity list and find the first entry that matches

	for (i = 0; i < m_iCount; i++)
		if (Matches(Item, m_pList[i])
				&& (m_pList[i].dwFlags & FLAG_BUYS))
			{
			int iPrice = ComputePrice(Item, m_pList[i]);
			if (iPrice < 0)
				return false;

			//	Compute the maximum number of this item that we are willing
			//	to buy. First we figure out how much money the station has left

			int iBalance = pObj->GetBalance();
			int iMaxCount = (iPrice > 0 ? (iBalance / iPrice) : 0);

			//	Done

			if (retiMaxCount)
				*retiMaxCount = iMaxCount;

			if (retiPrice)
				*retiPrice = iPrice;

			return true;
			}

	return false;
	}

int CTradingDesc::Charge (CSpaceObject *pObj, int iCharge)

//	Charge
//
//	Charge out of the station's balance

	{
	if (m_iMaxCurrency)
		return pObj->ChargeMoney(iCharge);
	else
		return 0;
	}

bool CTradingDesc::Matches (const CItem &Item, const SCommodityDesc &Commodity)

//	Matches
//
//	Returns TRUE if the given item matches the commodity

	{
	if (Commodity.pItemType
			&& (Commodity.pItemType != Item.GetType()))
		return false;

	return Item.MatchesCriteria(Commodity.ItemCriteria);
	}

void CTradingDesc::OnCreate (CSpaceObject *pObj)

//	OnCreate
//
//	Station is created

	{
	//	Give the station a limited amount of money

	if (m_iMaxCurrency)
		{
		int iMaxCurrency = ComputeMaxCurrency(pObj);
		pObj->CreditMoney(iMaxCurrency);
		}
	}

ALERROR CTradingDesc::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Design loaded

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_iCount; i++)
		if (error = m_pList[i].pItemType.Bind(Ctx))
			return error;

	return NOERROR;
	}

void CTradingDesc::OnUpdate (CSpaceObject *pObj)

//	OnUpdate
//
//	Station updates (call roughly every 900 ticks)

	{
	if (m_iMaxCurrency && m_iReplenishCurrency)
		{
		int iBalance = pObj->GetBalance();
		int iMaxCurrency = ComputeMaxCurrency(pObj);

		if (iBalance < iMaxCurrency)
			pObj->CreditMoney(m_iReplenishCurrency);
		}
	}

bool CTradingDesc::Sells (CSpaceObject *pObj, const CItem &Item, int *retiPrice)

//	Sells
//
//	Returns TRUE if the given object can currently sell the given item type.
//	Optionally returns a price

	{
	int i;

	//	Loop over the commodity list and find the first entry that matches

	for (i = 0; i < m_iCount; i++)
		if (Matches(Item, m_pList[i])
				&& (m_pList[i].dwFlags & FLAG_SELLS))
			{
			int iPrice = ComputePrice(Item, m_pList[i]);
			if (iPrice <= 0)
				return false;

			//	Done

			if (retiPrice)
				*retiPrice = iPrice;

			return true;
			}

	return false;
	}
