//	CCExtensions.cpp
//
//	CodeChain extensions

#include "PreComp.h"

#define FN_ARM_NAME					1
#define FN_ARM_HITPOINTS			2
#define FN_ARM_REPAIRCOST			3

#define FN_ARM_REPAIRTECH			5
#define FN_ARM_IS_RADIATION_IMMUNE	6

ICCItem *fnArmGet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_ENV_HAS_ATTRIBUTE		1

ICCItem *fnEnvironmentGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_ITEM_COUNT				1
#define FN_ITEM_PRICE				2
#define FN_ITEM_NAME				3
#define FN_ITEM_DAMAGED				4
#define FN_ITEM_USE_SCREEN			5
#define FN_ITEM_TYPE_DATA			6
#define FN_ITEM_SET_KNOWN			7
#define FN_ITEM_KNOWN				8
#define FN_ITEM_ARMOR_TYPE			9
#define FN_ITEM_MATCHES				10
#define FN_ITEM_CATEGORY			11
#define FN_ITEM_ENHANCED			12
#define FN_ITEM_MASS				13
#define FN_ITEM_HAS_MODIFIER		14
#define FN_ITEM_INSTALLED			15
#define FN_ITEM_SET_REFERENCE		16
#define FN_ITEM_REFERENCE			17
#define FN_ITEM_LEVEL				18
#define FN_ITEM_UNID				19
#define FN_ITEM_ARMOR_INSTALLED_LOCATION	20
#define FN_ITEM_CHARGES				21
#define FN_ITEM_INSTALL_COST		22
#define FN_ITEM_ACTUAL_PRICE		23
#define FN_ITEM_MAX_APPEARING		24
#define FN_ITEM_FREQUENCY			25
#define FN_ITEM_IMAGE_DESC			26
#define FN_ITEM_DAMAGE_TYPE			27
#define FN_ITEM_TYPES				28
#define FN_ITEM_DATA				29
#define FN_ITEM_GET_STATIC_DATA		30
#define FN_ITEM_GET_GLOBAL_DATA		31
#define FN_ITEM_SET_GLOBAL_DATA		32
#define FN_ITEM_AVERAGE_APPEARING	33

