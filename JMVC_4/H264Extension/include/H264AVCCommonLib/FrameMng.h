/*
********************************************************************************

NOTE - One of the two copyright statements below may be chosen
       that applies for the software.

********************************************************************************

This software module was originally developed by

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




#if !defined(AFX_FRAMEMNG_H__FCFD4695_2766_4D95_BFD2_B2496827BC03__INCLUDED_)
#define AFX_FRAMEMNG_H__FCFD4695_2766_4D95_BFD2_B2496827BC03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "H264AVCCommonLib/FrameUnit.h"
#include "H264AVCCommonLib/YuvBufferCtrl.h"
#include <algorithm>
#include <list>



H264AVC_NAMESPACE_BEGIN

class QuarterPelFilter;

#if defined( WIN32 )
# pragma warning( disable: 4251 )
#endif


class H264AVCCOMMONLIB_API FrameMng
{
  enum OutputMode
  {
    OM_BUMPING = 0,
    OM_DELAYED,
    OM_DELAYED_LOCKED,
    OM_FLUSH
  };

  class FUList : public std::list<FrameUnit*>
  {
  public:
    FrameUnit* popBack()  { FrameUnit* pcFU = back();   pop_back();   return pcFU; }
    FrameUnit* popFront() { FrameUnit* pcFU = front();  pop_front();  return pcFU; }
// big clean up, and fix, add view id for all list getting. ying

  Void setRefPicList( RefPicList<RefPic>& rcRefPicList , SliceHeader& rcSH)
  {
    for( iterator iter = begin(); iter != end(); iter++ )
    {
      if( (*iter)->isUsed() && (*iter)->getPicBuffer() && rcSH.getViewId() == (*iter)->getFrame().getViewId() ) 
	    {
		    rcRefPicList.next().setFrame( &( (*iter)->getFrame() ) );
	    }
    }
  }

  Void setRefFrameList( RefPicList<Frame*>& rcRefFrameList , SliceHeader& rcSH) 
  {
    for( iterator iter = begin(); iter != end(); iter++ )
    {
      if( (*iter)->isUsed() && (*iter)->getPicBuffer()&& rcSH.getViewId() == (*iter)->getFrame().getViewId() )  
      {
         rcRefFrameList.add( &( (*iter)->getFrame() ) );
      }
    }
  }

    Void setRefFrameUnitList( RefPicList<FrameUnit*>& rcRefFrameUnitList )
    {
      for( iterator iter = begin(); iter != end(); iter++ )
      {
        if( (*iter)->isUsed() )
        {
            rcRefFrameUnitList.add( *iter );
        }
      }
    }

    iterator findShortTerm( UInt uiPicNum )
    {
      for( iterator iter = begin(); iter != end(); iter++ )
      {
        if( (*iter)->getFrameNumber() == uiPicNum )
        {
          return iter;
        }
      }
      return end();
    }
  

            iterator findShortTerm_iter( FrameUnit *FrameU )
            {
              for( iterator iter = begin(); iter != end(); iter++ )
              {
                if( (*iter) == FrameU )
                {
                  return iter;
                }
              }
              return end();
            }

            iterator findShortTermMVC( UInt uiPicNum, UInt uiViewId )
            {
              for( iterator iter = begin(); iter != end(); iter++ )
              {
                if( ((*iter)->getFrameNumber() == uiPicNum) && ((*iter)->getFrame().getViewId() == uiViewId) )
                {
                  return iter;
                }
              }
              return end();
            }

//JVT-V043  {{
            iterator findInterView(SliceHeader& rcSH, UInt uiViewIdx, ListIdx eListIdx )
            {
              Bool bAnchor      = rcSH.getAnchorPicFlag();
              UInt targetViewId = rcSH.getSPS().getSpsMVC()->getViewIDByViewIndex( rcSH.getViewId(), uiViewIdx, eListIdx, bAnchor );
      
              for( iterator iter = begin(); iter != end(); iter++ )
              {
                if( ((*iter)->getFrame().getViewId() == targetViewId && 
                      rcSH.getPoc() == (*iter)->getFrame().getPOC() ) )
                {
                  return iter;
                }
              }
              return end();
            }

            Void setRefFrameListFGSMVCView( RefPicList<Frame*>& rcRefFrameList, 
                                                 SliceHeader& rcSH, enum ListIdx eListIdx ) // memory 
            {
              Bool bAnchor =rcSH.getAnchorPicFlag() ;
              UInt n = rcSH.getSPS().getSpsMVC()->getNumRefsForListX(rcSH.getViewId(), eListIdx, bAnchor);
                      
              for( UInt ui = 0; ui < n; ui++)
              {
                for( iterator iter = begin(); iter != end(); iter++ )
                {
                  FrameUnit* pcFU = (*iter);
                  if( pcFU->isUsed() && !pcFU->getBaseRep() ) //JVT-S036 lsj 
                  {
                    if((/*pcFU->getFGSPicBuffer() || */pcFU->getPicBuffer()) && 
                      rcSH.getSPS().getSpsMVC()->getViewIDByViewIndex(rcSH.getViewId(), ui, eListIdx, bAnchor)== pcFU->getFrame().getViewId() 
					&& rcSH.getPoc() == pcFU->getFrame().getPOC() && pcFU->getFrame().getInterViewFlag()) //JVT-W056   
											
                    {
                      rcRefFrameList.add(  & pcFU->getFrame()  ); //				  rcRefFrameList.add(  &( pcFU->getFGSPicBuffer() ? pcFU->getFGSFrame() : pcFU->getFrame() ) );
                    }
                  }
                }
              }        
            }

            Void setRefFrameListFGSMVCViewNonRef(RefPicList<Frame*>& rcRefFrameList, 
                                                 SliceHeader& rcSH, enum ListIdx eListIdx ) //memory 
            {
              Bool bAnchor =rcSH.getAnchorPicFlag() ;
              UInt n = rcSH.getSPS().getSpsMVC()->getNumRefsForListX(rcSH.getViewId(), eListIdx, bAnchor);

              for( UInt ui = 0; ui < n; ui++)
              {
                for( iterator iter = begin(); iter != end(); iter++ )
                {
                  FrameUnit* pcFU = (*iter);
                  if((/*pcFU->getFGSPicBuffer() || */pcFU->getPicBuffer()) && 
                     rcSH.getSPS().getSpsMVC()->getViewIDByViewIndex(rcSH.getViewId(), ui, eListIdx, bAnchor) == pcFU->getFrame().getViewId() 
                     && rcSH.getPoc() == pcFU->getFrame().getPOC() ) 
                  {
                    //rcRefFrameList.add(  &( pcFU->getFGSPicBuffer() ? pcFU->getFGSFrame() : pcFU->getFrame() ) );
                    rcRefFrameList.add(  & pcFU->getFrame() ); // memory
                  }
                }
              }        
            }
