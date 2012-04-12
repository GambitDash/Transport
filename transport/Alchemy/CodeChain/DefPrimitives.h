//	DefPrimitives.h
//
//	Default primitives

static PRIMITIVEPROCDEF g_DefPrimitives[] =
	{
		{	"<",				fnEquality,		FN_EQUALITY_LESSER,		"",		NULL,	0,	},
		{	"ls",				fnEquality,		FN_EQUALITY_LESSER,		
			"(ls a b) -> True if a < b",
			NULL,	0,	},

		{	"<=",				fnEquality,		FN_EQUALITY_LESSER_EQ,	"",		NULL,	0,	},
		{	"leq",				fnEquality,		FN_EQUALITY_LESSER_EQ,
			"(leq a b) -> True if a <= b",
			NULL,	0,	},

		{	">",				fnEquality,		FN_EQUALITY_GREATER,	"",		NULL,	0,	},
		{	"gr",				fnEquality,		FN_EQUALITY_GREATER,
			"(gr a b) -> True if a > b",
			NULL,	0,	},

		{	">=",				fnEquality,		FN_EQUALITY_GREATER_EQ,	"",		NULL,	0,	},
		{	"geq",				fnEquality,		FN_EQUALITY_GREATER_EQ,
			"(geq a b) -> True if a >= b",		NULL,	0,	},

		{	"add",				fnMathList,		FN_MATH_ADD,
			"(add x1 x2 ... xn) -> z",
			"v*",	0,	},

		{	"and",				fnLogical,		FN_LOGICAL_AND,
			"(and exp1 exp2 ... expn) -> True/Nil",
			NULL,	0,	},

		{	"append",			fnAppend,		0,
			"(append a b [...]) -> lists are concatenated",
			"vv*",	0,	},

		{	"apply",			fnApply,		0,						"",		NULL,	0,	},
		{	"atmAddEntry",		fnAtmTable,		FN_ATMTABLE_ADDENTRY,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		{	"atmDeleteEntry",	fnAtmTable,		FN_ATMTABLE_DELETEENTRY,"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		{	"atmList",			fnAtmTable,		FN_ATMTABLE_LIST,		"",		NULL,	0,	},
		{	"atmLookup",		fnAtmTable,		FN_ATMTABLE_LOOKUP,		"",		NULL,	0,	},
		{	"atmAtomTable",		fnAtmCreate,	0,						"",		NULL,	0,	},
		{	"block",			fnBlock,		FN_BLOCK_BLOCK,			"",		NULL,	0,	},
		{	"cat",				fnCat,			0,
			"(cat s1 s2 ... sn) -> string",		NULL,	0,	},

		{	"count",			fnCount,		0,
			"(count list) -> number of items",
			NULL,	0,	},

		{	"divide",			fnMathOld,			FN_MATH_DIVIDE,
			"(divide x y) -> z",		NULL,	0,	},

		{	"enum",				fnEnum,			0,
			"(enum list itemVar exp)",
			NULL,	0,	},

		{	"enumwhile",		fnEnum,			FN_ENUM_WHILE,
			"(enum list condition itemVar exp)",
			NULL,	0,	},

		{	"errblock",			fnBlock,		FN_BLOCK_ERRBLOCK,		"",		NULL,	0,	},
		{	"eq",				fnEquality,		FN_EQUALITY_EQ,			"",		NULL,	0,	},
		{	"eval",				fnEval,			0,						"",		NULL,	0,	},
		{	"filter",			fnFilter,		0,
			"(filter list var boolean-exp) -> filtered list",
			"lqu",	0,	},

		{	"find",				fnFind,			0,
			"(find source target) -> position of target in source (0-based)",
			"vv",	0,	},

		{	"fncHelp",			fnItemInfo,		FN_ITEMINFO_HELP,		"",		NULL,	0,	},

		{	"for",				fnForLoop,		0,
			"(for var from to exp)",
			NULL,	0,	},

		{	"if",				fnIf,			0,						"",		NULL,	0,	},
		{	"int",				fnItemInfo,		FN_ITEMINFO_ASINT,		"",		NULL,	0,	},
		{	"isatom",			fnItemInfo,		FN_ITEMINFO_ISATOM,		"",		NULL,	0,	},
		{	"iserror",			fnItemInfo,		FN_ITEMINFO_ISERROR,	"",		NULL,	0,	},
		{	"isint",			fnItemInfo,		FN_ITEMINFO_ISINT,		"",		NULL,	0,	},
		{	"isfunction",		fnItemInfo,		FN_ITEMINFO_ISFUNCTION,	"",		NULL,	0,	},
		{	"item",				fnItem,			0,
			"(item list index)",
			NULL,	0,	},

		{	"lambda",			fnLambda,		0,						"",		NULL,	0,	},
		{	"loop",				fnLoop,			0,
			"(loop condition exp)",
			NULL,	0,	},

		{	"link",				fnLink,			0,						"",		"s",	0,	},
		{	"list",				fnList,			0,
			"(list i1 i2 ... in) -> list",
			NULL,	0,	},

		{	"lnkAppend",		fnLinkedList,	FN_LINKEDLIST_APPEND,	
			"(lnkAppend list item) -> list",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"lnkRemove",		fnLinkedList,	FN_LINKEDLIST_REMOVE,
			"(lnkRemove list index) -> list",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"lnkRemoveNil",		fnLinkedList,	FN_LINKEDLIST_REMOVE_NIL,
			"(lnkRemoveNil list) -> list",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"lnkReplace",		fnLinkedList,	FN_LINKEDLIST_REPLACE,
			"(lnkReplace list index item) -> list",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"max",				fnMathList,			FN_MATH_MAX,
			"(max x1 x2 ... xn) -> z",
			"v*",	0,	},

		{	"min",				fnMathList,			FN_MATH_MIN,
			"(min x1 x2 ... xn) -> z",
			"v*",	0,	},

		{	"modulo",			fnMathOld,			FN_MATH_MODULUS,
			"(modulo x y) -> z",
			NULL,	0,	},

		{	"multiply",			fnMathList,			FN_MATH_MULTIPLY,
			"(multiply x1 x2 ... xn) -> z",
			"v*",	0,	},

		{	"not",				fnLogical,		FN_LOGICAL_NOT,
			"(not exp) -> True/Nil",
			NULL,	0,	},

		{	"or",				fnLogical,		FN_LOGICAL_OR,
			"(or exp1 exp2 ... expn) -> True/Nil",
			NULL,	0,	},

		{	"power",			fnMathOld,			FN_MATH_POWER,
			"(power x y) -> z",
			NULL,	0,	},

		{	"random",			fnRandom,		0,
			"(random from to)\n(random list)",
			NULL,	0,	},

		{	"seededRandom",		fnSeededRandom,		0,
			"(seededRandom seed from to)\n(seededRandom seed list)",
			"i*",	0,	},

		{	"set",				fnSet,			FN_SET_SET,				"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		{	"setq",				fnSet,			FN_SET_SETQ,			"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		{	"shuffle",			fnShuffle,		0,
			"(shuffle list) -> shuffled list",
			"l",	0,	},

		{	"sqrt",				fnMath,			FN_MATH_SQRT,
			"(sqrt x) -> z",
			"i",	0,	},

		{	"strCapitalize",	fnStrCapitalize,0,
			"(strCapitalize string) -> string",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"strFind",			fnStrFind,		0,
			"(strFind string target) -> pos of target in string (0-based)",
			"ss",	0,	},

		{	"subst",			fnSubst,		0,
			"(subst string arg1 arg2 ... argn) -> string",
			NULL,	PPFLAG_SIDEEFFECTS,	},

		{	"subtract",			fnMathOld,			FN_MATH_SUBTRACT,
			"(subtract x y) -> z",
			NULL,	0,	},

		{	"switch",			fnSwitch,		0,						"",		NULL,	0,	},
		{	"symAddEntry",		fnSymTable,		FN_SYMTABLE_ADDENTRY,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		{	"symCreate",		fnSymCreate,	0,						"",		NULL,	0,	},
		{	"symDeleteEntry",	fnSymTable,		FN_SYMTABLE_DELETEENTRY,"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		{	"symList",			fnSymTable,		FN_SYMTABLE_LIST,		"",		NULL,	0,	},
		{	"symLookup",		fnSymTable,		FN_SYMTABLE_LOOKUP,		"",		NULL,	0,	},
		{	"sysGlobals",		fnSysInfo,		FN_SYSINFO_GLOBALS,		"",		NULL,	0,	},
		{	"sysPoolUsage",		fnSysInfo,		FN_SYSINFO_POOLUSAGE,	"",		NULL,	0,	},
		{	"sysTicks",			fnSysInfo,		FN_SYSINFO_TICKS,		"",		NULL,	0,	},
		{	"vecVector",		fnVecCreate,	0,						"",		NULL,	0,	},
		{	"vecSetElement",	fnVector,		FN_VECTOR_SETELEMENT,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
	};

#define DEFPRIMITIVES_COUNT		(sizeof(g_DefPrimitives) / sizeof(g_DefPrimitives[0]))

