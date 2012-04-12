//	TSETypes.h
//
//	Basic types

#ifndef INCL_TSE_TYPES
#define INCL_TSE_TYPES

#define OBJID_CUNIVERSE				MakeOBJCLASSID(1)
#define OBJID_CSYSTEM				MakeOBJCLASSID(2)
#define OBJID_CSPACEOBJECT			MakeOBJCLASSID(3)
#define OBJID_CSHIPCLASS			MakeOBJCLASSID(4)
#define OBJID_CSHIP					MakeOBJCLASSID(5)
#define OBJID_COBJECTIMAGEARRAY		MakeOBJCLASSID(6)
#define OBJID_CSTATION				MakeOBJCLASSID(7)
#define OBJID_CSTANDARDSHIPAI		MakeOBJCLASSID(8)
#define OBJID_CBEAM					MakeOBJCLASSID(9)
#define OBJID_CEFFECT				MakeOBJCLASSID(10)
#define OBJID_CSOVEREIGN			MakeOBJCLASSID(11)
#define OBJID_CARMORCLASS			MakeOBJCLASSID(12)
#define OBJID_CWEAPONCLASS			MakeOBJCLASSID(13)
#define OBJID_CITEMTYPE				MakeOBJCLASSID(14)
#define OBJID_CMARKER				MakeOBJCLASSID(15)
#define OBJID_CMISSILE				MakeOBJCLASSID(16)
#define OBJID_CPARTICLEEFFECT		MakeOBJCLASSID(17)
#define OBJID_CPOVMARKER			MakeOBJCLASSID(18)
#define OBJID_CAREADAMAGE			MakeOBJCLASSID(19)
#define OBJID_CSTATICEFFECT			MakeOBJCLASSID(20)
#define OBJID_CFLEETSHIPAI			MakeOBJCLASSID(21)
#define OBJID_CPARTICLEDAMAGE		MakeOBJCLASSID(22)
#define OBJID_CFERIANSHIPAI			MakeOBJCLASSID(23)
#define OBJID_CAUTONAI				MakeOBJCLASSID(24)
#define OBJID_CDISINTEGRATIONEFFECT	MakeOBJCLASSID(25)
#define OBJID_CFRACTUREEFFECT		MakeOBJCLASSID(26)
#define OBJID_CGLADIATORAI			MakeOBJCLASSID(27)
#define OBJID_CFLEETCOMMANDAI		MakeOBJCLASSID(28)
#define OBJID_CGAIANPROCESSORAI		MakeOBJCLASSID(29)
#define OBJID_CITEMTABLE			MakeOBJCLASSID(30)
#define OBJID_CZOANTHROPEAI			MakeOBJCLASSID(31)
#define OBJID_CRADIUSDAMAGE			MakeOBJCLASSID(32)
#define OBJID_CSHIPTABLE			MakeOBJCLASSID(33)

const int g_DestinyRange = 360;
const int g_RotationRange = 20;
const int STD_ROTATION_COUNT = 20;

const int g_RotationAngle = (360 / g_RotationRange);

//	Measurements
//
//	Distance: in kilometers (or "klicks")
//	Time: in seconds
//	Velocity: kilometers/second

const Metric g_Epsilon = 0.0000001;					//	Small value close to 0
const Metric LIGHT_SPEED = 299792.5;				//	c in Kilometers per second
const Metric LIGHT_SECOND = LIGHT_SPEED;			//	1 light-second
const Metric LIGHT_MINUTE = LIGHT_SECOND * 60;	//	1 light-minute
const Metric g_AU = 149600000.0;					//	1 AU in Kilometers

//	Scales

const Metric KLICKS_PER_PIXEL =			12500.0;	//	Default kilometers per pixel
const Metric TIME_SCALE =				60.0;		//	Seconds of game time per second of real time
const Metric STD_SECONDS_PER_UPDATE =	2.0;		//	Standard adjustment when converting to ticks
const int MAX_SOUND_DISTANCE =			4000;		//	Maximum distance at which sound is heard (in pixels)

const int g_TicksPerSecond =			30;			//	Frames per second of time
const Metric g_LRSRange =				(100.0 * LIGHT_SECOND);	//	Range of LRS
const Metric g_MapKlicksPerPixel =		g_AU / 400.0;//	400 pixels per AU
const Metric g_InfiniteDistance =		g_AU * g_AU * 1000000.0;	//	A very large number
const Metric g_SpaceDragFactor =		0.98;		//	Coefficient of drag in space (1.0 = no drag)
const Metric g_BounceCoefficient =		0.50;		//	Speed coefficient when bouncing
const Metric g_MomentumConstant =		25.0;		//	Arbitrary constant for momentum
const int g_MWPerFuelUnit =				15;			//	Tenth-MW-ticks produced per fuel unit
const Metric g_MinSpeed2 = 100.0;					//	If moving less than this speed, force to 0

const int MAX_TECH_LEVEL =				25;

const DWORD UNID_DOMAIN_AND_MODULE_MASK =	0xffff0000;

//	Special UNIDs
//	LATER: UNID should not be hard-coded

