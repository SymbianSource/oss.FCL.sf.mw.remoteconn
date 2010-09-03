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

#ifndef RMTPFILEDPSINGLETONS_H
#define RMTPFILEDPSINGLETONS_H

#include "mtpdebug.h"

/** 
Implements the MTP file data processor singletons reference manager.
@internalComponent

*/

class CMTPFileDpConfigMgr;
class MMTPDataProviderFramework;

class RMTPFileDpSingletons
    {
public:  
    RMTPFileDpSingletons();  

    void OpenL(MMTPDataProviderFramework& aFramework);
    void Close();
    
    CMTPFileDpConfigMgr& FrameworkConfig();
    
private:
    
    /**
    Implements the singletons reference block.
    */
    class CSingletons : public CObject
        {
    public: 

        static CSingletons& OpenL(MMTPDataProviderFramework& aFramework);
        void Close();

    private: // From CObject

    private:

        static CSingletons* NewL(MMTPDataProviderFramework& aFramework);
        virtual ~CSingletons();
        void ConstructL(MMTPDataProviderFramework& aFramework);

    public: // Owned
            
        /**
        The file config manager.
        */
        CMTPFileDpConfigMgr*    iConfigMgr;
    	};
    
private: // Owned

    /**
    The singletons reference block.
    */
    CSingletons*    iSingletons;
};

#endif // RMTPFILEDPSINGLETONS_H
