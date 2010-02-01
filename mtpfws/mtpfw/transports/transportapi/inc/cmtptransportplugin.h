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

#ifndef CMTPTRANSPORTPLUGIN_H
#define CMTPTRANSPORTPLUGIN_H

#include <e32base.h>
#include <mtp/mtpdataproviderapitypes.h>

class MMTPConnectionMgr;

/** 
CMTPDataProviderPlugin interface UID.
*/
const TUid KMTPTransportPluginInterfaceUid = {0x102827AE};

/** 
Defines the MTP transport layer plug-in interface.
@internalComponent 
*/
class CMTPTransportPlugin : public CBase
    {
public:

    /**
    MTP transport layer plug-in factory method.
    @param aImplementationUid A UID specifying the required interface 
    implementation.
    @return Pointer to the plug-in instance. Ownership IS transfered.
    @leave KErrNoMemory If the plug-in instance could not be allocated.
    @leave KErrArgument If an invalid aImplementationUid was specified.
    */
    IMPORT_C static CMTPTransportPlugin* NewL(TUid aImplementationUid);
    
    /**
    MTP transport layer plug-in factory method.
    @param aImplementationUid A UID specifying the required interface 
    implementation.
    @param aParameter is the Parameter needed by MTPBT transport plugin. Since MTPBT support to initiate connection, it require listensee application to provide address and psm for remote host.
    @return Pointer to the plug-in instance. Ownership IS transfered.
    @leave KErrNoMemory If the plug-in instance could not be allocated.
    @leave KErrArgument If an invalid aImplementationUid was specified.
    */
	IMPORT_C static CMTPTransportPlugin* NewL(TUid aImplementationUid,const TAny * aParameter);
    
    /**
    Destructor.
    */
    IMPORT_C virtual ~CMTPTransportPlugin();

public: // MTP Transport plug-in API methods. Not Exported.

    /**
    Notifies the transport layer plug-in of a change in the operational mode
    of the MTP protocol layer.
    @param aMode The new operational mode of the MTP protocol layer.
    */
    virtual void ModeChanged(TMTPOperationalMode aMode) = 0;

    /**
    Initiates the startup of the MTP transport layer. The transport layer should
    use the supplied MTP connection manager interface to signal the 
    availability and unavailability of transport layer connections.
    @param aConnectionMgr The MTP connection manager interface.
    @see MMTPConnectionMgr::ConnectionClosedL
    @see MMTPConnectionMgr::ConnectionOpenedL
    @leave One of the system wide error codes.
    */
    virtual void StartL(MMTPConnectionMgr& aConnectionMgr) = 0;
   
    /**
    Initiates the shutdown of the MTP transport layer. The transport layer should
    use the supplied connection manager interface to signal the availability
    and unavailability of transport layer connections.
    @param aConnectionMgr The connection manager interface.
    @see MMTPConnectionMgr::ConnectionClosedL.
    @see MMTPConnectionMgr::ConnectionOpenedL.
    */
    virtual void Stop(MMTPConnectionMgr& aConnectionMgr) = 0;
    
    /**
    Provides an MTP transport plug-in extension interface implementation 
    for the specified interface Uid. 
    @param aInterfaceUid Unique identifier for the extension interface being 
    requested.
    @return Pointer to an interface instance or 0 if the interface is not 
    supported. Ownership is NOT transfered.
    */
    virtual TAny* GetExtendedInterface(TUid aInterfaceUid) = 0;

protected:

    /**
    Constructor.
    */
    IMPORT_C CMTPTransportPlugin();
    


private: // Owned

    /**
    MTP transport layer ECOM plug-in identifier.
    */
    TUid iDtorIdKey;
    };
#endif // CMTPTRANSPORTPLUGIN_H
