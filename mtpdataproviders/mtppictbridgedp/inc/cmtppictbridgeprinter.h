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


#ifndef CMTPPICTBRIDGEPRINTER_H
#define CMTPPICTBRIDGEPRINTER_H
#include <mtp/tmtptypeevent.h>
#include "mtpdebug.h"

class CMTPTypeObjectInfo;
class MMTPConnection;
class CPtpServer;
class CPtpSession;
class MServiceHandlerObserver;
class CPtpReceivedMsgHandler;
class CMTPPictBridgeUsbConnection; 

class CMTPPictBridgePrinter : public CBase
    {
    public:
        enum TPrinterStatus
            {
            ENotConnected,
            EConnected,
            ENotPrinter
            };
        static CMTPPictBridgePrinter* NewL(CPtpServer& aServer);
        ~CMTPPictBridgePrinter();
        void ConnectionClosed();
        void DpsDiscovery(const TFileName& aFileName, MMTPConnection* aConnectionP);
        void SendDpsFileL(const TDesC& aFile, TBool aTimeout, TInt aSize); 
        void CancelSendDpsFile();
        void DpsFileSent(TInt aError);
        
        void DpsObjectReceived(TUint32 aHandle);
        void ObjectReceived(TDes& aFile);

        void DeRegisterDpsPrinterNotify(CPtpSession* aSession);
        void RegisterDpsPrinterNotify(CPtpSession* aSession);

        void RegisterObserver(MServiceHandlerObserver* aObserverP);
        void  NoDpsDiscovery();
        TBool SendObjectPending() const;
        MMTPConnection* ConnectionP() const;
        TPrinterStatus Status() const;
        CPtpReceivedMsgHandler* MsgHandlerP() const;
        inline const CMTPPictBridgeUsbConnection* UsbConnectionP() const { return iUsbConnectionP; }
    private:
        void ConstructL();
        CMTPPictBridgePrinter(CPtpServer& aServer);
        void CreateRequestObjectTransfer(TUint32 aHandle, TMTPTypeEvent& aEvent);
    private:
        CPtpServer& iServer;
        MMTPConnection* iPrinterConnectionP;  // not owned
        TPrinterStatus iPrinterStatus;
        CPtpSession* iDpsPrinterNotifyCbP; // not owned
        CPtpReceivedMsgHandler* iMsgHandlerP; // owned
        MServiceHandlerObserver* iObserverP; // not owned
        TUint32 iOutgoingObjectHandle;
        CMTPPictBridgeUsbConnection* iUsbConnectionP; // owned
        TMTPTypeEvent iEvent;
        /**
        FLOGGER debug trace member variable.
        */
        __FLOG_DECLARATION_MEMBER_MUTABLE;
    };
#endif // CMTPPICTBRIDGEPRINTER_H