/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2010
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

#include <gecode/int/bin-packing.hh>

namespace Gecode { namespace Int { namespace BinPacking {

  /*
   * Packing propagator
   *
   */

  PropCost
  Pack::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::HI,bs.size());
  }

  void
  Pack::reschedule(Space& home) {
    l.reschedule(home,*this,PC_INT_BND);
    bs.reschedule(home,*this,PC_INT_DOM);
  }

  Actor*
  Pack::copy(Space& home) {
    return new (home) Pack(home,*this);
  }

  /*
   * Propagation proper
   *
   */
  ExecStatus
  Pack::propagate(Space& home, const ModEventDelta& med) {
    // Number of items
    int n = bs.size();
    // Number of bins
    int m = l.size();

    Region region;
    {

      // Possible sizes for bins
      int* ps = region.alloc<int>(m);

      for (int j=0; j<m; j++)
        ps[j] = 0;

      // Compute sizes for bins
      if (OffsetView::me(med) == ME_INT_VAL) {
        // Also eliminate assigned items
        int k=0;
        for (int i=0; i<n; i++)
          if (bs[i].assigned()) {
            int j = bs[i].bin().val();
            l[j].offset(l[j].offset() - bs[i].size());
            t -= bs[i].size();
          } else {
            for (ViewValues<IntView> j(bs[i].bin()); j(); ++j)
              ps[j.val()] += bs[i].size();
            bs[k++] = bs[i];
          }
        n=k; bs.size(n);
      } else {
        for (int i=0; i<n; i++) {
          assert(!bs[i].assigned());
          for (ViewValues<IntView> j(bs[i].bin()); j(); ++j)
            ps[j.val()] += bs[i].size();
        }
      }

      {
        ExecStatus es = load(home,ps);
        if (es != ES_OK)
          return es;
        if (n == 0) {
          assert(l.assigned());
          return home.ES_SUBSUMED(*this);
        }
      }

      {
        TellCache tc(region,m);

        int k=0;
        for (int i=0; i<n; i++) {
          for (ViewValues<IntView> j(bs[i].bin()); j(); ++j) {
            if (bs[i].size() > l[j.val()].max())
              tc.nq(j.val());
            if (ps[j.val()] - bs[i].size() < l[j.val()].min())
              tc.eq(j.val());
          }
          GECODE_ES_CHECK(tc.tell(home,bs[i].bin()));
          // Eliminate assigned bin
          if (bs[i].assigned()) {
            int j = bs[i].bin().val();
            l[j].offset(l[j].offset() - bs[i].size());
            t -= bs[i].size();
          } else {
            bs[k++] = bs[i];
          }
        }
        n=k; bs.size(n);
      }
      region.free();
    }

    // Only if the propagator is at fixpoint here, continue with the more
    // expensive stage for propagation.
    if (IntView::me(modeventdelta()) != ME_INT_NONE)
      return ES_NOFIX;

    {
      ExecStatus es = expensive(home,region);
      return (es != ES_OK) ? es : ES_NOFIX;
    }
  }

  ExecStatus
  Pack::post(Home home, ViewArray<OffsetView>& l, ViewArray<Item>& bs) {
    // Number of items
    int n = bs.size();
    // Number of bins
    int m = l.size();

    if (n == 0) {
      // No items to be packed
      for (int j=0; j<m; j++)
        GECODE_ME_CHECK(l[j].eq(home,0));
      return ES_OK;
    } else if (m == 0) {
      // No bins available
      return ES_FAILED;
    }

    // Sort according to size
    Support::quicksort(&bs[0], n);
    // Total size of items
    int s = 0;
    // Constrain bins
    for (int i=0; i<n; i++) {
      s += bs[i].size();
      GECODE_ME_CHECK(bs[i].bin().gq(home,0));
      GECODE_ME_CHECK(bs[i].bin().le(home,m));
    }
    // Constrain load
    for (int j=0; j<m; j++) {
      GECODE_ME_CHECK(l[j].gq(home,0));
      GECODE_ME_CHECK(l[j].lq(home,s));
    }
    (void) new (home) Pack(home,l,bs,s);
    return ES_OK;
  }


}}}

// STATISTICS: int-prop

