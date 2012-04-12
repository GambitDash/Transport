# Microsoft Developer Studio Project File - Name="JPEGUtil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=JPEGUtil - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JPEGUtil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JPEGUtil.mak" CFG="JPEGUtil - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JPEGUtil - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "JPEGUtil - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Transcendence", BAAAAAAA"
# PROP Scc_LocalPath "..\transcendence"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "JPEGUtil - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "JPEGUtil - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\Libraries\JPEGUtil.lib"

!ENDIF 

# Begin Target

# Name "JPEGUtil - Win32 Release"
# Name "JPEGUtil - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "c;cpp"
# Begin Source File

SOURCE=.\JCAPIMIN.C
# End Source File
# Begin Source File

SOURCE=.\JCAPISTD.C
# End Source File
# Begin Source File

SOURCE=.\JCCOEFCT.C
# End Source File
# Begin Source File

SOURCE=.\JCDCTMGR.C
# End Source File
# Begin Source File

SOURCE=.\JCHUFF.C
# End Source File
# Begin Source File

SOURCE=.\JCINIT.C
# End Source File
# Begin Source File

SOURCE=.\JCMAINCT.C
# End Source File
# Begin Source File

SOURCE=.\JCMARKER.C
# End Source File
# Begin Source File

SOURCE=.\JCMASTER.C
# End Source File
# Begin Source File

SOURCE=.\JCOMAPI.C
# End Source File
# Begin Source File

SOURCE=.\JCPARAM.C
# End Source File
# Begin Source File

SOURCE=.\JCPHUFF.C
# End Source File
# Begin Source File

SOURCE=.\JCPREPCT.C
# End Source File
# Begin Source File

SOURCE=.\JCSAMPLE.C
# End Source File
# Begin Source File

SOURCE=.\JCTRANS.C
# End Source File
# Begin Source File

SOURCE=.\JDAPIMIN.C
# End Source File
# Begin Source File

SOURCE=.\JDAPISTD.C
# End Source File
# Begin Source File

SOURCE=.\JDATADST.C
# End Source File
# Begin Source File

SOURCE=.\JDATASRC.C
# End Source File
# Begin Source File

SOURCE=.\JDCOEFCT.C
# End Source File
# Begin Source File

SOURCE=.\JDCOLOR.C
# End Source File
# Begin Source File

SOURCE=.\JDDCTMGR.C
# End Source File
# Begin Source File

SOURCE=.\JDHUFF.C
# End Source File
# Begin Source File

SOURCE=.\JDINPUT.C
# End Source File
# Begin Source File

SOURCE=.\JDMAINCT.C
# End Source File
# Begin Source File

SOURCE=.\JDMARKER.C
# End Source File
# Begin Source File

SOURCE=.\JDMASTER.C
# End Source File
# Begin Source File

SOURCE=.\JDMERGE.C
# End Source File
# Begin Source File

SOURCE=.\JDPHUFF.C
# End Source File
# Begin Source File

SOURCE=.\JDPOSTCT.C
# End Source File
# Begin Source File

SOURCE=.\JDSAMPLE.C
# End Source File
# Begin Source File

SOURCE=.\JDTRANS.C
# End Source File
# Begin Source File

SOURCE=.\JERROR.C
# End Source File
# Begin Source File

SOURCE=.\JFDCTFLT.C
# End Source File
# Begin Source File

SOURCE=.\JFDCTFST.C
# End Source File
# Begin Source File

SOURCE=.\JFDCTINT.C
# End Source File
# Begin Source File

SOURCE=.\JIDCTFLT.C
# End Source File
# Begin Source File

SOURCE=.\JIDCTFST.C
# End Source File
# Begin Source File

SOURCE=.\JIDCTINT.C
# End Source File
# Begin Source File

SOURCE=.\JIDCTRED.C
# End Source File
# Begin Source File

SOURCE=.\JMEMMGR.C
# End Source File
# Begin Source File

SOURCE=.\JMemNoBS.c
# End Source File
# Begin Source File

SOURCE=.\JPEGWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\JQUANT1.C
# End Source File
# Begin Source File

SOURCE=.\JQUANT2.C
# End Source File
# Begin Source File

SOURCE=.\JUTILS.C
# End Source File
# Begin Source File

SOURCE=.\MemDataSource.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\JCHUFF.H
# End Source File
# Begin Source File

SOURCE=.\JConfig.h
# End Source File
# Begin Source File

SOURCE=.\JDCT.H
# End Source File
# Begin Source File

SOURCE=.\JDHUFF.H
# End Source File
# Begin Source File

SOURCE=.\JERROR.H
# End Source File
# Begin Source File

SOURCE=.\JINCLUDE.H
# End Source File
# Begin Source File

SOURCE=.\JMEMSYS.H
# End Source File
# Begin Source File

SOURCE=.\JMORECFG.H
# End Source File
# Begin Source File

SOURCE=.\JPEGINT.H
# End Source File
# Begin Source File

SOURCE=.\JPEGLIB.H
# End Source File
# Begin Source File

SOURCE=..\Includes\JPEGUtil.h
# End Source File
# Begin Source File

SOURCE=.\JVERSION.H
# End Source File
# End Group
# End Target
# End Project
