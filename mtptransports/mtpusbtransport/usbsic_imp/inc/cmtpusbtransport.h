// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/**
 @file
 @internalComponent
*/
#ifndef CMTPUSBTRANSPORT_H
#define CMTPUSBTRANSPORT_H

#include "cmtptransportplugin.h"

class CMTPUsbConnection;

/**
Implements the USB MTP device class transport plug-in.
@internalComponent
 
*/
class CMTPUsbTransport: public CMTPTransportPlugin
    {
    
public:

    static TAny* NewL(TAny* aParameter);
    ~CMTPUsbTransport();

    const CMTPUsbConnection& MTPUsbConnection();

    
public: // From CMTPTransportPlugin

    void ModeChanged(TMTPOperationalMode aMode);
    void StartL(MMTPConnectionMgr& aConnectionMgr);
    void Stop(MMTPConnectionMgr& aConnectionMgr);
    TAny* GetExtendedInterface(TUid aInterfaceUid);
    
private:

    CMTPUsbTransport();
    void ConstructL();
      
private: // Owned

    /**
    The USB MTP device class connection
    */
    CMTPUsbConnection* iConnection;
    };
    
#endif // CMTPUSBTRANSPORT_H
