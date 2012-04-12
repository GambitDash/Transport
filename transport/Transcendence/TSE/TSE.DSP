# Microsoft Developer Studio Project File - Name="TSE" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TSE - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TSE.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TSE.MAK" CFG="TSE - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TSE - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TSE - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Transcendence", BAAAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TSE - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\Includes" /I "..\..\Includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "TSE - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GR /GX /Z7 /Od /Ob2 /I "..\..\Includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /YX"PreComp.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "TSE - Win32 Release"
# Name "TSE - Win32 Debug"
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\PreComp.h
# End Source File
# Begin Source File

SOURCE=.\TSE.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\CAreaDamage.cpp
# End Source File
# Begin Source File

SOURCE=.\CArmorClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CAttributeDataBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\CAutoDefenseClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CAutonAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CBaseShipAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CBeam.cpp
# End Source File
# Begin Source File

SOURCE=.\CCargoSpaceClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CCExtensions.cpp
# End Source File
# Begin Source File

SOURCE=.\CCommunicationsHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\CCyberDeckClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CDisintegrationEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\CDockingPorts.cpp
# End Source File
# Begin Source File

SOURCE=.\CDriveClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\CEffectCreator.cpp
# End Source File
# Begin Source File

SOURCE=.\CEnhancerClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CEventHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\CFerianShipAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CFleetCommandAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CFleetShipAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CFractureEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\CGaianProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\CGladiatorAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CItem.cpp
# End Source File
# Begin Source File

SOURCE=.\CItemEnhancement.cpp
# End Source File
# Begin Source File

SOURCE=.\CItemList.cpp
# End Source File
# Begin Source File

SOURCE=.\CItemListManipulator.cpp
# End Source File
# Begin Source File

SOURCE=.\CItemTable.cpp
# End Source File
# Begin Source File

SOURCE=.\CItemType.cpp
# End Source File
# Begin Source File

SOURCE=.\CMiscellaneousClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CMissile.cpp
# End Source File
# Begin Source File

SOURCE=.\CObjectImageArray.cpp
# End Source File
# Begin Source File

SOURCE=.\CParticleDamage.cpp
# End Source File
# Begin Source File

SOURCE=.\CParticleEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\CPower.cpp
# End Source File
# Begin Source File

SOURCE=.\CRandomEntryGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\CReactorClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\CRepairerClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CResourceDb.cpp
# End Source File
# Begin Source File

SOURCE=.\CShieldClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CShip.cpp
# End Source File
# Begin Source File

SOURCE=.\CShipClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CShipEnergyFieldType.cpp
# End Source File
# Begin Source File

SOURCE=.\CSolarDeviceClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CSovereign.cpp
# End Source File
# Begin Source File

SOURCE=.\CSpaceEnvironmentType.cpp
# End Source File
# Begin Source File

SOURCE=.\CSpaceObject.cpp
# End Source File
# Begin Source File

SOURCE=.\CSpaceObjectList.cpp
# End Source File
# Begin Source File

SOURCE=.\CSpaceObjectTable.cpp
# End Source File
# Begin Source File

SOURCE=.\CStandardShipAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CStaticEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\CStation.cpp
# End Source File
# Begin Source File

SOURCE=.\CStationType.cpp
# End Source File
# Begin Source File

SOURCE=.\CSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\CTileMap.cpp
# End Source File
# Begin Source File

SOURCE=.\CUniverse.cpp
# End Source File
# Begin Source File

SOURCE=.\CWeaponClass.cpp
# End Source File
# Begin Source File

SOURCE=.\CWeaponFireDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\Damage.cpp
# End Source File
# Begin Source File

SOURCE=.\Devices.cpp
# End Source File
# Begin Source File

SOURCE=.\Events.cpp
# End Source File
# Begin Source File

SOURCE=.\Extensions.cpp
# End Source File
# Begin Source File

SOURCE=.\Markers.cpp
# End Source File
# Begin Source File

SOURCE=.\Orbits.cpp
# End Source File
# Begin Source File

SOURCE=.\PaintUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\RandomItemTable.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXMoltenBolt.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXParticleExplosion.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXPlasmaSphere.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXPolyflash.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXShockwave.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXStarbust.cpp
# End Source File
# Begin Source File

SOURCE=.\Topology.cpp
# End Source File
# Begin Source File

SOURCE=.\Utilities.cpp
# End Source File
# Begin Source File

SOURCE=.\XMLLoader.cpp
# End Source File
# End Group
# End Target
# End Project
