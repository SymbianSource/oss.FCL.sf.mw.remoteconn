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

#ifndef RMTPDEVICEDPSINGLETONS_H
#define RMTPDEVICEDPSINGLETONS_H

#include <e32def.h>

class CMTPDeviceDataStore;
class CMTPDeviceDpConfigMgr;

/** 
Implements the MTP device data processor singletons reference manager.
@internalComponent
*/
class RMTPDeviceDpSingletons
    {
    
public:  

    RMTPDeviceDpSingletons();  

    void OpenL(MMTPDataProviderFramework& aFramework);
    void Close();
    
    CMTPDeviceDataStore& DeviceDataStore();
    CMTPDeviceDpConfigMgr& ConfigMgr();
    
    RArray<TUint>& PendingStorages();
    
private:
    
    /**
    Implements the singletons reference block.
    */
    class CSingletons : public CObject
        {
    public: 

        static CSingletons& OpenL(MMTPDataProviderFramework& aFramework);
        void Close();

    private:

        static CSingletons* NewL(MMTPDataProviderFramework& aFramework);
        virtual ~CSingletons();
        void ConstructL(MMTPDataProviderFramework& aFramework);

    public: // Owned
            
        /**
        The device information data manager.
        */
        CMTPDeviceDataStore*    iDeviceDataStore;
        
        /**
        The device dp config manager.
        */
        CMTPDeviceDpConfigMgr*	iConfigMgr;
        
        /*
         * The pending storageids which is not enumerated.
         */
        RArray<TUint>         iPendingStorages;
    };
    
private: // Owned

    /**
    The singletons reference block.
    */
    CSingletons*    iSingletons;
};

#endif // RMTPDEVICEDPSINGLETONS_H
