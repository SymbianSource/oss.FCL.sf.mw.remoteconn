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
 @internalComponent
*/

#ifndef CPTPIPTRANSPORT_H_
#define CPTPIPTRANSPORT_H_

#include "cmtptransportplugin.h"

class CPTPIPConnection;

/**

PTPIP plugin transport:

Implements the transport plug-in for using MTP over TCPIP as defined by the 
PTPIP Specification.

The MTP protocol is currrently supported by Symbian over two transports, 
USB and TCP/IP. 
The PTPIP specification details how the MPT protocol can be used over TCP/IP. 
Thus the transport plugin for taking MTP over TCPIP is called PTPIP, 
after the specification which it implements. 

This class implements the API functions given by the mtp framework for a 
transport as defined in the CMTPTransportPlugin class. 
@internalComponent
 
*/
class CPTPIPTransport : public CMTPTransportPlugin
	{

public :
	static TAny* NewL(TAny* aParameter);
	~CPTPIPTransport();

public : // from CMTPTransportPlugin	
	void ModeChanged(TMTPOperationalMode aMode) ;
	void StartL(MMTPConnectionMgr& aConnectionMgr) ;
	void Stop(MMTPConnectionMgr& aConnectionMgr) ;
	TAny* GetExtendedInterface(TUid aInterfaceUid) ;

private : 
	CPTPIPTransport();
	void ConstructL();
	
private : //Owned
	/**
	The PTPIP device class connection
	*/
	CPTPIPConnection* iConnection;
  
	};   
  
#endif /*CPTPIPTRANSPORT_H_*/
