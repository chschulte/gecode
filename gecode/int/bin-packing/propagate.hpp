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

namespace Gecode { namespace Int { namespace BinPacking {

  /*
   * Item
   *
   */
  forceinline
  Item::Item(void)
    : s(0) {}
  forceinline
  Item::Item(IntView b, int s0)
    : DerivedView<IntView>(b), s(s0) {}

  forceinline IntView
  Item::bin(void) const {
    return x;
  }
  forceinline
  void Item::bin(IntView b) {
    x = b;
  }
  forceinline int
  Item::size(void) const {
    return s;
  }
  forceinline void
  Item::size(int s0) {
    s = s0;
  }

  forceinline void
  Item::update(Space& home, Item& i) {
    x.update(home,i.x);
    s = i.s;
  }


  forceinline bool
  operator ==(const Item& i, const Item& j) {
    return (i.bin() == j.bin()) && (i.size() == j.size());
  }
  forceinline bool
  operator !=(const Item& i, const Item& j) {
    return !(i == j);
  }

  /// For sorting according to size
  forceinline bool
  operator <(const Item& i, const Item& j) {
    return ((i.size() > j.size()) ||
            ((i.size() == j.size()) && (i.bin() < j.bin())));
  }


  template<class Char, class Traits>
  inline std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const Item& x) {
    return os << '[' << x.bin() << ',' << x.size() << ']';
  }

  /*
   * Size set
   *
   */
  forceinline
  SizeSet::SizeSet(void) {}
  forceinline
  SizeSet::SizeSet(Region& region, int n_max)
    : n(0), t(0), s(region.alloc<int>(n_max)) {}
  forceinline void
  SizeSet::add(int s0) {
    t += s0; s[n++] = s0;
  }
  forceinline int
  SizeSet::card(void) const {
    return n;
  }
  forceinline int
  SizeSet::total(void) const {
    return t;
  }
  forceinline int
  SizeSet::operator [](int i) const {
    return s[i];
  }

  forceinline
  SizeSetMinusOne::SizeSetMinusOne(void) {}
  forceinline
  SizeSetMinusOne::SizeSetMinusOne(Region& region, int n_max)
    : SizeSet(region,n_max), p(-1) {}
  forceinline void
  SizeSetMinusOne::minus(int s0) {
    // This rests on the fact that items are removed in order
    do
      p++;
    while (s[p] > s0);
    assert(p < n);
  }
  forceinline int
  SizeSetMinusOne::card(void) const {
    assert(p >= 0);
    return n - 1;
  }
  forceinline int
  SizeSetMinusOne::total(void) const {
    assert(p >= 0);
    return t - s[p];
  }
  forceinline int
  SizeSetMinusOne::operator [](int i) const {
    assert(p >= 0);
    return s[(i < p) ? i : i+1];
  }


  forceinline
  TellCache::TellCache(Region& region, int m)
    : _nq(region.alloc<int>(m)), _n_nq(0), _eq(-1) {}
  forceinline void
  TellCache::nq(int j) {
    _nq[_n_nq++] = j;
  }
  forceinline void
  TellCache::eq(int j) {
    // For eq: -1 mean not yet assigned, -2 means failure, positive means value
    if (_eq == -1)
      _eq = j;
    else
      _eq = -2;
  }
  forceinline ExecStatus
  TellCache::tell(Space& home, IntView x) {
    if (_eq == -2) {
      return ES_FAILED;
    } else if (_eq >= 0) {
      GECODE_ME_CHECK(x.eq(home,_eq));
    }
    Iter::Values::Array nqi(_nq, _n_nq);
    GECODE_ME_CHECK(x.minus_v(home, nqi));
    _n_nq=0; _eq=-1;
    return ES_OK;
  }



  /*
   * Packing propagator
   *
   */

  forceinline
  Pack::Pack(Home home, ViewArray<OffsetView>& l0, ViewArray<Item>& bs0,
             int t0)
    : Propagator(home), l(l0), bs(bs0), t(t0) {
    l.subscribe(home,*this,PC_INT_BND);
    bs.subscribe(home,*this,PC_INT_DOM);
  }

