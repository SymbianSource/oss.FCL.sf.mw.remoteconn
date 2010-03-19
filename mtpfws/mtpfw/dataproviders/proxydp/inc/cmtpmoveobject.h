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

#ifndef CMTPMOVEOBJECT_H
#define CMTPMOVEOBJECT_H

#include <badesca.h>

#include "cmtprequestprocessor.h"
#include "mmtptransactionproxy.h"

class MMTPRequestProcessor;
class CMTPObjectBrowser;
class CFileMan;

/** 
Implements the proxy data provider MoveObject request processor.
@internalComponent
  
*/
class CMTPMoveObject : public CMTPRequestProcessor, public MMTPTransactionProxy
    {
private:
    enum TState
        {
        EInit,
        ERemoveSourceFolderTree
        };
    
public:
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPMoveObject(); 

private: // From CMTPRequestProcessor
    void ServiceL();    

private: // From MMTPTransactionProxy
    void ProxyReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxySendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, TRequestStatus& aStatus);
    void ProxyTransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    
private: // Helpers
    /*TMTPResponseCode CreateNewFoldersL();
    void CreateAllSubFoldersL(TUint aParentHandle);
	TBool IsFolderObject(const CMTPObjectMetaData& aObject);
	void FinalPhaseCreateL();*/
	void GetParametersL();
    void GetDefaultParentObjectL( TDes& aObjectName );
	TMTPResponseCode CanMoveObjectL(const TDesC& aOldName, const TDesC& aNewName) const;
	void GetSuidFromHandleL(TUint aHandle, TDes& aSuid) const;

private: // From CActive
    void RunL();
    TInt RunError(TInt aError);
    
private:    
    CMTPMoveObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
	void ConstructL();
    void SendResponseL(TUint16 aCode);
    void Schedule(TInt aError);
    
private:
    TMTPResponseCode CreateFolderL();
    void RemoveSourceFolderTreeL();
    void BrowseHandlesL();
    void NextObjectHandleL();
    static void OnBrowseObjectL( TAny* aSelf, TUint aHandle, TUint32 aCurDepth );
    
private: 
	/**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;     
       
    RMTPFramework       iSingletons;
    TUint                iOwnerDp;
	RBuf				iNewParent;
	TUint32				iStorageId; 
	CDesCArray*			iPathToCreate;
	
    CMTPObjectBrowser*      iObjBrowser;
    RArray< TUint32 >      iHandles;
    RArray< TUint32 >      iHandleDepths;
    RArray< TUint32 >       iNewHandleParentStack;
    TInt                    iCurrentHandle;
    TMTPTypeRequest         iCurrentRequest;
    TBool                   iIsMovingFolder;
    CMTPObjectMetaData*     iObjInfoCache;
    RBuf                    iFolderToRemove;
    TInt                    iState;
    CFileMan*               iFileMan;
    RArray<TUint>           iTargetDps;
    };
    
#endif // CMTPMOVEOBJECT_H