//JVT-V043  }}
          };

  typedef FUList::iterator          FUIter;
  typedef FUList::reverse_iterator  FURIter;

  class FrameUnitBuffer
  {
  public:
    FrameUnitBuffer();
    ~FrameUnitBuffer();
    ErrVal init( YuvBufferCtrl* pcYuvFullPelBufferCtrl, YuvBufferCtrl* pcYuvHalfPelBufferCtrl );
    ErrVal uninit();
    ErrVal getFrameUnit( FrameUnit*& rpcFrameUnit );
    ErrVal releaseFrameUnit( FrameUnit* pcFrameUnit );
    YuvBufferCtrl* getYuvFullPelBufferCtrl()  { return m_pcYuvFullPelBufferCtrl;  }

  protected:
    FUList            m_cFreeList;
    YuvBufferCtrl*    m_pcYuvFullPelBufferCtrl;
    YuvBufferCtrl*    m_pcYuvHalfPelBufferCtrl;
  };

  class PocOrder
  {
  public:
    Int operator() ( const Frame* pcFrame1, const Frame* pcFrame2 )
    {
      return pcFrame1->getPOC() < pcFrame2->getPOC();
    }
  };

protected:
  FrameMng              ();
	virtual ~FrameMng     ();

public:

          FrameUnit*  getCurrentFrameUnit   () { return m_pcCurrentFrameUnit; }
          IntFrame*   getRefinementIntFrame () { return m_pcRefinementIntFrame; }
          IntFrame*   getRefinementIntFrame2() { return m_pcRefinementIntFrameSpatial; }
          IntFrame*   getPredictionIntFrame()  { return m_pcPredictionIntFrame; }
  
          ErrVal			xSlidingWindowUpdateBase    ( UInt mCurrFrameNum); //JVT-S036 
          ErrVal			xMMCOUpdateBase				( SliceHeader* rcSH ); //JVT-S036 

          ErrVal initSlice( SliceHeader *rcSH );
          ErrVal initSPS( const SequenceParameterSet& rcSPS );


          ErrVal initFrame( SliceHeader& rcSH, PicBuffer* pcPicBuffer );

  
          ErrVal initPic( SliceHeader& rcSH );

          static  ErrVal  create          ( FrameMng*& rpcFrameMng );
          static  UInt    MaxRefFrames    ( UInt uiLevel, UInt uiNumMbs );
          ErrVal storePicture( const SliceHeader& rcSH );

