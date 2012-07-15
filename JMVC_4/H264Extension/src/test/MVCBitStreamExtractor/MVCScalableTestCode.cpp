/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************

This software module was based the software developed by

Heiko Schwarz    (Fraunhofer HHI),
Tobias Hinz      (Fraunhofer HHI),
Karsten Suehring (Fraunhofer HHI)

in the course of development of the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video
Coding) for reference purposes and its performance may not have been optimized.
This software module is an implementation of one or more tools as specified by
the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding).

Those intending to use this software module in products are advised that its
use may infringe existing patents. ISO/IEC have no liability for use of this
software module or modifications thereof.

Assurance that the originally developed software module can be used
(1) in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) once the
ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) has been adopted; and
(2) to develop the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding): 

To the extent that Fraunhofer HHI owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, Fraunhofer HHI will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

Fraunhofer HHI retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards. 

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005. 

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The Fraunhofer HHI hereby donate this source code to the ITU, with the following
understanding:
    1. Fraunhofer HHI retain the right to do whatever they wish with the
       contributed source code, without limit.
    2. Fraunhofer HHI retain full patent rights (if any exist) in the technical
       content of techniques and algorithms herein.
    3. The ITU shall make this code available to anyone, free of license or
       royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from 
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/

/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
that applies for the software.

********************************************************************************
This software module was originally created for Nokia, Inc.
Author: Liu Hui (liuhui@mail.ustc.edu.cn)

in the course of development of the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video
Coding) for reference purposes and its performance may not have been optimized.
This software module is an implementation of one or more tools as specified by
the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding).

Those intending to use this software module in products are advised that its
use may infringe existing patents. ISO/IEC have no liability for use of this
software module or modifications thereof.

Assurance that the originally developed software module can be used
(1) in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) once the
ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) has been adopted; and
(2) to develop the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding):

To the extent that Nokia Inc.  owns patent rights that would be required to
make, use, or sell the originally developed software module or portions thereof
included in the ISO/IEC 14496-10:2005 Amd.1 (Scalable Video Coding) in a
conforming product, Nokia Inc.  will assure the ISO/IEC that it is willing
to negotiate licenses under reasonable and non-discriminatory terms and
conditions with applicants throughout the world.

Nokia Inc. retains full right to modify and use the code for its own
purpose, assign or donate the code to a third party and to inhibit third
parties from using the code for products that do not conform to MPEG-related
ITU Recommendations and/or ISO/IEC International Standards.

This copyright notice must be included in all copies or derivative works.
Copyright (c) ISO/IEC 2005.

********************************************************************************

COPYRIGHT AND WARRANTY INFORMATION

Copyright 2005, International Telecommunications Union, Geneva

The Nokia Inc.  hereby donate this source code to the ITU, with the following
understanding:
1. Nokia Inc. retain the right to do whatever they wish with the
contributed source code, without limit.
2. Nokia Inc. retain full patent rights (if any exist) in the technical
content of techniques and algorithms herein.
3. The ITU shall make this code available to anyone, free of license or
royalty fees.

DISCLAIMER OF WARRANTY

These software programs are available to the user without any license fee or
royalty on an "as is" basis. The ITU disclaims any and all warranties, whether
express, implied, or statutory, including any implied warranties of
merchantability or of fitness for a particular purpose. In no event shall the
contributor or the ITU be liable for any incidental, punitive, or consequential
damages of any kind whatsoever arising from the use of these programs.

This disclaimer of warranty extends to the user of these programs and user's
customers, employees, agents, transferees, successors, and assigns.

The ITU does not represent or warrant that the programs furnished hereunder are
free of infringement of any third-party patents. Commercial implementations of
ITU-T Recommendations, including shareware, may be subject to royalty fees to
patent holders. Information regarding the ITU-T patent policy is available from
the ITU Web site at http://www.itu.int.

THIS IS NOT A GRANT OF PATENT RIGHTS - SEE THE ITU-T PATENT POLICY.

********************************************************************************
*/


#include "MVCScalableTestCode.h"

MVCScalableTestCode::MVCScalableTestCode()
{
}
MVCScalableTestCode::~MVCScalableTestCode()
{
}

