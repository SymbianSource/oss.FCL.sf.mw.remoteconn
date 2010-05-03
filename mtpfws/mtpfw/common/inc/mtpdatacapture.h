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

#ifndef CMTPDATACAPTURE_H
#define CMTPDATACAPTURE_H

// MTP data caputre defines

#if defined(__WINS__)
_LIT(KRequestCaptureRootDir, "c:\\test\\data\\");
#elif defined(__SERIES60_) 
_LIT(KRequestCaptureRootDir, "c:\\test\\data\\");
#else
_LIT(KRequestCaptureRootDir, "e:\\test\\data\\");
#endif

_LIT(KRequestCaptureDirPrefix, "testcase");
_LIT(KIToRDataPhaseOutputFile, "i_to_r_dataphase.dat");
_LIT(KRToIDataPhaseOutputFile, "r_to_i_dataphase.dat");
_LIT(KTestCaseFilename, "testcase.xml");

#endif
