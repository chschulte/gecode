/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christopher Mears <Chris.Mears@monash.edu>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christopher Mears, 2011
 *     Christian Schulte, 2011
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/int/precede.hh>

#include <algorithm>

namespace Gecode {

  void
  precede(Home home, const IntVarArgs& x, int s, int t, IntPropLevel) {
    using namespace Int;
    Limits::check(s,"Int::precede");
    Limits::check(t,"Int::precede");
    GECODE_POST;

    ViewArray<IntView> y(home, x);
    GECODE_ES_FAIL(Precede::Single<IntView>::post(home, y, s, t));
  }

  void
  precede(Home home, const IntVarArgs& x, int l, IntPropLevel ipl) {
    using namespace Int;
    Limits::check(l,"Int::precede");
    GECODE_POST;

    int u = l + x.size() - 1;
    for (int i=0; i<x.size(); i++) {
      GECODE_ME_FAIL(IntView(x[i]).gq(home,l));
      GECODE_ME_FAIL(IntView(x[i]).lq(home,u));
    }
    IntVarArgs h(home,x.size(),l,u);
    rel(home, h[0], IRT_LQ, l);
    IntVar z(home,l-1,l-1);
    max(home, x[0], z, h[0], ipl);
    for (int i=1; i<x.size(); i++) {
      IntArgs c({1,-1});
      IntVarArgs y({h[i],h[i-1]});
      linear(home, c, y, IRT_LQ, 1);
      max(home, x[i], h[i-1], h[i], ipl);
    }
  }

  void
  precede(Home home, const IntVarArgs& x, const IntArgs& c, IntPropLevel ipl) {
    using namespace Int;
    int n = c.size();
    if (n < 2)
      return;
    if (n == 2) {
      precede(home, x, c[0], c[1], ipl);
      return;
    }
    for (int i=0; i<n; i++)
      Limits::check(c[i],"Int::precede");

    int l = x[0].min();
    int u = x[0].max();
    for (int i=1; i<n; i++) {
      l = std::min(l,x[i].min());
      u = std::max(u,x[i].max());
    }

    if ((l == c[0]) && (u == c[n-1]) && (x.size() == u-l+1)) {
      for (int i=1; i<n; i++)
        if (c[i-1] + 1 != c[i])
          goto normal;
      precede(home, x, l, ipl);
      return;
    }
  normal:
    GECODE_POST;
    if (u-l <= Precede::limit) {
      IntSharedArray p(u-l+1);
      for (int j=0; j<=u-l; j++)
        p[j]=0;
      for (int i=0; i<n; i++) 
        p[c[i]-l]=i+1;
      std::cout << "p: " << p << std::endl;
      IntVarArgs y(home,x.size(),0,c.size()+1);
      for (int i=0; i<x.size(); i++) {
        //       IntVar z(home,l,u);
        IntVar z(home,-10000,10000);
        linear(home,IntArgs({1,-1}),IntVarArgs({x[i],z}),IRT_EQ,l,IPL_DOM);
        element(home,p,z,y[i]);
      }
      static_cast<Space&>(home).status();
      std::cout << "y: " << y << std::endl;
      precede(home,y,1,ipl);
    } else {
      for (int i=1; i<c.size(); i++) {
        ViewArray<IntView> y(home, x);
        GECODE_ES_FAIL(Precede::Single<IntView>::post(home, y, c[i-1], c[i]));
      }
    }
  }

}

// STATISTICS: int-post

