#ifndef INCL_CG16BITREGION
#define INCL_CG16BITREGION

class CG16bitRegion
	{
	public:
		struct SRun
			{
			int xStart;
			int xEnd;
			int y;
			};

		CG16bitRegion (void);
		~CG16bitRegion (void);

		void CreateFromPolygon (int iVertexCount, SPoint *pVertexList);
		void CreateFromConvexPolygon (int iVertexCount, SPoint *pVertexList);
		void Fill (CG16bitImage &Dest, int x, int y, COLORREF wColor) const;
		void FillTrans (CG16bitImage &Dest, int x, int y, COLORREF wColor, DWORD byOpacity) const;
		inline int GetCount (void) const { return m_iCount; }
		inline const SRun &GetRun (int iIndex) const { return m_pList[iIndex]; }

	private:
		inline SRun &Run (int iIndex) { return m_pList[iIndex]; }
		void ScanEdge (int X1,
					   int Y1,
					   int X2,
					   int Y2,
					   bool bSetX,
					   int iSkipFirst,
					   SRun **iopRun);

		int m_iCount;
		SRun *m_pList;
	};

bool IsConvexPolygon (int iVertexCount, SPoint *pVertexList);
bool IsSimplePolygon (int iVertexCount, SPoint *pVertexList);
int CreateScanLinesFromPolygon (int iVertexCount, SPoint *pVertexList, CG16bitRegion::SRun **retpLines);
int CreateScanLinesFromSimplePolygon (int iVertexCount, SPoint *pVertexList, CG16bitRegion::SRun **retpLines);

#endif

