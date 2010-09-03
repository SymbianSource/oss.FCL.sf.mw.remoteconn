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

#include <mtp/rptpipframework.h>
#include "cptpipcontroller.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "rptpipframeworkTraces.h"
#endif


EXPORT_C RPTPIPFramework::RPTPIPFramework()
{
	OstTraceFunctionEntry0( RPTPIPFRAMEWORK_RPTPIPFRAMEWORK_ENTRY );
	
	OstTraceFunctionExit0( RPTPIPFRAMEWORK_RPTPIPFRAMEWORK_EXIT );
}

EXPORT_C void RPTPIPFramework::OpenL()
{
	OstTraceFunctionEntry0( RPTPIPFRAMEWORK_OPENL_ENTRY );
	iCtrl=CPTPIPController::NewL();	
	OstTraceFunctionExit0( RPTPIPFRAMEWORK_OPENL_EXIT );
}

EXPORT_C MPTPIPController& RPTPIPFramework::Controller()
{
	OstTraceFunctionEntry0( RPTPIPFRAMEWORK_CONTROLLER_ENTRY );
	OstTraceFunctionExit0( RPTPIPFRAMEWORK_CONTROLLER_EXIT );
	return (MPTPIPController&)*iCtrl;
}


EXPORT_C void RPTPIPFramework::Close()
{	
	OstTraceFunctionEntry0( RPTPIPFRAMEWORK_CLOSE_ENTRY );
	delete iCtrl;
	iCtrl=NULL;			
	OstTraceFunctionExit0( RPTPIPFRAMEWORK_CLOSE_EXIT );
}