const DWORD g_PlayerSovereignUNID =			0x00001001;	//	svPlayer
const DWORD g_CommonwealthSovereignUNID =	0x00001002;	//	svCommonwealth
const DWORD g_PirateSovereignUNID =			0x00001005;	//	svPirates

const DWORD g_ShipWreckUNID =				0x00002001;	//	stShipwreck

const DWORD PLAYER_EI500_FREIGHTER_UNID =	0x00003802;	//	scEI100XPlayer
const DWORD g_DebugHenchmenShipUNID =		0x00103001;	//	scCenturion
const DWORD g_InsuranceShipUNID =			0x00153001;	//	scCorporateCruiser

const DWORD TRITIUM_PROPULSION_UPGRADE_UNID = 0x0000404B;	//	Tritium propulsion upgrade
const DWORD CARGO_HOLD_EXPANSION_UNID =		0x00004079;	//	Cargo hold expansion
const DWORD g_SuperconductingShieldsUNID =	0x0000407D;	//	Superconducting shield generator
const DWORD g_SuperconductingCoilUNID =		0x0000407E;	//	Superconducting shield generator
const DWORD NOVA25_REACTOR_UNID =			0x00004107;	//	Nova-25 reactor

const DWORD g_ExplosionUNID =				0x00009001;	//	efMediumExplosion1
const DWORD g_LargeExplosionUNID =			0x00009002;	//	efLargeExplosion1
const DWORD g_HitEffectUNID =				0x00009003;	//	efSmallExplosion1
const DWORD g_ShieldEffectUNID =			0x00009004;	//	efShieldHit1
const DWORD g_StargateInUNID =				0x00009005;	//	efStargateIn
const DWORD g_StargateOutUNID =				0x00009006;	//	efStargateOut
const DWORD g_StationDestroyedUNID =		0x00009009;	//	efStationExplosion

const DWORD DEFAULT_ADVENTURE_UNID =		0x00009301;	//	adPart1Desc (only for compatibility)

const DWORD DEFAULT_SHIP_SCREEN_UNID =		0x0000A001;	//	dsShipInterior
const DWORD g_unidShipInterior =			0x0000A001;	//	dsShipInterior

const DWORD g_DamageImageUNID =				0x0000F001;	//	rsMediumDamage
const DWORD SHIP_INTERIOR_IMAGE_UNID =		0x0000F002;	//	rsShipInterior
const DWORD g_LRSImageUNID =				0x0000F006;	//	rsZubrinLRS
const DWORD g_TargetingImageUNID =			0x0000F007;	//	rsZubrinTargeting
const DWORD g_SRSSnowImageUNID =			0x0000F008;	//	rsSRSSnow
const DWORD g_LRSBorderUNID =				0x0000F009;	//	rsZubrinLRSBorder
const DWORD DEFAULT_DOCK_SCREEN_IMAGE_UNID = 0x0000F013;	//	Default dock screen background
const DWORD DEFAULT_DOCK_SCREEN_MASK_UNID =	0x0000F014;	//	Default dock screen mask
const DWORD g_ShipExplosionParticlesUNID =	0x0000F114;	//	rsDebris1
const DWORD g_ShipExplosionSoundUNID =		0x0000F204;	//	snShipExplosion1
const DWORD g_StationExplosionSoundUNID =	0x0000F205;	//	snStationExplosion1

//	Global constants

extern Metric g_KlicksPerPixel;
extern Metric g_TimeScale;
extern Metric g_SecondsPerUpdate;

//	Utility types

enum NounFlags
	{
	nounDefiniteArticle		= 0x00000001,	//	Article must be "the"
	nounFirstPlural			= 0x00000002,	//	First word should be pluralized
	nounPluralES			= 0x00000004,	//	Append "es" to pluralize
	nounCustomPlural		= 0x00000008,	//	Plural form follows after ";"
	nounSecondPlural		= 0x00000010,	//	Second word should be pluralized
	nounVowelArticle		= 0x00000020,	//	Should be "a" instead of "an" or vice versa
	nounNoArticle			= 0x00000040,	//	Do not use "a" or "the"
	};

enum NounPhraseFlags
	{
	nounCapitalize			= 0x00000001,	//	Capitalize the first letter
	nounPlural				= 0x00000002,	//	Pluralize noun phrase (regardless of item count)
	nounArticle				= 0x00000004,	//	Prefix with either "the" or "a" (regardless of item count)
	nounCount				= 0x00000008,	//	Prefix with (if > 1) a count or (if == 1) article
	nounCountOnly			= 0x00000010,	//	Prefix with (if > 1) a count or (if == 1) nothing
	nounNoModifiers			= 0x00000020,	//	Do not prefix with "enhanced" or "damaged"
	nounDemonstrative		= 0x00000040,	//	Prefix with either "the" or "this" or "these"
	nounShort				= 0x00000080,	//	Use short form of name
	nounActual				= 0x00000100,	//	Use actual form (not unidentified form)
	};

enum FrequencyTypes
	{
	ftCommon				= 20,
	ftUncommon				= 10,
	ftRare					= 4,
	ftVeryRare				= 1,
	ftNotRandom				= 0
	};

//	Other types

enum GenomeTypes
	{
	genomeUnknown		= 0,
	genomeHumanMale		= 1,
	genomeHumanFemale	= 2,
	};

#endif

