#ifndef INCL_CREGISTRY
#define INCL_CREGISTRY

struct config_t;

//	Registry classes
class CRegKey
	{
	public:
		CRegKey (void);
		~CRegKey (void);

		static ALERROR OpenUserAppKey (const CString &sCompany, 
									   const CString &sAppName,
									   CRegKey *retKey);

		bool FindStringValue (const CString &sValue, CString *retsData);
		void SetStringValue (const CString &sValue, const CString &sData);

	private:

		struct config_t *conf;
	};

#endif

