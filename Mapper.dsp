# Microsoft Developer Studio Project File - Name="Mapper" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Mapper - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Mapper.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Mapper.mak" CFG="Mapper - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Mapper - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Mapper - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Mapper - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "Mapper - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Mapper - Win32 Release"
# Name "Mapper - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "sonar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RmBayesCertaintyGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\RmBayesSonarModel.cpp
# End Source File
# Begin Source File

SOURCE=.\RmGlobalMap.cpp
# End Source File
# Begin Source File

SOURCE=.\RmLocalMap.cpp
# End Source File
# Begin Source File

SOURCE=.\RmSonarMap.cpp
# End Source File
# Begin Source File

SOURCE=.\RmSonarMapper.cpp
# End Source File
# End Group
# Begin Group "util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RmExceptions.cpp
# End Source File
# Begin Source File

SOURCE=.\RmSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\RmUtility.cpp
# End Source File
# Begin Source File

SOURCE=.\RmUtilityExt.cpp
# End Source File
# End Group
# Begin Group "viewer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RmJavaGridModel.cpp
# End Source File
# End Group
# Begin Group "pioneer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RmActionHook.cpp
# End Source File
# Begin Source File

SOURCE=.\RmPioneerController.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\RmServer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "sonar_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RmBayesCertaintyGrid.h
# End Source File
# Begin Source File

SOURCE=.\RmBayesSonarModel.h
# End Source File
# Begin Source File

SOURCE=.\RmGlobalMap.h
# End Source File
# Begin Source File

SOURCE=.\RmLocalMap.h
# End Source File
# Begin Source File

SOURCE=.\RmSonarMap.h
# End Source File
# Begin Source File

SOURCE=.\RmSonarMapper.h
# End Source File
# End Group
# Begin Group "util_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RmExceptions.h
# End Source File
# Begin Source File

SOURCE=.\RmMutableCartesianGrid.h
# End Source File
# Begin Source File

SOURCE=.\RmMutableMatrix.h
# End Source File
# Begin Source File

SOURCE=.\RmSettings.h
# End Source File
# Begin Source File

SOURCE=.\RmUtilityExt.h
# End Source File
# End Group
# Begin Group "viewer_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RmJavaGridModel.h
# End Source File
# End Group
# Begin Group "pioneer_h"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=.\RmServer.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\polygon\Debug\Polygon.lib
# End Source File
# Begin Source File

SOURCE="..\..\..\..\Program Files\ActivMedia Robotics\Aria-2.1-1\lib\ARIA.lib"
# End Source File
# End Target
# End Project
