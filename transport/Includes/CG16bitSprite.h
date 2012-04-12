#ifndef INCL_CG16BITSPRITE
#define INCL_CG16BITSPRITE

class CG16bitSprite
	{
	public:
		CG16bitSprite (void);
		~CG16bitSprite (void);

		ALERROR CreateFromImage (const CG16bitImage &Source);
		void ColorTransBlt (CG16bitImage &Dest, int xDest, int yDest, int xSrc, int ySrc, int cxWidth, int cyHeight);
		inline int GetHeight (void) const { return m_cyHeight; }
		inline int GetWidth (void) const { return m_cxWidth; }

	private:
		enum SpriteCodes
			{
			codeSkip =	0x0001,
			codeRun =	0x0002,
			};

		void DeleteData (void);

		int m_cxWidth;
		int m_cyHeight;
		WORD *m_pCode;
		WORD **m_pLineIndex;
	};

#endif


