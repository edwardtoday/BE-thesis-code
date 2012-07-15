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

#include "MVCScalableModifyCode.h"

MVCScalableModifyCode::MVCScalableModifyCode() :
	m_pcBinData( NULL ),
	m_pulStreamPacket( NULL ),
	m_uiBitCounter( 0 ),
	m_uiPosCounter( 0 ),
	m_uiCoeffCost ( 0 ),
	m_uiDWordsLeft    ( 0 ),
	m_uiBitsWritten   ( 0 ),
	m_iValidBits      ( 0 ),
	m_ulCurrentBits   ( 0 )
{

}

MVCScalableModifyCode::~MVCScalableModifyCode()
{
}


ErrVal
MVCScalableModifyCode::Destroy()
{
	delete this;
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::init( ULong* pulStream )
{
	ROT( pulStream == NULL );
	m_pulStreamPacket = pulStream;

	m_uiDWordsLeft = 0x400/4;
	m_iValidBits = 32;
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::WriteUVLC( UInt uiValue )
{
	UInt uiLength = 1;
	UInt uiTemp = ++uiValue;

	while( uiTemp != 1 )
	{
		uiTemp >>= 1;
		uiLength += 2;
	}

	RNOK( Write( uiValue, uiLength ) );
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::WriteCode( UInt uiValue, UInt uiLength )
{
	RNOK( Write( uiValue, uiLength ) );
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::WriteFlag( Bool bFlag )
{
	RNOK( Write( bFlag? 1 : 0 , 1) );
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::Write( UInt uiBits, UInt uiNumberOfBits )
{
	m_uiBitsWritten += uiNumberOfBits;

	if( (Int)uiNumberOfBits < m_iValidBits)  // one word
	{
		m_iValidBits -= uiNumberOfBits;

		m_ulCurrentBits |= uiBits << m_iValidBits;

		return Err::m_nOK;
	}


	ROT( 0 == m_uiDWordsLeft );
	m_uiDWordsLeft--;

	UInt uiShift = uiNumberOfBits - m_iValidBits;

	// add the last bits
	m_ulCurrentBits |= uiBits >> uiShift;

	*m_pulStreamPacket++ = xSwap( m_ulCurrentBits );


	// note: there is a problem with left shift with 32
	m_iValidBits = 32 - uiShift;

	m_ulCurrentBits = uiBits << m_iValidBits;

	if( 0 == uiShift )
	{
		m_ulCurrentBits = 0;
	}

	return Err::m_nOK;
}
ErrVal
MVCScalableModifyCode::WritePayloadHeader( enum h264::SEI::MessageType eType, UInt uiSize )
{
	//type
	{
		UInt uiTemp = eType;
		UInt uiByte = 0xFF;	
		while( 0xFF == uiByte )
		{
			uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
			uiTemp -= 0xFF;
			RNOK( WriteCode( uiByte, 8 ) );
		}
	}

	// size
	{
		UInt uiTemp = uiSize;
		UInt uiByte = 0xFF;

		while( 0xFF == uiByte )
		{
			uiByte  = (0xFF > uiTemp) ? uiTemp : 0xff;
			uiTemp -= 0xFF;
			RNOK( WriteCode( uiByte, 8 ) );
		}
	}
	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::WriteAlignZero()
{
	return Write( 0, m_iValidBits & 0x7 );
}

ErrVal
MVCScalableModifyCode::WriteTrailingBits()
{
	RNOK( WriteFlag( 1 ) );
	RNOK( WriteAlignZero() );
	return Err::m_nOK;
}

ErrVal 
MVCScalableModifyCode::flushBuffer()
{
	*m_pulStreamPacket = xSwap( m_ulCurrentBits );

	m_uiBitsWritten = (m_uiBitsWritten+7)/8;

	m_uiBitsWritten *= 8;

	return Err::m_nOK;
}

ErrVal
MVCScalableModifyCode::ConvertRBSPToPayload( UChar* m_pucBuffer,
																				 UChar pulStreamPacket[],
																			UInt& ruiBytesWritten,
																			UInt  uiHeaderBytes )
{
	UInt uiZeroCount    = 0;
	UInt uiReadOffset   = uiHeaderBytes;
	UInt uiWriteOffset  = uiHeaderBytes;

	//===== NAL unit header =====
	for( UInt uiIndex = 0; uiIndex < uiHeaderBytes; uiIndex++ )
	{
		m_pucBuffer[uiIndex] = (UChar)pulStreamPacket[uiIndex];
	}

	//===== NAL unit payload =====
	for( ; uiReadOffset < ruiBytesWritten ; uiReadOffset++, uiWriteOffset++ )
	{
		if( 2 == uiZeroCount && 0 == ( pulStreamPacket[uiReadOffset] & 0xfc ) )
		{
			uiZeroCount                   = 0;
			m_pucBuffer[uiWriteOffset++]  = 0x03;
		}

		m_pucBuffer[uiWriteOffset] = (UChar)pulStreamPacket[uiReadOffset];

		if( 0 == pulStreamPacket[uiReadOffset] )
		{
			uiZeroCount++;
		}
		else
		{
			uiZeroCount = 0;
		}
	}
	if( ( 0x00 == m_pucBuffer[uiWriteOffset-1] ) && ( 0x00 == m_pucBuffer[uiWriteOffset-2] ) )
	{
		m_pucBuffer[uiWriteOffset++] = 0x03;
	}
	ruiBytesWritten = uiWriteOffset;

	return Err::m_nOK;
}




ErrVal
MVCScalableModifyCode::SEICode( h264::SEI::ViewScalabilityInfoSei* pcViewScalInfoSei, MVCScalableModifyCode *pcScalableModifyCode )
{
	UInt uiNumOperationPointsMinus1 = pcViewScalInfoSei->getNumOperationPointsMinus1();
	pcScalableModifyCode->WriteUVLC( uiNumOperationPointsMinus1 );
	for( UInt uiOpId = 0; uiOpId <= uiNumOperationPointsMinus1; uiOpId++ )
	{
		pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getOperationPointId( uiOpId ) );
		pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getPriorityId( uiOpId ), 5 );
		pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getTemporalId( uiOpId ), 3 );

		UInt uiNumTargetOutputViewsMinus1 = pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId );//SEI JJ
		pcScalableModifyCode->WriteUVLC( uiNumTargetOutputViewsMinus1 );//SEI JJ

		for( UInt j = 0; j <= uiNumTargetOutputViewsMinus1; j++ )//SEI JJ
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getViewId( uiOpId, j ) );

		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) );
		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) );
		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) );
		if(!pcViewScalInfoSei->getNumTargetOutputViewsMinus1( uiOpId ))//SEI JJ 
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) );//SEI JJ 
		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) );//SEI JJ
		pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag( uiOpId ) );///SEI JJ

		if( pcViewScalInfoSei->getProfileLevelInfoPresentFlag( uiOpId ) )
		{
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getOpProfileLevelIdc( uiOpId ), 8 );//SEI JJ
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet0Flag( uiOpId ) );
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet1Flag( uiOpId ) );
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet2Flag( uiOpId ) );
			pcScalableModifyCode->WriteFlag( pcViewScalInfoSei->getOpConstraintSet3Flag( uiOpId ) );
			pcScalableModifyCode->WriteCode( 0, 4 );
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getOpLevelIdc( uiOpId ), 8 );
		}
		else
		{
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getProfileLevelInfoSrcOpIdDelta( uiOpId ) );
		}

		if( pcViewScalInfoSei->getBitRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getAvgBitrate( uiOpId ), 16 );
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getMaxBitrate( uiOpId ), 16 );
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getMaxBitrateCalcWindow( uiOpId ), 16 );
		}

		if( pcViewScalInfoSei->getFrmRateInfoPresentFlag( uiOpId ) )
		{
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getConstantFrmRateIdc( uiOpId ), 2 );
			pcScalableModifyCode->WriteCode( pcViewScalInfoSei->getAvgFrmRate( uiOpId ), 16 );
		}


		if( pcViewScalInfoSei->getViewDependencyInfoPresentFlag( uiOpId ) )//SEI JJ
		{
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j < pcViewScalInfoSei->getNumDirectlyDependentViews( uiOpId ); j++ )//SEI JJ 
				pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getDirectlyDependentViewId( uiOpId, j ) );//SEI JJ 
		}
		else
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getViewDependencyInfoSrcOpId( uiOpId ) );//SEI JJ 

		if( pcViewScalInfoSei->getParameterSetsInfoPresentFlag( uiOpId ) )//SEI JJ
		{
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j <= pcViewScalInfoSei->getNumSeqParameterSetMinus1( uiOpId ); j++ )//SEI JJ 
				pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getSeqParameterSetIdDelta( uiOpId, j ) );//SEI JJ 
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ) );//SEI JJ
			for ( UInt j=0;j<=pcViewScalInfoSei->getNumSubsetSeqParameterSetMinus1( uiOpId ); j++)//SEI JJ
				pcScalableModifyCode->WriteUVLC(pcViewScalInfoSei->getSubsetSeqParameterSetIdDelta(uiOpId,j));//SEI JJ
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ) );//SEI JJ 
			for( UInt j = 0; j <= pcViewScalInfoSei->getNumPicParameterSetMinus1( uiOpId ); j++ )//SEI JJ 
				pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getPicParameterSetIdDelta( uiOpId, j ) );//SEI JJ 
		}
		else
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getParameterSetsInfoSrcOpId( uiOpId ) );//SEI JJ 
		if (pcViewScalInfoSei->getBitstreamRestrictionInfoPresentFlag(uiOpId))
		{
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getMotionVectorsOverPicBoundariesFlag(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getMaxBytesPerPicDenom(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getMaxBitsPerMbDenom(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthHorizontal(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getLog2MaxMvLengthVertical(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getNumReorderFrames(uiOpId));
			pcScalableModifyCode->WriteUVLC( pcViewScalInfoSei->getMaxDecFrameBuffering(uiOpId));
		}

	}// for

	return Err::m_nOK;
}
