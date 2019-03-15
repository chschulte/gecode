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


  /*
   * Cardinality packing propagator
   *
   */

  void
  CardPack::reschedule(Space& home) {
    Pack::reschedule(home);
    c.reschedule(home,*this,PC_INT_DOM);
  }

  Actor*
  CardPack::copy(Space& home) {
    return new (home) CardPack(home,*this);
  }


  void CardPack::print(const char* s) {
    if (true) {
      std::cout << s << std::endl
                << "\t bs = " << bs << std::endl
                << "\t l = {";
      for (int i=0; i<l.size(); i++) {
        int o=l[i].offset();
        std::cout << l[i] << " (";
        l[i].offset(0);
        std::cout << l[i] << ")";
        l[i].offset(o);
        if (i+1 != l.size())
          std::cout << ", ";
      }
      std::cout << "}" << std::endl
                << "\t c = {";
      for (int i=0; i<c.size(); i++) {
        int o=c[i].offset();
        std::cout << c[i] << " (";
        c[i].offset(0);
        std::cout << c[i] << ")";
        c[i].offset(o);
        if (i+1 != c.size())
          std::cout << ", ";
      }
      std::cout << "}" << std::endl << std::endl;
    }
 }

  /*
   * Propagation proper
   *
   */
  ExecStatus
  CardPack::propagate(Space& home, const ModEventDelta& med) {
    print("propagate");

    // Number of items
    int n = bs.size();
    // Number of bins
    int m = l.size();
    assert(m == c.size());

    Region region;

    {
      // Possible sizes for bins
      int* ps = region.alloc<int>(m);
      // Possible capacity for bins
      int* pc = region.alloc<int>(m);

      for (int j=0; j<m; j++)
        ps[j] = pc[j] = 0;

      if (OffsetView::me(med) == ME_INT_VAL) {
        // Eliminate assigned items
        int k=0;
        for (int i=0; i<n; i++)
          if (bs[i].assigned()) {
            int j = bs[i].bin().val();
            l[j].offset(l[j].offset() - bs[i].size());
            c[j].offset(c[j].offset() - 1);
            t -= bs[i].size();
          } else {
            for (ViewValues<IntView> j(bs[i].bin()); j(); ++j) {
              ps[j.val()] += bs[i].size();
              pc[j.val()] += 1;
            }
            bs[k++] = bs[i];
          }
        n=k; bs.size(n);
      } else {
        for (int i=0; i<n; i++) {
          assert(!bs[i].assigned());
          for (ViewValues<IntView> j(bs[i].bin()); j(); ++j) {
            ps[j.val()] += bs[i].size();
            pc[j.val()] += 1;
          }
        }
      }

      print("After elimination:");

      if (true) {
        std::cout << "Possible size and cardinality" << std::endl;
        for (int j=0; j<m; j++) {
          std::cout << "\tps[" << j << "] = " << ps[j] << ", "
                    << "pc[" << j << "] = " << pc[j] << std::endl;
        }
      }

      // Constrain load
      {
        ExecStatus es = load(home,ps);
        if (es != ES_OK)
          return es;
      }

      print("After load propagation:");


      for (int j=0; j<m; j++) {
        GECODE_ME_CHECK(c[j].gq(home,0));
        GECODE_ME_CHECK(c[j].lq(home,pc[j]));
      }

      print("After cardinality propagation:");

      {    
        int* minsum = region.alloc<int>(n+1);
        int* maxsum = region.alloc<int>(n+1);
        
        minsum[0] = 0;
        maxsum[0] = 0;
        for (int i=0; i<n; i++) {
          minsum[i+1] = minsum[i] + bs[n-i-1].size();
          maxsum[i+1] = maxsum[i] + bs[i].size();
        }
      
        if (true) {
          std::cout << "\tminsum = {";
          for (int i=0; i<n; i++)
            std::cout << minsum[i] << ", ";
          std::cout << std::endl;
          std::cout << "\tmaxsum = {";
          for (int i=0; i<n; i++)
            std::cout << maxsum[i] << ", ";
          std::cout << std::endl;
        }
      
        // Propagate load from cardinality
        for (int j=0; j<m; j++) {
          GECODE_ME_CHECK(l[j].lq(home,maxsum[c[j].max()]));
          GECODE_ME_CHECK(l[j].gq(home,minsum[c[j].min()]));
        }
        
        print("After load from cardinality");
        
        // Propagate cardinality from load
        if (0)
        for (int j=0; j<m; j++) {
          {
            // Find number of items which are needed at least
            int k=c[j].min();
            while ((k < c[j].max()) && (maxsum[k] < l[j].min()))
              k++;
            GECODE_ME_CHECK(c[j].gq(home,k));
          }
          {
            // Find number of items which are needed at most
            int k=c[j].max();
            while ((k > c[j].min()) && (minsum[k] > l[j].max()))
              k--;
            GECODE_ME_CHECK(c[j].lq(home,k));
          }
        }
        
        print("After cardinality from load");

      
        {
          int k = 0;
          for (int i=0; i<n; i++) {
            for (int j=0; j<m; j++) {
              //              assert(!bs[i].assigned());
              // Item i too large for bin j?
              if ((c[j].min() > 0) &&
                  (minsum[c[j].min()-1] + bs[i].size() > l[j].max()))
                GECODE_ME_CHECK(bs[i].bin().nq(home,j));
              // Item i too small for bin j?
              if ((c[j].max() > 0) &&
                  (maxsum[c[j].max()-1] + bs[i].size() < l[j].min()))
                GECODE_ME_CHECK(bs[i].bin().nq(home,j));
            }
            // Eliminate assigned bin
            if (false && bs[i].assigned()) {
              int j = bs[i].bin().val();
              l[j].offset(l[j].offset() - bs[i].size());
              c[j].offset(c[j].offset() - 1);
              t -= bs[i].size();
            } else {
              bs[k++] = bs[i];
            }
          }
          n=k; bs.size(n);
        }

        print("After bin from cardinality & load");


                if (true) {
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
              c[j].offset(c[j].offset() - 1);
              t -= bs[i].size();
            } else {
              bs[k++] = bs[i];
            }
          }
          n=k; bs.size(n);
        }
      }
    }

    print("After bin from load");

    // Only if the propagator is at fixpoint here, continue with the more
    // expensive stage for propagation.
    if (IntView::me(modeventdelta()) != ME_INT_NONE)
      return ES_NOFIX;


    {
      ExecStatus es = expensive(home,region);
      print("After expensive");
      return (es != ES_OK) ? es : ES_NOFIX;
    }

                return (n == 0) ? home.ES_SUBSUMED(*this) : ES_NOFIX;
  }

  ExecStatus
  CardPack::post(Home home,
                 ViewArray<OffsetView>& l, ViewArray<OffsetView>& c,
                 ViewArray<Item>& bs) {
    // Number of items
    int n = bs.size();
    // Number of bins
    int m = l.size();
    assert(m == c.size());

    if (n == 0) {
      // No items to be packed
      for (int i=0; i<m; i++) {
        GECODE_ME_CHECK(l[i].eq(home,0));
        GECODE_ME_CHECK(c[i].eq(home,0));
      }
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
    // Constrain load & capacity
    for (int j=0; j<m; j++) {
      GECODE_ME_CHECK(l[j].gq(home,0));
      GECODE_ME_CHECK(l[j].lq(home,s));
      GECODE_ME_CHECK(c[j].gq(home,0));
      GECODE_ME_CHECK(c[j].lq(home,n));
    }
    (void) new (home) CardPack(home,l,c,bs,s);
    return ES_OK;
  }

}}}

// STATISTICS: int-prop