ICCItem *fnItemGetTypes (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnItemGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnItemSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_ITEM_TYPE_SET_KNOWN		1

ICCItem *fnItemTypeSet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_OBJ_GETDATA				1
#define FN_OBJ_SETDATA				2
#define FN_OBJ_GET_OBJREF_DATA		3
#define FN_OBJ_SET_OBJREF_DATA		4
#define FN_OBJ_GET_STATIC_DATA		5
#define FN_OBJ_INCREMENT_DATA		6
#define FN_OBJ_GET_GLOBAL_DATA		7
#define FN_OBJ_SET_GLOBAL_DATA		8
#define FN_OBJ_GET_STATIC_DATA_FOR_STATION_TYPE	9

ICCItem *fnObjData (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnObjAddRandomItems (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_OBJ_NAME					1
#define FN_OBJ_IS_SHIP				2
#define FN_OBJ_DISTANCE				3
#define FN_OBJ_KNOWN				4
#define FN_OBJ_SET_KNOWN			5
#define FN_OBJ_IS_UNDER_ATTACK		6
#define FN_OBJ_IS_ABANDONED			7
#define FN_OBJ_MESSAGE				8
#define FN_OBJ_NEAREST_STARGATE		9
#define FN_OBJ_POSITION				10
#define FN_OBJ_JUMP					11
#define FN_OBJ_REGISTER_EVENTS		12
#define FN_OBJ_UNREGISTER_EVENTS	13
#define FN_OBJ_DAMAGE_TYPE			14
#define FN_OBJ_PARALYSIS			15
#define FN_OBJ_LOWER_SHIELDS		16
#define FN_OBJ_VISIBLE_DAMAGE		17
#define FN_OBJ_TARGET				18
#define FN_OBJ_CARGO_SPACE_LEFT		19
#define FN_OBJ_ATTRIBUTE			20
#define FN_OBJ_ORDER_GIVER			21
#define FN_OBJ_MAX_POWER			22
#define FN_OBJ_DESTINY				23
#define FN_OBJ_SHIELD_LEVEL			24
#define FN_OBJ_INSTALLED_ITEM_DESC	25
#define FN_OBJ_COMBAT_POWER			26
#define FN_OBJ_SOVEREIGN			27
#define FN_OBJ_ADD_SUBORDINATE		28
#define FN_OBJ_ENEMY				29
#define FN_OBJ_DESTROY				30
#define FN_OBJ_CAN_ATTACK			31
#define FN_OBJ_INCREMENT_VELOCITY	32
#define FN_OBJ_GET_SELL_PRICE		33
#define FN_OBJ_GET_BUY_PRICE		34
#define FN_OBJ_CHARGE				35
#define FN_OBJ_GET_BALANCE			36
#define FN_OBJ_GET_ID				37
#define FN_OBJ_FIRE_EVENT			38
#define FN_OBJ_CREDIT				39
#define FN_OBJ_IDENTIFIED			40
#define FN_OBJ_CLEAR_IDENTIFIED		41
#define FN_OBJ_DEPLETE_SHIELDS		42
#define FN_OBJ_OBJECT				43
#define FN_OBJ_MATCHES				44
#define FN_OBJ_DOCKED_AT			45
#define FN_OBJ_IMAGE				46
#define FN_OBJ_VELOCITY				47
#define FN_OBJ_LEVEL				48
#define FN_OBJ_FIRE_ITEM_EVENT		49
#define FN_OBJ_MASS					50
#define FN_OBJ_OPEN_DOCKING_PORT_COUNT	51

ICCItem *fnObjGet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnObjSet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnObjGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnObjSetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnObjIDGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_OBJ_ADD_ITEM				1
#define FN_OBJ_REMOVE_ITEM			2
#define FN_OBJ_ENUM_ITEMS			3
#define FN_OBJ_HAS_ITEM				4

ICCItem *fnObjItem (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_OBJ_ARMOR_TYPE			1
#define FN_OBJ_ARMOR_DAMAGE			2
#define FN_OBJ_REPAIR_ARMOR			3
#define FN_OBJ_ARMOR_MAX_HP			5
#define FN_OBJ_DAMAGE_ARMOR			6
#define FN_OBJ_ARMOR_NAME			7
#define FN_OBJ_ARMOR_ITEM			8

ICCItem *fnObjGetArmor (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_SHIP_DEVICE_SLOT_AVAIL	1
#define FN_SHIP_INSTALL_AUTOPILOT	2
#define FN_SHIP_HAS_AUTOPILOT		3
//	spare
#define FN_SHIP_INSTALL_TARGETING	5
#define FN_SHIP_HAS_TARGETING		6
#define FN_SHIP_CLASS				7
#define FN_SHIP_INSTALL_DEVICE		8
#define FN_SHIP_INSTALL_ARMOR		9
#define FN_SHIP_FUEL_NEEDED			10
#define FN_SHIP_REFUEL_FROM_ITEM	11
#define FN_SHIP_REMOVE_DEVICE		12
#define FN_SHIP_ORDER_DOCK			13
#define FN_SHIP_BLINDNESS			14
#define FN_SHIP_ENHANCE_ITEM		15
#define FN_SHIP_CAN_INSTALL_DEVICE	16
#define FN_SHIP_DECONTAMINATE		17
#define FN_SHIP_IS_RADIOACTIVE		18
#define FN_SHIP_CANCEL_ORDERS		19
#define FN_SHIP_ADD_ENERGY_FIELD	20
#define FN_SHIP_IS_FUEL_COMPATIBLE	21
#define FN_SHIP_ITEM_DEVICE_NAME	22
#define FN_SHIP_ORDER_ATTACK		23
#define FN_SHIP_IS_SRS_ENHANCED		24
#define FN_SHIP_ENHANCE_SRS			25
#define FN_SHIP_SHIELD_ITEM_UNID	26
#define FN_SHIP_SHIELD_DAMAGE		27
#define FN_SHIP_SHIELD_MAX_HP		28
#define FN_SHIP_RECHARGE_SHIELD		29
#define FN_SHIP_MAKE_RADIOACTIVE	30
#define FN_SHIP_ORDER_GATE			31
#define FN_SHIP_ORDER_ESCORT		32
#define FN_SHIP_ORDER_LOOT			33
#define FN_SHIP_FUEL				34
#define FN_SHIP_ORDER_PATROL		35
#define FN_SHIP_ARMOR_COUNT			36
#define FN_SHIP_RECHARGE_ITEM		37
#define FN_SHIP_ITEM_CHARGES		38
#define FN_SHIP_DIRECTION			39
#define FN_SHIP_ORDER_WAIT			40
#define FN_SHIP_FIX_BLINDNESS		41
#define FN_SHIP_ORDER_FOLLOW		42
#define FN_SHIP_DAMAGE_ITEM			43
#define FN_SHIP_CAN_INSTALL_ARMOR	44
#define FN_SHIP_IS_RADIATION_IMMUNE	45
#define FN_SHIP_REPAIR_ITEM			46
#define FN_SHIP_CAN_REMOVE_DEVICE	47
#define FN_SHIP_ORDER_MINE			48
#define FN_SHIP_CONTROLLER			49
#define FN_SHIP_ORDER				50
#define FN_SHIP_ORDER_TARGET		51
#define FN_SHIP_GET_GLOBAL_DATA		52
#define FN_SHIP_SET_GLOBAL_DATA		53
#define FN_SHIP_ORDER_HOLD			54
#define FN_SHIP_ORDER_GUARD			55
#define FN_SHIP_COMMAND_CODE		56
#define FN_SHIP_ORDER_GOTO			57
#define FN_SHIP_IS_NAMED_DEVICE		58
#define FN_SHIP_MAX_SPEED			59
#define FN_SHIP_CLASS_NAME			60
#define FN_SHIP_GET_IMAGE_DESC		62
#define FN_SHIP_DATA_FIELD			63

ICCItem *fnShipGet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnShipGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnShipSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnShipSetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnShipClass (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_STATION_GET_DOCKED_SHIPS		1
#define FN_STATION_IMAGE_VARIANT		2
#define FN_STATION_SET_ACTIVE			3
#define FN_STATION_SET_INACTIVE			4
#define FN_STATION_GET_SUBORDINATES		5
#define FN_STATION_GET_TYPE				6
#define FN_STATION_RECON				7
#define FN_STATION_CLEAR_RECON			8
#define FN_STATION_CLEAR_FIRE_RECON		9
#define FN_STATION_SET_FIRE_RECON		10
#define FN_STATION_GET_GLOBAL_DATA		11
#define FN_STATION_SET_GLOBAL_DATA		12
#define FN_STATION_STRUCTURAL_HP		13
#define FN_STATION_ENCOUNTERED			14
#define FN_STATION_MAX_STRUCTURAL_HP	15

ICCItem *fnStationGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnStationGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnStationSet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnStationType (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_SYS_CREATE_WEAPON_FIRE		1

ICCItem *fnSystemCreate (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

ICCItem *fnRollDice (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnSystemCreateEffect (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnSystemCreateMarker (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnSystemCreateShip (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnSystemCreateStation (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnSystemFind (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnSystemGetObjectByName (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnSystemStopTime (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnSystemVectorOffset (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnObjComms (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnObjEnumItems (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnObjGateTo (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnProgramDamage (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnItemEnumTypes (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnItemList (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnItemCreate (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnItemCreateByName (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnItemCreateRandom (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_ADD_ENCOUNTER_FROM_GATE		0
#define FN_ADD_ENCOUNTER_FROM_DIST		1

ICCItem *fnSystemAddEncounterEvent (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_ADD_TIMER_NORMAL				0
#define FN_ADD_TIMER_RECURRING			1
#define FN_CANCEL_TIMER					2

ICCItem *fnSystemAddStationTimerEvent (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_SYS_NAME						1
#define FN_SYS_LEVEL					2
#define FN_SYS_NODE						3
#define FN_SYS_GET_DATA					4
#define FN_SYS_SET_DATA					5
#define FN_SYS_NAV_PATH_POINT			6
#define FN_SYS_GET_TRAVEL_TIME			7
#define FN_SYS_ENVIRONMENT				8
#define FN_SYS_ALL_NODES				9
#define FN_SYS_HAS_ATTRIBUTE			10
#define FN_SYS_CREATE_STARGATE			11
#define FN_SYS_STARGATES				12
#define FN_SYS_STARGATE_DESTINATION_NODE	13
#define FN_SYS_ADD_STARGATE_TOPOLOGY	14
#define FN_SYS_SYSTEM_TYPE				15

ICCItem *fnSystemGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_NODE_LEVEL					1
#define FN_NODE_SYSTEM_NAME				2
#define FN_NODE_HAS_ATTRIBUTE			3

ICCItem *fnTopologyGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_DESIGN_GET_GLOBAL_DATA		1
#define FN_DESIGN_SET_GLOBAL_DATA		2
#define FN_DESIGN_GET_STATIC_DATA		3
#define FN_DESIGN_INC_GLOBAL_DATA		4

ICCItem *fnDesignGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_UNIVERSE_TICK				0
#define FN_UNIVERSE_UNID				1

ICCItem *fnUniverseGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_VECTOR_ADD					0
#define FN_VECTOR_SUBTRACT				1
#define FN_VECTOR_MULTIPLY				2
#define FN_VECTOR_DIVIDE				3
#define FN_VECTOR_DISTANCE				4
#define FN_VECTOR_RANDOM				5
#define FN_VECTOR_ANGLE					6
#define FN_VECTOR_SPEED					7

ICCItem *fnSystemVectorMath (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define FN_SOVEREIGN_DISPOSITION		0

ICCItem *fnSovereignSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define DISP_NEUTRAL					CONSTLIT("neutral")
#define DISP_ENEMY						CONSTLIT("enemy")
#define DISP_FRIEND						CONSTLIT("friend")

static PRIMITIVEPROCDEF g_Extensions[] =
	{
		//	ArmorClass functions
		//	--------------------

		{	"armGetName",					fnArmGet,		FN_ARM_NAME,
			"(armGetName type) -> Name of the armor",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"armGetHitPoints",				fnArmGet,		FN_ARM_HITPOINTS,
			"(armGetHitPoints type) -> Hit points of armor",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"armIsRadiationImmune",			fnArmGet,		FN_ARM_IS_RADIATION_IMMUNE,
			"(armIsRadiationImmune type) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"armGetRepairCost",				fnArmGet,		FN_ARM_REPAIRCOST,
			"(armGetRepairCost type) -> Cost to repair 1 hit point",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"armGetRepairTech",				fnArmGet,		FN_ARM_REPAIRTECH,
			"(armGetRepairTech type) -> Tech level required to repair",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		//	Item functions
		//	--------------

		{	"itmCreate",					fnItemCreate,	0,	
			"(itmCreate itemUNID count) -> item",
			"ii",	PPFLAG_SIDEEFFECTS,	},

		{	"itmCreateByName",				fnItemCreateByName,	0,	
			"(itmCreateByName criteria name [count]) -> item",
			"ss*",	PPFLAG_SIDEEFFECTS,	},

		{	"itmCreateRandom",				fnItemCreateRandom,	0,
			"(itmCreateRandom criteria levelDistribution) -> item",
			"ss",	PPFLAG_SIDEEFFECTS,	},

		{	"itmEnumTypes",					fnItemEnumTypes,	0,
			"(itmEnumTypes criteria item-var exp)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"itmGetActualPrice",			fnItemGet,		FN_ITEM_ACTUAL_PRICE,
			"(itmGetActualPrice item) -> actual price of a single item",
			"v",	0,	},

		{	"itmGetArmorInstalledLocation",	fnItemGet,		FN_ITEM_ARMOR_INSTALLED_LOCATION,
			"(itmGetArmorInstalledLocation item) -> segment #",
			"v",	0,	},

		{	"itmGetArmorType",				fnItemGet,		FN_ITEM_ARMOR_TYPE,
			"(itmGetArmorType item)",
			"v",	0,	},

		{	"itmGetAverageAppearing",			fnItemGet,		FN_ITEM_AVERAGE_APPEARING,
			"(itmGetAverageAppearing item) -> average number that appear randomly",
			"v",	0,	},

		{	"itmGetCategory",				fnItemGet,		FN_ITEM_CATEGORY,
			"(itmGetCategory item) -> item category",
			"v",	0,	},

		{	"itmGetCharges",				fnItemGet,		FN_ITEM_CHARGES,
			"(itmGetCharges item) -> charges",
			"v",	0,	},

		{	"itmGetCount",					fnItemGet,		FN_ITEM_COUNT,
			"(itmGetCount item)",
			"v",	0,	},

		{	"itmGetDamageType",				fnItemGet,		FN_ITEM_DAMAGE_TYPE,
			"(itmGetDamageType item) -> damage type",
			"v",	0,	},

		{	"itmGetData",					fnItemGet,		FN_ITEM_DATA,
			"(itmGetData item attrib) -> data",
			"vs",	0,	},

		{	"itmGetFrequency",				fnItemGet,		FN_ITEM_FREQUENCY,
			"(itmGetFrequency item) -> frequency",
			"v",	0,	},

		{	"itmGetGlobalData",				fnItemGet,		FN_ITEM_GET_GLOBAL_DATA,
			"(itmGetGlobalData item attrib) -> data",
			"vs",	0,	},

		{	"itmGetImageDesc",				fnItemGet,		FN_ITEM_IMAGE_DESC,
			"(itmGetImageDesc item) -> imageDesc",
			"v",	0,	},

		{	"itmGetInstallCost",			fnItemGet,		FN_ITEM_INSTALL_COST,
			"(itmGetInstallCost item) -> cost",
			"v",	0,	},

		{	"itmGetLevel",					fnItemGet,		FN_ITEM_LEVEL,
			"(itmGetLevel item) -> level",
			"v",	0,	},

		{	"itmGetMass",					fnItemGet,		FN_ITEM_MASS,
			"(itmGetMass item) -> mass of single item in Kg",
			"v",	0,	},

		{	"itmGetMaxAppearing",			fnItemGet,		FN_ITEM_MAX_APPEARING,
			"(itmGetMaxAppearing item) -> max number that appear randomly",
			"v",	0,	},

		{	"itmGetName",					fnItemGet,		FN_ITEM_NAME,
			"(itmGetName item flags)",
		//		flag 0x001 (1) = capitalize
		//		flag 0x002 (2) = pluralize
		//		flag 0x004 (4) = prefix with 'the' or 'a'
		//		flag 0x008 (8) = prefix with count (or 'a')
		//		flag 0x010 (16) = prefix with count
		//		flag 0x020 (32) = no modifiers
		//		flag 0x040 (64) = prefix with 'the' or 'this' or 'these'
		//		flag 0x080 (128) = short form of name
		//		flag 0x100 (256) = actual name
			"vi",	0,	},

		{	"itmGetPrice",					fnItemGet,		FN_ITEM_PRICE,
			"(itmGetPrice item) -> price of a single item",
			"v",	0,	},

		{	"itmGetStaticData",				fnItemGet,		FN_ITEM_GET_STATIC_DATA,
			"(itmGetStaticData item attrib) -> data",
			"vs",	0,	},

		{	"itmGetTypeData",				fnItemGet,		FN_ITEM_TYPE_DATA,
			"(itmGetTypeData item)",
			"v",	0,	},

		{	"itmGetTypes",				fnItemGetTypes,			0,
			"(itmGetTypes criteria) -> list of itemUNIDs",
			"s",	0,	},

		{	"itmGetUNID",					fnItemGet,		FN_ITEM_UNID,
			"(itmGetUNID item) -> itemUNID",
			"v",	0,	},

		{	"itmGetUseScreen",				fnItemGet,		FN_ITEM_USE_SCREEN,
			"(itmGetUseScreen item)",
			"v",	0,	},

		{	"itmHasAttribute",				fnItemGet,		FN_ITEM_HAS_MODIFIER,
			"(itmHasAttribute item attrib) -> True/Nil",
			"vs",	0,	},

		{	"itmHasModifier",				fnItemGet,		FN_ITEM_HAS_MODIFIER,
			"DEPRECATED: Use itmHasAttribute instead.",
			"vs",	0,	},

		{	"itmIsDamaged",					fnItemGet,		FN_ITEM_DAMAGED,
			"(itmIsDamaged item)",
			"v",	0,	},

		{	"itmIsEnhanced",				fnItemGet,		FN_ITEM_ENHANCED,
			"(itmIsEnhanced item) -> Nil or mods",
			"v",	0,	},

		{	"itmIsInstalled",				fnItemGet,		FN_ITEM_INSTALLED,
			"(itmIsInstalled item)",
			"v",	0,	},

		{	"itmIsKnown",					fnItemGet,		FN_ITEM_KNOWN,
			"(itmIsKnown item)",
			"v",	0,	},

		{	"itmMatches",					fnItemGet,		FN_ITEM_MATCHES,
			"(itmMatches item criteria)",
			"vs",	0,	},

		{	"itmSetCharges",				fnItemSet,		FN_ITEM_CHARGES,
			"(itmSetCharges item charges) -> item",
			"vi",	0,	},

		{	"itmSetCount",					fnItemSet,		FN_ITEM_COUNT,
			"(itmSetCount item count) -> item",
			"vi",	0,	},

		{	"itmSetDamaged",				fnItemSet,		FN_ITEM_DAMAGED,
			"(itmSetDamaged item [True/Nil]) -> item",
			"v*",	0,	},

		{	"itmSetData",					fnItemSet,		FN_ITEM_DATA,
			"(itmSetData item attrib data) -> item",
			"vsv",	0,	},

		{	"itmSetEnhanced",				fnItemSet,		FN_ITEM_ENHANCED,
			"(itmSetEnhanced item mods) -> item",
			"vi",	0,	},

		{	"itmSetGlobalData",				fnItemGet,		FN_ITEM_SET_GLOBAL_DATA,
			"(itmSetGlobalData item attrib data) -> True/Nil",
			"vsv",	0,	},

		{	"itmSetKnown",					fnItemTypeSet,	FN_ITEM_TYPE_SET_KNOWN,
			"(itmSetKnown {item | itemUNID})",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"itmSetReference",				fnItemGet,		FN_ITEM_SET_REFERENCE,
			"(itmSetReference item)",
			"v",	PPFLAG_SIDEEFFECTS,	},

		{	"itmHasReference",				fnItemGet,		FN_ITEM_REFERENCE,
			"(itmHasReference item)",
			"v",	0,	},

		//	Miscellaneous functions
		//	-----------------------

		{	"rollDice",						fnRollDice,		0,
			"(rollDice count sides bonus)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"itmAtCursor",					fnItemList,		0,
			"DEPRECATED",
			"v",	0,	},

		//	Ship functions
		//	--------------

		{	"shpAddEnergyField",			fnShipSetOld,		FN_SHIP_ADD_ENERGY_FIELD,
			"(shpAddEnergyField ship fieldUNID lifetime)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpCancelOrders",				fnShipGetOld,		FN_SHIP_CANCEL_ORDERS,
			"(shpCancelOrders ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpCanInstallArmor",			fnShipSetOld,		FN_SHIP_CAN_INSTALL_ARMOR,
			"(shpCanInstallArmor ship item) ->",
		//			0 = OK
		//			1 = Armor too heavy
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpCanInstallDevice",			fnShipSetOld,		FN_SHIP_CAN_INSTALL_DEVICE,
			"(shpCanInstallDevice ship item) ->",
		//			0 = OK
		//			1 = Not a device
		//			2 = No slots
		//			3 = Already installed
		//			4 = Shields already installed
		//			5 = Drive already installed
		//			6 = Missile launcher already installed
		//			7 = Reactor too weak
		//			8 = Cargo expansion already installed
		//			9 = Reactor already installed
		//			10 = Cargo expansion does not fit
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpCanRemoveDevice",			fnShipSetOld,		FN_SHIP_CAN_REMOVE_DEVICE,
			"(shpCanRemoveDevice ship item) ->",
		//			0 = OK
		//			1 = Too much cargo to remove cargo hold
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpConsumeFuel",				fnShipSetOld,		FN_SHIP_FUEL,
			"(shpConsumeFuel ship fuel)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpDamageArmor",				fnObjGetArmor,	FN_OBJ_DAMAGE_ARMOR,
			"(shpDamageArmor obj armorSegment damageType damage)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpDamageItem",				fnShipSetOld,		FN_SHIP_DAMAGE_ITEM,
			"(shpDamageItem ship item)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpDecontaminate",				fnShipGetOld,		FN_SHIP_DECONTAMINATE,
			"(shpDecontaminate ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpFixBlindness",				fnShipGetOld,		FN_SHIP_FIX_BLINDNESS,
			"(shpFixBlindness ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetArmor",					fnObjGetArmor,	FN_OBJ_ARMOR_ITEM,
			"(shpGetArmor ship armorSegment) -> item struct",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetArmorCount",				fnShipGetOld,		FN_SHIP_ARMOR_COUNT,
			"(shpGetArmorCount ship) -> number of armor segments",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetArmorDamage",			fnObjGetArmor,	FN_OBJ_ARMOR_DAMAGE,
			"(objGetArmorDamage obj armorSegment) -> damage to armor segment",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetArmorMaxHitPoints",		fnObjGetArmor,	FN_OBJ_ARMOR_MAX_HP,
			"(shpGetArmorMaxHitPoints obj armorSegment) -> damage to armor segment",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetArmorName",				fnObjGetArmor,	FN_OBJ_ARMOR_NAME,
			"(objGetArmorName obj armorSegment) -> name of armor (e.g., 'forward', etc.)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetArmorType",				fnObjGetArmor,	FN_OBJ_ARMOR_TYPE,
			"(objGetArmorType obj armorSegment) -> CArmorClass *",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetClass",					fnShipGetOld,		FN_SHIP_CLASS,
			"(shpGetClass ship) -> class UNID",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetClassName",				fnShipClass,		FN_SHIP_CLASS_NAME,
			"(shpGetClassName class flags) -> class name",
			"ii",	0,	},

		{	"shpGetDirection",				fnShipGetOld,		FN_SHIP_DIRECTION,
			"(shpGetDirection ship) -> angle",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetDataField",					fnShipClass,		FN_SHIP_DATA_FIELD,
			"(shpGetDataField class field) -> data",
			"is",	0,	},

		{	"shpGetFuelNeeded",				fnShipSetOld,		FN_SHIP_FUEL_NEEDED,
			"(shpGetFuelNeeed ship item) -> items needed",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetFuelLeft",				fnShipGetOld,		FN_SHIP_FUEL,
			"(shpGetFuelLeft ship) -> fuel left",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetGlobalData",				fnShipClass,	FN_SHIP_GET_GLOBAL_DATA,
			"DEPRECATED: Use typGetGlobalData instead",
			"is",	0,	},

		{	"shpGetImageDesc",				fnShipClass,	FN_SHIP_GET_IMAGE_DESC,
			"(shpGetImageDesc class [rotationAngle]) -> imageDesc",
			"i*",	0,	},

		{	"shpGetItemCharges",			fnShipSetOld,		FN_SHIP_ITEM_CHARGES,
			"DEPRECATED: Use itmGetCharges instead",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetItemDeviceName",			fnShipSetOld,		FN_SHIP_ITEM_DEVICE_NAME,
			"(shpGetItemDeviceName ship item) -> device name of item (or -1)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetMaxPower",				fnObjGetOld,		FN_OBJ_MAX_POWER,
			"(objGetMaxPower obj) -> power (in 1/10 MWs)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetMaxSpeed",				fnShipGet,		FN_SHIP_MAX_SPEED,
			"(shpGetMaxSpeed ship) -> max speed in % of lightspeed",
			"i",	0,	},

		{	"shpGetOrder",					fnShipGetOld,		FN_SHIP_ORDER,
			"(shpGetOrder obj) -> order",
		//		0 = none
		//		1 = guard
		//		2 = dock
		//		3 = attack
		//		4 = wait
		//		5 = gate
		//		8 = patrol
		//		9 = escort
		//		15 = loot
		//		16 = hold
		//		17 = mine
		//		20 = follow
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetOrderTarget",			fnShipGetOld,		FN_SHIP_ORDER_TARGET,
			"(objGetOrderTarget obj) -> obj",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetOrderGiver",				fnObjGet,			FN_OBJ_ORDER_GIVER,
			"(objGetOrderGiver obj [destroyReason]) -> obj",
			"i*",	0,	},

		{	"shpGetShieldItemUNID",			fnShipGetOld,		FN_SHIP_SHIELD_ITEM_UNID,
			"(shpGetShieldItemUNID ship) -> UNID (or Nil)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetShieldDamage",			fnShipGetOld,		FN_SHIP_SHIELD_DAMAGE,
			"(shpGetShieldDamage ship) -> damage to shields",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpGetShieldMaxHitPoints",		fnShipGetOld,		FN_SHIP_SHIELD_MAX_HP,
			"(shpGetShieldMaxHitPoints ship) -> max hp of shields",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpInstallArmor",				fnShipSetOld,		FN_SHIP_INSTALL_ARMOR,
			"(shpInstallArmor ship item armorSegment)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpInstallAutopilot",			fnShipGetOld,		FN_SHIP_INSTALL_AUTOPILOT,
			"(shpInstallAutopilot ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpInstallTargetingComputer",	fnShipGetOld,		FN_SHIP_INSTALL_TARGETING,
			"(shpInstallTargetingComputer ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpInstallDevice",				fnShipSetOld,		FN_SHIP_INSTALL_DEVICE,
			"(shpInstallDevice ship item)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objIsAbandoned",				fnObjGetOld,		FN_OBJ_IS_ABANDONED,
			"(objIsAbandoned obj) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpIsAutopilotInstalled",		fnShipGetOld,		FN_SHIP_HAS_AUTOPILOT,
			"(shpIsAutopilotInstalled ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpIsBlind",					fnShipGetOld,		FN_SHIP_BLINDNESS,
			"(shpMakeBlind ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpIsFuelCompatible",			fnShipSetOld,		FN_SHIP_IS_FUEL_COMPATIBLE,
			"(shpIsFuelCompatible ship item) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS, },

		{	"shpIsRadiationImmune",			fnShipGet,		FN_SHIP_IS_RADIATION_IMMUNE,
			"(shpIsRadiationImmune ship [item])",
			"i*",	0,	},

		{	"shpIsRadioactive",				fnShipGetOld,		FN_SHIP_IS_RADIOACTIVE,
			"(shpIsRadioactive ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpIsTargetingComputerInstalled",		fnShipGetOld,		FN_SHIP_HAS_TARGETING,
			"(shpIsTargetingComputerInstalled ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpIsSRSEnhanced",				fnShipGetOld,		FN_SHIP_IS_SRS_ENHANCED,
			"(shpIsSRSEnhanced ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objIsDeviceSlotAvailable",		fnShipGetOld,		FN_SHIP_DEVICE_SLOT_AVAIL,
			"(objIsDeviceSlotAvailable ship) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objIsShip",					fnObjGetOld,		FN_OBJ_IS_SHIP,
			"(objIsShip obj) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objIsUnderAttack",				fnObjGetOld,		FN_OBJ_IS_UNDER_ATTACK,
			"(objIsUnderAttack obj) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpEnhanceItem",				fnShipSetOld,		FN_SHIP_ENHANCE_ITEM,
			"(shpEnhanceItem ship item [mods]) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpEnhanceSRS",				fnShipGetOld,		FN_SHIP_ENHANCE_SRS,
			"(shpEnhanceSRS ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpMakeBlind",					fnShipSetOld,		FN_SHIP_BLINDNESS,
			"(shpMakeBlind ship blindTime)",
		//		blindTime in ticks
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpMakeRadioactive",			fnShipGetOld,		FN_SHIP_MAKE_RADIOACTIVE,
			"(shpMakeRadioactive ship)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrder",						fnShipSet,		FN_SHIP_ORDER,
			"(shpOrder ship order [target] [count]) -> True/Nil",
			"is*",	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderAttack",				fnShipSetOld,		FN_SHIP_ORDER_ATTACK,
			"(shpOrderAttack ship target) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderDock",					fnShipSetOld,		FN_SHIP_ORDER_DOCK,
			"(shpOrderDock ship destination) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderEscort",				fnShipSetOld,		FN_SHIP_ORDER_ESCORT,
			"(shpOrderEscort ship protect [formation]) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderFollow",				fnShipSetOld,		FN_SHIP_ORDER_FOLLOW,
			"(shpOrderFollow ship follow) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderGate",					fnShipSetOld,		FN_SHIP_ORDER_GATE,
			"(shpOrderGate ship [gate]) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderGoto",				fnShipSet,		FN_SHIP_ORDER_GOTO,
			"(shpOrderGoto ship obj) -> True/Nil",
			"ii",	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderGuard",				fnShipSetOld,		FN_SHIP_ORDER_GUARD,
			"(shpOrderGuard ship base) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderHold",					fnShipSetOld,		FN_SHIP_ORDER_HOLD,
			"(shpOrderHold ship [waitTime]) -> True/Nil",
		//		wait time in seconds (real time)
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderLoot",					fnShipSetOld,		FN_SHIP_ORDER_LOOT,
			"(shpOrderLoot ship station) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderMine",					fnShipSetOld,		FN_SHIP_ORDER_MINE,
			"(shpOrderMine ship base) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderPatrol",				fnShipSetOld,		FN_SHIP_ORDER_PATROL,
			"(shpOrderPatrol ship center dist) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpOrderWait",					fnShipSetOld,		FN_SHIP_ORDER_WAIT,
			"(shpOrderWait ship waitTime)",
		//		wait time in seconds (real time)
		//			(0 = infinite)
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpRechargeItem",				fnShipSetOld,		FN_SHIP_RECHARGE_ITEM,
			"(shpRechargeItem ship item charges)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpRechargeShield",			fnShipSetOld,		FN_SHIP_RECHARGE_SHIELD,
			"(shpRechargeShield ship hpToRecharge)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpRefuelFromItem",			fnShipSetOld,		FN_SHIP_REFUEL_FROM_ITEM,
			"(shpRefuelFromItem ship item) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpRemoveDevice",				fnShipSetOld,		FN_SHIP_REMOVE_DEVICE,
			"(shpRemoveDevice ship item)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objRepairArmor",				fnObjGetArmor,	FN_OBJ_REPAIR_ARMOR,
			"(objRepairArmor ship armorSegment [hpToRepair])",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpRepairItem",				fnShipSet,		FN_SHIP_REPAIR_ITEM,
			"(shpRepairItem ship item)",
			"iv",	PPFLAG_SIDEEFFECTS,	},

		{	"shpSetCommandCode",			fnShipSet,		FN_SHIP_COMMAND_CODE,
			"(shpSetCommandCode ship code) -> True/Nil",
			"iu",	PPFLAG_SIDEEFFECTS,	},

		{	"shpSetController",				fnShipSetOld,		FN_SHIP_CONTROLLER,
			"(shpSetController ship controller) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"shpSetGlobalData",				fnShipClass,	FN_SHIP_SET_GLOBAL_DATA,
			"DEPRECATED: Use typSetGlobalData instead",
			"isv",	PPFLAG_SIDEEFFECTS,	},

		//	SpaceObject functions
		//	---------------------

		{	"objAddItem",					fnObjItem,		FN_OBJ_ADD_ITEM,
			"(objAddItem obj item [count])",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objAddRandomItems",			fnObjAddRandomItems,	0,
			"(objAddRandomItems obj table count)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objAddSubordinate",			fnObjSetOld,		FN_OBJ_ADD_SUBORDINATE,
			"(objAddSubordinate obj subordinate) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objCanAttack",					fnObjGetOld,		FN_OBJ_CAN_ATTACK,
			"(objCanAttack obj) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objCharge",					fnObjSet,		FN_OBJ_CHARGE,	
			"(objCharge obj amount) -> remaining balance",
			"ii",		PPFLAG_SIDEEFFECTS,	},

		{	"objClearIdentified",			fnObjSet,		FN_OBJ_CLEAR_IDENTIFIED,
			"(objClearIdentified obj)",
			"i",	PPFLAG_SIDEEFFECTS,	},

		{	"objCommunicate",				fnObjComms,		0,
			"(objCommunicate obj senderObj msg [obj] [data])",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objCredit",					fnObjSet,		FN_OBJ_CREDIT,	
			"(objCredit obj amount) -> new balance",
			"ii",		PPFLAG_SIDEEFFECTS,	},

		{	"objDepleteShields",			fnObjSet,		FN_OBJ_DEPLETE_SHIELDS,	
			"(objDepleteShields obj)",
			"i",		PPFLAG_SIDEEFFECTS,	},

		{	"objDestroy",					fnObjGetOld,		FN_OBJ_DESTROY,
			"(objDestroy obj) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objEnumItems",					fnObjEnumItems,	0,
			"(objEnumItems obj criteria itemVar exp)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objFireEvent",					fnObjSet,		FN_OBJ_FIRE_EVENT,
			"(objFireEvent obj event) -> result of event",
			"is",	PPFLAG_SIDEEFFECTS,	},

		{	"objFireItemEvent",				fnObjSet,		FN_OBJ_FIRE_ITEM_EVENT,
			"(objFireItemEvent obj item event) -> result of event",
			"ivs",	PPFLAG_SIDEEFFECTS,	},

		{	"objGateTo",					fnObjGateTo,	0,
			"(objGateTo obj node entrypoint [effectID])",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetBalance",				fnObjGet,		FN_OBJ_GET_BALANCE,	
			"(objGetBalance obj) -> balance",
			"i",		PPFLAG_SIDEEFFECTS,	},

		{	"objGetBuyPrice",				fnObjGet,		FN_OBJ_GET_BUY_PRICE,	
			"(objGetBuyPrice obj item) -> price",
			"il",		PPFLAG_SIDEEFFECTS,	},

		{	"objGetCargoSpaceLeft",			fnObjGetOld,		FN_OBJ_CARGO_SPACE_LEFT,
			"(objGetCargoSpaceLeft obj) -> space left in Kg",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetCombatPower",			fnObjGetOld,		FN_OBJ_COMBAT_POWER,
			"(objGetCombatPower obj) -> 0-100",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetDamageType",				fnObjGetOld,		FN_OBJ_DAMAGE_TYPE,
			"(objGetDamageType obj) -> damage type",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetData",					fnObjData,		FN_OBJ_GETDATA,
			"(objGetData obj attrib) -> data",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetDestiny",				fnObjGetOld,		FN_OBJ_DESTINY,
			"(objGetDestiny obj) -> 0-359",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetDistance",				fnObjGetOld,		FN_OBJ_DISTANCE,
			"(objGetDistance obj destObj) -> distance in light-seconds",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetGlobalData",				fnObjData,		FN_OBJ_GET_GLOBAL_DATA,
			"(objGetGlobalData obj attrib) -> data",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetID",						fnObjGet,		FN_OBJ_GET_ID,
			"(objGetID obj) -> objID",
			"i",	PPFLAG_SIDEEFFECTS,	},

		{	"objGetImageDesc",				fnObjGet,		FN_OBJ_IMAGE,
			"(objGetImageDesc obj) -> imageDesc",
			"i",	PPFLAG_SIDEEFFECTS,	},

		{	"objGetInstalledItemDesc",		fnObjGetOld,		FN_OBJ_INSTALLED_ITEM_DESC,
			"(objGetInstalledItemDesc obj item) -> 'installed as forward armor'",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetItems",					fnObjItem,		FN_OBJ_ENUM_ITEMS,
			"(objGetItems obj criteria) -> list of items",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetLevel",					fnObjGet,		FN_OBJ_LEVEL,
			"(objGetLevel obj) -> level",
			"i",	0,	},

		{	"objGetMass",					fnObjGet,		FN_OBJ_MASS,	
			"(objGetMass obj) -> mass in tons",
			"i",		0,	},

		{	"objGetName",					fnObjGetOld,		FN_OBJ_NAME,
			"(objGetName obj flags) -> Name of the object",
		//		flag 1 = capitalize
		//		flag 2 = pluralize
		//		flag 4 = prefix with 'the' or 'a'
		//		flag 8 = prefix with count
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetNearestStargate",		fnObjGetOld,		FN_OBJ_NEAREST_STARGATE,
			"(objGetNearestStargate obj) -> obj",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetObjByID",				fnObjIDGet,			FN_OBJ_OBJECT,
			"(objGetObjByID objID) -> obj",
			"i",	0,	},

		{	"objGetObjRefData",				fnObjData,		FN_OBJ_GET_OBJREF_DATA,
			"(objGetObjRefData obj attrib) -> obj",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetOpenDockingPortCount",	fnObjGet,		FN_OBJ_OPEN_DOCKING_PORT_COUNT,	
			"(objGetOpenDockingPortCount obj) -> count of open docking ports",
			"i",		0,	},

		{	"objGetPos",					fnObjGetOld,		FN_OBJ_POSITION,
			"(objGetPos obj) -> vector",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetSellPrice",				fnObjGet,		FN_OBJ_GET_SELL_PRICE,	
			"(objGetSellPrice obj item) -> price",
			"il",		PPFLAG_SIDEEFFECTS,	},

		{	"objGetShieldLevel",			fnObjGetOld,		FN_OBJ_SHIELD_LEVEL,
			"(objGetShieldLevel obj) -> 0-100% (or -1 for no shields)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetSovereign",				fnObjGetOld,		FN_OBJ_SOVEREIGN,
			"(objGetSovereign obj) -> sovereignID",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetStaticData",				fnObjData,		FN_OBJ_GET_STATIC_DATA,
			"(objGetStaticData obj attrib) -> data",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetStaticDataForStationType",fnObjData,		FN_OBJ_GET_STATIC_DATA_FOR_STATION_TYPE,
			"DEPRECATED: Use typGetStaticData instead.",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetTarget",					fnObjGetOld,		FN_OBJ_TARGET,
			"(objGetTarget obj) -> obj",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objGetVel",					fnObjGet,		FN_OBJ_VELOCITY,	
			"(objGetVel obj) -> velVector",
			"i",		0,	},

		{	"objGetVisibleDamage",			fnObjGetOld,		FN_OBJ_VISIBLE_DAMAGE,
			"(objGetVisibleDamage obj) -> damage %",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objHasAttribute",				fnObjGetOld,		FN_OBJ_ATTRIBUTE,
			"(objHasAttribute obj attrib) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objHasItem",					fnObjItem,		FN_OBJ_HAS_ITEM,
			"(objHasItem obj item [count]) -> number of items (or Nil)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objIncData",					fnObjData,		FN_OBJ_INCREMENT_DATA,
			"(objIncData obj attrib increment)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objIncVel",					fnObjSet,		FN_OBJ_INCREMENT_VELOCITY,	
			"(objIncVel obj velVector) -> velVector",
		//		velVector in % of light-speed
			"il",		PPFLAG_SIDEEFFECTS,	},

		{	"objIsDockedAt",				fnObjGet,		FN_OBJ_DOCKED_AT,
			"(objIsDockedAt obj stationObj) -> True/Nil",
			"ii",	0,	},

		{	"objIsEnemy",					fnObjGetOld,		FN_OBJ_ENEMY,
			"(objIsEnemy obj target) -> True/Nil",
			NULL,	0,	},

		{	"objIsIdentified",				fnObjGet,		FN_OBJ_IDENTIFIED,
			"(objIsIdentified obj) -> True/Nil",
			"i",	0,	},

		{	"objIsKnown",					fnObjGetOld,		FN_OBJ_KNOWN,
			"(objIsKnown obj)",
			NULL,	0,	},

		{	"objJumpTo",					fnObjSetOld,		FN_OBJ_JUMP,
			"(objJumpTo obj posVector)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objLowerShields",				fnObjGetOld,		FN_OBJ_LOWER_SHIELDS,
			"(objLowerShields obj)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objMakeParalyzed",				fnObjSetOld,		FN_OBJ_PARALYSIS,
			"(objMakeParalyzed obj ticks)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objMatches",					fnObjGet,			FN_OBJ_MATCHES,
			"(objMatches obj source filter) -> True/Nil",
			"iis",	0,	},

		{	"objMoveTo",					fnObjSetOld,		FN_OBJ_POSITION,
			"(objMoveTo obj posVector)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objProgramDamage",				fnProgramDamage,0,
			"(objProgramDamage obj hacker progName aiLevel code)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objRegisterForEvents",			fnObjSetOld,		FN_OBJ_REGISTER_EVENTS,
			"(objRegisterForEvents target obj)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objRemoveItem",				fnObjItem,		FN_OBJ_REMOVE_ITEM,
			"(objRemoveItem obj item [count])",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objSendMessage",				fnObjSetOld,		FN_OBJ_MESSAGE,
			"(objSendMessage obj sender msg)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objSetData",					fnObjData,		FN_OBJ_SETDATA,
			"(objSetData obj attrib data)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objSetGlobalData",				fnObjData,		FN_OBJ_SET_GLOBAL_DATA,
			"(objSetGlobalData obj attrib data)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objSetIdentified",				fnObjSet,		FN_OBJ_IDENTIFIED,
			"(objSetIdentified obj)",
			"i",	PPFLAG_SIDEEFFECTS,	},

		{	"objSetKnown",					fnObjGetOld,		FN_OBJ_SET_KNOWN,
			"(objSetKnown obj)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objSetName",					fnObjSetOld,		FN_OBJ_NAME,
			"(objSetName obj name)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objSetObjRefData",				fnObjData,		FN_OBJ_SET_OBJREF_DATA,
			"(objSetObjRefData obj attrib obj)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objSetSovereign",				fnObjSetOld,		FN_OBJ_SOVEREIGN,
			"(objSetSovereign obj sovereignID) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"objUnregisterForEvents",		fnObjSetOld,		FN_OBJ_UNREGISTER_EVENTS,
			"(objUnregisterForEvents target obj)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		//	Station functions
		//	-----------------

		{	"staClearFireReconEvent",		fnStationGetOld,	FN_STATION_CLEAR_FIRE_RECON,
			"(staClearFireReconEvent station)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staClearReconned",				fnStationGetOld,	FN_STATION_CLEAR_RECON,
			"(staClearReconned station)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staGetDockedShips",			fnStationGetOld,	FN_STATION_GET_DOCKED_SHIPS,
			"(staGetDockedShips station) -> list of docked ships",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staGetGlobalData",				fnStationType,	FN_STATION_GET_GLOBAL_DATA,	
			"DEPRECATED: Use typGetGlobalData instead",
			"is",	0,	},

		{	"staGetImageVariant",			fnStationGetOld,	FN_STATION_IMAGE_VARIANT,
			"(staGetImageVariant station) -> variant",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staGetMaxStructuralHP",		fnStationGet,		FN_STATION_MAX_STRUCTURAL_HP,
			"(staGetMaxStructuralHP station) -> hp",
			"i",	0,	},

		{	"staGetStructuralHP",			fnStationGetOld,	FN_STATION_STRUCTURAL_HP,
			"(staGetStructuralHP station) -> hp",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staGetSubordinates",			fnStationGetOld,	FN_STATION_GET_SUBORDINATES,
			"(staGetSubordinates station) -> list of subordinates (e.g., guardians)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staGetType",					fnStationGetOld,	FN_STATION_GET_TYPE,
			"(staGetType station) -> UNID",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staIsEncountered",				fnStationType,	FN_STATION_ENCOUNTERED,	
			"(staIsEncountered type) -> True/Nil",
			"i",	0,	},

		{	"staIsReconned",				fnStationGetOld,	FN_STATION_RECON,
			"(staIsReconned station) -> True/Nil",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staSetActive",					fnStationGetOld,	FN_STATION_SET_ACTIVE,
			"(staSetActive station)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staSetFireReconEvent",			fnStationGetOld,	FN_STATION_SET_FIRE_RECON,
			"(staSetFireReconEvent station)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staSetGlobalData",				fnStationType,	FN_STATION_SET_GLOBAL_DATA,	
			"DEPRECATED: Use typSetGlobalData instead",
			"isv",	PPFLAG_SIDEEFFECTS,	},

		{	"staSetImageVariant",			fnStationSet,	FN_STATION_IMAGE_VARIANT,
			"(staSetImageVariant station variant)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staSetInactive",				fnStationGetOld,	FN_STATION_SET_INACTIVE,
			"(staSetInactive station)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"staSetStructuralHP",			fnStationSet,	FN_STATION_STRUCTURAL_HP,
			"(staSetStructuralHP station hp)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		//	System functions
		//	----------------

		{	"sysAddEncounterEvent",			fnSystemAddEncounterEvent,	FN_ADD_ENCOUNTER_FROM_GATE,
			"(sysAddEncounterEvent delay target encounterID gate)",
		//		delay in ticks
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"sysAddEncounterEventAtDist",	fnSystemAddEncounterEvent,	FN_ADD_ENCOUNTER_FROM_DIST,
			"(sysAddEncounterEventAtDist delay target encounterID distance)",
		//		delay in ticks
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"sysAddObjTimerEvent",			fnSystemAddStationTimerEvent,	FN_ADD_TIMER_NORMAL,	
			"(sysAddObjTimerEvent delay obj event)",		"iis",	PPFLAG_SIDEEFFECTS,	},
		//		delay in ticks

		{	"sysAddObjRecurringTimerEvent",	fnSystemAddStationTimerEvent,	FN_ADD_TIMER_RECURRING,	
			"(sysAddObjRecurringTimerEvent interval obj event)",
		//		interval in ticks
			"iis",	PPFLAG_SIDEEFFECTS,	},

		{	"sysAddStargateTopology",			fnSystemGet,	FN_SYS_ADD_STARGATE_TOPOLOGY,
			"(sysAddStargateTopology [nodeID] gateID destNodeID destGateID) -> True/Nil",
			"sss*",	PPFLAG_SIDEEFFECTS,	},

		{	"sysCalcTravelTime",				fnSystemGet,	FN_SYS_GET_TRAVEL_TIME,
			"(sysCalcTravelTime distance speed) -> time in ticks",
			"ii",	0,	},

		{	"sysCancelTimerEvent",				fnSystemAddStationTimerEvent,	FN_CANCEL_TIMER,	
			"(sysCancelTimerEvent obj event) -> True/Nil",
			"is",	0,	},

		{	"sysCreateEffect",				fnSystemCreateEffect,	0,
			"(sysCreateEffect effectID anchor posVector)",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"sysCreateMarker",				fnSystemCreateMarker,	0,
			"(sysCreateMarker name pos sovereignID) -> marker",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"sysCreateShip",				fnSystemCreateShip,	0,
			"(sysCreateShip classID pos sovereignID [controller]) -> ship",
		//		pos is either a position vector or a gate object
		//		controller 
		//			""					= standard
		//			"auton"				= auton
		//			"ferian"			= Ferian
		//			"fleet"				= fleet member
		//			"fleetcommand"		= fleet squad leader
		//			"gaianprocessor"	= Gaian processor
		//			"gladiator"			= Maximus Battle Arena contestant
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"sysCreateStargate",			fnSystemGet,	FN_SYS_CREATE_STARGATE,
			"(sysCreateStargate classID posVector gateID destNodeID destGateID) -> stargate",
			"ivsss",	PPFLAG_SIDEEFFECTS,	},

		{	"sysCreateStation",				fnSystemCreateStation,	0,
			"(sysCreateStation classID posVector) -> station",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"sysCreateWeaponFire",			fnSystemCreate,			FN_SYS_CREATE_WEAPON_FIRE,
			"(sysCreateWeaponFire weaponID objSource posVector dir speed objTarget [detonateNow]) -> obj",
			"viliii*",	PPFLAG_SIDEEFFECTS,	},

		{	"sysFindObject",				fnSystemFind,	0,	
			"(sysFindObject source filter) -> list of objects",
		//		G			Stargates only
		//		G:xyz;		Stargate with ID 'xyz'
		//		s			Include ships
		//		t			Include stations (including planets)
		//		T			Include structure-scale stations
		//		T:xyz;		Include structure-scale stations with attribute 'xyz'
		//
		//		A			Active objects only (i.e., objects that can attack)
		//		B:xyz;		Only objects with attribute 'xyz'
		//		D:xyz;		Only objects with data 'xyz'
		//		E			Enemy objects only
		//		F			Friendly objects only
		//		H			Only objects whose home base is the source
		//		M			Manufactured objects only (i.e., no planets or asteroids)
		//		N			Return only the nearest object to the source
		//		N:nn;		Return only objects within nn light-seconds
		//		O:docked;	Ships that are currently docked at source
		//		O:escort;	Ships ordered to escort source
		//		R			Return only the farthes object to the source
		//		R:nn;		Return only objects greater than nn light-seconds away
			"is",	0,	},

		{	"sysGetData",					fnSystemGet,	FN_SYS_GET_DATA,
			"(sysGetData [nodeID] attrib) -> data",
			"s*",	0,	},

		{	"sysGetEnvironment",			fnSystemGet,	FN_SYS_ENVIRONMENT,
			"(sysGetEnvironment vector) -> environmentUNID",
			"v",	PPFLAG_SIDEEFFECTS,	},

		{	"sysGetLevel",					fnSystemGet,	FN_SYS_LEVEL,
			"(sysGetLevel [nodeID]) -> level",
			"*",	0,	},

		{	"sysGetName",					fnSystemGet,	FN_SYS_NAME,
			"(sysGetName [nodeID]) -> name",
			"*",	0,	},

		{	"sysGetNavPathPoint",			fnSystemGet,	FN_SYS_NAV_PATH_POINT,
			"(sysGetNavPathPoint sovereignID objFrom objTo %path) -> vector",
			"iiii",	0,	},

		{	"sysGetNode",					fnSystemGet,	FN_SYS_NODE,
			"(sysGetNode) -> nodeID",
			NULL,	0,	},

		{	"sysGetNodes",					fnSystemGet,	FN_SYS_ALL_NODES,
			"(sysGetNodes) -> list of nodeIDs",
			NULL,	0,	},

		{	"sysGetObjectByName",			fnSystemGetObjectByName,	0,
			"(sysGetObjectByName source name) -> obj",
			NULL,	0,	},

		{	"sysGetStargateDestinationNode",	fnSystemGet,	FN_SYS_STARGATE_DESTINATION_NODE,
			"(sysGetStargateDestinationNode [nodeID] gateID) -> nodeID",
			"s*",	0,	},

		{	"sysGetStargates",				fnSystemGet,	FN_SYS_STARGATES,
			"(sysGetStargates [nodeID]) -> list of gateIDs",
			"*",	0,	},

		{	"sysGetSystemType",				fnSystemGet,	FN_SYS_SYSTEM_TYPE,
			"(sysGetSystemType [nodeID]) -> systemUNID",
			"*",	0,	},

		{	"sysHasAttribute",				fnSystemGet,	FN_SYS_HAS_ATTRIBUTE,
			"(sysHasAttribute [nodeID] attrib) -> True/Nil",
			"s*",	0,	},

		{	"sysSetData",					fnSystemGet,	FN_SYS_SET_DATA,
			"(sysSetData [nodeID] attrib data) -> data",
			"sv*",	PPFLAG_SIDEEFFECTS,	},

		{	"sysStopTime",					fnSystemStopTime,	0,
			"(sysStopTime duration except)",
			"ii",	PPFLAG_SIDEEFFECTS,	},

		{	"sysVectorPolarOffset",			fnSystemVectorOffset,	0,
			"(sysVectorPolarOffset center angle radius) -> vector",
		//			center is either Nil, an object, or a vector
		//			radius in light-seconds
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"sysVectorAdd",					fnSystemVectorMath,		FN_VECTOR_ADD,	
			"(sysVectorAdd vector vector) -> vector",
			"ll",	0,	},

		{	"sysVectorAngle",				fnSystemVectorMath,		FN_VECTOR_ANGLE,	
			"(sysVectorAngle vector) -> angle of vector",
			"l",	0,	},

		{	"sysVectorDistance",			fnSystemVectorMath,		FN_VECTOR_DISTANCE,
			"(sysVectorDistance vector [vector]) -> distance in light-seconds",
			"l*",	0,	},

		{	"sysVectorDivide",				fnSystemVectorMath,		FN_VECTOR_DIVIDE,
			"(sysVectorDivide vector scalar) -> vector",
			"li",	0,	},

		{	"sysVectorMultiply",			fnSystemVectorMath,		FN_VECTOR_MULTIPLY,
			"(sysVectorMultiply vector scalar) -> vector",
			"li",	0,	},

		{	"sysVectorRandom",				fnSystemVectorMath,		FN_VECTOR_RANDOM,
			"(sysVectorRandom center radius minSeparation [filter]) -> vector",
		//			center is either Nil, an object, or a vector
		//			radius in light-seconds from center
		//			minSeparation is the min distance from other objects (in light-seconds)
		//			filter defines the set of objects to be away from
			"vi*",	0,	},

		{	"sysVectorSpeed",			fnSystemVectorMath,		FN_VECTOR_SPEED,
			"(sysVectorSpeed vector) -> % of light speed",
			"l",	0,	},

		{	"sysVectorSubtract",			fnSystemVectorMath,		FN_VECTOR_SUBTRACT,
			"(sysVectorSubtract vector vector) -> vector",
			"ll",	0,	},

		//	Design Type functions
		//	---------------------

		{	"typGetGlobalData",				fnDesignGet,		FN_DESIGN_GET_GLOBAL_DATA,
			"(typGetGlobalData unid attrib) -> data",
			"is",	0,	},

		{	"typIncGlobalData",				fnDesignGet,		FN_DESIGN_INC_GLOBAL_DATA,
			"(typIncGlobalData unid attrib [increment]) -> new value",
			"is*",	0,	},

		{	"typSetGlobalData",				fnDesignGet,		FN_DESIGN_SET_GLOBAL_DATA,
			"(typSetGlobalData unid attrib data) -> True/Nil",
			"isv",	0,	},

		{	"typGetStaticData",				fnDesignGet,		FN_DESIGN_GET_STATIC_DATA,
			"(typGetStaticData unid attrib) -> data",
			"is",	0,	},

		//	Environment functions
		//	---------------------

		{	"envHasAttribute",				fnEnvironmentGet,		FN_ENV_HAS_ATTRIBUTE,
			"(envHasAttribute environmentUNID attrib) -> True/Nil",
			"is",	0,	},

		//	Sovereign functions
		//	----------------

		{	"sovSetDisposition",			fnSovereignSet,			FN_SOVEREIGN_DISPOSITION,
			"(sovSetDisposition sovereignID targetSovereignID disposition)",
		//			0 = enemy
		//			1 = neutral
		//			2 = friend
			"iiv",	PPFLAG_SIDEEFFECTS,	},

		//	Universe functions
		//	----------------

		{	"unvGetTick",					fnUniverseGet,	FN_UNIVERSE_TICK,
			"(unvGetTick) -> time",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"unvUNID",						fnUniverseGet,	FN_UNIVERSE_UNID,
			"(unvUNID string) -> ('itemtype unid name) or ('shipclass unid name)",
			"s",	0,	},
	};

#define EXTENSIONS_COUNT		(sizeof(g_Extensions) / sizeof(g_Extensions[0]))

CItem GetItemArg (CCodeChain &CC, ICCItem *pArg);
inline CSpaceObject *GetObjArg (ICCItem *pArg) { return (CSpaceObject *)pArg->GetIntegerValue(); }
inline CShip *GetShipArg (ICCItem *pArg) 
	{
	CSpaceObject *pObj;
	if (pObj = (CSpaceObject *)pArg->GetIntegerValue())
		return pObj->AsShip();
	else
		return NULL;
	}
inline CStation *GetStationArg (ICCItem *pArg)
	{
	CSpaceObject *pObj;
	if (pObj = (CSpaceObject *)pArg->GetIntegerValue())
		return pObj->AsStation();
	else
		return NULL;
	}
inline CArmorClass *GetArmorClassArg (ICCItem *pArg) { return (CArmorClass *)pArg->GetIntegerValue(); }
CWeaponFireDesc *GetWeaponFireDescArg (ICCItem *pArg);

ALERROR CUniverse::InitCodeChainPrimitives (void)

//	InitCodeChainPrimitives
//
//	Registers extensions

	{
	ALERROR error;
	int i;

	//	Define some global variables (we do this because otherwise when we fire
	//	events, we may get undefined variable errors when trying to save
	//	old versions of the globals)

	m_CC.DefineGlobal(CONSTLIT("gPlayer"), m_CC.CreateNil());
	m_CC.DefineGlobal(CONSTLIT("gPlayerShip"), m_CC.CreateNil());
	m_CC.DefineGlobal(CONSTLIT("gSource"), m_CC.CreateNil());
	m_CC.DefineGlobal(CONSTLIT("gItem"), m_CC.CreateNil());

	//	Register primitives

	for (i = 0; i < EXTENSIONS_COUNT; i++)
		if (error = m_CC.RegisterPrimitive(&g_Extensions[i]))
			return error;

	return NOERROR;
	}

void CreateBinaryFromList (CCodeChain &CC, ICCItem *pList, void *pvDest)

//	CreateBinaryFromList
//
//	Initializes binary structure from a list

	{
	DWORD *pDest = (DWORD *)pvDest;
	for (int i = 0; i < pList->GetCount(); i++)
		*pDest++ = (DWORD)pList->GetElement(i)->GetIntegerValue();
	}

CItem CreateItemFromList (CCodeChain &CC, ICCItem *pList)

//	CreateItemFromList
//
//	Creates an item from a code chain list

	{
	CItem NewItem;
	NewItem.ReadFromCCItem(CC, pList);
	return NewItem;
	}

ICCItem *CreateListFromBinary (CCodeChain &CC, void const *pvSource, int iLengthBytes)

//	CreateListFromBinary
//
//	Creates a code chain list from a block of memory

	{
	ICCItem *pResult = CC.CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	CItem is two DWORD long

	DWORD *pSource = (DWORD *)pvSource;
	int iCount = AlignUp(iLengthBytes, sizeof(DWORD)) / sizeof(DWORD);

	for (int i = 0; i < iCount; i++)
		{
		ICCItem *pInt = CC.CreateInteger(*pSource++);
		pList->Append(&CC, pInt, NULL);
		pInt->Discard(&CC);
		}

	return pResult;
	}

ICCItem *CreateListFromImage (CCodeChain &CC, const CObjectImageArray &Image, int iRotation)

//	CreateListFromImage
//
//	Creates an imageDesc from an image

	{
	ICCItem *pResult = CC.CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Add the bitmap UNID

	ICCItem *pValue = CC.CreateInteger(Image.GetBitmapUNID());
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Get the rect

	RECT rcRect = Image.GetImageRect(0, iRotation);

	//	Add the x coordinate

	pValue = CC.CreateInteger(rcRect.left);
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Add the y coordinate

	pValue = CC.CreateInteger(rcRect.top);
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Add width

	pValue = CC.CreateInteger(RectWidth(rcRect));
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Add height

	pValue = CC.CreateInteger(RectHeight(rcRect));
	pList->Append(&CC, pValue, NULL);
	pValue->Discard(&CC);

	//	Done

	return pResult;
	}

ICCItem *CreateListFromItem (CCodeChain &CC, const CItem &Item)

//	CreateListFromItem
//
//	Creates a code chain list from an item

	{
	return Item.WriteToCCItem(CC);
	}

ICCItem *CreateListFromVector (CCodeChain &CC, const CVector &vVector)

//	CreateListFromVector
//
//	Creates a code chain list from a vector

	{
	return CreateListFromBinary(CC, &vVector, sizeof(vVector));
	}

CVector CreateVectorFromList (CCodeChain &CC, ICCItem *pList)

//	CreateVectorFromList
//
//	Creates a vector from a code chain list

	{
	CVector vVec;

	if (pList->IsList())
		CreateBinaryFromList(CC, pList, &vVec);
	else if (pList->IsInteger())
		{
		CSpaceObject *pObj = GetObjArg(pList);
		if (pObj)
			vVec = pObj->GetPos();
		}

	return vVec;
	}

void GetImageDescFromList (CCodeChain &CC, ICCItem *pList, CG16bitImage **retpBitmap, RECT *retrcRect)

//	GetImageDescFromList
//
//	Returns an image

	{
	*retpBitmap = NULL;

	if (pList->GetCount() < 5)
		return;

	*retpBitmap = g_pUniverse->GetLibraryBitmap(pList->GetElement(0)->GetIntegerValue());
	if (*retpBitmap == NULL)
		return;

	retrcRect->left = pList->GetElement(1)->GetIntegerValue();
	retrcRect->top = pList->GetElement(2)->GetIntegerValue();
	retrcRect->right = retrcRect->left + pList->GetElement(3)->GetIntegerValue();
	retrcRect->bottom = retrcRect->top + pList->GetElement(4)->GetIntegerValue();
	}

CItem GetItemArg (CCodeChain &CC, ICCItem *pArg)

//	GetItemArg
//
//	Arg can be an item list cursor or an item

	{
	if (pArg->IsList())
		return CreateItemFromList(CC, pArg);
	else if (pArg->IsInteger())
		{
		CItemListManipulator *pItemList = (CItemListManipulator *)pArg->GetIntegerValue();
		return pItemList->GetItemAtCursor();
		}
	else
		return CItem();
	}

CWeaponFireDesc *GetWeaponFireDescArg (ICCItem *pArg)

//	GetWeaponFireDescArg
//
//	If arg is a weapon UNID, then we return the first weapon desc
//	If arg is a missile, then we return the first weapon desc we find for the missil
//	If arg is a list, then the first is a weapon UNID and the second is a variant
//	Returns NULL on error

	{
	int i;
	DWORD dwWeaponUNID;
	int iWeaponVariant;

	//	If the argument is a list, then we get the weapon UNID and the variant
	//	from the list.

	if (pArg->IsList())
		{
		dwWeaponUNID = (DWORD)pArg->GetElement(0)->GetIntegerValue();
		iWeaponVariant = Max(0, pArg->GetElement(1)->GetIntegerValue());
		}

	//	Otherwise, get the first variant of the weapon

	else
		{
		dwWeaponUNID = (DWORD)pArg->GetIntegerValue();
		iWeaponVariant = 0;
		}

	//	Get the item associated with the UNID

	CItemType *pType = g_pUniverse->FindItemType(dwWeaponUNID);
	if (pType == NULL)
		return NULL;

	//	If this is a weapon, then return the weapon fire desc

	if (pType->GetCategory() == itemcatWeapon || pType->GetCategory() == itemcatLauncher)
		{
		CDeviceClass *pClass = pType->GetDeviceClass();
		if (pClass == NULL)
			return NULL;

		CWeaponClass *pWeapon = dynamic_cast<CWeaponClass *>(pClass);
		if (pWeapon == NULL)
			return NULL;

		return pWeapon->GetVariant(Min(iWeaponVariant, pWeapon->GetVariantCount() - 1));
		}

	//	Otherwise, if this is a missile, then find the appropriate weapon

	else if (pType->GetCategory() == itemcatMissile)
		{
		for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
			{
			CItemType *pWeaponType = g_pUniverse->GetItemType(i);
			CDeviceClass *pClass;
			if (pClass = pWeaponType->GetDeviceClass())
				{
				if ((iWeaponVariant = pClass->GetAmmoVariant(pType)) != -1)
					{
					CWeaponClass *pWeapon = dynamic_cast<CWeaponClass *>(pClass);
					if (pWeapon)
						return pWeapon->GetVariant(iWeaponVariant);
					}
				}
			}

		return NULL;
		}

	//	Otheriwe, nothing

	else
		return NULL;
	}

void DefineGlobalSpaceObject (CCodeChain &CC, const CString &sVar, CSpaceObject *pObj)

//	DefineGlobalSpaceObject
//
//	Defines a global variable and assigns it the given space object. If pObj
//	is NULL then the variable is Nil.

	{
	if (pObj)
		CC.DefineGlobalInteger(sVar, (int)pObj);
	else
		{
		ICCItem *pValue = CC.CreateNil();
		CC.DefineGlobal(sVar, pValue);
		pValue->Discard(&CC);
		}
	}

void DefineGlobalVector (CCodeChain &CC, const CString &sVar, const CVector &vVector)

//	DefineGlobalVector
//
//	Defines a global variable and assigns it the given vector

	{
	ICCItem *pValue = CreateListFromVector(CC, vVector);
	CC.DefineGlobal(sVar, pValue);
	pValue->Discard(&CC);
	}

ICCItem *fnArmGet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnArmGet
//
//	Gets data about an armor type
//
//	(armGetName type) -> Name of the armor
//	(armGetHitPoints type) -> Hit points of armor
//	(armGetRepairCost type) -> Cost to repair 1 hit point

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into an armor type

	CArmorClass *pArmor = GetArmorClassArg(pArgs->GetElement(0));
	if (pArmor == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(CONSTLIT("Invalid armor class:"), pArgs->GetElement(0));
		}

	//	No longer needed

	pArgs->Discard(pCC);

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_ARM_NAME:
			pResult = pCC->CreateString(pArmor->GetName());
			break;

		case FN_ARM_HITPOINTS:
			pResult = pCC->CreateInteger(pArmor->GetHitPoints(CItemEnhancement()));
			break;

		case FN_ARM_REPAIRCOST:
			pResult = pCC->CreateInteger(pArmor->GetRepairCost());
			break;

		case FN_ARM_REPAIRTECH:
			pResult = pCC->CreateInteger(pArmor->GetRepairTech());
			break;

		case FN_ARM_IS_RADIATION_IMMUNE:
			pResult = pCC->CreateBool(pArmor->IsRadiationImmune(NULL));
			break;

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnDesignGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnDesignGet
//
//	Returns design data

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	The first argument is an UNID

	CDesignType *pType = g_pUniverse->FindDesignType(pArgs->GetElement(0)->GetIntegerValue());
	if (pType == NULL)
		return pCC->CreateNil();

	//	Implement

	switch (dwData)
		{
		case FN_DESIGN_GET_GLOBAL_DATA:
			{
			CString sData = pType->GetGlobalData(pArgs->GetElement(1)->GetStringValue());
			return pCC->Link(sData, 0, NULL);
			}

		case FN_DESIGN_GET_STATIC_DATA:
			{
			CString sData = pType->GetStaticData(pArgs->GetElement(1)->GetStringValue());
			return pCC->Link(sData, 0, NULL);
			}

		case FN_DESIGN_INC_GLOBAL_DATA:
			{
			//	Get parameters

			CString sAttrib = pArgs->GetElement(1)->GetStringValue();
			int iInc = (pArgs->GetCount() > 2 ? pArgs->GetElement(2)->GetIntegerValue() : 1);

			//	Get the current value

			ICCItem *pValue = pCC->Link(pType->GetGlobalData(sAttrib), 0, NULL);
			if (pValue->IsError())
				return pValue;

			//	Compose the new value

			ICCItem *pNewValue = pCC->CreateInteger(pValue->GetIntegerValue() + iInc);
			pValue->Discard(pCC);

			//	Save back

			pType->SetGlobalData(sAttrib, pCC->Unlink(pNewValue));

			//	Done

			return pNewValue;
			}

		case FN_DESIGN_SET_GLOBAL_DATA:
			{
			CString sAttrib = pArgs->GetElement(1)->GetStringValue();
			if (sAttrib.IsBlank())
				return pCC->CreateNil();

			//	Set quoted before we unlink

			BOOL bOldQuoted = pArgs->GetElement(2)->IsQuoted();
			pArgs->GetElement(2)->SetQuoted();
			CString sData = pCC->Unlink(pArgs->GetElement(2));
			if (!bOldQuoted)
				pArgs->GetElement(2)->ClearQuoted();

			pType->SetGlobalData(sAttrib, sData);
			return pCC->CreateTrue();
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnEnvironmentGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnEnvironmentGet
//
//	Environment functions

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	The first argument is an environment UNID

	CSpaceEnvironmentType *pEnv = g_pUniverse->FindSpaceEnvironment(pArgs->GetElement(0)->GetIntegerValue());
	if (pEnv == NULL)
		return pCC->CreateNil();

	//	Implement

	switch (dwData)
		{
		case FN_ENV_HAS_ATTRIBUTE:
			return pCC->CreateBool(pEnv->HasAttribute(pArgs->GetElement(1)->GetStringValue()));

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnItemCreate (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	ItemCreate
//
//	Creates an item struct
//
//	(itmCreate type count) -> item struct

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	CItemType *pType = g_pUniverse->FindItemType(pArgs->GetElement(0)->GetIntegerValue());
	if (pType == NULL)
		return pCC->CreateError(CONSTLIT("Unknown item type"), pArgs->GetElement(0));

	CItem Item(pType, pArgs->GetElement(1)->GetIntegerValue());

	return CreateListFromItem(*pCC, Item);
	}

ICCItem *fnItemCreateByName (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	ItemCreateByName
//
//	Creates a random item struct
//
//	(itmCreateByName criteria name [count]) -> item

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	CString sCriteria = pArgs->GetElement(0)->GetStringValue();
	CString sName = pArgs->GetElement(1)->GetStringValue();
	int iCount = -1;
	if (pArgs->GetCount() > 2)
		iCount = pArgs->GetElement(2)->GetIntegerValue();

	//	Parse criteria

	CItemCriteria Criteria;
	CItem::ParseCriteria(sCriteria, &Criteria);

	//	Create the item

	CItem Item = CItem::CreateItemByName(sName, Criteria, true);
	if (iCount > 0)
		Item.SetCount(iCount);

	if (Item.GetType() == NULL)
		return pCC->CreateNil();
	else
		return CreateListFromItem(*pCC, Item);
	}

ICCItem *fnItemCreateRandom (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	ItemCreateRandom
//
//	Creates a random item struct
//
//	(itmCreateRandom criteria levelDistribution) -> item struct

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	CString sCriteria = pArgs->GetElement(0)->GetStringValue();
	CString sLevelFrequency = pArgs->GetElement(1)->GetStringValue();

	//	Parse criteria

	CItemCriteria Criteria;
	CItem::ParseCriteria(sCriteria, &Criteria);

	//	Create the item

	CItem Item;
	if (g_pUniverse->CreateRandomItem(Criteria, sLevelFrequency, &Item) != NOERROR)
		return pCC->CreateNil();

	return CreateListFromItem(*pCC, Item);
	}

ICCItem *fnItemGetTypes (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnItemGetTypes
//
//	Returns a list of items types that match criteria

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Get the criteria

	CItemCriteria Criteria;
	CItem::ParseCriteria(pArgs->GetElement(0)->GetStringValue(), &Criteria);

	//	Create a linked list for the result

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Loop over the items

	for (int i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		CItem Item(pType, 1);

		if (Item.MatchesCriteria(Criteria))
			{
			ICCItem *pItem = pCC->CreateInteger(pType->GetUNID());
			pList->Append(pCC, pItem, NULL);
			pItem->Discard(pCC);
			}
		}

	if (pList->GetCount() == 0)
		{
		pList->Discard(pCC);
		return pCC->CreateNil();
		}
	else
		return pResult;
	}

ICCItem *fnItemGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnItemGet
//
//	Gets and sets items
//
//	(itmGetCount item)

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pResult;

	//	Convert the first argument into an item

	CItem Item = GetItemArg(*pCC, pArgs->GetElement(0));
	CItemType *pType = Item.GetType();
	if (pType == NULL)
		return pCC->CreateNil();

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_ITEM_ACTUAL_PRICE:
			pResult = pCC->CreateInteger(Item.GetValue(true));
			break;

		case FN_ITEM_ARMOR_INSTALLED_LOCATION:
			pResult = pCC->CreateInteger(Item.GetInstalled());
			break;

		case FN_ITEM_ARMOR_TYPE:
			{
			CArmorClass *pArmor = pType->GetArmorClass();
			if (pArmor)
				pResult = pCC->CreateInteger((int)pArmor);
			else
				pResult = pCC->CreateError(CONSTLIT("Item is not an armor segment:"), pArgs->GetElement(0));

			break;
			}

		case FN_ITEM_CHARGES:
			pResult = pCC->CreateInteger(Item.GetCharges());
			break;

		case FN_ITEM_COUNT:
			pResult = pCC->CreateInteger(Item.GetCount());
			break;

		case FN_ITEM_DAMAGE_TYPE:
			{
			int iDamageType;
			CDeviceClass *pClass = pType->GetDeviceClass();
			if (pClass)
				iDamageType = pClass->GetDamageType(NULL);
			else
				iDamageType = -1;

			if (iDamageType != -1)
				pResult = pCC->CreateInteger(iDamageType);
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_ITEM_DATA:
			{
			CString sAttrib = pArgs->GetElement(1)->GetStringValue();
			pResult = pCC->Link(Item.GetData(sAttrib), 0, NULL);
			break;
			}

		case FN_ITEM_IMAGE_DESC:
			pResult = CreateListFromImage(*pCC, pType->GetImage());
			break;

		case FN_ITEM_INSTALL_COST:
			{
			int iCost = pType->GetInstallCost();
			if (iCost == -1)
				pResult = pCC->CreateNil();
			else
				pResult = pCC->CreateInteger(iCost);
			break;
			}

		case FN_ITEM_GET_STATIC_DATA:
			{
			CString sData = pType->GetStaticData(pArgs->GetElement(1)->GetStringValue());
			pResult = pCC->Link(sData, 0, NULL);
			break;
			}

		case FN_ITEM_GET_GLOBAL_DATA:
			{
			CString sData = pType->GetGlobalData(pArgs->GetElement(1)->GetStringValue());
			pResult = pCC->Link(sData, 0, NULL);
			break;
			}

		case FN_ITEM_SET_GLOBAL_DATA:
			{
			CString sAttrib = pArgs->GetElement(1)->GetStringValue();
			if (sAttrib.IsBlank())
				{
				pResult = pCC->CreateNil();
				break;
				}

			//	Set quoted before we unlink

			BOOL bOldQuoted = pArgs->GetElement(2)->IsQuoted();
			pArgs->GetElement(2)->SetQuoted();
			CString sData = pCC->Unlink(pArgs->GetElement(2));
			if (!bOldQuoted)
				pArgs->GetElement(2)->ClearQuoted();

			pType->SetGlobalData(sAttrib, sData);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_ITEM_LEVEL:
			pResult = pCC->CreateInteger(pType->GetLevel());
			break;

		case FN_ITEM_PRICE:
			pResult = pCC->CreateInteger(Item.GetValue());
			break;

		case FN_ITEM_FREQUENCY:
			pResult = pCC->CreateInteger(pType->GetFrequency());
			break;

		case FN_ITEM_DAMAGED:
			pResult = pCC->CreateBool(Item.IsDamaged());
			break;

		case FN_ITEM_ENHANCED:
			if (Item.GetMods())
				pResult = pCC->CreateInteger(Item.GetMods());
			else if (Item.IsEnhanced())
				pResult = pCC->CreateTrue();
			else
				pResult = pCC->CreateNil();
			break;

		case FN_ITEM_INSTALLED:
			pResult = pCC->CreateBool(Item.IsInstalled());
			break;

		case FN_ITEM_KNOWN:
			pResult = pCC->CreateBool(pType->IsKnown());
			break;

		case FN_ITEM_REFERENCE:
			pResult = pCC->CreateBool(pType->ShowReference());
			break;

		case FN_ITEM_UNID:
			pResult = pCC->CreateInteger(pType->GetUNID());
			break;

		case FN_ITEM_MASS:
			pResult = pCC->CreateInteger(pType->GetMassKg());
			break;

		case FN_ITEM_MAX_APPEARING:
			pResult = pCC->CreateInteger(pType->GetNumberAppearing().GetMaxValue());
			break;

		case FN_ITEM_AVERAGE_APPEARING:
			pResult = pCC->CreateInteger(pType->GetNumberAppearing().GetAveValue());
			break;

		case FN_ITEM_NAME:
			{
			DWORD dwFlags = pArgs->GetElement(1)->GetIntegerValue();
			pResult = pCC->CreateString(Item.GetNounPhrase(dwFlags));
			break;
			}

		case FN_ITEM_CATEGORY:
			pResult = pCC->CreateInteger(pType->GetCategory());
			break;

		case FN_ITEM_USE_SCREEN:
			{
			CXMLElement *pScreen = pType->GetUseScreen();
			if (pScreen == NULL)
				pResult = pCC->CreateNil();
			else
				{
				//	We want the UNID, not the screen itself

				pResult = pCC->CreateString(pScreen->GetAttribute(CONSTLIT("UNID")));
				}
			break;
			}

		case FN_ITEM_TYPE_DATA:
			pResult = pCC->Link(pType->GetData(), 0, NULL);
			break;

		case FN_ITEM_SET_KNOWN:
			pType->SetKnown();
			pResult = pCC->CreateTrue();
			break;

		case FN_ITEM_SET_REFERENCE:
			pType->SetShowReference();
			pResult = pCC->CreateTrue();
			break;

		case FN_ITEM_MATCHES:
			{
			CString sCriteria = pArgs->GetElement(1)->GetStringValue();
			CItemCriteria Criteria;
			CItem::ParseCriteria(sCriteria, &Criteria);
			pResult = pCC->CreateBool(Item.MatchesCriteria(Criteria));
			break;
			}

		case FN_ITEM_HAS_MODIFIER:
			{
#ifdef DEBUG_HASMODIFIER
			::OutputDebugString("(hasModifier)\n");
#endif
			CString sCriteria = pArgs->GetElement(1)->GetStringValue();
			pResult = pCC->CreateBool(pType->HasModifier(sCriteria));
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnItemList (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnItemList
//
//	(itmAtCursor itemListCursor) -> item

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	return pArgs->GetElement(0)->Reference();
	}

ICCItem *fnItemSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnItemSet
//
//	Sets data to an item

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	CItemList Temp;
	CItemListManipulator TempList(Temp);

	//	If the first arg is an int, then we expect a list manipulator pointer

	CItemListManipulator *pItemList = NULL;
	if (pArgs->GetElement(0)->IsInteger())
		{
		pItemList = (CItemListManipulator *)pArgs->GetElement(0)->GetIntegerValue();
		if (pItemList == NULL)
			return pCC->CreateNil();

		if (!pItemList->IsCursorValid())
			return pCC->CreateNil();
		}

	//	Otherwise, we fake a manipulator

	else if (pArgs->GetElement(0)->IsList())
		{
		CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(0)));
		if (Item.GetType())
			{
			TempList.AddItem(Item);
			pItemList = &TempList;
			}
		else
			return pCC->CreateNil();
		}
	else
		return pCC->CreateNil();

	//	Set the values

	switch (dwData)
		{
		case FN_ITEM_CHARGES:
			pItemList->SetChargesAtCursor((DWORD)pArgs->GetElement(1)->GetIntegerValue());
			return CreateListFromItem(*pCC, pItemList->GetItemAtCursor());

		case FN_ITEM_COUNT:
			{
			int iCount = pArgs->GetElement(1)->GetIntegerValue();
			if (iCount >= 0)
				{
				pItemList->SetCountAtCursor(iCount);
				if (iCount > 0)
					return CreateListFromItem(*pCC, pItemList->GetItemAtCursor());
				else
					return pCC->CreateNil();
				}
			else
				return pCC->CreateNil();
			}

		case FN_ITEM_DAMAGED:
			{
			bool bDamaged;
			if (pArgs->GetCount() > 1)
				bDamaged = !pArgs->GetElement(1)->IsNil();
			else
				bDamaged = true;

			pItemList->SetDamagedAtCursor(bDamaged);
			return CreateListFromItem(*pCC, pItemList->GetItemAtCursor());
			}

		case FN_ITEM_DATA:
			{
			//	Set quoted before we unlink
			//	Note: This might be a hack...it probably makes more sense to mark
			//	all function return values as 'quoted'

			BOOL bOldQuoted = pArgs->GetElement(2)->IsQuoted();
			pArgs->GetElement(2)->SetQuoted();
			CString sData = pCC->Unlink(pArgs->GetElement(2));
			if (!bOldQuoted)
				pArgs->GetElement(2)->ClearQuoted();

			pItemList->SetDataAtCursor(pArgs->GetElement(1)->GetStringValue(), sData);
			return CreateListFromItem(*pCC, pItemList->GetItemAtCursor());
			}

		case FN_ITEM_ENHANCED:
			pItemList->SetModsAtCursor((DWORD)pArgs->GetElement(1)->GetIntegerValue());
			return CreateListFromItem(*pCC, pItemList->GetItemAtCursor());

		default:
			ASSERT(false);
		}

	return pCC->CreateNil();
	}

ICCItem *fnItemTypeSet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnItemTypeSet
//
//	(itmSetKnown item)

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("v"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into an item type

	CItemType *pType = NULL;
	if (pArgs->GetElement(0)->IsList())
		{
		CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(0)));
		pType = Item.GetType();
		}
	else if (pArgs->GetElement(0)->IsInteger())
		pType = g_pUniverse->FindItemType((DWORD)pArgs->GetElement(0)->GetIntegerValue());
	
	if (pType == NULL)
		return pCC->CreateError(CONSTLIT("Invalid item type"), pArgs->GetElement(0));

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_ITEM_TYPE_SET_KNOWN:
			{
			pType->SetKnown();
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnObjAddRandomItems (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnObjAddRandomItems
//
//	Adds random items from a table

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iii"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a space object

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (pObj == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Other args

	DWORD dwTableID = pArgs->GetElement(1)->GetIntegerValue();
	int iCount = pArgs->GetElement(2)->GetIntegerValue();

	//	Do it

	CItemListManipulator theList(pObj->GetItemList());
	CItemTable *pTable = g_pUniverse->FindItemTable(dwTableID);
	if (pTable == NULL)
		return pCC->CreateError(CONSTLIT("Item table not found:"), pArgs->GetElement(1));

	for (int j = 0; j < iCount; j++)
		pTable->AddItems(theList);

	pObj->InvalidateItemListAddRemove();

	//	Done

	pArgs->Discard(pCC);
	return pCC->CreateTrue();
	}

ICCItem *fnObjData (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnObjData
//
//	Deals with opaque object data

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_OBJ_GETDATA 
			|| dwData == FN_OBJ_GET_OBJREF_DATA 
			|| dwData == FN_OBJ_GET_STATIC_DATA
			|| dwData == FN_OBJ_GET_STATIC_DATA_FOR_STATION_TYPE
			|| dwData == FN_OBJ_GET_GLOBAL_DATA)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("is"));
	else if (dwData == FN_OBJ_SET_OBJREF_DATA)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("isi"));
	else if (dwData == FN_OBJ_INCREMENT_DATA)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("is*"));
	else
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("isv"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a space object

	CSpaceObject *pObj = NULL;
	CStationType *pStationType = NULL;
	if (dwData == FN_OBJ_GET_STATIC_DATA_FOR_STATION_TYPE)
		{
		pStationType = g_pUniverse->FindStationType(pArgs->GetElement(0)->GetIntegerValue());
		if (pStationType == NULL)
			{
			pArgs->Discard(pCC);
			return pCC->CreateError(CONSTLIT("Unknown station type"), pArgs->GetElement(0));
			}
		}
	else
		{
		pObj = GetObjArg(pArgs->GetElement(0));
		if (pObj == NULL)
			{
			pArgs->Discard(pCC);
			return pCC->CreateNil();
			}
		}

	//	Second argument is the attribute

	CString sAttrib = pArgs->GetElement(1)->GetStringValue();

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_OBJ_GETDATA:
			{
			CString sData = pObj->GetData(sAttrib);
			pResult = pCC->Link(sData, 0, NULL);
			pArgs->Discard(pCC);
			break;
			}

		case FN_OBJ_GET_STATIC_DATA:
			{
			CString sData = pObj->GetStaticData(sAttrib);
			pResult = pCC->Link(sData, 0, NULL);
			pArgs->Discard(pCC);
			break;
			}

		case FN_OBJ_GET_STATIC_DATA_FOR_STATION_TYPE:
			{
			CString sData = pStationType->GetStaticData(sAttrib);
			pResult = pCC->Link(sData, 0, NULL);
			pArgs->Discard(pCC);
			break;
			}

		case FN_OBJ_GET_GLOBAL_DATA:
			{
			CString sData = pObj->GetGlobalData(sAttrib);
			pResult = pCC->Link(sData, 0, NULL);
			pArgs->Discard(pCC);
			break;
			}

		case FN_OBJ_SETDATA:
			{
#ifdef DEBUG_SETDATA_CHECK
			if (g_pUniverse->InDebugMode())
				{
				//	Make sure this is not an object (use objSetObjRefData instead)

				if (CObject::IsValidPointer((CObject *)pArgs->GetElement(2)->GetIntegerValue()))
					{
					CSpaceObject *pTest = dynamic_cast<CSpaceObject *>((CObject *)pArgs->GetElement(2)->GetIntegerValue());
					if (pTest)
						{
						DebugBreak();
						pArgs->Discard(pCC);
						pResult = pCC->CreateError(CONSTLIT("Use objSetObjRefData for objects"), NULL);
						break;
						}
					}
				}
#endif

			//	Set quoted before we unlink
			//	Note: This might be a hack...it probably makes more sense to mark
			//	all function return values as 'quoted'

			BOOL bOldQuoted = pArgs->GetElement(2)->IsQuoted();
			pArgs->GetElement(2)->SetQuoted();
			CString sData = pCC->Unlink(pArgs->GetElement(2));
			if (!bOldQuoted)
				pArgs->GetElement(2)->ClearQuoted();

			pObj->SetData(sAttrib, sData);
			pResult = pCC->CreateTrue();
			pArgs->Discard(pCC);
			break;
			}

		case FN_OBJ_SET_GLOBAL_DATA:
			{
			//	Set quoted before we unlink

			BOOL bOldQuoted = pArgs->GetElement(2)->IsQuoted();
			pArgs->GetElement(2)->SetQuoted();
			CString sData = pCC->Unlink(pArgs->GetElement(2));
			if (!bOldQuoted)
				pArgs->GetElement(2)->ClearQuoted();

			pObj->SetGlobalData(sAttrib, sData);
			pResult = pCC->CreateTrue();
			pArgs->Discard(pCC);
			break;
			}

		case FN_OBJ_INCREMENT_DATA:
			{
			//	Get the current value and increment

			CString sData = pObj->GetData(sAttrib);
			ICCItem *pValue = pCC->Link(sData, 0, NULL);
			int iIncrement;

			if (pArgs->GetCount() > 2)
				iIncrement = pArgs->GetElement(2)->GetIntegerValue();
			else
				iIncrement = 1;

			pArgs->Discard(pCC);

			//	Create the result

			ICCItem *pNewValue = pCC->CreateInteger(pValue->GetIntegerValue() + iIncrement);
			CString sNewData = pCC->Unlink(pNewValue);
			pObj->SetData(sAttrib, sNewData);

			//	Done

			pValue->Discard(pCC);
			pResult = pNewValue;
			break;
			}

		case FN_OBJ_GET_OBJREF_DATA:
			{
			CSpaceObject *pRef = pObj->GetObjRefData(sAttrib);
			if (pRef)
				pResult = pCC->CreateInteger((int)pRef);
			else
				pResult = pCC->CreateNil();
			pArgs->Discard(pCC);
			break;
			}

		case FN_OBJ_SET_OBJREF_DATA:
			{
			if (!pArgs->GetElement(2)->IsNil())
				{
				CSpaceObject *pRef = GetObjArg(pArgs->GetElement(2));
				pObj->SetObjRefData(sAttrib, pRef);
				}
			else
				pObj->SetObjRefData(sAttrib, NULL);

			pResult = pCC->CreateTrue();
			pArgs->Discard(pCC);
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnObjComms (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnObjComms
//
//	(objCommunicate obj senderObj msg [obj] [data])

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iii*"));
	if (pArgs->IsError())
		return pArgs;

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (pObj == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	CSpaceObject *pSender = GetObjArg(pArgs->GetElement(1));
	if (pSender == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	int iMessage = pArgs->GetElement(2)->GetIntegerValue();

	//	Optional args

	CSpaceObject *pParam1 = NULL;
	if (pArgs->GetCount() > 3)
		pParam1 = GetObjArg(pArgs->GetElement(3));

	DWORD dwParam2 = 0;
	if (pArgs->GetCount() > 4)
		dwParam2 = (DWORD)pArgs->GetElement(4)->GetIntegerValue();

	pArgs->Discard(pCC);

	//	Done

	DWORD dwResult = pSender->Communicate(pObj, (MessageTypes)iMessage, pParam1, dwParam2);
	if (dwResult == resNoAnswer)
		return pCC->CreateNil();
	else if (dwResult == resAck)
		return pCC->CreateTrue();
	else
		return pCC->CreateInteger(dwResult);
	}

ICCItem *fnObjEnumItems (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnObjEnumItems
//
//	(objEnumItems obj criteria item-var exp)

	{
	int i;
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pVar;
	ICCItem *pBody;
	ICCItem *pLocalSymbols;
	ICCItem *pOldSymbols;
	ICCItem *pError;
	int iVarOffset;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("isqu"));
	if (pArgs->IsError())
		return pArgs;

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (pObj == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	CItemCriteria Criteria;
	CItem::ParseCriteria(pArgs->GetElement(1)->GetStringValue(), &Criteria);

	pVar = pArgs->GetElement(2);
	pBody = pArgs->GetElement(3);

	//	Setup the locals. We start by creating a local symbol table

	pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pArgs->Discard(pCC);
		return pLocalSymbols;
		}

	//	Associate the enumaration variable

	pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pArgs->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pEvalCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pEvalCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pEvalCtx->pLexicalSymbols);
	pOldSymbols = pEvalCtx->pLocalSymbols;
	pEvalCtx->pLocalSymbols = pLocalSymbols;

	//	Start with a default result

	pResult = pCC->CreateNil();

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Generate a list of all the items

	CItemListManipulator ItemList(pObj->GetItemList());
	int iCount = 0;
	CItem *pTempList = new CItem [ItemList.GetCount()];
	while (ItemList.MoveCursorForward())
		{
		if (ItemList.GetItemAtCursor().MatchesCriteria(Criteria))
			pTempList[iCount++] = ItemList.GetItemAtCursor();
		}

	//	Loop over all items

	for (i = 0; i < iCount; i++)
		{
		//	Clean up the previous result

		pResult->Discard(pCC);

		//	Set the element

		ICCItem *pItem = CreateListFromItem(*pCC, pTempList[i]);
		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);
		pItem->Discard(pCC);

		//	Eval

		pResult = pCC->Eval(pEvalCtx, pBody);
		if (pResult->IsError())
			break;
		}

	//	Clean up

	delete [] pTempList;
	pEvalCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnObjGateTo (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnObjGateTo
//
//	(objGateTo obj node entryPoint)

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Evaluate the arguments and validate them

	ICCItem *pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iss*"));
	if (pArgs->IsError())
		return pArgs;

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (pObj == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	CString sNode = pArgs->GetElement(1)->GetStringValue();
	CString sEntryPoint = pArgs->GetElement(2)->GetStringValue();

	CEffectCreator *pEffect = NULL;
	if (pArgs->GetCount() > 3)
		{
		DWORD dwEffectUNID = pArgs->GetElement(3)->GetIntegerValue();
		pEffect = g_pUniverse->FindEffectType(dwEffectUNID);
		}

	pArgs->Discard(pCC);

	//	Find the node

	CTopologyNode *pNode = g_pUniverse->FindTopologyNode(sNode);
	if (pNode == NULL)
		return pCC->CreateError(strPatternSubst(CONSTLIT("Invalid node: %s"), sNode.GetASCIIZPointer()), NULL);

	//	Effect

	if (pEffect)
		pEffect->CreateEffect(pObj->GetSystem(),
				NULL,
				pObj->GetPos(),
				NullVector);

	//	Jump

	pObj->EnterGate(pNode, sEntryPoint, NULL);
	return pCC->CreateTrue();
	}

ICCItem *fnItemEnumTypes (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnItemEnumTypes
//
//	(itmEnumTypes criteria item-var exp)

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pVar;
	ICCItem *pBody;
	ICCItem *pLocalSymbols;
	ICCItem *pOldSymbols;
	ICCItem *pError;
	int iVarOffset;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("squ"));
	if (pArgs->IsError())
		return pArgs;

	CItemCriteria Criteria;
	CItem::ParseCriteria(pArgs->GetElement(0)->GetStringValue(), &Criteria);

	pVar = pArgs->GetElement(1);
	pBody = pArgs->GetElement(2);

	//	Setup the locals. We start by creating a local symbol table

	pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pArgs->Discard(pCC);
		return pLocalSymbols;
		}

	//	Associate the enumaration variable

	pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pArgs->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pEvalCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pEvalCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pEvalCtx->pLexicalSymbols);
	pOldSymbols = pEvalCtx->pLocalSymbols;
	pEvalCtx->pLocalSymbols = pLocalSymbols;

	//	Start with a default result

	pResult = pCC->CreateNil();

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Loop over all items

	for (int i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		CItem Item(pType, 1);

		if (Item.MatchesCriteria(Criteria))
			{
			//	Associate item list

			ICCItem *pItem = CreateListFromItem(*pCC, Item);
			pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);
			pItem->Discard(pCC);

			//	Clean up the previous result

			pResult->Discard(pCC);

			//	Eval

			pResult = pCC->Eval(pEvalCtx, pBody);
			if (pResult->IsError())
				break;
			}
		}

	//	Clean up

	pEvalCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnObjGetArmor (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnObjGetArmor
//
//	Gets data about a ship's armor
//
//	(objGetArmorDamage obj armorSegment) -> damage to armor segment
//	(objGetArmorType obj armorSegment) -> CArmorClass *
//	(objRepairArmor obj armorSegment [hpToRepair])

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iv*"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a space object

	CShip *pShip = GetShipArg(pArgs->GetElement(0));
	if (pShip == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Second argument is the armor segment

	int iArmorSeg = -1;
	CInstalledArmor *pSection = NULL;
	if (pArgs->GetElement(1)->IsList())
		{
		CItem Item = CreateItemFromList(*pCC, pArgs->GetElement(1));
		if (Item.GetType() && Item.IsInstalled())
			{
			iArmorSeg = Item.GetInstalled();
			pSection = pShip->GetArmorSection(iArmorSeg);
			}
		else
			{
			pArgs->Discard(pCC);
			return pCC->CreateNil();
			}
		}
	else
		{
		iArmorSeg = pArgs->GetElement(1)->GetIntegerValue();
		if (iArmorSeg >= 0 && iArmorSeg < pShip->GetArmorSectionCount())
			pSection = pShip->GetArmorSection(iArmorSeg);
		if (pSection == NULL)
			{
			pArgs->Discard(pCC);
			return pCC->CreateNil();
			}
		}

	//	If we're repairing armor, then there may be a third parameter

	int iRepairHP = -1;
	if (pArgs->GetElement(2))
		iRepairHP = pArgs->GetElement(2)->GetIntegerValue();

	//	If we're damaging armor, then get more parameters

	int iDamageHP;
	DamageTypes DamageType;
	if (dwData == FN_OBJ_DAMAGE_ARMOR)
		{
		DamageType = (DamageTypes)pArgs->GetElement(2)->GetIntegerValue();
		iDamageHP = pArgs->GetElement(3)->GetIntegerValue();
		}

	//	No longer needed

	pArgs->Discard(pCC);

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_OBJ_ARMOR_TYPE:
			pResult = pCC->CreateInteger((int)pSection->pArmorClass);
			break;

		case FN_OBJ_ARMOR_DAMAGE:
			pResult = pCC->CreateInteger(pSection->pArmorClass->GetHitPoints(pSection) - pSection->iHitPoints);
			break;

		case FN_OBJ_ARMOR_ITEM:
			{
			CItemListManipulator ItemList(pShip->GetItemList());
			pShip->SetCursorAtArmor(ItemList, iArmorSeg);
			if (ItemList.IsCursorValid())
				pResult = CreateListFromItem(*pCC, ItemList.GetItemAtCursor());
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_OBJ_ARMOR_NAME:
			{
			CString sName;
			const SPlayerSettings *pPlayer = pShip->GetClass()->GetPlayerSettings();
			if (pPlayer)
				if (iArmorSeg >=0 && iArmorSeg < pPlayer->m_iArmorDescCount)
					sName = pPlayer->m_pArmorDesc[iArmorSeg].sName;

			if (sName.IsBlank())
				pResult = pCC->CreateNil();
			else
				pResult = pCC->CreateString(sName);
			break;
			}

		case FN_OBJ_REPAIR_ARMOR:
			pShip->RepairArmor(iArmorSeg, iRepairHP);
			pResult = pCC->CreateTrue();
			break;

		case FN_OBJ_DAMAGE_ARMOR:
			pShip->DamageArmor(iArmorSeg, DamageType, iDamageHP);
			pResult = pCC->CreateTrue();
			break;

		case FN_OBJ_ARMOR_MAX_HP:
			pResult = pCC->CreateInteger(pSection->pArmorClass->GetHitPoints(pSection));
			break;

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnObjIDGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnObjIDGet
//
//	Gets data from an object ID

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_OBJ_OBJECT:
			{
			CSpaceObject *pObj = g_pUniverse->FindObject(pArgs->GetElement(0)->GetIntegerValue());
			if (pObj)
				return pCC->CreateInteger((int)pObj);
			else
				return pCC->CreateNil();
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnObjGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnObjGet
//
//	Gets data about a space object

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Get the object

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (pObj == NULL)
		return pCC->CreateNil();

	//	Set the data as appropriate

	switch (dwData)
		{
		case FN_OBJ_DOCKED_AT:
			{
			CSpaceObject *pStation = GetObjArg(pArgs->GetElement(1));
			if (pStation == NULL)
				return pCC->CreateNil();
			else
				return pCC->CreateBool(pStation->IsObjDocked(pObj));
			}

		case FN_OBJ_GET_BALANCE:
			return pCC->CreateInteger(pObj->GetBalance());

		case FN_OBJ_GET_BUY_PRICE:
			{
			CItem Item = CreateItemFromList(*pCC, pArgs->GetElement(1));
			int iValue = pObj->GetBuyPrice(Item);
			if (iValue >= 0)
				return pCC->CreateInteger(iValue);
			else
				return pCC->CreateNil();
			}

		case FN_OBJ_GET_ID:
			return pCC->CreateInteger(pObj->GetID());

		case FN_OBJ_IMAGE:
			return CreateListFromImage(*pCC, pObj->GetImage());

		case FN_OBJ_LEVEL:
			return pCC->CreateInteger(pObj->GetLevel());

		case FN_OBJ_MASS:
			return pCC->CreateInteger((int)pObj->GetMass());

		case FN_OBJ_ORDER_GIVER:
			{
			DestructionTypes iCause;
			if (pArgs->GetCount() > 1)
				iCause = GetDestructionCause(pArgs->GetElement(1)->GetStringValue());
			else
				iCause = killedNone;

			CSpaceObject *pOrderGiver = pObj->GetOrderGiver(iCause);
			if (pOrderGiver)
				return pCC->CreateInteger((int)pOrderGiver);
			else
				return pCC->CreateNil();
			}

		case FN_OBJ_GET_SELL_PRICE:
			{
			CItem Item = CreateItemFromList(*pCC, pArgs->GetElement(1));
			int iValue = pObj->GetSellPrice(Item);
			if (iValue > 0)
				return pCC->CreateInteger(iValue);
			else
				return pCC->CreateNil();
			}

		case FN_OBJ_OPEN_DOCKING_PORT_COUNT:
			return pCC->CreateInteger(pObj->GetOpenDockingPortCount());

		case FN_OBJ_IDENTIFIED:
			return pCC->CreateBool(pObj->IsIdentified());

		case FN_OBJ_MATCHES:
			{
			CSpaceObject *pSource = GetObjArg(pArgs->GetElement(1));

			CString sFilter = pArgs->GetElement(2)->GetStringValue();
			CSpaceObject::Criteria Criteria;
			CSpaceObject::ParseCriteria(pSource, sFilter, &Criteria);

			return pCC->CreateBool(pObj->MatchesCriteria(Criteria));
			}

		case FN_OBJ_VELOCITY:
			{
			CVector vVel = pObj->GetVel();
			Metric rSpeed;
			vVel = vVel.Normal(&rSpeed);
			vVel = vVel * (100.0 * rSpeed / LIGHT_SPEED);

			return CreateListFromVector(*pCC, vVel);
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnObjGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnObjGetOld
//
//	Gets data about a space object
//
//	(objGetName obj) -> Name of the object

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_OBJ_DISTANCE || dwData == FN_OBJ_NAME || FN_OBJ_INSTALLED_ITEM_DESC || FN_OBJ_ENEMY)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i*"));
	else if (dwData == FN_OBJ_ATTRIBUTE)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("is"));
	else
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Valid pObj is optional for some functions

	bool bRequireValidObj;
	if (dwData == FN_OBJ_IS_ABANDONED)
		bRequireValidObj = false;
	else
		bRequireValidObj = true;

	//	Convert the first argument into a space object

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (bRequireValidObj && pObj == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Second object

	DWORD dwFlags = 0;
	CSpaceObject *pObj2 = NULL;
	CString sAttribute;
	CItem Item;
	if (dwData == FN_OBJ_DISTANCE || dwData == FN_OBJ_ENEMY)
		{
		if (pArgs->GetCount() == 2)
			{
			pObj2 = GetObjArg(pArgs->GetElement(1));
			if (pObj2 == NULL)
				{
				pArgs->Discard(pCC);
				return pCC->CreateNil();
				}
			}
		}
	else if (dwData == FN_OBJ_NAME)
		{
		if (pArgs->GetCount() == 2)
			dwFlags = pArgs->GetElement(1)->GetIntegerValue();
		}
	else if (dwData == FN_OBJ_ATTRIBUTE)
		sAttribute = pArgs->GetElement(1)->GetStringValue();
	else if (dwData == FN_OBJ_INSTALLED_ITEM_DESC)
		{
		Item = CreateItemFromList(*pCC, pArgs->GetElement(1));
		}

	//	No longer needed

	pArgs->Discard(pCC);

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_OBJ_ATTRIBUTE:
			pResult = pCC->CreateBool(pObj->HasAttribute(sAttribute));
			break;

		case FN_OBJ_CAN_ATTACK:
			pResult = pCC->CreateBool(pObj->CanAttack());
			break;

		case FN_OBJ_CARGO_SPACE_LEFT:
			pResult = pCC->CreateInteger((int)(pObj->GetCargoSpaceLeft() * 1000.0));
			break;

		case FN_OBJ_COMBAT_POWER:
			pResult = pCC->CreateInteger(pObj->GetCombatPower());
			break;

		case FN_OBJ_DAMAGE_TYPE:
			pResult = pCC->CreateInteger(pObj->GetDamageType());
			break;

		case FN_OBJ_DESTINY:
			pResult = pCC->CreateInteger(pObj->GetDestiny());
			break;

		case FN_OBJ_DESTROY:
			{
			if (pObj != g_pUniverse->GetPlayer())
				{
				pObj->Destroy(removedFromSystem, NULL);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_OBJ_SHIELD_LEVEL:
			pResult = pCC->CreateInteger(pObj->GetShieldLevel());
			break;

		case FN_OBJ_NAME:
			pResult = pCC->CreateString(pObj->GetNounPhrase(dwFlags));
			break;

		case FN_OBJ_INSTALLED_ITEM_DESC:
			if (Item.GetType() && Item.IsInstalled())
				pResult = pCC->CreateString(pObj->GetInstallationPhrase(Item));
			else
				pResult = pCC->CreateNil();
			break;

		case FN_OBJ_IS_ABANDONED:
			if (pObj)
				pResult = pCC->CreateBool(pObj->IsAbandoned());
			else
				pResult = pCC->CreateBool(true);
			break;

		case FN_OBJ_IS_SHIP:
			pResult = pCC->CreateBool(pObj->GetCategory() == CSpaceObject::catShip);
			break;

		case FN_OBJ_ENEMY:
			if (pObj2)
				pResult = pCC->CreateBool(pObj->IsEnemy(pObj2) 
						&& !pObj2->IsEscortingFriendOf(pObj)
						&& !pObj->IsEscortingFriendOf(pObj2));
			else
				pResult = pCC->CreateNil();
			break;

		case FN_OBJ_KNOWN:
			pResult = pCC->CreateBool(pObj->IsKnown());
			break;

		case FN_OBJ_IS_UNDER_ATTACK:
			pResult = pCC->CreateBool(pObj->IsUnderAttack());
			break;

		case FN_OBJ_MAX_POWER:
			pResult = pCC->CreateInteger(pObj->GetMaxPower());
			break;

		case FN_OBJ_POSITION:
			pResult = CreateListFromVector(*pCC, pObj->GetPos());
			break;

		case FN_OBJ_SOVEREIGN:
			{
			CSovereign *pSovereign = pObj->GetSovereign();
			if (pSovereign == NULL)
				pResult = pCC->CreateNil();
			else
				pResult = pCC->CreateInteger(pSovereign->GetUNID());
			break;
			}

		case FN_OBJ_TARGET:
			{
			CSpaceObject *pTarget = pObj->GetTarget();
			if (pTarget)
				pResult = pCC->CreateInteger((int)pTarget);
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_OBJ_VISIBLE_DAMAGE:
			pResult = pCC->CreateInteger(pObj->GetVisibleDamage());
			break;

		case FN_OBJ_SET_KNOWN:
			pObj->SetKnown();
			pResult = pCC->CreateTrue();
			break;

		case FN_OBJ_DISTANCE:
			{
			Metric rDist;

			if (pObj2)
				{
				CVector vDist = pObj->GetPos() - pObj2->GetPos();
				rDist = vDist.Length();
				}
			else
				rDist = pObj->GetPos().Length();

			int iDistInLightSeconds = (int)((rDist / LIGHT_SECOND) + 0.5);
			pResult = pCC->CreateInteger(iDistInLightSeconds);
			break;
			}

		case FN_OBJ_NEAREST_STARGATE:
			{
			CSpaceObject *pGate = pObj->GetNearestStargate(true);
			if (pGate == NULL)
				pResult = pCC->CreateNil();
			else
				pResult = pCC->CreateInteger((int)pGate);
			break;
			}

		case FN_OBJ_LOWER_SHIELDS:
			{
			pObj->DeactivateShields();
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnObjSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnObjSet
//
//	Sets data about a space object

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Get the object

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (pObj == NULL)
		return pCC->CreateNil();

	//	Set the data as appropriate

	switch (dwData)
		{
		case FN_OBJ_CHARGE:
			{
			int iValue = Max(0, pArgs->GetElement(1)->GetIntegerValue());
			int iRemaining = pObj->ChargeMoney(iValue);
			if (iRemaining == -1)
				return pCC->CreateNil();
			else
				return pCC->CreateInteger(iRemaining);
			}

		case FN_OBJ_CLEAR_IDENTIFIED:
			{
			pObj->SetIdentified(false);
			return pCC->CreateTrue();
			}

		case FN_OBJ_CREDIT:
			{
			int iValue = Max(0, pArgs->GetElement(1)->GetIntegerValue());
			int iRemaining = pObj->CreditMoney(iValue);
			return pCC->CreateInteger(iRemaining);
			}

		case FN_OBJ_DEPLETE_SHIELDS:
			{
			pObj->DepleteShields();
			return pCC->CreateTrue();
			}

		case FN_OBJ_FIRE_EVENT:
			{
			ICCItem *pResult;
			pObj->FireCustomEvent(pArgs->GetElement(1)->GetStringValue(), &pResult);
			return pResult;
			}

		case FN_OBJ_FIRE_ITEM_EVENT:
			{
			ICCItem *pResult;
			CItem Item = GetItemArg(*pCC, pArgs->GetElement(1));
			pObj->FireCustomItemEvent(pArgs->GetElement(2)->GetStringValue(), Item, &pResult);
			return pResult;
			}

		case FN_OBJ_IDENTIFIED:
			{
			pObj->SetIdentified(true);
			return pCC->CreateTrue();
			}

		case FN_OBJ_INCREMENT_VELOCITY:
			{
			//	Set velocity (vel in percent of light-speed)

			CVector vVel = (CreateVectorFromList(*pCC, pArgs->GetElement(1)) / 100.0);
			pObj->Place(pObj->GetPos(), pObj->GetVel() + vVel);

			//	Return the resulting speed

			return CreateListFromVector(*pCC, pObj->GetVel());
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnObjSetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnObjSetOld
//
//	Sets data about a space object

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i*"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a space object

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (pObj == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_OBJ_ADD_SUBORDINATE:
			{
			CSpaceObject *pSubordinate = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			if (pSubordinate)
				{
				pObj->AddSubordinate(pSubordinate);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_OBJ_MESSAGE:
			{
			//	Second param is the sender; third is message

			CSpaceObject *pSender = GetObjArg(pArgs->GetElement(1));
			CString sMessage = pArgs->GetElement(2)->GetStringValue();
			pArgs->Discard(pCC);

			//	Do it

			pObj->SendMessage(pSender, sMessage);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_OBJ_NAME:
			{
			//	Second param is name

			pObj->SetName(pArgs->GetElement(1)->GetStringValue());
			pArgs->Discard(pCC);

			pResult = pCC->CreateTrue();
			break;
			}

		case FN_OBJ_JUMP:
			{
			//	Second param is vector

			CVector vPos(CreateVectorFromList(*pCC, pArgs->GetElement(1)));
			pArgs->Discard(pCC);

			//	Jump

			pObj->Jump(vPos);

			//	Done

			pResult = pCC->CreateTrue();
			break;
			}

		case FN_OBJ_PARALYSIS:
			{
			int iTime = pArgs->GetElement(1)->GetIntegerValue();
			pArgs->Discard(pCC);

			pObj->MakeParalyzed(iTime);

			pResult = pCC->CreateTrue();
			break;
			}

		case FN_OBJ_POSITION:
			{
			//	Second param is vector

			CVector vPos(CreateVectorFromList(*pCC, pArgs->GetElement(1)));
			pArgs->Discard(pCC);

			//	Move the ship

			pObj->Place(vPos);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_OBJ_REGISTER_EVENTS:
			{
			CSpaceObject *pObj2 = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			if (pObj2)
				{
				pObj->RegisterObjectForEvents(pObj2);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_OBJ_SOVEREIGN:
			{
			DWORD dwSovereignID = pArgs->GetElement(1)->GetIntegerValue();
			pArgs->Discard(pCC);

			CSovereign *pSovereign = g_pUniverse->FindSovereign(dwSovereignID);
			CSystem *pSystem = pObj->GetSystem();
			if (pSovereign && pSystem)
				{
				pSystem->SetObjectSovereign(pObj, pSovereign);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();

			break;
			}

		case FN_OBJ_UNREGISTER_EVENTS:
			{
			CSpaceObject *pObj2 = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			pObj->UnregisterObjectForEvents(pObj2);
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnObjItem (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnObjItem
//
//	Gets and sets items
//
//	(objAddItem obj item)
//	(objHasItem obj item [count])
//	(objRemoveItem obj item [count])

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_OBJ_ENUM_ITEMS)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("is"));
	else
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("il*"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a space object

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (pObj == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_OBJ_ADD_ITEM:
			{
			CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
			if (2 < pArgs->GetCount())
				Item.SetCount(pArgs->GetElement(2)->GetIntegerValue());
			pArgs->Discard(pCC);

			if (Item.GetCount() > 0)
				{
				CItemListManipulator ObjList(pObj->GetItemList());
				ObjList.AddItem(Item);
				pObj->OnComponentChanged(comCargo);
				pObj->InvalidateItemListAddRemove();
				}

			pResult = pCC->CreateTrue();
			break;
			}

		case FN_OBJ_HAS_ITEM:
			{
			CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
			int iCount = Item.GetCount();
			if (pArgs->GetElement(2))
				iCount = pArgs->GetElement(2)->GetIntegerValue();
			pArgs->Discard(pCC);

			CItemListManipulator ObjList(pObj->GetItemList());
			if (!ObjList.SetCursorAtItem(Item))
				return pCC->CreateNil();

			if (ObjList.GetItemAtCursor().GetCount() < iCount)
				return pCC->CreateNil();

			pResult = pCC->CreateInteger(ObjList.GetItemAtCursor().GetCount());
			break;
			}

		case FN_OBJ_REMOVE_ITEM:
			{
			CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
			int iCount = Item.GetCount();
			if (pArgs->GetElement(2))
				iCount = pArgs->GetElement(2)->GetIntegerValue();
			pArgs->Discard(pCC);

			CItemListManipulator ObjList(pObj->GetItemList());
			if (!ObjList.SetCursorAtItem(Item))
				return pCC->CreateError(CONSTLIT("Unable to find item in list"), NULL);

			ObjList.DeleteAtCursor(iCount);

			pObj->OnComponentChanged(comCargo);
			pObj->ItemsModified();
			pObj->InvalidateItemListAddRemove();
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_OBJ_ENUM_ITEMS:
			{
			CString sCriteria = pArgs->GetElement(1)->GetStringValue();
			pArgs->Discard(pCC);

			CItemCriteria Criteria;
			CItem::ParseCriteria(sCriteria, &Criteria);

			pResult = pCC->CreateLinkedList();
			if (pResult->IsError())
				return pResult;

			CCLinkedList *pList = (CCLinkedList *)pResult;

			CItemListManipulator ObjList(pObj->GetItemList());
			ObjList.ResetCursor();
			while (ObjList.MoveCursorForward())
				{
				if (ObjList.GetItemAtCursor().MatchesCriteria(Criteria))
					{
					ICCItem *pItem = CreateListFromItem(*pCC, ObjList.GetItemAtCursor());
					pList->Append(pCC, pItem, NULL);
					pItem->Discard(pCC);
					}
				}
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnProgramDamage (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnProgramDamage
//
//	(objProgramDamage obj hacker progName aiLevel code)

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Evaluate the arguments and validate them

	ICCItem *pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iisiu"));
	if (pArgs->IsError())
		return pArgs;

	CSpaceObject *pObj = GetObjArg(pArgs->GetElement(0));
	if (pObj == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	CSpaceObject *pHacker = GetObjArg(pArgs->GetElement(1));
	if (pHacker == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Create the program structure

	ProgramDesc Program;
	Program.iProgram = progCustom;
	Program.sProgramName = pArgs->GetElement(2)->GetStringValue();
	Program.iAILevel = pArgs->GetElement(3)->GetIntegerValue();
	Program.pCtx = pEvalCtx;
	Program.ProgramCode = pArgs->GetElement(4);

	//	Call the object

	pObj->ProgramDamage(pHacker, Program);

	//	Done

	pArgs->Discard(pCC);
	return pCC->CreateTrue();
	}

ICCItem *fnRollDice (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnRollDice
//
//	Rolls dice

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ii*"));
	if (pArgs->IsError())
		return pArgs;

	//	Arguments

	int iCount = pArgs->GetElement(0)->GetIntegerValue();
	int iFaces = pArgs->GetElement(1)->GetIntegerValue();
	int iBonus = 0;
	if (pArgs->GetElement(2))
		iBonus = pArgs->GetElement(2)->GetIntegerValue();

	pArgs->Discard(pCC);

	//	Roll Dice

	DiceRange Dice(iFaces, iCount, iBonus);
	return pCC->CreateInteger(Dice.Roll());
	}

ICCItem *fnShipClass (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnShipClass
//
//	Sets data about a ship class

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pResult;

	//	Convert the first argument into a ship class

	DWORD dwUNID = pArgs->GetElement(0)->GetIntegerValue();
	CShipClass *pClass = g_pUniverse->FindShipClass(dwUNID);
	if (pClass == NULL)
		{
		CShip *pShip = GetShipArg(pArgs->GetElement(0));
		if (pShip)
			pClass = pShip->GetClass();

		if (pClass == NULL)
			return pCC->CreateError(CONSTLIT("ship class expected:"), pArgs->GetElement(0));
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SHIP_CLASS_NAME:
			{
			DWORD dwFlags = pArgs->GetElement(1)->GetIntegerValue();

			DWORD dwNounFlags;
			CString sName = pClass->GetName(&dwNounFlags);

			pResult = pCC->CreateString(ComposeNounPhrase(sName, 1, CString(), dwNounFlags, dwFlags));
			break;
			}

		case FN_SHIP_DATA_FIELD:
			pResult = pCC->CreateString(pClass->GetDataField(pArgs->GetElement(1)->GetStringValue()));
			break;

		case FN_SHIP_GET_GLOBAL_DATA:
			{
			CString sAttrib = pArgs->GetElement(1)->GetStringValue();
			CString sData = pClass->GetGlobalData(sAttrib);
			pResult = pCC->Link(sData, 0, NULL);
			break;
			}

		case FN_SHIP_GET_IMAGE_DESC:
			{
			int iRotation = 0;
			if (pArgs->GetCount() > 1)
				iRotation = Angle2Direction(pArgs->GetElement(1)->GetIntegerValue(), pClass->GetRotationRange());

			pResult = CreateListFromImage(*pCC, pClass->GetImage(), iRotation);
			break;
			}

		case FN_SHIP_SET_GLOBAL_DATA:
			{
			CString sAttrib = pArgs->GetElement(1)->GetStringValue();

			//	Set quoted before we unlink

			BOOL bOldQuoted = pArgs->GetElement(2)->IsQuoted();
			pArgs->GetElement(2)->SetQuoted();
			CString sData = pCC->Unlink(pArgs->GetElement(2));
			if (!bOldQuoted)
				pArgs->GetElement(2)->ClearQuoted();

			pClass->SetGlobalData(sAttrib, sData);
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnShipGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnShipGet
//
//	Gets data about a ship

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Get the ship arg

	CShip *pShip = GetShipArg(pArgs->GetElement(0));
	if (pShip == NULL)
		return pCC->CreateNil();

	//	Do it

	switch (dwData)
		{
		case FN_SHIP_IS_NAMED_DEVICE:
			{
			CItemListManipulator *pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
			const CItem &Item = pItemList->GetItemAtCursor();
			if (!Item.IsInstalled())
				return pCC->CreateNil();

			int iDevInt = pArgs->GetElement(2)->GetIntegerValue();
			if (iDevInt < 0 || iDevInt >= devNamesCount)
				return pCC->CreateNil();

			DeviceNames iDev = (DeviceNames)iDevInt;

			CItemListManipulator TestList(pShip->GetItemList());
			pShip->SetCursorAtNamedDevice(TestList, iDev);

			return pCC->CreateBool(TestList.GetItemAtCursor().GetInstalled() == Item.GetInstalled());
			}

		case FN_SHIP_IS_RADIATION_IMMUNE:
			{
			if (pArgs->GetCount() > 1)
				{
				CItem Item = GetItemArg(*pCC, pArgs->GetElement(1));
				CItemType *pType = Item.GetType();
				if (pType == NULL)
					return pCC->CreateNil();

				CArmorClass *pArmor = pType->GetArmorClass();
				if (pArmor == NULL)
					return pCC->CreateNil();

				CInstalledArmor *pInstalled = NULL;
				if (Item.IsInstalled())
					pInstalled = pShip->GetArmorSection(Item.GetInstalled());

				return pCC->CreateBool(pArmor->IsRadiationImmune(pInstalled));
				}
			else
				return pCC->CreateBool(pShip->IsRadiationImmune());
			break;
			}

		case FN_SHIP_MAX_SPEED:
			return pCC->CreateInteger((int)((100.0 * pShip->GetMaxSpeed() / LIGHT_SPEED) + 0.5));

		default:
			ASSERT(false);
		}

	return pCC->CreateNil();
	}

ICCItem *fnShipGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnShipGetOld
//
//	Gets data about a ship

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a ship

	CShip *pShip = GetShipArg(pArgs->GetElement(0));
	if (pShip == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	No longer needed

	pArgs->Discard(pCC);

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SHIP_DEVICE_SLOT_AVAIL:
			pResult = pCC->CreateBool(pShip->IsDeviceSlotAvailable());
			break;

		case FN_SHIP_DIRECTION:
			pResult = pCC->CreateInteger(pShip->GetRotation());
			break;

		case FN_SHIP_FUEL:
			pResult = pCC->CreateInteger(pShip->GetFuelLeft());
			break;

		case FN_SHIP_HAS_AUTOPILOT:
			pResult = pCC->CreateBool(pShip->HasAutopilot());
			break;

		case FN_SHIP_INSTALL_AUTOPILOT:
			pShip->InstallAutopilot();
			pResult = pCC->CreateTrue();
			break;

		case FN_SHIP_HAS_TARGETING:
			pResult = pCC->CreateBool(pShip->HasTargetingComputer());
			break;

		case FN_SHIP_INSTALL_TARGETING:
			pShip->InstallTargetingComputer();
			pResult = pCC->CreateTrue();
			break;

		case FN_SHIP_DECONTAMINATE:
			pShip->Decontaminate();
			pResult = pCC->CreateTrue();
			break;

		case FN_SHIP_MAKE_RADIOACTIVE:
			pShip->MakeRadioactive();
			pResult = pCC->CreateTrue();
			break;

		case FN_SHIP_IS_RADIOACTIVE:
			pResult = pCC->CreateBool(pShip->IsRadioactive());
			break;

		case FN_SHIP_ARMOR_COUNT:
			pResult = pCC->CreateInteger(pShip->GetArmorSectionCount());
			break;

		case FN_SHIP_IS_SRS_ENHANCED:
			pResult = pCC->CreateBool(pShip->IsSRSEnhanced());
			break;

		case FN_SHIP_BLINDNESS:
			pResult = pCC->CreateBool(pShip->IsBlind());
			break;

		case FN_SHIP_FIX_BLINDNESS:
			pShip->ClearBlindness();
			pResult = pCC->CreateTrue();
			break;

		case FN_SHIP_ENHANCE_SRS:
			pShip->SetSRSEnhanced();
			pResult = pCC->CreateTrue();
			break;

		case FN_SHIP_CANCEL_ORDERS:
			pShip->GetController()->CancelAllOrders();
			pResult = pCC->CreateTrue();
			break;

		case FN_SHIP_CLASS:
			pResult = pCC->CreateInteger(pShip->GetClass()->GetUNID());
			break;

		case FN_SHIP_SHIELD_ITEM_UNID:
			{
			CInstalledDevice *pShield = pShip->GetNamedDevice(devShields);
			if (pShield == NULL)
				{
				pResult = pCC->CreateNil();
				break;
				}

			CItemType *pType = pShield->GetClass()->GetItemType();
			if (pType == NULL)
				{
				pResult = pCC->CreateNil();
				break;
				}

			pResult = pCC->CreateInteger(pType->GetUNID());
			break;
			}

		case FN_SHIP_SHIELD_DAMAGE:
			{
			CInstalledDevice *pShield = pShip->GetNamedDevice(devShields);
			if (pShield == NULL)
				pResult = pCC->CreateInteger(0);
			else
				{
				int iHP, iMaxHP;
				pShield->GetStatus(pShip, &iHP, &iMaxHP);
				pResult = pCC->CreateInteger(iMaxHP - iHP);
				}

			break;
			}

		case FN_SHIP_SHIELD_MAX_HP:
			{
			CInstalledDevice *pShield = pShip->GetNamedDevice(devShields);
			if (pShield == NULL)
				pResult = pCC->CreateInteger(0);
			else
				{
				int iHP, iMaxHP;
				pShield->GetStatus(pShip, &iHP, &iMaxHP);
				pResult = pCC->CreateInteger(iMaxHP);
				}

			break;
			}

		case FN_SHIP_ORDER:
			{
			IShipController *pController = pShip->GetController();
			if (pController)
				{
				CString sOrder = GetOrderName(pController->GetCurrentOrderEx());
				if (!sOrder.IsBlank())
					pResult = pCC->CreateString(sOrder);
				else
					pResult = pCC->CreateNil();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_SHIP_ORDER_TARGET:
			{
			IShipController *pController = pShip->GetController();
			if (pController)
				{
				CSpaceObject *pTarget;
				pController->GetCurrentOrderEx(&pTarget);
				if (pTarget)
					pResult = pCC->CreateInteger((int)pTarget);
				else
					pResult = pCC->CreateNil();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnShipSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnShipSet
//
//	Sets data about a ship

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Get the ship arg

	CShip *pShip = GetShipArg(pArgs->GetElement(0));
	if (pShip == NULL)
		return pCC->CreateNil();

	//	Do it

	switch (dwData)
		{
		case FN_SHIP_COMMAND_CODE:
			{
			if (pArgs->GetElement(1)->IsNil())
				pShip->SetCommandCode(NULL);
			else
				pShip->SetCommandCode(pArgs->GetElement(1));
			return pCC->CreateTrue();
			}

		case FN_SHIP_ORDER:
			{
			IShipController::OrderTypes iOrder = GetOrderType(pArgs->GetElement(1)->GetStringValue());
			CSpaceObject *pTarget = NULL;
			if (pArgs->GetElement(2))
				pTarget = GetObjArg(pArgs->GetElement(2));
			DWORD dwData = 0;
			if (pArgs->GetElement(3))
				dwData = (DWORD)pArgs->GetElement(3)->GetIntegerValue();

			if (iOrder != IShipController::orderNone)
				{
				pShip->GetController()->AddOrder(iOrder, pTarget, dwData);
				return pCC->CreateTrue();
				}
			else
				return pCC->CreateNil();
			}

		case FN_SHIP_ORDER_GOTO:
			{
			CSpaceObject *pObj = GetObjArg(pArgs->GetElement(1));

			if (pObj)
				{
				pShip->GetController()->AddOrder(IShipController::orderGoTo, pObj, 0);
				return pCC->CreateTrue();
				}
			else
				return pCC->CreateNil();
			}

		case FN_SHIP_REPAIR_ITEM:
			{
			CItemListManipulator *pItemList = NULL;
			CItemListManipulator ItemList(pShip->GetItemList());

			//	If the argument is a list then it is an item (which means we have to find
			//	the item in the manipulator). If the argument is an integer then we expect
			//	an item list manipulator pointer.

			if (pArgs->GetElement(1)->IsInteger())
				pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				if (ItemList.SetCursorAtItem(Item))
					pItemList = &ItemList;
				}

			if (pItemList == NULL)
				return pCC->CreateNil();
			else if (pItemList->GetItemAtCursor().IsDamaged())
				{
				pShip->EnhanceItem(*pItemList);
				return pCC->CreateTrue();
				}
			else
				return pCC->CreateNil();

			break;
			}

		default:
			ASSERT(false);
		}

	return pCC->CreateNil();
	}

ICCItem *fnShipSetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnShipSetOld
//
//	Sets data about a ship

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_SHIP_INSTALL_DEVICE || dwData == FN_SHIP_REMOVE_DEVICE 
			|| dwData == FN_SHIP_CAN_INSTALL_DEVICE || dwData == FN_SHIP_ITEM_CHARGES 
			|| dwData == FN_SHIP_CAN_INSTALL_ARMOR || dwData == FN_SHIP_CAN_REMOVE_DEVICE
			|| dwData == FN_SHIP_DAMAGE_ITEM)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iv"));
	else if (dwData == FN_SHIP_INSTALL_ARMOR)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ivi"));
	else if (dwData == FN_SHIP_ADD_ENERGY_FIELD)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iii"));
	else if (dwData == FN_SHIP_RECHARGE_ITEM)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ivi"));
	else if (dwData == FN_SHIP_FUEL_NEEDED 
			|| dwData == FN_SHIP_REFUEL_FROM_ITEM 
			|| dwData == FN_SHIP_ITEM_DEVICE_NAME
			|| dwData == FN_SHIP_IS_FUEL_COMPATIBLE)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("il"));
	else if (dwData == FN_SHIP_ORDER_GATE 
			|| dwData == FN_SHIP_ORDER_PATROL 
			|| dwData == FN_SHIP_ORDER_ESCORT 
			|| dwData == FN_SHIP_ORDER_FOLLOW 
			|| dwData == FN_SHIP_ORDER_MINE
			|| dwData == FN_SHIP_ORDER_HOLD
			|| dwData == FN_SHIP_CONTROLLER)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i*"));
	else if (dwData == FN_SHIP_ENHANCE_ITEM)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iv*"));
	else
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iv"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a ship

	CShip *pShip = GetShipArg(pArgs->GetElement(0));
	if (pShip == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SHIP_ADD_ENERGY_FIELD:
			{
			DWORD dwUNID = (DWORD)pArgs->GetElement(1)->GetIntegerValue();
			int iLifetime = pArgs->GetElement(2)->GetIntegerValue();
			pArgs->Discard(pCC);

			CEnergyFieldType *pField = g_pUniverse->FindShipEnergyFieldType(dwUNID);
			if (pField == NULL)
				return pCC->CreateError(CONSTLIT("Unknown ship energy field"), NULL);

			pShip->AddEnergyField(pField, iLifetime);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SHIP_FUEL:
			{
			pShip->ConsumeFuel(pArgs->GetElement(1)->GetIntegerValue());
			pResult = pCC->CreateInteger(pShip->GetFuelLeft());
			pArgs->Discard(pCC);
			break;
			}

		case FN_SHIP_CAN_INSTALL_ARMOR:
			{
			if (pArgs->GetElement(1)->IsInteger())
				{
				CItemListManipulator *pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
				pResult = pCC->CreateInteger((int)pShip->CanInstallArmor(*pItemList));
				pArgs->Discard(pCC);
				}
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				pResult = pCC->CreateInteger((int)pShip->CanInstallArmor(Item));
				pArgs->Discard(pCC);
				}
			break;
			}

		case FN_SHIP_CAN_INSTALL_DEVICE:
			{
			if (pArgs->GetElement(1)->IsInteger())
				{
				CItemListManipulator *pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
				pResult = pCC->CreateInteger((int)pShip->CanInstallDevice(*pItemList));
				pArgs->Discard(pCC);
				}
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				pResult = pCC->CreateInteger((int)pShip->CanInstallDevice(Item));
				pArgs->Discard(pCC);
				}
			break;
			}

		case FN_SHIP_CAN_REMOVE_DEVICE:
			{
			if (pArgs->GetElement(1)->IsInteger())
				{
				CItemListManipulator *pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
				pResult = pCC->CreateInteger((int)pShip->CanRemoveDevice(pItemList->GetItemAtCursor()));
				pArgs->Discard(pCC);
				}
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				pResult = pCC->CreateInteger((int)pShip->CanRemoveDevice(Item));
				pArgs->Discard(pCC);
				}
			break;
			}

		case FN_SHIP_INSTALL_DEVICE:
		case FN_SHIP_REMOVE_DEVICE:
			{
			CItemListManipulator *pItemList = NULL;
			CItemListManipulator ItemList(pShip->GetItemList());

			//	If the argument is a list then it is an item (which means we have to find
			//	the item in the manipulator). If the argument is an integer then we expect
			//	an item list manipulator pointer.

			if (pArgs->GetElement(1)->IsInteger())
				pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				if (ItemList.SetCursorAtItem(Item))
					pItemList = &ItemList;
				}

			pArgs->Discard(pCC);

			//	If no list, then we're done

			if (pItemList == NULL)
				{
				pResult = pCC->CreateNil();
				break;
				}

			//	Otherwise, install or remove

			if (dwData == FN_SHIP_INSTALL_DEVICE)
				pShip->InstallItemAsDevice(*pItemList);
			else
				pShip->RemoveItemAsDevice(*pItemList);

			//	Done

			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SHIP_INSTALL_ARMOR:
			{
			CItemListManipulator *pItemList = NULL;
			CItemListManipulator ItemList(pShip->GetItemList());

			//	If the argument is a list then it is an item (which means we have to find
			//	the item in the manipulator). If the argument is an integer then we expect
			//	an item list manipulator pointer.

			if (pArgs->GetElement(1)->IsInteger())
				pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				if (ItemList.SetCursorAtItem(Item))
					pItemList = &ItemList;
				}

			int iSegment = pArgs->GetElement(2)->GetIntegerValue();
			pArgs->Discard(pCC);

			if (pItemList)
				{
				pShip->InstallItemAsArmor(*pItemList, iSegment);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();

			break;
			}

		case FN_SHIP_FUEL_NEEDED:
		case FN_SHIP_REFUEL_FROM_ITEM:
			{
			CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
			pArgs->Discard(pCC);

			//	Do it

			if (dwData == FN_SHIP_FUEL_NEEDED)
				{
				//	Figure out how much fuel each item contains

				int iFuelPerItem = strToInt(Item.GetType()->GetData(), 0, NULL);
				if (iFuelPerItem == 0)
					iFuelPerItem = 1;

				int iFuelNeeded = pShip->GetMaxFuel() - pShip->GetFuelLeft();
				pResult = pCC->CreateInteger((iFuelNeeded / iFuelPerItem) + 1);
				}
			else
				{
				pShip->Refuel(Item);
				pResult = pCC->CreateTrue();
				}

			break;
			}

		case FN_SHIP_ITEM_DEVICE_NAME:
			{
			CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
			pArgs->Discard(pCC);
			pResult = pCC->CreateInteger(pShip->GetItemDeviceName(Item));
			break;
			}

		case FN_SHIP_IS_FUEL_COMPATIBLE:
			{
			CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
			pArgs->Discard(pCC);
			pResult = pCC->CreateBool(pShip->IsFuelCompatible(Item));
			break;
			}

		case FN_SHIP_ORDER_ATTACK:
			{
			CSpaceObject *pObj = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			if (pObj)
				{
				pShip->GetController()->AddOrder(IShipController::orderDestroyTarget, pObj, 0);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_SHIP_ORDER_DOCK:
			{
			CSpaceObject *pObj = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			if (pObj)
				{
				pShip->GetController()->AddOrder(IShipController::orderDock, pObj, 0);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_SHIP_ORDER_ESCORT:
			{
			CSpaceObject *pObj = GetObjArg(pArgs->GetElement(1));
			DWORD dwFormation = 0;
			if (pArgs->GetCount() > 2)
				dwFormation = pArgs->GetElement(2)->GetIntegerValue();
			pArgs->Discard(pCC);

			if (pObj)
				{
				pShip->GetController()->AddOrder(IShipController::orderEscort, pObj, dwFormation);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_SHIP_ORDER_FOLLOW:
			{
			CSpaceObject *pObj = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			if (pObj)
				{
				pShip->GetController()->AddOrder(IShipController::orderFollow, pObj, 0);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_SHIP_ORDER_GATE:
			{
			CSpaceObject *pGate = NULL;
			if (pArgs->GetCount() == 2)
				pGate = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			//	Note: OK if pGate == NULL.
			pShip->GetController()->AddOrder(IShipController::orderGate, pGate, 0);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SHIP_ORDER_GUARD:
			{
			CSpaceObject *pObj = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			if (pObj)
				{
				pShip->GetController()->AddOrder(IShipController::orderGuard, pObj, 0);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_SHIP_ORDER_LOOT:
			{
			CSpaceObject *pObj = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			if (pObj)
				{
				pShip->GetController()->AddOrder(IShipController::orderLoot, pObj, 0);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_SHIP_ORDER_MINE:
			{
			CSpaceObject *pObj = GetObjArg(pArgs->GetElement(1));
			pArgs->Discard(pCC);

			if (pObj)
				{
				pShip->GetController()->AddOrder(IShipController::orderMine, pObj, 0);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_SHIP_ORDER_PATROL:
			{
			if (pArgs->GetCount() == 3)
				{
				CSpaceObject *pCenter = GetObjArg(pArgs->GetElement(1));
				int iRadius = pArgs->GetElement(2)->GetIntegerValue();
				pArgs->Discard(pCC);

				if (pCenter)
					{
					pShip->GetController()->AddOrder(IShipController::orderPatrol, pCenter, iRadius);
					pResult = pCC->CreateTrue();
					}
				else
					pResult = pCC->CreateNil();
				}
			else
				pResult = pCC->CreateError(CONSTLIT("Not enough parameters for shpOrderPatrol"), NULL);

			break;
			}

		case FN_SHIP_ORDER_WAIT:
			{
			int iWaitTime = pArgs->GetElement(1)->GetIntegerValue();
			pArgs->Discard(pCC);
			pShip->GetController()->AddOrder(IShipController::orderWait, NULL, iWaitTime);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SHIP_ORDER_HOLD:
			{
			int iWaitTime = 0;
			if (pArgs->GetCount() >= 2)
				iWaitTime = pArgs->GetElement(1)->GetIntegerValue();
			pArgs->Discard(pCC);
			pShip->GetController()->AddOrder(IShipController::orderHold, NULL, iWaitTime);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SHIP_BLINDNESS:
			{
			int iBlindTime = pArgs->GetElement(1)->GetIntegerValue();
			pArgs->Discard(pCC);
			pShip->MakeBlind(iBlindTime);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SHIP_DAMAGE_ITEM:
			{
			CItemListManipulator *pItemList = NULL;
			CItemListManipulator ItemList(pShip->GetItemList());

			//	If the argument is a list then it is an item (which means we have to find
			//	the item in the manipulator). If the argument is an integer then we expect
			//	an item list manipulator pointer.

			if (pArgs->GetElement(1)->IsInteger())
				pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				if (ItemList.SetCursorAtItem(Item))
					pItemList = &ItemList;
				}

			if (pItemList == NULL)
				{
				pArgs->Discard(pCC);
				pResult = pCC->CreateNil();
				}
			else if (!pItemList->GetItemAtCursor().IsDamaged())
				{
				pArgs->Discard(pCC);
				pShip->DamageItem(*pItemList);
				pResult = pCC->CreateTrue();
				}
			else
				{
				pArgs->Discard(pCC);
				pResult = pCC->CreateNil();
				}
			break;
			}

		case FN_SHIP_CONTROLLER:
			{
			CString sController;
			if (pArgs->GetCount() > 1)
				sController = pArgs->GetElement(1)->GetStringValue();
			IShipController *pController = CreateShipController(sController);
			pArgs->Discard(pCC);

			if (pController)
				{
				pShip->SetController(pController);
				pResult = pCC->CreateTrue();
				}
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_SHIP_ENHANCE_ITEM:
			{
			CItemListManipulator *pItemList = NULL;
			CItemListManipulator ItemList(pShip->GetItemList());

			//	If the argument is a list then it is an item (which means we have to find
			//	the item in the manipulator). If the argument is an integer then we expect
			//	an item list manipulator pointer.

			if (pArgs->GetElement(1)->IsInteger())
				pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				if (ItemList.SetCursorAtItem(Item))
					pItemList = &ItemList;
				}

			if (pItemList == NULL)
				{
				pArgs->Discard(pCC);
				pResult = pCC->CreateNil();
				}
			else if (pItemList->GetItemAtCursor().GetType()->IsArmor())
				{
				DWORD dwMods = 0;
				if (pArgs->GetCount() > 2)
					dwMods = (DWORD)pArgs->GetElement(2)->GetIntegerValue();

				EnhanceItemStatus iResult = pShip->EnhanceItem(*pItemList, dwMods);
				pArgs->Discard(pCC);
				pResult = pCC->CreateInteger(iResult);
				}
			else if (pItemList->GetItemAtCursor().GetType()->IsDevice())
				{
				DWORD dwMods = etBinaryEnhancement;
				if (pArgs->GetCount() > 2)
					dwMods = (DWORD)pArgs->GetElement(2)->GetIntegerValue();

				EnhanceItemStatus iResult = pShip->EnhanceItem(*pItemList, dwMods);
				pArgs->Discard(pCC);
				pResult = pCC->CreateInteger(iResult);
				}
			else
				{
				pShip->EnhanceItem(*pItemList);
				pArgs->Discard(pCC);
				pResult = pCC->CreateTrue();
				}
			break;
			}

		case FN_SHIP_RECHARGE_SHIELD:
			{
			int iHP = pArgs->GetElement(1)->GetIntegerValue();
			pArgs->Discard(pCC);

			CInstalledDevice *pShield = pShip->GetNamedDevice(devShields);
			if (pShield == NULL)
				pResult = pCC->CreateNil();
			else
				{
				pShield->Recharge(pShip, iHP);
				pResult = pCC->CreateTrue();
				}
			break;
			}

		case FN_SHIP_ITEM_CHARGES:
			{
			CItemListManipulator *pItemList = NULL;
			CItemListManipulator ItemList(pShip->GetItemList());

			//	If the argument is a list then it is an item (which means we have to find
			//	the item in the manipulator). If the argument is an integer then we expect
			//	an item list manipulator pointer.

			if (pArgs->GetElement(1)->IsInteger())
				pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				if (ItemList.SetCursorAtItem(Item))
					pItemList = &ItemList;
				}

			pArgs->Discard(pCC);

			//	If no list, then we're done

			if (pItemList == NULL)
				{
				pResult = pCC->CreateNil();
				break;
				}

			//	Otherwise, return the charges for this item

			pResult = pCC->CreateInteger(pItemList->GetItemAtCursor().GetCharges());
			break;
			}

		case FN_SHIP_RECHARGE_ITEM:
			{
			CItemListManipulator *pItemList = NULL;
			CItemListManipulator ItemList(pShip->GetItemList());
			
			//	If the argument is a list then it is an item (which means we have to find
			//	the item in the manipulator). If the argument is an integer then we expect
			//	an item list manipulator pointer.

			if (pArgs->GetElement(1)->IsInteger())
				pItemList = (CItemListManipulator *)pArgs->GetElement(1)->GetIntegerValue();
			else
				{
				CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
				if (ItemList.SetCursorAtItem(Item))
					pItemList = &ItemList;
				}

			int iCharges = pArgs->GetElement(2)->GetIntegerValue();

			//	If no list, then we're done

			if (pItemList == NULL)
				{
				pResult = pCC->CreateNil();
				break;
				}

			//	Recharge

			pShip->RechargeItem(*pItemList, iCharges);
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnSovereignSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnSovereignSet
//
//	(sovSetDisposition sovereignID targetSovereignID disposition)

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Get the sovereign

	DWORD dwSovereignID = pArgs->GetElement(0)->GetIntegerValue();
	CSovereign *pSovereign = g_pUniverse->FindSovereign(dwSovereignID);
	if (pSovereign == NULL)
		return pCC->CreateNil();

	//	Handle the command

	switch (dwData)
		{
		case FN_SOVEREIGN_DISPOSITION:
			{
			//	Get the target sovereign and disposition

			DWORD dwTargetID = pArgs->GetElement(1)->GetIntegerValue();
			CSovereign *pTarget = g_pUniverse->FindSovereign(dwTargetID);
			if (pTarget == NULL)
				return pCC->CreateError(CONSTLIT("Invalid sovereign"), pArgs->GetElement(1));
			
			CSovereign::Disposition iDisp;
			if (pArgs->GetElement(2)->IsInteger())
				iDisp = (CSovereign::Disposition)pArgs->GetElement(2)->GetIntegerValue();
			else
				{
				CString sValue = pArgs->GetElement(2)->GetStringValue();
				if (strEquals(sValue, DISP_NEUTRAL))
					iDisp = CSovereign::dispNeutral;
				else if (strEquals(sValue, DISP_ENEMY))
					iDisp = CSovereign::dispEnemy;
				else if (strEquals(sValue, DISP_FRIEND))
					iDisp = CSovereign::dispFriend;
				else
					return pCC->CreateError(CONSTLIT("Invalid disposition"), pArgs->GetElement(2));
				}

			//	Do it

			pSovereign->SetDispositionTowards(pTarget, iDisp);
			return pCC->CreateTrue();
			}

		default:
			ASSERT(false);
		}

	return pCC->CreateNil();
	}

ICCItem *fnStationGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnStationGet
//
//	Gets data about a station

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Convert the first argument into a station

	CStation *pStation = GetStationArg(pArgs->GetElement(0));
	if (pStation == NULL)
		return pCC->CreateNil();

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_STATION_MAX_STRUCTURAL_HP:
			return pCC->CreateInteger(pStation->GetMaxStructuralHitPoints());

		default:
			ASSERT(FALSE);
		}

	return pCC->CreateNil();
	}

ICCItem *fnStationGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnStationGetOld
//
//	Gets data about a station

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a station

	CStation *pStation = GetStationArg(pArgs->GetElement(0));
	if (pStation == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	pArgs->Discard(pCC);

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_STATION_SET_ACTIVE:
			{
			pStation->SetActive();
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_STATION_SET_INACTIVE:
			{
			pStation->SetInactive();
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_STATION_GET_DOCKED_SHIPS:
			{
			pResult = pCC->CreateLinkedList();
			if (pResult->IsError())
				return pResult;

			CCLinkedList *pList = (CCLinkedList *)pResult;

			//	Add each docked ship

			for (int i = 0; i < pStation->GetDockingPortCount(); i++)
				{
				CSpaceObject *pShip = pStation->GetShipAtDockingPort(i);
				if (pShip)
					{
					ICCItem *pInt = pCC->CreateInteger((int)pShip);
					pList->Append(pCC, pInt, NULL);
					pInt->Discard(pCC);
					}
				}

			break;
			}

		case FN_STATION_GET_SUBORDINATES:
			{
			pResult = pCC->CreateLinkedList();
			if (pResult->IsError())
				return pResult;

			CCLinkedList *pList = (CCLinkedList *)pResult;

			//	Add each subordinate

			for (int i = 0; i < pStation->GetSubordinateCount(); i++)
				{
				CSpaceObject *pShip = pStation->GetSubordinate(i);
				if (pShip)
					{
					ICCItem *pInt = pCC->CreateInteger((int)pShip);
					pList->Append(pCC, pInt, NULL);
					pInt->Discard(pCC);
					}
				}

			break;
			}

		case FN_STATION_GET_TYPE:
			{
			pResult = pCC->CreateInteger(pStation->GetType()->GetUNID());
			break;
			}

		case FN_STATION_RECON:
			{
			pResult = pCC->CreateBool(pStation->IsReconned());
			break;
			}

		case FN_STATION_CLEAR_RECON:
			{
			pStation->ClearReconned();
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_STATION_CLEAR_FIRE_RECON:
			{
			pStation->ClearFireReconEvent();
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_STATION_SET_FIRE_RECON:
			{
			pStation->SetFireReconEvent();
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_STATION_IMAGE_VARIANT:
			{
			pResult = pCC->CreateInteger(pStation->GetImageVariant());
			break;
			}

		case FN_STATION_STRUCTURAL_HP:
			{
			pResult = pCC->CreateInteger(pStation->GetStructuralHitPoints());
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnStationSet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnStationSet
//
//	Sets data about a station

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ii"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a station

	CStation *pStation = GetStationArg(pArgs->GetElement(0));
	if (pStation == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_STATION_IMAGE_VARIANT:
			{
			pStation->SetImageVariant(pArgs->GetElement(1)->GetIntegerValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_STATION_STRUCTURAL_HP:
			{
			pStation->SetStructuralHitPoints(pArgs->GetElement(1)->GetIntegerValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnStationType (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnStationType
//
//	Sets data about a station type

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pResult;

	//	Convert the first argument into a station type

	DWORD dwUNID = pArgs->GetElement(0)->GetIntegerValue();
	CStationType *pType = g_pUniverse->FindStationType(dwUNID);
	if (pType == NULL)
		return pCC->CreateError(CONSTLIT("station type expected:"), pArgs->GetElement(0));

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_STATION_ENCOUNTERED:
			{
			pResult = pCC->CreateBool(!pType->CanBeEncountered());
			break;
			}

		case FN_STATION_GET_GLOBAL_DATA:
			{
			CString sAttrib = pArgs->GetElement(1)->GetStringValue();
			CString sData = pType->GetGlobalData(sAttrib);
			pResult = pCC->Link(sData, 0, NULL);
			break;
			}

		case FN_STATION_SET_GLOBAL_DATA:
			{
			CString sAttrib = pArgs->GetElement(1)->GetStringValue();

			//	Set quoted before we unlink

			BOOL bOldQuoted = pArgs->GetElement(2)->IsQuoted();
			pArgs->GetElement(2)->SetQuoted();
			CString sData = pCC->Unlink(pArgs->GetElement(2));
			if (!bOldQuoted)
				pArgs->GetElement(2)->ClearQuoted();

			pType->SetGlobalData(sAttrib, sData);
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnSystemAddEncounterEvent (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnSystemAddEncounterEvent
//
//	Adds an encounter

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iiii"));
	if (pArgs->IsError())
		return pArgs;

	//	Arguments

	int iTime = pArgs->GetElement(0)->GetIntegerValue();
	CSpaceObject *pTarget = GetObjArg(pArgs->GetElement(1));
	if (pTarget == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	DWORD dwEncounterID = (DWORD)pArgs->GetElement(2)->GetIntegerValue();

	//	Either from a gate or from a distance from the target

	int iDistance = 0;
	CSpaceObject *pGate = NULL;

	if (dwData == FN_ADD_ENCOUNTER_FROM_DIST)
		iDistance =	pArgs->GetElement(3)->GetIntegerValue();
	else
		pGate = GetObjArg(pArgs->GetElement(3));
	
	pArgs->Discard(pCC);

	//	Create the event

	CSystem *pSystem = pTarget->GetSystem();

	CTimedEncounterEvent *pEvent = new CTimedEncounterEvent(
			pSystem->GetTick() + iTime,
			pTarget,
			dwEncounterID,
			pGate,
			iDistance * LIGHT_SECOND);

	pSystem->AddTimedEvent(pEvent);

	return pCC->CreateTrue();
	}

ICCItem *fnSystemAddStationTimerEvent (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnSystemAddStationTimerEvent
//
//	Adds an station timer

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_ADD_TIMER_NORMAL:
		case FN_ADD_TIMER_RECURRING:
			{
			int iTime = pArgs->GetElement(0)->GetIntegerValue();
			CSpaceObject *pTarget = (CSpaceObject *)GetObjArg(pArgs->GetElement(1));
			if (pTarget == NULL)
				return pCC->CreateNil();
			CString sEvent = pArgs->GetElement(2)->GetStringValue();
			CSystem *pSystem = pTarget->GetSystem();

			CTimedEvent *pEvent;
			if (dwData == FN_ADD_TIMER_NORMAL)
				pEvent = new CTimedCustomEvent(
						pSystem->GetTick() + iTime,
						pTarget,
						sEvent);
			else
				pEvent = new CTimedRecurringEvent(
						iTime,
						pTarget,
						sEvent);

			pSystem->AddTimedEvent(pEvent);

			return pCC->CreateTrue();
			}

		case FN_CANCEL_TIMER:
			{
			CSpaceObject *pTarget = (CSpaceObject *)GetObjArg(pArgs->GetElement(0));
			if (pTarget == NULL)
				return pCC->CreateNil();
			CString sEvent = pArgs->GetElement(1)->GetStringValue();
			
			pTarget->GetSystem()->CancelTimedEvent(pTarget, sEvent);

			return pCC->CreateTrue();
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnSystemCreate (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnSystemCreate
//
//	Creates an object
//
//	(sysCreateWeaponFire weaponID objSource posVector dir speed objTarget) -> obj

	{
	ALERROR error;
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_SYS_CREATE_WEAPON_FIRE:
			{
			CSystem *pSystem = g_pUniverse->GetCurrentSystem();

			//	Get parameters

			CWeaponFireDesc *pDesc = GetWeaponFireDescArg(pArgs->GetElement(0));
			if (pDesc == NULL)
				return pCC->CreateNil();

			CSpaceObject *pSource = GetObjArg(pArgs->GetElement(1));
			CVector vPos = CreateVectorFromList(*pCC, pArgs->GetElement(2));
			int iDir = pArgs->GetElement(3)->GetIntegerValue();
			iDir = Max(0, iDir) % 360;
			Metric rSpeed = LIGHT_SPEED * pArgs->GetElement(4)->GetIntegerValue() / 100.0;
			CSpaceObject *pTarget = GetObjArg(pArgs->GetElement(5));
			bool bDetonateNow = ((pArgs->GetCount() > 6) ? !pArgs->GetElement(6)->IsNil() : false);

			//	Create the weapon shot

			CSpaceObject *pObj;
			if (error = pSystem->CreateWeaponFire(pDesc,
					0,
					killedByDamage,
					CDamageSource(pSource),
					vPos,
					PolarToVector(iDir, rSpeed),
					iDir,
					pTarget,
					&pObj))
				return pCC->CreateNil();

			//	Detonate the shot

			if (bDetonateNow)
				pObj->DetonateNow(NULL);

			//	DOne

			return pCC->CreateInteger((int)pObj);
			}

		default:
			ASSERT(false);
		}

	return pCC->CreateNil();
	}

ICCItem *fnSystemCreateEffect (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnSystemCreateEffect
//
//	(sysCreateEffect effectID anchor pos)

	{
	ALERROR error;

	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ivl"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the arguments

	DWORD dwUNID = pArgs->GetElement(0)->GetIntegerValue();

	CSpaceObject *pAnchor = NULL;
	if (!pArgs->GetElement(1)->IsNil())
		pAnchor = GetObjArg(pArgs->GetElement(1));

	CVector vPos(CreateVectorFromList(*pCC, pArgs->GetElement(2)));

	//	Validate

	CEffectCreator *pCreator = g_pUniverse->FindEffectType(dwUNID);
	if (pCreator == NULL)
		return pCC->CreateError(CONSTLIT("Unknown effect ID"), pArgs->GetElement(0));

	pArgs->Discard(pCC);

	//	Create

	CSystem *pSystem = g_pUniverse->GetCurrentSystem();
	if (error = pCreator->CreateEffect(	pSystem,
			pAnchor,
			vPos,
			NullVector))
		return pCC->CreateError(CONSTLIT("Error creating effect"), pCC->CreateInteger(error));

	//	Done

	return pCC->CreateTrue();
	}

ICCItem *fnSystemCreateMarker (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnSystemCreateMarker
//
//	(sysCreateMarker name pos sovereignID) -> marker

	{
	ALERROR error;

	CCodeChain *pCC = pEvalCtx->pCC;

	//	Evaluate the arguments and validate them

	ICCItem *pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("svi"));
	if (pArgs->IsError())
		return pArgs;

	CString sName = pArgs->GetElement(0)->GetStringValue();

	//	The position can be either a list (in which case it is a position)
	//	or an integer (in which case it is a gate object)

	CVector vPos = CreateVectorFromList(*pCC, pArgs->GetElement(1));

	//	Sovereign

	DWORD dwSovereignID = pArgs->GetElement(2)->GetIntegerValue();
	CSovereign *pSovereign = g_pUniverse->FindSovereign(dwSovereignID);

	pArgs->Discard(pCC);

	//	Create

	CSystem *pSystem = g_pUniverse->GetCurrentSystem();
	CMarker *pObj;
	if (error = CMarker::Create(pSystem,
			pSovereign,
			vPos,
			NullVector,
			sName,
			&pObj))
		return pCC->CreateError(CONSTLIT("Error creating marker"), pCC->CreateInteger(error));

	//	Done

	return pCC->CreateInteger((int)pObj);
	}

ICCItem *fnSystemCreateShip (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnSystemCreateShip
//
//	(sysCreateShip classID pos sovereignID [controller]) -> ship

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ivi*"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the arguments

	DWORD dwClassID = pArgs->GetElement(0)->GetIntegerValue();
	DWORD dwSovereignID = pArgs->GetElement(2)->GetIntegerValue();

	//	The position can be either a list (in which case it is a position)
	//	or an integer (in which case it is a gate object)

	CVector vPos;
	CSpaceObject *pGate = NULL;
	if (pArgs->GetElement(1)->IsList())
		vPos = CreateVectorFromList(*pCC, pArgs->GetElement(1));
	else
		{
		pGate = GetObjArg(pArgs->GetElement(1));
		if (pGate)
			vPos = pGate->GetPos();
		}

	//	Controller

	IShipController *pController = NULL;
	if (pArgs->GetCount() > 3)
		pController = CreateShipController(pArgs->GetElement(3)->GetStringValue());

	//	Validate

	CShipClass *pClass = g_pUniverse->FindShipClass(dwClassID);
	if (pClass == NULL)
		return pCC->CreateError(CONSTLIT("Unknown ship class ID"), pArgs->GetElement(0));

	CSovereign *pSovereign = g_pUniverse->FindSovereign(dwSovereignID);
	if (pSovereign == NULL)
		return pCC->CreateError(CONSTLIT("Unknown sovereign ID"), pArgs->GetElement(2));

	pArgs->Discard(pCC);

	//	Create

	CSystem *pSystem = g_pUniverse->GetCurrentSystem();
	CShip *pShip;
	ALERROR error;
	if (error = pSystem->CreateShip(dwClassID,
				pController,
				pSovereign,
				vPos,
				CVector(),
				mathRandom(0, 359),
				pGate,
				&pShip))
		return pCC->CreateError(CONSTLIT("Error creating ship"), pCC->CreateInteger(error));

	//	Done

	return pCC->CreateInteger((int)pShip);
	}

ICCItem *fnSystemCreateStation (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnSystemCreateStation
//
//	(sysCreateStation classID pos) -> station

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("il"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the arguments

	DWORD dwClassID = pArgs->GetElement(0)->GetIntegerValue();
	CVector vPos(CreateVectorFromList(*pCC, pArgs->GetElement(1)));
	CSystem *pSystem = g_pUniverse->GetCurrentSystem();

	//	Validate

	CStationType *pType = g_pUniverse->FindStationType(dwClassID);
	if (pType == NULL)
		return pCC->CreateError(CONSTLIT("Unknown station type ID"), pArgs->GetElement(0));

	pArgs->Discard(pCC);

	//	Make sure we can encounter the station

	if (!pType->CanBeEncountered(pSystem))
		return pCC->CreateError(CONSTLIT("Unique station already encountered"), NULL);

	//	Create the station (or ship encounter)

	CSpaceObject *pStation;
	ALERROR error;
	CString sError;
	if ((error = pSystem->CreateStation(pType, vPos, &pStation)))
		return pCC->CreateError(CONSTLIT("Unable to create station"), NULL);

	//	Done

	if (pStation)
		return pCC->CreateInteger((int)pStation);
	else
		return pCC->CreateNil();
	}

ICCItem *fnSystemFind (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnSystemFind
//
//	Finds an object in the system
//
//	Filter:
//
//		G			Stargates only
//		s			Include ships
//		t			Include stations (including planets)
//		T			Include structure-scale stations
//		T:xyz;		Include stations with attribute 'xyz'
//
//		A			Active objects only (i.e., objects that can attack)
//		B:xyz;		Only objects with attribute 'xyz'
//		D:xyz;		Only objects with data 'xyz'
//		E			Enemy objects only
//		F			Friendly objects only
//		M			Manufactured objects only (i.e., no planets or asteroids)
//		N			Return only the nearest object to the source
//		N:nn;		Return only objects within nn light-seconds
//		O:escort;	Ships ordered to escort source
//		R:nn;		Return only objects greater than nn light-seconds away

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pResult;

	//	Convert the first argument into an object (may be Nil)

	CSpaceObject *pSource = GetObjArg(pArgs->GetElement(0));

	//	Second argument is the filter

	CString sFilter = pArgs->GetElement(1)->GetStringValue();
	CSpaceObject::Criteria Criteria;
	CSpaceObject::ParseCriteria(pSource, sFilter, &Criteria);

	//	Prepare result list
	
	pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Do the search

	int i;
	CSystem *pSystem = g_pUniverse->GetCurrentSystem();
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj && pObj->MatchesCriteria(Criteria))
			{
			ICCItem *pInt = pCC->CreateInteger((int)pObj);
			if (pInt->IsError())
				{
				pResult->Discard(pCC);
				return pInt;
				}

			ICCItem *pError;
			pList->Append(pCC, pInt, &pError);
			pInt->Discard(pCC);

			if (pError->IsError())
				{
				pResult->Discard(pCC);
				return pError;
				}
			}
		}

	//	If we only want the nearest/farthest object, then find it now

	if (Criteria.bSingleObj)
		{
		CSpaceObject *pBestObj = NULL;
		Metric rBestDist2;
		if (Criteria.bNearestOnly)
			rBestDist2 = g_InfiniteDistance * g_InfiniteDistance;
		else
			rBestDist2 = 0.0;

		for (i = 0; i < pList->GetCount(); i++)
			{
			CSpaceObject *pObj = (CSpaceObject *)pList->GetElement(i)->GetIntegerValue();
			CVector vDist = pObj->GetPos() - pSource->GetPos();
			Metric rDist2 = vDist.Length2();

			if ((Criteria.bNearestOnly && rDist2 < rBestDist2)
					|| (Criteria.bFarthestOnly && rDist2 > rBestDist2))
				{
				pBestObj = pObj;
				rBestDist2 = rDist2;
				}
			}

		//	Done. Return the object (or Nil)

		pResult->Discard(pCC);

		if (pBestObj)
			pResult = pCC->CreateInteger((int)pBestObj);
		else
			pResult = pCC->CreateNil();
		}

	return pResult;
	}

ICCItem *fnSystemGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnSystemGet
//
//	(sysGetName)

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_SYS_ADD_STARGATE_TOPOLOGY:
			{
			CTopologyNode *pNode;
			int iArg = 0;

			//	If we have more than one arg, then the first arg is
			//	the node ID.

			if (pArgs->GetCount() == 3)
				{
				CSystem *pSystem = g_pUniverse->GetCurrentSystem();
				if (pSystem == NULL)
					return pCC->CreateError(CONSTLIT("No current system"), pArgs);

				pNode = pSystem->GetTopology();
				}
			else
				pNode = g_pUniverse->FindTopologyNode(pArgs->GetElement(iArg++)->GetStringValue());

			if (pNode == NULL)
				return pCC->CreateError(CONSTLIT("Invalid nodeID"), pArgs->GetElement(0));

			CString sGateID = pArgs->GetElement(iArg++)->GetStringValue();
			CString sDestNodeID = pArgs->GetElement(iArg++)->GetStringValue();
			CString sDestGateID = pArgs->GetElement(iArg++)->GetStringValue();

			if (pNode->FindStargate(sGateID))
				return pCC->CreateNil();

			if (pNode->AddStargate(sGateID, sDestNodeID, sDestGateID) != NOERROR)
				return pCC->CreateError(CONSTLIT("Unable to add stargate"), pArgs);

			return pCC->CreateTrue();
			}

		case FN_SYS_CREATE_STARGATE:
			{
			CSystem *pSystem = g_pUniverse->GetCurrentSystem();
			if (pSystem == NULL)
				return pCC->CreateError(CONSTLIT("No current system"), pArgs);

			DWORD dwClassID = pArgs->GetElement(0)->GetIntegerValue();
			CStationType *pType = g_pUniverse->FindStationType(dwClassID);
			if (pType == NULL)
				return pCC->CreateError(CONSTLIT("Invalid station type"), pArgs->GetElement(0));

			CVector vPos(CreateVectorFromList(*pCC, pArgs->GetElement(1)));
			CString sGateID = pArgs->GetElement(2)->GetStringValue();
			CString sDestNodeID = pArgs->GetElement(3)->GetStringValue();
			CString sDestGateID = pArgs->GetElement(4)->GetStringValue();

			CSpaceObject *pNewGate;
			if (pSystem->CreateStargate(pType, vPos, sGateID, sDestNodeID, sDestGateID, &pNewGate) != NOERROR)
				return pCC->CreateError(CONSTLIT("Unable to create gate"), pArgs);

			return pCC->CreateInteger((int)pNewGate);			
			}

		case FN_SYS_ENVIRONMENT:
			{
			CSystem *pSystem = g_pUniverse->GetCurrentSystem();
			CVector vPos = CreateVectorFromList(*pCC, pArgs->GetElement(0));
			CSpaceEnvironmentType *pEnvironment = pSystem->GetSpaceEnvironment(vPos);
			if (pEnvironment)
				return pCC->CreateInteger(pEnvironment->GetUNID());
			else
				return pCC->CreateNil();
			}

		case FN_SYS_GET_TRAVEL_TIME:
			{
			Metric rDistance = pArgs->GetElement(0)->GetIntegerValue();
			if (rDistance < 0.0)
				return pCC->CreateError(CONSTLIT("Distance cannot be negative"), pArgs->GetElement(0));

			Metric rSpeed = g_SecondsPerUpdate * pArgs->GetElement(1)->GetIntegerValue() / 100.0;
			if (rSpeed < 0.0)
				return pCC->CreateError(CONSTLIT("Speed cannot be negative"), pArgs->GetElement(1));
			else if (rSpeed == 0.0)
				return pCC->CreateNil();

			return pCC->CreateInteger((int)((rDistance / rSpeed) + 0.5));
			}

		case FN_SYS_GET_DATA:
		case FN_SYS_SET_DATA:
			{
			CTopologyNode *pNode;
			CString sAttrib;
			int iArg = 0;

			//	If we have more than one arg, then the first arg is
			//	the node ID.

			if ((pArgs->GetCount() == 1 && dwData == FN_SYS_GET_DATA)
					|| (pArgs->GetCount() == 2 && dwData == FN_SYS_SET_DATA))
				{
				CSystem *pSystem = g_pUniverse->GetCurrentSystem();
				if (pSystem == NULL)
					return pCC->CreateNil();

				pNode = pSystem->GetTopology();
				}
			else
				pNode = g_pUniverse->FindTopologyNode(pArgs->GetElement(iArg++)->GetStringValue());

			//	Node better be valid

			if (pNode == NULL)
				return pCC->CreateNil();

			//	Get the attribute

			sAttrib = pArgs->GetElement(iArg++)->GetStringValue();

			//	Do it

			if (dwData == FN_SYS_GET_DATA)
				return pCC->Link(pNode->GetData(sAttrib), 0, NULL);
			else
				{
				CString sData;

				//	Set quoted before we unlink
				//	Note: This might be a hack...it probably makes more sense to mark
				//	all function return values as 'quoted'

				BOOL bOldQuoted = pArgs->GetElement(iArg)->IsQuoted();
				pArgs->GetElement(iArg)->SetQuoted();
				sData = pCC->Unlink(pArgs->GetElement(iArg));
				if (!bOldQuoted)
					pArgs->GetElement(iArg)->ClearQuoted();

				pNode->SetData(sAttrib, sData);
				return pCC->CreateTrue();
				}
			}

		case FN_SYS_LEVEL:
			{
			if (pArgs->GetCount() == 0)
				{
				CSystem *pSystem = g_pUniverse->GetCurrentSystem();
				return pCC->CreateInteger(pSystem->GetLevel());
				}
			else
				return fnTopologyGet(pEvalCtx, pArgs, FN_NODE_LEVEL);
			}

		case FN_SYS_NAME:
			{
			if (pArgs->GetCount() == 0)
				{
				CSystem *pSystem = g_pUniverse->GetCurrentSystem();
				return pCC->CreateString(pSystem->GetName());
				}
			else
				return fnTopologyGet(pEvalCtx, pArgs, FN_NODE_SYSTEM_NAME);
			}

		case FN_SYS_SYSTEM_TYPE:
			{
			CTopologyNode *pNode;
			if (pArgs->GetCount() == 0)
				pNode = g_pUniverse->GetCurrentSystem()->GetTopology();
			else
				pNode = g_pUniverse->FindTopologyNode(pArgs->GetElement(0)->GetStringValue());
			if (pNode == NULL)
				return pCC->CreateError(CONSTLIT("Invalid nodeID"), pArgs->GetElement(0));

			return pCC->CreateInteger(pNode->GetSystemDescUNID());
			}

		case FN_SYS_STARGATES:
			{
			CTopologyNode *pNode;
			if (pArgs->GetCount() == 0)
				pNode = g_pUniverse->GetCurrentSystem()->GetTopology();
			else
				pNode = g_pUniverse->FindTopologyNode(pArgs->GetElement(0)->GetStringValue());
			if (pNode == NULL)
				return pCC->CreateError(CONSTLIT("Invalid nodeID"), pArgs->GetElement(0));

			//	Create a list

			ICCItem *pResult = pCC->CreateLinkedList();
			if (pResult->IsError())
				return pResult;

			CCLinkedList *pList = (CCLinkedList *)pResult;

			//	Loop over all stargates in this node

			for (int i = 0; i < pNode->GetStargateCount(); i++)
				{
				ICCItem *pValue = pCC->CreateString(pNode->GetStargate(i));
				pList->Append(pCC, pValue, NULL);
				pValue->Discard(pCC);
				}

			return pResult;
			}

		case FN_SYS_STARGATE_DESTINATION_NODE:
			{
			CString sGateID;
			CTopologyNode *pNode;
			if (pArgs->GetCount() == 2)
				{
				pNode = g_pUniverse->FindTopologyNode(pArgs->GetElement(0)->GetStringValue());
				sGateID = pArgs->GetElement(1)->GetStringValue();
				}
			else
				{
				pNode = g_pUniverse->GetCurrentSystem()->GetTopology();
				sGateID = pArgs->GetElement(0)->GetStringValue();
				}

			if (pNode == NULL)
				return pCC->CreateError(CONSTLIT("Invalid nodeID"), pArgs->GetElement(0));

			CString sDestNode;
			pNode->FindStargate(sGateID, &sDestNode);

			return pCC->CreateString(sDestNode);
			}

		case FN_SYS_HAS_ATTRIBUTE:
			{
			if (pArgs->GetCount() == 1)
				{
				CSystem *pSystem = g_pUniverse->GetCurrentSystem();
				return pCC->CreateBool(pSystem->GetTopology()->HasAttribute(pArgs->GetElement(0)->GetStringValue()));
				}
			else
				return fnTopologyGet(pEvalCtx, pArgs, FN_NODE_HAS_ATTRIBUTE);
			}

		case FN_SYS_NAV_PATH_POINT:
			{
			CSystem *pSystem = g_pUniverse->GetCurrentSystem();

			//	Get the parameters

			CSovereign *pSovereign = g_pUniverse->FindSovereign(pArgs->GetElement(0)->GetIntegerValue());
			CSpaceObject *pStart = GetObjArg(pArgs->GetElement(1));
			CSpaceObject *pEnd = GetObjArg(pArgs->GetElement(2));
			if (pStart == NULL || pEnd == NULL)
				return CreateListFromVector(*pCC, CVector());
			int iPercent = pArgs->GetElement(3)->GetIntegerValue();
			iPercent = Max(0, Min(iPercent, 100));

			//	Get the navigation path

			CNavigationPath *pNavPath = pSystem->GetNavPath(pSovereign, pStart, pEnd);
			if (pNavPath == NULL)
				return CreateListFromVector(*pCC, pStart->GetPos());

			//	Compute the point on the path

			Metric rPathLen = pNavPath->ComputePathLength(pSystem);
			CVector vPos = pNavPath->ComputePointOnPath(pSystem, rPathLen * (iPercent / 100.0));
			return CreateListFromVector(*pCC, vPos);
			}

		case FN_SYS_NODE:
			{
			CSystem *pSystem = g_pUniverse->GetCurrentSystem();
			return pCC->CreateString(pSystem->GetTopology()->GetID());
			}

		case FN_SYS_ALL_NODES:
			{
			//	Create a list

			ICCItem *pResult = pCC->CreateLinkedList();
			if (pResult->IsError())
				return pResult;

			CCLinkedList *pList = (CCLinkedList *)pResult;

			//	Loop over all topology nodes and add the IDs to the list

			for (int i = 0; i < g_pUniverse->GetTopologyNodeCount(); i++)
				{
				CTopologyNode *pNode = g_pUniverse->GetTopologyNode(i);

				ICCItem *pValue = pCC->CreateString(pNode->GetID());
				pList->Append(pCC, pValue, NULL);
				pValue->Discard(pCC);
				}

			return pResult;
			}

		default:
			ASSERT(false);
		}

	return pCC->CreateNil();
	}

ICCItem *fnSystemGetObjectByName (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnSystemGetObjectByName
//
//	(sysGetObjectByName source name) -> obj

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("is"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the arguments

	CSpaceObject *pSource = GetObjArg(pArgs->GetElement(0));
	if (pSource == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	CString sName = pArgs->GetElement(1)->GetStringValue();
	pArgs->Discard(pCC);

	//	Compute

	CSpaceObject *pObj = pSource->GetSystem()->GetNamedObject(sName);
	if (pObj == NULL)
		return pCC->CreateNil();
	else
		return pCC->CreateInteger((int)pObj);
	}

ICCItem *fnSystemStopTime (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnSystemStopTime
//
//	(sysStopTime duration except)

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Get the arguments

	int iDuration = pArgs->GetElement(0)->GetIntegerValue();
	CSpaceObject *pExcept = GetObjArg(pArgs->GetElement(1));
	if (pExcept == NULL)
		return pCC->CreateNil();

	//	Compute

	CSystem *pSystem = g_pUniverse->GetCurrentSystem();
	pSystem->StopTimeForAll(iDuration, pExcept);
	return pCC->CreateTrue();
	}

ICCItem *fnSystemVectorMath (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnSystemVectorMath
//
//	(sysVectorAdd vector vector) -> vector
//	(sysVectorDivide vector scalar) -> vector
//	(sysVectorMultiply vector scalar) -> vector
//	(sysVectorSubtract vector vector) -> vector

	{
	int i;
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_VECTOR_ADD:
			{
			CVector vPos1(CreateVectorFromList(*pCC, pArgs->GetElement(0)));
			CVector vPos2(CreateVectorFromList(*pCC, pArgs->GetElement(1)));

			return CreateListFromVector(*pCC, vPos1 + vPos2);
			}

		case FN_VECTOR_ANGLE:
			{
			CVector vVec1(CreateVectorFromList(*pCC, pArgs->GetElement(0)));
			return pCC->CreateInteger(VectorToPolar(vVec1));
			}

		case FN_VECTOR_DIVIDE:
			{
			CVector vPos1(CreateVectorFromList(*pCC, pArgs->GetElement(0)));
			int iFactor = pArgs->GetElement(1)->GetIntegerValue();

			if (iFactor == 0)
				return pCC->CreateError(CONSTLIT("division by zero"), NULL);

			return CreateListFromVector(*pCC, vPos1 / (Metric)iFactor);
			}

		case FN_VECTOR_MULTIPLY:
			{
			CVector vPos1(CreateVectorFromList(*pCC, pArgs->GetElement(0)));
			int iFactor = pArgs->GetElement(1)->GetIntegerValue();

			return CreateListFromVector(*pCC, vPos1 * (Metric)iFactor);
			}

		case FN_VECTOR_RANDOM:
			{
			CSpaceObject *pSource = NULL;

			//	Get the center

			CVector vCenter;
			if (pArgs->GetElement(0)->IsNil())
				;
			else if (pArgs->GetElement(0)->IsList())
				vCenter = CreateVectorFromList(*pCC, pArgs->GetElement(0));
			else
				{
				CSpaceObject *pCenter = GetObjArg(pArgs->GetElement(0));
				if (pCenter)
					{
					vCenter = pCenter->GetPos();
					pSource = pCenter;
					}
				}

			//	Get the radius and separation

			Metric rRadius = LIGHT_SECOND * pArgs->GetElement(1)->GetIntegerValue();
			Metric rSeparation = 0.0;
			if (pArgs->GetCount() > 2)
				rSeparation = LIGHT_SECOND * pArgs->GetElement(2)->GetIntegerValue();

			//	Get the filter

			if (pSource == NULL)
				pSource = g_pUniverse->GetPlayer();
			if (pSource == NULL)
				pSource = g_pUniverse->GetPOV();

			CString sFilter = CONSTLIT("st");
			if (pArgs->GetCount() > 3)
				sFilter = pArgs->GetElement(3)->GetStringValue();

			CSpaceObject::Criteria Criteria;
			CSpaceObject::ParseCriteria(pSource, sFilter, &Criteria);

			//	Keep trying random positions until we find something that works
			//	(or until we run out of tries)

			Metric rMinDist2 = rSeparation * rSeparation;
			CSystem *pSystem = g_pUniverse->GetCurrentSystem();

			int iTries = 100;
			while (iTries > 0)
				{
				CVector vTry = vCenter + PolarToVector(mathRandom(0, 359), rRadius);

				//	See if any object is within the separation range

				bool bTooClose = false;
				for (i = 0; i < pSystem->GetObjectCount(); i++)
					{
					CSpaceObject *pObj = pSystem->GetObject(i);
					if (pObj && pObj->MatchesCriteria(Criteria))
						{
						Metric rDist2 = (pObj->GetPos() - vTry).Length2();
						if (rDist2 < rMinDist2)
							{
							bTooClose = true;
							break;
							}
						}
					}

				//	If we didn't find a good spot, then continue

				if (bTooClose && --iTries > 0)
					continue;

				//	Otherwise, use the vector

				return CreateListFromVector(*pCC, vTry);
				}
			}

		case FN_VECTOR_SUBTRACT:
			{
			CVector vPos1(CreateVectorFromList(*pCC, pArgs->GetElement(0)));
			CVector vPos2(CreateVectorFromList(*pCC, pArgs->GetElement(1)));

			return CreateListFromVector(*pCC, vPos1 - vPos2);
			}

		case FN_VECTOR_DISTANCE:
			{
			CVector vPos1(CreateVectorFromList(*pCC, pArgs->GetElement(0)));
			CVector vPos2;
			if (pArgs->GetCount() > 1)
				vPos2 = CreateVectorFromList(*pCC, pArgs->GetElement(1));

			CVector vDist = vPos1 - vPos2;
			return pCC->CreateInteger((int)((vDist.Length() / LIGHT_SECOND) + 0.5));
			}

		case FN_VECTOR_SPEED:
			{
			CVector vPos(CreateVectorFromList(*pCC, pArgs->GetElement(0)));
			return pCC->CreateInteger((int)(vPos.Length() + 0.5));
			}

		default:
			ASSERT(false);
			return NULL;
		}
	}

ICCItem *fnSystemVectorOffset (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnSystemVectorOffset
//
//	(sysVectorPolarOffset obj angle radius) -> vector

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("vii"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the arguments

	CVector vCenter = CreateVectorFromList(*pCC, pArgs->GetElement(0));
	int iAngle = pArgs->GetElement(1)->GetIntegerValue();
	Metric rRadius = LIGHT_SECOND * pArgs->GetElement(2)->GetIntegerValue();

	pArgs->Discard(pCC);

	//	Compute

	CVector vVec = vCenter + PolarToVector(iAngle, rRadius);

	//	Done

	return CreateListFromVector(*pCC, vVec);
	}

ICCItem *fnTopologyGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnTopologyGet

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	First arg is topology node

	CTopologyNode *pNode = g_pUniverse->FindTopologyNode(pArgs->GetElement(0)->GetStringValue());
	if (pNode == NULL)
		return pCC->CreateError(CONSTLIT("Invalid nodeID"), pArgs->GetElement(0));

	switch (dwData)
		{
		case FN_NODE_HAS_ATTRIBUTE:
			return pCC->CreateBool(pNode->HasAttribute(pArgs->GetElement(1)->GetStringValue()));

		case FN_NODE_LEVEL:
			return pCC->CreateInteger(pNode->GetLevel());

		case FN_NODE_SYSTEM_NAME:
			return pCC->CreateString(pNode->GetSystemName());

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnUniverseGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnUniverseGet
//
//	Gets data about the universe

	{
	int i;
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pResult;

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_UNIVERSE_TICK:
			pResult = pCC->CreateInteger(g_pUniverse->GetTicks());
			break;

		case FN_UNIVERSE_UNID:
			{
			CString sName = pArgs->GetElement(0)->GetStringValue();

			//	Figure out what the name refers to

			bool bFound = false;
			CItemType *pFoundItem = NULL;
			CShipClass *pFoundShip = NULL;

			//	Look for an exact match with an item name

			if (!bFound)
				{
				for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
					{
					CItemType *pType = g_pUniverse->GetItemType(i);
					if (strEquals(sName, pType->GetName(NULL, true)))
						{
						bFound = true;
						pFoundItem = pType;
						break;
						}
					}
				}

			//	Look for an exact match with a ship class

			if (!bFound)
				{
				for (i = 0; i < g_pUniverse->GetShipClassCount(); i++)
					{
					CShipClass *pClass = g_pUniverse->GetShipClass(i);
					if (strEquals(sName, pClass->GetShortName()))
						{
						bFound = true;
						pFoundShip = pClass;
						break;
						}
					}
				}

			//	Look for a partial match with a ship class

			if (!bFound)
				{
				for (i = 0; i < g_pUniverse->GetShipClassCount(); i++)
					{
					CShipClass *pClass = g_pUniverse->GetShipClass(i);
					if (strFind(sName, pClass->GetShortName()) != -1)
						{
						bFound = true;
						pFoundShip = pClass;
						break;
						}
					}
				}

			//	Look for partial item name match

			if (!bFound)
				{
				CItemCriteria Crit;
				CItem::InitCriteriaAll(&Crit);
				CItem Item = CItem::CreateItemByName(sName, Crit, true);
				if (Item.GetType())
					{
					bFound = true;
					pFoundItem = Item.GetType();
					}
				}

			//	If we found something, generate a list 

			if (bFound)
				{
				//	Create the result list

				pResult = pCC->CreateLinkedList();
				if (pResult->IsError())
					return pResult;

				CCLinkedList *pList = (CCLinkedList *)pResult;

				//	Figure out the elements of the list

				CString sType;
				DWORD dwUNID;
				CString sName;

				if (pFoundItem)
					{
					sType = CONSTLIT("itemtype");
					dwUNID = pFoundItem->GetUNID();
					sName = pFoundItem->GetName(NULL, true);
					}
				else if (pFoundShip)
					{
					sType = CONSTLIT("shipclass");
					dwUNID = pFoundShip->GetUNID();
					sName = pFoundShip->GetName(NULL);
					}
				else
					ASSERT(false);

				ICCItem *pValue = pCC->CreateInteger(dwUNID);
				pList->Append(pCC, pValue, NULL);
				pValue->Discard(pCC);

				pValue = pCC->CreateString(sType);
				pList->Append(pCC, pValue, NULL);
				pValue->Discard(pCC);

				pValue = pCC->CreateString(sName);
				pList->Append(pCC, pValue, NULL);
				pValue->Discard(pCC);
				}
			else
				pResult = pCC->CreateNil();

			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

