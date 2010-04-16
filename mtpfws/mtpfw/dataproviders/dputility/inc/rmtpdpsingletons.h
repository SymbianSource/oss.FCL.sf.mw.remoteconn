// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef RMTPDPSINGLETONS_H
#define RMTPDPSINGLETONS_H

#include "mtpdebug.h"

#include "rmtputility.h"

class MMTPDataProviderFramework;
class CMTPDpConfigMgr;
class CMTPFSExclusionMgr;
class CMTPFSEntryCache;

/** 
Implements the MTP data processor singletons reference manager.
@internalComponent
*/

class RMTPDpSingletons
    {
public:  
    IMPORT_C RMTPDpSingletons();  

    IMPORT_C void OpenL(MMTPDataProviderFramework& aFramework);
    IMPORT_C void Close();
    
    IMPORT_C CMTPFSExclusionMgr& ExclusionMgrL() const;
    IMPORT_C void SetExclusionMgrL(CMTPFSExclusionMgr& aExclusionMgr);
    
    IMPORT_C RMTPUtility& MTPUtility() const;
    IMPORT_C CMTPFSEntryCache& CopyingBigFileCache() const;
    IMPORT_C CMTPFSEntryCache& MovingBigFileCache() const;
    
private:
    class TExclusionMgrEntry
    	{
    public:
    	CMTPFSExclusionMgr* iExclusionMgr;
    	TInt iDpId;
    	
    	static TInt Compare(const TExclusionMgrEntry& aFirst, const TExclusionMgrEntry& aSecond);
    	};
    
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
        FLOGGER debug trace member variable.
        */
        __FLOG_DECLARATION_MEMBER_MUTABLE;
            
    	
    	/**
    	The file system exclusion manager list. This list contains all the Data Providers'
    	file system exclusion manager associated with its Data Provider ID. only a single one will 
    	be used corresponding to the Data Provider ID.
    	*/
    	RArray<TExclusionMgrEntry>	iExclusionList;
    	
    	/*
    	 * The utility for DPutility 
    	 */
    	RMTPUtility					iMTPUtility;
    	
    	CMTPFSEntryCache*   iCopyingBigFileCache;
    	CMTPFSEntryCache*   iMovingBigFileCache;
      };
      
private: //Not owned
	/**
	Reference to data provider framework.
	*/
	MMTPDataProviderFramework* iFramework;
    
private: // Owned
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    /**
    The singletons reference block.
    */
    CSingletons*    iSingletons;
};

#endif // RMTPDPSINGLETONS_H
