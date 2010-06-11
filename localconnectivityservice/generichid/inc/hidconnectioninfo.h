/*
* Copyright (c) 2008-2008 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  ?Description
*
*/


#ifndef C_CONNECTIONINFO_H_
#define C_CONNECTIONINFO_H_


#include <e32base.h>

class CReportRoot;
class CHidDriver;

/**
 *  CConnectionInfo
 *  Connection information
 * 
 *
 *  @lib generichid.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS ( CConnectionInfo ) : public CBase
    {
 
public:
    /**
     * Two-phased constructor.
     * @param aConnectionID Connection identifier
     * @param aReportRoot Connection ReportRoot
     */
    static CConnectionInfo* NewL(TInt aConnectionID, CReportRoot* aReportRoot);    
    static CConnectionInfo* NewLC(TInt aConnectionID, CReportRoot* aReportRoot);
    
    /**
    * Destructor.
    */
    virtual ~CConnectionInfo();
    
public:
    /**
     * Get Connection id
     * 
     * @since S60 v5.0     
     * @return connection id
     */
    TInt            ConnectionID();
    
    /**
     * Get report id
     * 
     * @since S60 v5.0      
     * @return report id 
     */
    CReportRoot*    ReportRoot();
    
    /**
     * Set last command handler
     *
     * @since S60 v5.0
     * @param aHidDriverItem hiddriver item     
     * @return None
     */
    void            SetLastCommandHandler(CHidDriver* aHidDriverItem);
    
    /**
     * Set last command handler
     *
     * @since S60 v5.0
     * @param aHidDriverItem hiddriver item     
     * @return None
     */
    CHidDriver*     ReturnLastCommandHandler();
    
private:
    CConnectionInfo(TInt aConnectionID, CReportRoot* aReportRoot);

private:

    /**
     * Connection id
     */
    TInt  iConnectionID;
    
    /**
     * Connection id
     * Own.
     */ 
    CReportRoot*    iReportRoot;
    
    /**
     * Pointer to last driver which handled last command
     * Not own.
     */
    CHidDriver*     iLastCmdDriver;
    };
        
#endif /* C_CONNECTIONINFO_H_ */