  forceinline
  Pack::Pack(Space& home, Pack& p)
    : Propagator(home,p), t(p.t) {
    l.update(home,p.l);
    bs.update(home,p.bs);
  }

  forceinline size_t
  Pack::dispose(Space& home) {
    l.cancel(home,*this,PC_INT_BND);
    bs.cancel(home,*this,PC_INT_DOM);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template<class SizeSet>
  forceinline bool
  Pack::nosum(const SizeSet& s, int a, int b, int& ap, int& bp) {
    if ((a <= 0) || (b >= s.total()))
      return false;
    int n=s.card()-1;
    int sc=0;
    int kp=0;
    while (sc + s[n-kp] < a) {
      sc += s[n-kp];
      kp++;
    }
    int k=0;
    int sa=0, sb = s[n-kp];
    while ((sa < a) && (sb <= b)) {
      sa += s[k++];
      if (sa < a) {
        kp--;
        sb += s[n-kp];
        sc -= s[n-kp];
        while (sa + sc >= a) {
          kp--;
          sc -= s[n-kp];
          sb += s[n-kp] - s[n-kp-k-1];
        }
      }
    }
    ap = sa + sc; bp = sb;
    return sa < a;
  }

  template<class SizeSet>
  forceinline bool
  Pack::nosum(const SizeSet& s, int a, int b) {
    int ap, bp;
    return nosum(s, a, b, ap, bp);
  }

  forceinline ExecStatus
  Pack::expensive(Space& home, Region& region) {
    int n = bs.size();
    int m = l.size();
    // Now the invariant holds that no more assigned bins exist!

    {
      // Size of items
      SizeSetMinusOne* s = region.alloc<SizeSetMinusOne>(m);
      
      for (int j=0; j<m; j++)
        s[j] = SizeSetMinusOne(region,n);
      
      // Set up size information
      for (int i=0; i<n; i++) {
        assert(!bs[i].assigned());
        for (ViewValues<IntView> j(bs[i].bin()); j(); ++j)
          s[j.val()].add(bs[i].size());
      }
      
      for (int j=0; j<m; j++) {
        // Can items still be packed into bin?
        if (nosum(static_cast<SizeSet&>(s[j]), l[j].min(), l[j].max()))
          return ES_FAILED;
        int ap, bp;
        // Must there be packed more items into bin?
        if (nosum(static_cast<SizeSet&>(s[j]), l[j].min(), l[j].min(),
                  ap, bp))
          GECODE_ME_CHECK(l[j].gq(home,bp));
        // Must there be packed less items into bin?
        if (nosum(static_cast<SizeSet&>(s[j]), l[j].max(), l[j].max(),
                  ap, bp))
          GECODE_ME_CHECK(l[j].lq(home,ap));
      }

      TellCache tc(region,m);
      
      int k=0;
      for (int i=0; i<n; i++) {
        assert(!bs[i].assigned());
        for (ViewValues<IntView> j(bs[i].bin()); j(); ++j) {
          // Items must be removed in decreasing size!
          s[j.val()].minus(bs[i].size());
          // Can item i still be packed into bin j?
          if (nosum(s[j.val()],
                    l[j.val()].min() - bs[i].size(),
                    l[j.val()].max() - bs[i].size()))
            tc.nq(j.val());
          // Must item i be packed into bin j?
          if (nosum(s[j.val()], l[j.val()].min(), l[j.val()].max()))
            tc.eq(j.val());
        }
        GECODE_ES_CHECK(tc.tell(home,bs[i].bin()));
        if (bs[i].assigned()) {
          int j = bs[i].bin().val();
          l[j].offset(l[j].offset() - bs[i].size());
          t -= bs[i].size();
        } else {
          bs[k++] = bs[i];
        }
      }
      n=k; bs.size(n);
      region.free();
    }

    // Perform lower bound checking
    if (n > 0) {
      // Find capacity estimate (we start from bs[0] as it might be
      // not packable, actually (will be detected later anyway)!
      int c = bs[0].size();
      for (int j=0; j<m; j++)
        c = std::max(c,l[j].max());
      
      // Count how many items have a certain size (bucket sort)
      int* n_s = region.alloc<int>(c+1);
      
      for (int i=0; i<c+1; i++)
        n_s[i] = 0;
      
      // Count unpacked items
      for (int i=0; i<n; i++)
        n_s[bs[i].size()]++;
      
      // Number of items and remaining bin load
      int nm = n;
      
      // Only count positive remaining bin loads
      for (int j=0; j<m; j++)
        if (l[j].max() < 0) {
        return ES_FAILED;
      } else if (c > l[j].max()) {
        n_s[c - l[j].max()]++; nm++;
      }
    
      // Sizes of items and remaining bin loads
      int* s = region.alloc<int>(nm);
      
      // Setup sorted sizes
      {
        int k=0;
        for (int i=c+1; i--; )
          for (int j=n_s[i]; j--; )
            s[k++]=i;
        assert(k == nm);
      }
      
      // Items in N1 are from 0 ... n1 - 1
      int n1 = 0;
      // Items in N2 are from n1 ... n12 - 1, we count elements in N1 and N2
      int n12 = 0;
      // Items in N3 are from n12 ... n3 - 1
      int n3 = 0;
      // Free space in N2
      int f2 = 0;
      // Total size of items in N3
      int s3 = 0;
      
      // Initialize n12 and f2
      for (; (n12 < nm) && (s[n12] > c/2); n12++)
        f2 += c - s[n12];
      
      // Initialize n3 and s3
      for (n3 = n12; n3 < nm; n3++)
        s3 += s[n3];
      
      // Compute lower bounds
      for (int k=0; k<=c/2; k++) {
        // Make N1 larger by adding elements and N2 smaller
        for (; (n1 < nm) && (s[n1] > c-k); n1++)
          f2 -= c - s[n1];
        assert(n1 <= n12);
        // Make N3 smaller by removing elements
        for (; (s[n3-1] < k) && (n3 > n12); n3--)
          s3 -= s[n3-1];
        // Overspill
        int o = (s3 > f2) ? ((s3 - f2 + c - 1) / c) : 0;
        if (n12 + o > m)
          return ES_FAILED;
      }
      region.free();
    }

    return ES_OK;
  }

  forceinline ExecStatus
  Pack::load(Space& home, int* ps) {
    int n = bs.size();
    int m = l.size();
    
    // Propagate bin loads and compute lower and upper bound
    int min = t, max = t;
    for (int j=0; j<m; j++) {
      GECODE_ME_CHECK(l[j].gq(home,0));
      GECODE_ME_CHECK(l[j].lq(home,ps[j]));
      min -= l[j].max(); max -= l[j].min();
    }

    // Propagate that load must be equal to total size
    for (bool mod = true; mod; ) {
      mod = false; ModEvent me;
      for (int j=0; j<m; j++) {
        int lj_min = l[j].min();
        me = l[j].gq(home, min + l[j].max());
        if (me_failed(me))
          return ES_FAILED;
        if (me_modified(me)) {
          max += lj_min - l[j].min(); mod = true;
        }
        int lj_max = l[j].max();
        me = l[j].lq(home, max + l[j].min());
        if (me_failed(me))
          return ES_FAILED;
        if (me_modified(me)) {
          min += lj_max - l[j].max(); mod = true;
        }
      }
    }

    if (n == 0) {
      assert(l.assigned());
      return home.ES_SUBSUMED(*this);
    }
    return ES_OK;
  }


  /*
   * Cardinality packing propagator
   *
   */

  forceinline
  CardPack::CardPack(Home home,
                     ViewArray<OffsetView>& l, ViewArray<OffsetView>& c0,
                     ViewArray<Item>& bs, int t)
    : Pack(home,l,bs,t), c(c0) {
    c.subscribe(home,*this,PC_INT_BND);
  }

  forceinline
  CardPack::CardPack(Space& home, CardPack& p)
    : Pack(home,p) {
    c.update(home,p.c);
  }

  forceinline size_t
  CardPack::dispose(Space& home) {
    c.cancel(home,*this,PC_INT_BND);
    (void) Pack::dispose(home);
    return sizeof(*this);
  }

}}}

// STATISTICS: int-prop

