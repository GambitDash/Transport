//	CRegKey.cpp
//
//	CRegKey class
//	Copyright (c) 2003 by NeuroHack, Inc. All Rights Reserved.

#include <libconfig.h>

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "Kernel.h"

#include "CRegistry.h"

CRegKey::CRegKey (void) : conf(NULL)

//	CRegKey constructor

	{
	int ret;

	conf = new config_t;
	config_init(conf);
	ret = config_read_file(conf, "trans.conf"); /* XXX Should be set globally? */
	if (ret == CONFIG_FALSE)
		{
		kernelDebugLogMessage("Unable to load 'trans.conf', quiting.");
		exit(-1);
		}
	}

CRegKey::~CRegKey (void)

//	CRegKey destructor

	{
	config_write_file(conf, "trans.conf");
	config_destroy(conf);
	delete conf;
	conf = NULL;
	}

bool CRegKey::FindStringValue (const CString &sValue, CString *retsData)

//	FindStringValue
//
//	Find a string value

	{
	const char *val;
	char *result;
    int ret;

	ret = config_lookup_string(conf, sValue.GetASCIIZPointer(), &val);
	if (val == NULL)
		{
		return false;
		}

	/* XXX Might have an off-by-1 error here... */
	result = retsData->GetWritePointer(strlen(val) + 1); 

	memcpy(result, val, strlen(val) + 1);

	return true;
	}

ALERROR CRegKey::OpenUserAppKey (const CString &sCompany, 
							     const CString &sAppName,
							     CRegKey *retKey)

//	OpenUserAppKey
//
//	Opens the key for the application's user data

	{
	return NOERROR;
	}

void CRegKey::SetStringValue (const CString &sValue, const CString &sData)

//	SetStringValue
//
//	Sets a string value under this key

	{
	config_setting_t *setting;
	int ret;

	setting = config_lookup(conf, sValue.GetASCIIZPointer());
	if (setting == NULL)
		{
		setting = config_setting_add(config_root_setting(conf),
				sValue.GetASCIIZPointer(), CONFIG_TYPE_STRING);
		}

	ASSERT(setting != NULL);

	ret = config_setting_set_string(setting, sData.GetASCIIZPointer());
	ASSERT(ret == CONFIG_TRUE);
	}

