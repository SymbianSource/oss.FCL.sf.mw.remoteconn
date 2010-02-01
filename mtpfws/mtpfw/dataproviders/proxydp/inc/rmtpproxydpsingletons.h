// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#ifndef RMTPPROXYDPSINGLETONS_H
#define RMTPPROXYDPSINGLETONS_H

#include <e32base.h>
class CMTPProxyDpConfigMgr;
class MMTPDataProviderFramework;

class RMTPProxyDpSingletons
    {
public:  
    RMTPProxyDpSingletons();  

    void OpenL(MMTPDataProviderFramework& aFramework);
    void Close();
    
    CMTPProxyDpConfigMgr& FrameworkConfig();
    
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
        The file config manager.
        */
        CMTPProxyDpConfigMgr*    iConfigMgr;
    	};
    
private: // Owned

    /**
    The singletons reference block.
    */
    CSingletons*    iSingletons;
};	
	
#endif
