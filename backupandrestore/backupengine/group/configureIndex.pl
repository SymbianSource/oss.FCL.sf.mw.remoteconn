#!/usr/local/bin/perl
# unused in NT but there for completeness

# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:
# Configures the index.hhc file for generating compiled html help
# by removing lines that should not be displayed

# Usage: configureIndex index.hhc > index.hhc

if ($#ARGV != 0)
	{
	die "Usage: configureIndex index.hhc > index.hhc\n";
	}

$file=shift(@ARGV);
open (FILE, $file) or die "Can't open $file: $!\n";
my $printLine = 1;
my $inRelatedPages = 0;
my $compoundMembersLine = "\n";
while (<FILE>)
{
    if (/^<LI>.*/)
    {
	if (/.*Compound List.*/ || /.*Namespace List.*/ || 
	    /.*Namespace Members.*/ || /.*File Members.*/)
	{
	    $printLine = 0;
	}
	elsif ( /.*Compound Members.*/)
	{
	    $compoundMembersLine = $_;
	    ##print $compoundMembersLine;
	}
	else
	{
	    if (/.*Related Pages.*/)
	    {
		$inRelatedPages = 1;
	    }
	    $printLine = 1;
	    print $_;
	}
    }
    elsif ( /^\s*<\/UL>$/ && $inRelatedPages == 1)
    {
	$inRelatedPages = 0;
	print "  ";
	print $compoundMembersLine;
	print $_;
    }
    else
    {
	if ($printLine == 1)
	{
	    print $_;
	}
    }
}
close FILE;
