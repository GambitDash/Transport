# Microsoft Developer Studio Project File - Name="Transcendence" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Transcendence - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Transcendence.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Transcendence.mak" CFG="Transcendence - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Transcendence - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Transcendence - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Transcendence", BAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Transcendence - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\Includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 dxguid.lib version.lib vfw32.lib winmm.lib dsound.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"Game/Transcendence.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "Transcendence - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\Includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /Yu"PreComp.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dxguid.lib version.lib vfw32.lib winmm.lib dsound.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"Game/Transcendence.exe"

!ENDIF 

# Begin Target

# Name "Transcendence - Win32 Release"
# Name "Transcendence - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\CArmorDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CButtonBarData.cpp
# End Source File
# Begin Source File

SOURCE=.\CButtonBarDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CDeviceCounterDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CDockScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\CGameFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CGItemListArea.cpp
# End Source File
# Begin Source File

SOURCE=.\CHighScoreDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CHighScoreList.cpp
# End Source File
# Begin Source File

SOURCE=.\CIntroShipController.cpp
# End Source File
# Begin Source File

SOURCE=.\CInvokeDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CMenuData.cpp
# End Source File
# Begin Source File

SOURCE=.\CMenuDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CMessageDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeChainExtensions.cpp
# End Source File
# Begin Source File

SOURCE=.\CPickerDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CPlayerDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CReactorDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CTargetDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CTextCrawlDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CTranscendenceWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\Deities.cpp
# End Source File
# Begin Source File

SOURCE=.\Help.cpp
# End Source File
# Begin Source File

SOURCE=.\IntroScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadingScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Main.cpp

!IF  "$(CFG)" == "Transcendence - Win32 Release"

!ELSEIF  "$(CFG)" == "Transcendence - Win32 Debug"

# ADD CPP /Yc"PreComp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MainWndProc.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerController.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\PreComp.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Transcendence.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Documents\Brainstorming.txt
# End Source File
# Begin Source File

SOURCE=.\Documents\Bugs.txt
# End Source File
# Begin Source File

SOURCE=.\Documents\Features.txt
# End Source File
# Begin Source File

SOURCE=.\FunctionReference.txt
# End Source File
# Begin Source File

SOURCE=.\Game\Resources\GameButtonIcons.jpg
# End Source File
# Begin Source File

SOURCE=.\Game\Resources\GameOver.jpg
# End Source File
# Begin Source File

SOURCE=.\Game\Resources\GameOverEscape.jpg
# End Source File
# Begin Source File

SOURCE=".\Game\Resources\Help Screen.jpg"
# End Source File
# Begin Source File

SOURCE=.\Game\Resources\IconDisplay.jpg
# End Source File
# Begin Source File

SOURCE=.\Documents\MediaNotes.txt
# End Source File
# Begin Source File

SOURCE=.\Documents\Schedule.txt
# End Source File
# Begin Source File

SOURCE=.\Game\Resources\Stargate.JPG
# End Source File
# Begin Source File

SOURCE=.\Game\Resources\StargateMask.BMP
# End Source File
# Begin Source File

SOURCE=.\Documents\Stations.txt
# End Source File
# Begin Source File

SOURCE=.\Game\Resources\Title.jpg
# End Source File
# Begin Source File

SOURCE=.\Transcendence.ico
# End Source File
# Begin Source File

SOURCE=.\Transcendence.rc
# End Source File
# Begin Source File

SOURCE=.\Game\Transcendence.xml
# End Source File
# Begin Source File

SOURCE=".\Worklist (Completed).txt"
# End Source File
# Begin Source File

SOURCE=.\Worklist.txt
# End Source File
# End Target
# End Project
