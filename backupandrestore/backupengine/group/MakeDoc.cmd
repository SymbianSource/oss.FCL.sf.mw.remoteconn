@rem
@rem Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
@rem All rights reserved.
@rem This component and the accompanying materials are made available
@rem under the terms of "Eclipse Public License v1.0"
@rem which accompanies this distribution, and is available
@rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
@rem
@rem Initial Contributors:
@rem Nokia Corporation - initial contribution.
@rem
@rem Contributors:
@rem
@rem Description:
@rem

md C:\public_html\zephyr-connect\SecureBackupEngine
doxygen Doxyfile

set CURRENT_DIR=%CD%
set CURRENT_DRIVE=%CD:~0,2%

C:
cd \public_html\zephyr-connect\SecureBackupEngine\html

%CURRENT_DIR%\configureIndex.pl index.hhc > _index.hhc
del index.hhc
ren _index.hhc index.hhc

hhc index.hhp

copy index.chm %CURRENT_DIR%\SecureBackupEngine.chm
%CURRENT_DRIVE%


