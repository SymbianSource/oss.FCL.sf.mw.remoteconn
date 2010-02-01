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


#ifndef CPTPIPSOCKETHANDLER_H_
#define CPTPIPSOCKETHANDLER_H_

#include <e32base.h>
#include <es_sock.h>
#include <mtp/mmtptype.h>


const TInt KMaxBufSize=64;

enum TSocketHandlerState
{
	EReadState,EWriteState
};
/*
 Handles the Common Socket-Operations
*/
class CPTPIPSocketHandler:public CActive
{
  	public:
  			
  		static CPTPIPSocketHandler* NewL();
  		static CPTPIPSocketHandler *NewLC();
  		~CPTPIPSocketHandler();   		 		
  		void ReadFromSocket(MMTPType& aData,TRequestStatus& aStatus);
  		void WriteToSocket(MMTPType& aData,TRequestStatus& aStatus);  		  		
  		RSocket& Socket();  			 		  				  			      	    
		TSocketHandlerState& State();
  		
  	private:
  	
  		void RunL();
  		void DoCancel();  		
  		TInt RunError(TInt aErr);
  		CPTPIPSocketHandler();
  		void ConstructL();  		  		  		
  		  		
  	private:
  	
  		TPtr8 iReceiveChunk;
  		TPtrC8 iWriteChunk;  		  	  			  		
  		TRequestStatus* iCallerStatus;
  		TBool iChunkStatus;  		  				 		  
  		RSocket iSocket; 
  		MMTPType* iReadData,*iWriteData;
  		TSocketHandlerState iState;
  		TSockXfrLength iLen;
  		TInt iLength;
};


#endif /*CPTPIPSOCKETHANDLER_H_*/
