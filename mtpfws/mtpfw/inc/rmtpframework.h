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
#ifndef RMTPFRAMEWORK_H
#define RMTPFRAMEWORK_H

#include <e32base.h>
#include <f32file.h>

class CMTPConnectionMgr;
class CMTPDataProviderController;
class CMTPFrameworkConfig;
class CMTPObjectMgr;
class CMTPParserRouter;
class CMTPReferenceMgr;
class CMTPStorageMgr;
class RFs;
class CMTPDataCodeGenerator;
class CMTPServiceMgr;

/** 
Implements the MTP framework singletons manager.
@internalComponent
*/
class RMTPFramework
	{
public:	

    IMPORT_C RMTPFramework();

    IMPORT_C void OpenL();
    IMPORT_C void OpenLC();
    IMPORT_C void Close();
    
	IMPORT_C CMTPConnectionMgr& ConnectionMgr() const;
	IMPORT_C CMTPDataProviderController& DpController() const;
	IMPORT_C CMTPFrameworkConfig& FrameworkConfig() const;
	IMPORT_C RFs& Fs() const;
	IMPORT_C CMTPObjectMgr& ObjectMgr() const;
	IMPORT_C CMTPReferenceMgr& ReferenceMgr() const;
	IMPORT_C CMTPParserRouter& Router() const;
	IMPORT_C CMTPStorageMgr& StorageMgr() const;
	IMPORT_C CMTPDataCodeGenerator& DataCodeGenerator() const;	
	IMPORT_C CMTPServiceMgr& ServiceMgr() const;
    
private: // Owned
	
    class CSingletons : public CObject
        {
    public: 

        static CSingletons& OpenL();
        
    public: // From CObject
        
        void Close();

    private:

        virtual ~CSingletons();
        void ConstructL();

    public: // Owned

        /**
        The construction in-progress flag, which is used to manage nested 
        (recursive) opens.
        */
        TBool                       iConstructing;

        /**
        The connection manager singleton.
        */
        CMTPConnectionMgr*          iSingletonConnectionMgr;	

        /**
        The data provider controller singleton.
        */
        CMTPDataProviderController* iSingletonDpController;
        
        /**
        The configurability parameter data singleton.
        */
        CMTPFrameworkConfig*        iSingletonFrameworkConfig;	

        /**
        The file server session singleton
        */
        RFs                         iSingletonFs;   

        /**
        The object manager singleton.
        */
        CMTPObjectMgr*              iSingletonObjectMgr;

        /**
        The request and event parser/router singleton.
        */
        CMTPParserRouter*           iSingletonRouter;
        
        /** 
        The storage manager singleton.
        */
        CMTPStorageMgr*             iSingletonStorageMgr;
        
        /** 
        The data code manager singleton.
        */
        CMTPDataCodeGenerator*      iSingleDataCodeGenerator;
        
        /** 
        The device service manager singleton.
        */
        CMTPServiceMgr*           iSingleServiceMgr;
        
        };

private: // Owned
    /**
    The nested flag which indicates if the singletons manager reference was 
    recursively opened.
    */
    TBool           iNested;

    /**
    The singletons reference block.
    */
    CSingletons*    iSingletons;
	};
	
#endif // RMTPFRAMEWORK_H
