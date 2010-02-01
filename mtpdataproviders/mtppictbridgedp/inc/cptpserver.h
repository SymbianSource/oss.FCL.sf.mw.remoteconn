// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef PTPSERVER_H
#define PTPSERVER_H


#include "s60dependency.h"

#include <e32svr.h>   
#include <mtp/cmtptypeobjectinfo.h>

#include "ptpdef.h"
#include "rptp.h"
#include "cptpsession.h"
#include "cmtppictbridgedp.h"

//FORWARD DECLARATION

class MMTPDataProviderFramework;
class TMTPTypeEvent;
class CMTPPictBridgePrinter;
class CMTPObjectMetaData;
class CFileMan;

static const TInt KMTPMaxPtpFolderPathLength = 256;

/**
*  A server class to initialize server. 
*
*  @since S60 5.2
*/
NONSHARABLE_CLASS(CPtpServer) : public CServer2
    {
     
public:
    static CPtpServer* NewL(MMTPDataProviderFramework& aFramework, CMTPPictBridgeDataProvider& aDataProvider);
    ~CPtpServer();

public:// from CServer2, creates a new session.
    CSession2* NewSessionL( const TVersion& aVersion, const RMessage2& aMessage ) const;
    
public:
    const TDesC& PtpFolder();
      
    void GetObjectHandleByNameL(const TDesC& aNameAndPath, TUint32& aHandle);
    void GetObjectNameByHandleL(TDes& aNameAndPath, const TUint32 aHandle);

    void SendEventL(TMTPTypeEvent& ptpEvent);
    void AddTemporaryObjectL(const TDesC& aPathAndFileName, TUint32& aHandle);
    void RemoveTemporaryObjects();
    void RemoveObjectL(const TDesC& aSuid);
    void NotifyOnMtpSessionOpen(CPtpSession* aSession);

    inline CMTPPictBridgePrinter* Printer() const{ return iPrinterP; }
    inline TInt NumSession() const { return iNumSession; }
    inline void IncrementSessionCount(){ iNumSession++; }
    inline void DecrementSessionCount(){ iNumSession--; }
    inline TUint32 DeviceDiscoveryHandle() const{ return iDataProvider.DeviceDiscoveryHandle(); }
    inline TBool MtpSessionOpen() const{ return iMtpSessionOpen; }

    MMTPDataProviderFramework& Framework() const;

    void MtpSessionOpened();
    void MtpSessionClosed();
    void CancelNotifyOnMtpSessionOpen(CPtpSession* aSession);

private:    
    CPtpServer(MMTPDataProviderFramework& aFramework, CMTPPictBridgeDataProvider& aDataProvider);
    void ConstructL();

private:   
    TBuf<KMTPMaxPtpFolderPathLength> iPtpFolder;
    CMTPPictBridgePrinter* iPrinterP;  // owned
    TInt iNumSession;
    MMTPDataProviderFramework& iFramework;
    CMTPPictBridgeDataProvider& iDataProvider;
    RPointerArray<CMTPObjectMetaData> iTemporaryObjects;
    TBool iMtpSessionOpen;
    CPtpSession* iSessionOpenNotifyClientP;
    CFileMan*          iFileMan;
    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    };
#endif // PTPSERVER_H

