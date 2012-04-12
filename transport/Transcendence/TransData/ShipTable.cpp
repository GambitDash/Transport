//	ShipTable.cpp
//
//	Generate basic stats about ship classes

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define MAX_LEVEL			25

#define FIELD_CARGO_SPACE						CONSTLIT("cargoSpace")
#define FIELD_EXPLOSION_TYPE					CONSTLIT("explosionType")
#define FIELD_FIRE_ACCURACY						CONSTLIT("fireAccuracy")
#define FIELD_FIRE_RANGE_ADJ					CONSTLIT("fireRangeAdj")
#define FIELD_FIRE_RATE_ADJ						CONSTLIT("fireRateAdj")
#define FIELD_LAUNCHER							CONSTLIT("launcher")
#define FIELD_LAUNCHER_UNID						CONSTLIT("launcherUNID")
#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_MAX_SPEED							CONSTLIT("maxSpeed")
#define FIELD_MANEUVER							CONSTLIT("maneuver")
#define FIELD_MANUFACTURER						CONSTLIT("manufacturer")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_PRIMARY_ARMOR						CONSTLIT("primaryArmor")
#define FIELD_PRIMARY_ARMOR_UNID				CONSTLIT("primaryArmorUNID")
#define FIELD_PRIMARY_WEAPON					CONSTLIT("primaryWeapon")
#define FIELD_PRIMARY_WEAPON_RANGE				CONSTLIT("primaryWeaponRange")
#define FIELD_PRIMARY_WEAPON_RANGE_ADJ			CONSTLIT("primaryWeaponRangeAdj")
#define FIELD_PRIMARY_WEAPON_UNID				CONSTLIT("primaryWeaponUNID")
#define FIELD_SCORE								CONSTLIT("score")
#define FIELD_SHIELD							CONSTLIT("shield")
#define FIELD_SHIELD_UNID						CONSTLIT("shieldsUNID")
#define FIELD_THRUST_TO_WEIGHT					CONSTLIT("thrustToWeight")

#define AddField(FIELD)		if (pCmdLine->GetAttributeBool(FIELD)) Cols.AppendString(FIELD)

void GenerateShipTable (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i, j;

	//	Some options

	bool bAllClasses = pCmdLine->GetAttributeBool(CONSTLIT("allClasses"));

	//	Generate a list of columns to display

	CStringArray Cols;
	Cols.AppendString(FIELD_LEVEL);
	Cols.AppendString(FIELD_NAME);

	AddField(FIELD_MAX_SPEED);
	AddField(FIELD_THRUST_TO_WEIGHT);
	AddField(FIELD_MANEUVER);
	AddField(FIELD_PRIMARY_ARMOR);
	AddField(FIELD_PRIMARY_WEAPON);
	AddField(FIELD_PRIMARY_WEAPON_RANGE);
	AddField(FIELD_PRIMARY_WEAPON_RANGE_ADJ);
	AddField(FIELD_LAUNCHER);
	AddField(FIELD_SHIELD);
	AddField(FIELD_FIRE_RATE_ADJ);
	AddField(FIELD_FIRE_ACCURACY);
	AddField(FIELD_FIRE_RANGE_ADJ);
	AddField(FIELD_SCORE);

	//	Output the header

	for (j = 0; j < Cols.GetCount(); j++)
		{
		if (j != 0)
			printf("\t");

		printf(Cols.GetStringValue(j).GetASCIIZPointer());
		}

	printf("\n");

	//	Generate a table

	CSymbolTable Table(FALSE, TRUE);

	//	Loop over all items that match and add them to
	//	a sorted table.

	for (i = 0; i < Universe.GetShipClassCount(); i++)
		{
		CShipClass *pClass = Universe.GetShipClass(i);

		//	Only include generic classes unless otherwise specified

		if (!bAllClasses && !pClass->HasAttribute(CONSTLIT("genericClass")))
			continue;

		//	Figure out the sort order

		char szBuffer[1024];
		wsprintf(szBuffer, "%08d%s%d", 
				pClass->GetScore(),
				pClass->GetNounPhrase(0).GetASCIIZPointer(),
				pClass->GetUNID());
		Table.AddEntry(CString(szBuffer), (CObject *)pClass);
		}

	//	Output table

	for (i = 0; i < Table.GetCount(); i++)
		{
		CShipClass *pClass = (CShipClass *)Table.GetValue(i);

		//	Output each row

		for (j = 0; j < Cols.GetCount(); j++)
			{
			if (j != 0)
				printf("\t");

			CString sField = Cols.GetStringValue(j);
			CString sValue = pClass->GetDataField(sField);

			if (strEquals(sField, FIELD_MANEUVER) 
					|| strEquals(sField, FIELD_THRUST_TO_WEIGHT))
				printf("%.1f", strToInt(sValue, 0, NULL) / 1000.0);
			else if (strEquals(sField, FIELD_SCORE))
				{
				int iActualScore = pClass->GetScore();
				int iScore = pClass->CalcScore();

				if (iScore == iActualScore)
					printf("%d", iScore);
				else
					printf("%d (%d)\n", iActualScore);
				}
			else
				printf(sValue.GetASCIIZPointer());
			}

		printf("\n");
		}

	printf("\n");
	}
