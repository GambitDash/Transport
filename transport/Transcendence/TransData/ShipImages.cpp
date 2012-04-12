//	ShipTable.cpp
//
//	Generate basic stats about ship classes

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

class CCellMap
	{
	public:
		CCellMap (int iCols, int iRows)
			{
			m_pCellMap = new bool [iCols * iRows];
			m_iCols = iCols;
			m_iRows = iRows;

			for (int i = 0; i < iCols * iRows; i++)
				m_pCellMap[i] = false;
			}

		~CCellMap (void)
			{
			delete [] m_pCellMap;
			}

		bool GetSpace (int iCellsNeeded, int *retx, int *rety)
			{
			int x = 0;
			int y = 0;
			while (!SpaceAvailable(x, y, iCellsNeeded))
				{
				x++;
				if (x >= m_iCols)
					{
					x = 0;
					y++;
					if (y >= m_iRows)
						return false;
					}
				}

			for (int y1 = 0; y1 < iCellsNeeded; y1++)
				for (int x1 = 0; x1 < iCellsNeeded; x1++)
					MarkUsed(x + x1, y + y1);

			*retx = x;
			*rety = y;

			return true;
			}

		bool InUse (int x, int y)
			{
			return m_pCellMap[y * m_iCols + x];
			}

		void MarkUsed (int x, int y)
			{
			m_pCellMap[y * m_iCols + x] = true;
			}

		bool SpaceAvailable (int x, int y, int iCellsNeeded)
			{
			if (x + iCellsNeeded > m_iCols)
				return false;
			else if (y + iCellsNeeded > m_iRows)
				return false;
			else
				{
				for (int y1 = 0; y1 < iCellsNeeded; y1++)
					for (int x1 = 0; x1 < iCellsNeeded; x1++)
						if (InUse(x + x1, y + y1))
							return false;

				return true;
				}
			}

	private:
		bool *m_pCellMap;
		int m_iCols;
		int m_iRows;
	};

void GenerateShipImages (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;
	int x, y;

	//	Generate a table of ships

	CSymbolTable Table(FALSE, TRUE);
	for (i = 0; i < Universe.GetShipClassCount(); i++)
		{
		CShipClass *pClass = Universe.GetShipClass(i);

		//	Skip player ship classes

		if (pClass->GetPlayerSettings())
			continue;

		//	Skip non-generic classes

		if (!pClass->HasAttribute(CONSTLIT("genericClass")))
			continue;

		//	Compute the sort key

		char szBuffer[1024];
		wsprintf(szBuffer, "%04d%s",
				RectWidth(pClass->GetImage().GetImageRect()),
				pClass->GetName().GetASCIIZPointer());

		//	Add to list

		Table.AddEntry(CString(szBuffer), (CObject *)pClass);
		}

	//	Compute the size of each image cell

	int cxCell = 30;
	int cyCell = 30;

	//	Compute the number of rows & columns

	int iCols = 38;
	int iRows = AlignUp(20 * Table.GetCount(), iCols) / iCols;
	int cxWidth = iCols * cxCell;
	int cyHeight = iRows * cyCell;

	//	Create a large image

	CG16bitImage Output;
	Output.CreateBlank(cxWidth, cyHeight, false);

	//	Create an array that keeps track of which cells we've used up

	CCellMap CellMap(iCols, iRows);

	//	Null transform

	ViewportTransform Trans(CVector(), 0.0, 0, 0);

	//	Paint each ship image

	for (i = 0; i < Table.GetCount(); i++)
		{
		CShipClass *pClass = (CShipClass *)Table.GetValue(i);

		//	Figure out how many cells we need

		int cxSize = RectWidth(pClass->GetImage().GetImageRect());
		int iCellsNeeded = AlignUp(cxSize, cxCell) / cxCell;
		int cxAreaSize = iCellsNeeded * cxCell;

		//	Look for room in the cell map

		bool bFound = CellMap.GetSpace(iCellsNeeded, &x, &y);

		//	Paint the ship

		if (bFound)
			{
			pClass->GetImage().PaintImageUL(Output,
					x * cxCell + (cxAreaSize - cxSize) / 2,
					y * cyCell + (cxAreaSize - cxSize) / 2,
					0,
					Angle2Direction(0));
			}
		}

	//	Compute the last row that is in use

	int iUsedRows = 0;
	for (y = 0; y < iRows; y++)
		{
		bool bRowInUse = false;
		for (x = 0; x < iCols; x++)
			if (CellMap.InUse(x, y))
				{
				bRowInUse = true;
				break;
				}

		if (!bRowInUse)
			{
			iUsedRows = y;
			break;
			}
		}

	cyHeight = iUsedRows * cyCell;

	//	Create an HBITMAP

	HWND hDesktopWnd = ::GetDesktopWindow();
	HDC hDesktopDC = ::GetDC(hDesktopWnd);
	HDC hDC = ::CreateCompatibleDC(hDesktopDC);
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hDesktopDC, cxWidth, cyHeight);
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hDC, hBitmap);
	Output.BltToDC(hDC, 0, 0);
	::SelectObject(hDC, hOldBitmap);
	::DeleteDC(hDC);
	::ReleaseDC(hDesktopWnd, hDesktopDC);

	//	Copy to the clipboard

	::OpenClipboard(NULL);
	::EmptyClipboard();
	::SetClipboardData(CF_BITMAP, hBitmap);
	::CloseClipboard();
	}
