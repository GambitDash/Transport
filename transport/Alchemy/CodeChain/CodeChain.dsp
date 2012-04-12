# Microsoft Developer Studio Project File - Name="CodeChain" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CodeChain - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CodeChain.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CodeChain.mak" CFG="CodeChain - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CodeChain - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CodeChain - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CodeChain - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\..\Includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\CodeChain.lib"

!ELSEIF  "$(CFG)" == "CodeChain - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /I "..\..\Includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Libraries\CodeChain.lib"

!ENDIF 

# Begin Target

# Name "CodeChain - Win32 Release"
# Name "CodeChain - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\CCAtomTable.cpp
# End Source File
# Begin Source File

SOURCE=.\CCInteger.cpp
# End Source File
# Begin Source File

SOURCE=.\CCItemPool.cpp
# End Source File
# Begin Source File

SOURCE=.\CCLambda.cpp
# End Source File
# Begin Source File

SOURCE=.\CCLinkedList.cpp
# End Source File
# Begin Source File

SOURCE=.\CCNil.cpp
# End Source File
# Begin Source File

SOURCE=.\CConsPool.cpp
# End Source File
# Begin Source File

SOURCE=.\CCPrimitive.cpp
# End Source File
# Begin Source File

SOURCE=.\CCString.cpp
# End Source File
# Begin Source File

SOURCE=.\CCSymbolTable.cpp
# End Source File
# Begin Source File

SOURCE=.\CCTrue.cpp
# End Source File
# Begin Source File

SOURCE=.\CCVector.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeChain.cpp
# End Source File
# Begin Source File

SOURCE=.\Functions.cpp
# End Source File
# Begin Source File

SOURCE=.\ICCItem.cpp
# End Source File
# Begin Source File

SOURCE=.\Link.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadApp.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\Includes\CodeChain.h
# End Source File
# Begin Source File

SOURCE=.\DefPrimitives.h
# End Source File
# Begin Source File

SOURCE=.\Functions.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
