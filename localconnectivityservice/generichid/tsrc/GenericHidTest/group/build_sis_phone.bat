@rem
@rem Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
@rem Description: Test script config file
@rem
@rem

rd /q /s x:\epoc32\build
del GenericHidTest_phone.sisx

call bldmake bldfiles
call abld test build armv5 udeb
call makesis GenericHidTest_phone.pkg
call signsis GenericHidTest_phone.sis GenericHidTest_phone.sisx rd.cer rd-key.pem

del GenericHidTest_phone.sis
call pause