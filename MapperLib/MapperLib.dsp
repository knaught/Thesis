# Microsoft Developer Studio Project File - Name="MapperLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MapperLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MapperLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MapperLib.mak" CFG="MapperLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MapperLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MapperLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MapperLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "MapperLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\obj"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Mapper.lib"

!ENDIF 

# Begin Target

# Name "MapperLib - Win32 Release"
# Name "MapperLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\RmActionHook.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmBayesCertaintyGrid.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmBayesSonarModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmClient.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmExceptions.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmGlobalMap.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmLocalMap.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmPioneerController.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmPolygon.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmServer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmSettings.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmSonarMap.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmSonarMapper.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmUtility.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RmUtilityExt.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\RmActionHandler.h
# End Source File
# Begin Source File

SOURCE=..\include\RmActionHook.h
# End Source File
# Begin Source File

SOURCE=..\include\RmBayesCertaintyGrid.h
# End Source File
# Begin Source File

SOURCE=..\include\RmBayesSonarModel.h
# End Source File
# Begin Source File

SOURCE=..\include\RmClient.h
# End Source File
# Begin Source File

SOURCE=..\include\RmExceptions.h
# End Source File
# Begin Source File

SOURCE=..\include\RmGlobalMap.h
# End Source File
# Begin Source File

SOURCE=..\include\RmLocalMap.h
# End Source File
# Begin Source File

SOURCE=..\include\RmMutableCartesianGrid.h
# End Source File
# Begin Source File

SOURCE=..\include\RmMutableMatrix.h
# End Source File
# Begin Source File

SOURCE=..\include\RmPioneerController.h
# End Source File
# Begin Source File

SOURCE=..\include\RmPolygon.h
# End Source File
# Begin Source File

SOURCE=..\include\RmServer.h
# End Source File
# Begin Source File

SOURCE=..\include\RmSettings.h
# End Source File
# Begin Source File

SOURCE=..\include\RmSonarMap.h
# End Source File
# Begin Source File

SOURCE=..\include\RmSonarMapper.h
# End Source File
# Begin Source File

SOURCE=..\include\RmUtility.h
# End Source File
# Begin Source File

SOURCE=..\include\RmUtilityExt.h
# End Source File
# End Group
# End Target
# End Project
