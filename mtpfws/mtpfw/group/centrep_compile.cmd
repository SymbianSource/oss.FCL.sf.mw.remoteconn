@ECHO OFF
rem Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
rem All rights reserved.
rem This component and the accompanying materials are made available
rem under the terms of "Eclipse Public License v1.0"
rem which accompanies this distribution, and is available
rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
rem
rem Initial Contributors:
rem Nokia Corporation - initial contribution.
rem
rem Contributors:
rem
rem Description:
rem Script to convert a central repository text file to a binary format. The 
rem source file should be located in the current default directory, and should 
rem be named as follows:
rem <UID>_<platform>.txt (e.g 10282FCC_ARMV5.txt).
rem The binary format target file is created in the current default directory.
rem 
rem

SET RETURNCODE=0

SET _UID=%1
SET _PLATFORM=%2

IF "%_UID%"=="" GOTO HELP
IF "%_PLATFORM%"=="" SET _PLATFORM=BOTH

IF /I "%_PLATFORM%"=="ARMV5" GOTO ARMV5
IF /I "%_PLATFORM%"=="WINSCW" GOTO WINSCW
IF /I "%_PLATFORM%"=="BOTH" GOTO WINSCW
GOTO DONE

:WINSCW
SET _SOURCE=%_UID%_winscw.txt
SET _TARGET=%_UID%_winscw.cre
IF EXIST %_SOURCE% GOTO WINSCW_COMPILE
ECHO %_SOURCE% not found
GOTO WINSCW_DONE

:WINSCW_COMPILE
ECHO Compiling %_SOURCE% as %_TARGET%
COPY %_SOURCE% %EPOCROOT%epoc32\winscw\c\%_UID%.txt
CALL %EPOCROOT%epoc32\release\winscw\udeb\CentRepConv -nowait -o C:\%_UID%.cre C:\%_UID%.txt
COPY %EPOCROOT%epoc32\winscw\c\%_UID%.cre %_TARGET% 
ERASE %EPOCROOT%epoc32\winscw\c\%_UID%.cre
ERASE %EPOCROOT%epoc32\winscw\c\%_UID%.txt

:WINSCW_DONE
IF /I NOT "%_PLATFORM%"=="BOTH" GOTO DONE

:ARMV5
SET _SOURCE=%_UID%_armv5.txt
SET _TARGET=%_UID%_armv5.cre
IF EXIST %_SOURCE% GOTO ARMV5_COMPILE
ECHO %_SOURCE% not found
GOTO ARMV5_DONE

:ARMV5_COMPILE
ECHO Compiling %_SOURCE% as %_TARGET%
COPY %_SOURCE% %EPOCROOT%epoc32\winscw\c\%_UID%.txt
CALL %EPOCROOT%epoc32\release\winscw\udeb\CentRepConv -nowait -o C:\%_UID%.cre C:\%_UID%.txt
COPY %EPOCROOT%epoc32\winscw\c\%_UID%.cre %_TARGET% 
ERASE %EPOCROOT%epoc32\winscw\c\%_UID%.cre
ERASE %EPOCROOT%epoc32\winscw\c\%_UID%.txt

:ARMV5_DONE
GOTO DONE

:HELP
SET _SYNTAX="%0% <UID> [WINSCW|ARMV5]"
ECHO Syntax: %_SYNTAX%
GOTO DONE

:DONE
EXIT /B %RETURNCODE%
