# Microsoft Developer Studio Project File - Name="Kernel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Kernel - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Kernel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Kernel.mak" CFG="Kernel - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Kernel - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Kernel - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Kernel - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir "."
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\..\Includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX"Kernel.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\Kernel.lib"

!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir "."
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I "..\..\Includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX"Kernel.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Libraries\Kernel.lib"

!ENDIF 

# Begin Target

# Name "Kernel - Win32 Release"
# Name "Kernel - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\CArchiver.cpp
# End Source File
# Begin Source File

SOURCE=.\CAtomTable.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CDictionary.cpp
# End Source File
# Begin Source File

SOURCE=.\CFileDirectory.cpp
# End Source File
# Begin Source File

SOURCE=.\CFileReadBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\CFileReadStream.cpp
# End Source File
# Begin Source File

SOURCE=.\CFileWriteStream.cpp
# End Source File
# Begin Source File

SOURCE=.\CIDTable.cpp
# End Source File
# Begin Source File

SOURCE=.\CIntArray.cpp
# End Source File
# Begin Source File

SOURCE=.\CINTDynamicArray.cpp
# End Source File
# Begin Source File

SOURCE=.\CIntSet.cpp
# End Source File
# Begin Source File

SOURCE=.\CMemoryStream.cpp
# End Source File
# Begin Source File

SOURCE=.\CObject.cpp
# End Source File
# Begin Source File

SOURCE=.\CObjectArray.cpp
# End Source File
# Begin Source File

SOURCE=.\CRegKey.cpp
# End Source File
# Begin Source File

SOURCE=.\CResourceReadBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\CSharedObjectQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\CString.cpp
# End Source File
# Begin Source File

SOURCE=.\CStringArray.cpp
# End Source File
# Begin Source File

SOURCE=.\CStructArray.cpp
# End Source File
# Begin Source File

SOURCE=.\CSymbolTable.cpp
# End Source File
# Begin Source File

SOURCE=.\CTextFileLog.cpp
# End Source File
# Begin Source File

SOURCE=.\Kernel.cpp
# End Source File
# Begin Source File

SOURCE=.\Math.cpp
# End Source File
# Begin Source File

SOURCE=.\Path.cpp
# End Source File
# Begin Source File

SOURCE=.\Pattern.cpp
# End Source File
# Begin Source File

SOURCE=.\Time.cpp
# End Source File
# Begin Source File

SOURCE=.\Utilities.cpp

!IF  "$(CFG)" == "Kernel - Win32 Release"

!ELSEIF  "$(CFG)" == "Kernel - Win32 Debug"

# ADD CPP /Z7 /Od

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\Includes\Kernel.h
# End Source File
# Begin Source File

SOURCE=..\..\Includes\KernelObjID.h
# End Source File
# Begin Source File

SOURCE=..\..\Includes\TQueue.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