ErrVal
MVCScalableTestCode::Destroy()
{
	delete this;
	return Err::m_nOK;
}
ErrVal
MVCScalableTestCode::WriteUVLC( UInt uiValue )
{
	UInt uiLength = 1;
	UInt uiTemp = ++uiValue;
	while( uiTemp != 1 )
	{
		uiTemp >>= 1;
		uiLength += 2;
	}
	RNOK( WriteCode( uiValue, uiLength ) );
	return Err::m_nOK;
}

ErrVal
MVCScalableTestCode::SEICode( h264::SEI::ViewScalabilityInfoSei* pcViewScalInfoSei, MVCScalableTestCode *pcScalableTestCode )
{
	UInt uiNumOperationPointsMinus1 = pcViewScalInfoSei->getNumOperationPointsMinus1();
	pcScalableTestCode->WriteUVLC( uiNumOperationPointsMinus1 );
	for( UInt uiOpId = 0; uiOpId <= uiNumOperationPointsMinus1; uiOpId++ )
	{
		pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getOperationPointId( uiOpId ) );
		pcScalableTestCode->WriteCode( pcViewScalInfoSei->getPriorityId( uiOpId ), 5 );
		pcScalableTestCode->WriteCode( pcViewScalInfoSei->getTemporalId( uiOpId ), 3 );

		UInt uiNumTargetOutputViewsMinus1 = pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId );//SEI JJ
		pcScalableTestCode->WriteUVLC( uiNumTargetOutputViewsMinus1 );//SEI JJ

		for( UInt j = 0; j <= uiNumTargetOutputViewsMinus1; j++ )//SEI JJ
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getViewId( uiOpId, j ) );

		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) );
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) );
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) );
		if(!pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId ))//SEI JJ
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) );//SEI JJ
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) );//SEI JJ
		pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag( uiOpId ) );//SEI JJ


		if( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) )
		{
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getOpProfileLevelIdc( uiOpId ), 8 );//SEI JJ
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet0Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet1Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet2Flag( uiOpId ) );
			pcScalableTestCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet3Flag( uiOpId ) );
			pcScalableTestCode->WriteCode( 0, 4 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getOpLevelIdc( uiOpId ), 8 );
		}
		else
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getProfileLevelInfoSrcOpIdDelta( uiOpId ) );
		}

		if( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getAvgBitrate( uiOpId ), 16 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getMaxBitrate( uiOpId ), 16 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getMaxBitrateCalcWindow( uiOpId ), 16 );
		}

		if( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getConstantFrmRateIdc( uiOpId ), 2 );
			pcScalableTestCode->WriteCode( pcViewScalInfoSei->getAvgFrmRate( uiOpId ), 16 );
		}

		if( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) )//SEI JJ 
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j < pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ); j++ )//SEI JJ 
				pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getDirectlyDependentViewId( uiOpId, j ) );//SEI JJ 
		}
		else
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getViewDependencyInfoSrcOpId( uiOpId ) );//SEI JJ 

		if( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) )//SEI JJ 
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j <= pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ); j++ )//SEI JJ 
				pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getSeqParameterSetIdDelta( uiOpId, j ) );//SEI JJ 
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ) );//SEI JJ
			for ( UInt j=0;j<=pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ); j++)//SEI JJ
				pcScalableTestCode->WriteUVLC(pcViewScalInfoSei->getSubsetSeqParameterSetIdDelta(uiOpId,j));//SEI JJ
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ) );//SEI JJ
			for( UInt j = 0; j <= pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ); j++ )//SEI JJ 
				pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getPicParameterSetIdDelta( uiOpId, j ) );//SEI JJ 
		}
		else
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getParameterSetsInfoSrcOpId( uiOpId ) );//SEI JJ 
		if (pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag(uiOpId))
		{
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMotionVectorsOverPicBoundariesFlag(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMaxBytesPerPicDenom(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMaxBitsPerMbDenom(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthHorizontal(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthVertical(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getNumReorderFrames(uiOpId));
			pcScalableTestCode->WriteUVLC( pcViewScalInfoSei->getMaxDecFrameBuffering(uiOpId));
		}
	}// for

	return Err::m_nOK;
}