//  ErrVal storeFGSPicture( PicBuffer* pcPicBuffer );
  ErrVal setRefPicLists( SliceHeader& rcSH, Bool bDoNotRemap );
  ErrVal  destroy                 ();
  ErrVal  init                    ( YuvBufferCtrl* pcYuvFullPelBufferCtrl, YuvBufferCtrl* pcYuvHalfPelBufferCtrl = NULL, QuarterPelFilter* pcQuarterPelFilter = NULL );
  ErrVal  uninit                  ();
  ErrVal  RefreshOrederedPOCList  (); //JVT-S036 
  ErrVal  setPicBufferLists       ( PicBufferList& rcPicBufferOutputList, PicBufferList& rcPicBufferUnusedList );
  ErrVal  outputAll               ();
  ErrVal  getRecYuvBuffer         ( YuvPicBuffer*& rpcRecYuvBuffer );

  FrameUnit*        getReconstructedFrameUnit( Int iPoc );

  YuvBufferCtrl*    getYuvFullPelBufferCtrl() { return m_cFrameUnitBuffer.getYuvFullPelBufferCtrl();  }
  //void    SetCodeAsVFrameFlag(const bool flag) { m_codeAsVFrame = flag; }
  //bool    CodeAsVFrameP() const {return m_codeAsVFrame; }


protected:
  ErrVal            xCheckMissingFrameNums( SliceHeader& rcSH );

  ErrVal            xSetReferenceLists          ( SliceHeader& rcSH );
  ErrVal            xSetReferenceListsMVC       ( SliceHeader& rcSH );
  ErrVal            xClearListsIDR              ( const SliceHeader& rcSH );
  ErrVal            xManageMemory               ( const SliceHeader& rcSH );
  ErrVal            xSlidingWindowUpdate        ();
  ErrVal            xStoreCurrentPicture        ( const SliceHeader& rcSH );                // MMCO 6
  ErrVal            xReferenceListRemapping     ( SliceHeader& rcSH, ListIdx eListIdx );
  ErrVal            xMmcoMarkShortTermAsUnused( const FrameUnit* pcCurrFrameUnit, UInt uiDiffOfPicNums );
  ErrVal            xMmcoMarkShortTermAsUnusedBase( const FrameUnit* pcCurrFrameUnit, UInt uiDiffOfPicNums ); //JVT-S036 

          ErrVal            xSetOutputListMVC              ( FrameUnit* pcFrameUnit, UInt uiNumOfViews );

          private:
          UInt              xSortPocOrderedList                 ();
          ErrVal            xSetInitialReferenceListPFrame      ( SliceHeader& rcSH );
          ErrVal            xSetInitialReferenceListBFrame      ( SliceHeader& rcSH );
          __inline ErrVal   xRemoveFromRefList( FUList& rcFUList, FUIter iter );
          __inline ErrVal   xRemoveFromRefList( FUList& rcFUList );
          __inline ErrVal   xRemove           ( FrameUnit* pcFrameUnit );
          __inline ErrVal   xAddToFreeList    ( FrameUnit* pcFrameUnit );
          __inline ErrVal   xAddToFreeList    ( FUList& rcFUList );
          __inline Bool     xFindAndErase     ( FUList& rcFUList, FrameUnit* pcFrameUnit );

          ErrVal            xMmcoMarkShortTermAsUnusedMVC( const FrameUnit* pcCurrFrameUnit, UInt uiDiffOfPicNums, UInt uiCurrViewId );
          ErrVal            xDumpRefList( ListIdx eListIdx, SliceHeader& rcSH );
          ErrVal            xSetBFrameListMVC ( SliceHeader& rcSH); 
          ErrVal            xSetPFrameListMVC ( SliceHeader& rcSH); 
private:
  Bool              m_bInitDone;
  QuarterPelFilter* m_pcQuarterPelFilter;
  UInt              m_uiPrecedingRefFrameNum;

  PicBufferList     m_cPicBufferOutputList;
  PicBufferList     m_cPicBufferUnusedList;
  FrameUnit*        m_pcOriginalFrameUnit;
  FrameUnit*        m_pcCurrentFrameUnit;
  FrameUnit*		m_pcCurrentFrameUnitBase; //JVT-S036 

  RefPicList<Frame*>     m_cPocOrderedFrameList;

  Int               m_iEntriesInDPB;
  Int               m_iMaxEntriesinDPB;

  UInt              m_uiNumRefFrames;
  UInt              m_uiMaxFrameNumCurr;
  UInt              m_uiMaxFrameNumPrev;
  FUList            m_cShortTermList;
  FUList            m_cNonRefList;
  FUList            m_cOrderedPOCList;

  FrameUnitBuffer   m_cFrameUnitBuffer;

  static UInt       m_uiDBPMemory[256];
  IntFrame*         m_pcRefinementIntFrame;
  IntFrame*         m_pcRefinementIntFrameSpatial;
  IntFrame*         m_pcPredictionIntFrame;
  Bool                m_codeAsVFrame;
  UInt              m_uiLastViewId;
        };

#if defined( WIN32 )
# pragma warning( default: 4251 )
#endif



H264AVC_NAMESPACE_END


#endif // !defined(AFX_FRAMEMNG_H__FCFD4695_2766_4D95_BFD2_B2496827BC03__INCLUDED_)
