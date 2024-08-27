/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2018 Live Networks, Inc.  All rights reserved.
// Framed Filters
// Implementation

#include "FramedFilter.hh"
#include "../../live_ipv4_ipv6.h"
////////// FramedFilter //////////
#include <string.h>

void FramedFilter::detachInputSource() {
  if (fInputSource != NULL) 
  {
	  if (fInputSource->GetFlag() == LIVE_FLAG)
	  {
			fInputSource->stopGettingFrames();
	  }
	  else
	  {
		  FILE *f = fopen("FramedFilter.txt", "wb");
		  if (NULL != f)
		  {
			  fclose(f);
		  }
	  }
    reassignInputSource(NULL);
  }
}

FramedFilter::FramedFilter(UsageEnvironment& env,
			   FramedSource* inputSource)
  : FramedSource(env),
    fInputSource(inputSource) {
}

FramedFilter::~FramedFilter() {
	if ( (fInputSource) && (fInputSource->GetFlag() == LIVE_FLAG) )
	{
		Medium::close(fInputSource);
	}
	else if (fInputSource)
	{
		  FILE *f = fopen("FramedFilter2.txt", "wb");
		  if (NULL != f)
		  {
			  fclose(f);
		  }
	}
	fInputSource = NULL;
}

// Default implementations of needed virtual functions.  These merely
// call the same function in the input source - i.e., act like a 'null filter

char const* FramedFilter::MIMEtype() const {
  if (fInputSource == NULL) return "";

  return fInputSource->MIMEtype();
}

void FramedFilter::getAttributes() const {
  if (fInputSource != NULL) fInputSource->getAttributes();
}

void FramedFilter::doStopGettingFrames() {
  FramedSource::doStopGettingFrames();
  if (fInputSource != NULL) fInputSource->stopGettingFrames();
}
