# Microsoft Developer Studio Project File - Name="DirectXUtil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DirectXUtil - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DirectXUtil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DirectXUtil.mak" CFG="DirectXUtil - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DirectXUtil - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DirectXUtil - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Transcendence", BAAAAAAA"
# PROP Scc_LocalPath "..\transcendence"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DirectXUtil - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\Include" /I "..\Includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DirectXUtil - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /O2 /I "..\Includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\DirectXUtil.lib"

!ENDIF 

# Begin Target

# Name "DirectXUtil - Win32 Release"
# Name "DirectXUtil - Win32 Debug"
# Begin Source File

SOURCE=.\16bitImageDrawing.cpp
# End Source File
# Begin Source File

SOURCE=.\AGArea.cpp
# End Source File
# Begin Source File

SOURCE=.\AGScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\CG16bitFont.cpp
# End Source File
# Begin Source File

SOURCE=.\CG16BitImage.cpp
# End Source File
# Begin Source File

SOURCE=.\CG16bitRegion.cpp
# End Source File
# Begin Source File

SOURCE=.\CG16bitSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\CGButtonArea.cpp
# End Source File
# Begin Source File

SOURCE=.\CGFrameArea.cpp
# End Source File
# Begin Source File

SOURCE=.\CGImageArea.cpp
# End Source File
# Begin Source File

SOURCE=.\CGTextArea.cpp
# End Source File
# Begin Source File

SOURCE=.\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectDraw.cpp
# End Source File
# Begin Source File

SOURCE=..\Includes\DirectXUtil.h
# End Source File
# Begin Source File

SOURCE=.\Sound.cpp
# End Source File
# End Target
# End Project
