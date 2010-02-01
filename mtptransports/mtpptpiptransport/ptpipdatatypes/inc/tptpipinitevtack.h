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
#ifndef _TPTPIPInitEvtAck_H_
#define _TPTPIPInitEvtAck_H_

#include <mtp/tmtptypeflatbase.h>

/**
Defines the PTP/IP InitAck DataSet.
@internalComponent
 */
class TPTPIPInitEvtAck: public TMTPTypeFlatBase 
    {
public:

	/**
	PTP/IP Init Ack packet.
	*/
    enum TElements
        {
        ELength,
        EType,
        ENumElements
        };
        
public:
    IMPORT_C TPTPIPInitEvtAck();
     
public: // From TMTPTypeFlatBase
    IMPORT_C TUint Type() const;
    
private: // From TMTPTypeFlatBase
    IMPORT_C const TMTPTypeFlatBase::TElementInfo& ElementInfo(TInt aElementId) const;

private:
	/**
	The dataset size in bytes.
	*/
    static const TInt                               KSize = 8;
    
	/**
	The dataset element metadata table content.
	*/
    static const TMTPTypeFlatBase::TElementInfo     iElementMetaData[];
    
	/**
	The dataset element metadata table.
	*/
    const TFixedArray<TElementInfo, ENumElements>   iElementInfo;
    
	/**
	The data buffer.
	*/
    TBuf8<KSize>                                    iBuffer;        
    };

#endif // _TPTPIPInitEvtAck_H_


