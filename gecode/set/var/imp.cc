/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/set.hh"

#include "gecode/set/var/imp-body.icc"

namespace Gecode { namespace Set {

  /*
   * "Standard" tell operations
   *
   */
  ModEvent
  SetVarImp::cardMin_full(Space* home) {
    ModEvent me = ME_SET_CARD;
    if (_cardMin == lub.size()) {
      glb.become(home, lub);
      me = ME_SET_VAL;
    }
    notify(home, me);
    return me;
  }

  ModEvent
  SetVarImp::cardMax_full(Space* home) {
    ModEvent me = ME_SET_CARD;
    if (_cardMax == glb.size()) {
      lub.become(home, glb);
      me = ME_SET_VAL;
    }
    notify(home, me);
    return me;
  }

  ModEvent
  SetVarImp::processLubChange(Space* home) {
    ModEvent me = ME_SET_LUB;
    if (_cardMax > lub.size()) {
      _cardMax = lub.size();
      if (_cardMin > _cardMax) {
        glb.become(home, lub);
        _cardMin = glb.size();
        _cardMax = glb.size();
        return ME_SET_FAILED;
      }
      me = ME_SET_CLUB;
    }
    if (_cardMax == lub.size() && _cardMin == _cardMax) {
      glb.become(home, lub);
      me = ME_SET_VAL;
    }
    notify(home, me);
    return me;
  }

  ModEvent
  SetVarImp::processGlbChange(Space* home) {
    ModEvent me = ME_SET_GLB;
    if (_cardMin < glb.size()) {
      _cardMin = glb.size();
      if (_cardMin > _cardMax) {
        glb.become(home, lub);
        _cardMin = glb.size();
        _cardMax = glb.size();
        return ME_SET_FAILED;
      }
      me = ME_SET_CGLB;
    }
    if (_cardMin == glb.size() && _cardMin == _cardMax) {
      lub.become(home, glb);
      me = ME_SET_VAL;
    }
    notify(home, me);
    return me;
  }

  /*
   * Copying variables
   *
   */

  forceinline
  SetVarImp::SetVarImp(Space* home, bool share, SetVarImp& x)
    : SetVarImpBase(home,share,x),
      _cardMin(x._cardMin), _cardMax(x._cardMax) {
    lub.update(home, x.lub);
    if (x.assigned()) {
      glb.become(home,lub);
    } else {
      glb.update(home,x.glb);
    }
  }


  SetVarImp*
  SetVarImp::perform_copy(Space* home, bool share) {
    return new (home) SetVarImp(home,share,*this);
  }

  Reflection::Arg*
  SetVarImp::spec(Space* home, Reflection::VarMap& m) {
    int specIndex = m.getIndex(this);
    if (specIndex != -1)
      return new Reflection::VarArg(specIndex);

    int glbsize = 0;
    {
      BndSetRanges lrs(glb);
      while (lrs()) { glbsize++; ++lrs; }
    }
    
    Reflection::ArrayArg<int>* glbdom = new Reflection::ArrayArg<int>(glbsize*2);
    BndSetRanges lr(glb);
    for (int count=0; lr(); ++lr) {
      (*glbdom)[count++] = lr.min();
      (*glbdom)[count++] = lr.max();
    }

    int lubsize = 0;
    {
      BndSetRanges urs(lub);
      while (urs()) { lubsize++; ++urs; }
    }
    
    BndSetRanges ur(lub);
    Reflection::ArrayArg<int>* lubdom = new Reflection::ArrayArg<int>(lubsize*2);
    for (int count=0; ur(); ++ur) {
      (*lubdom)[count++] = ur.min();
      (*lubdom)[count++] = ur.max();
    }

    Reflection::PairArg* pair =
      new Reflection::PairArg(new Reflection::PairArg(glbdom, new Reflection::IntArg(_cardMin)),
                        new Reflection::PairArg(lubdom, new Reflection::IntArg(_cardMax)));

    Reflection::VarSpec* spec = new Reflection::VarSpec(VTI_SET, pair);
    return (new Reflection::VarArg(m.put(this, spec)));
  }

}}

// STATISTICS: set-var

