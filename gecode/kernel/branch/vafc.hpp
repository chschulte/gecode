/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

#include <cfloat>

namespace Gecode {

  /**
   * \brief Class for action management
   *
   */
  class VAFC : public SharedHandle {
  protected:
    template<class View>
    class Recorder;
    /// Object for storing action values
    class GECODE_VTABLE_EXPORT Storage : public SharedHandle::Object {
    public:
      /// Mutex to synchronize globally shared access
      GECODE_KERNEL_EXPORT static Support::Mutex m;
      /// Number of action values
      int n;
      /// Inverse decay factor
      double invd;
      /// VAFC values (more follow)
      double* a;
      /// Initialize action values
      template<class View>
      Storage(Home home, ViewArray<View>& x, double d,
              typename BranchTraits<typename View::VarType>::Merit bm);
      /// Update action value at position \a i
      void update(int i);
      /// Delete object
      GECODE_KERNEL_EXPORT
      ~Storage(void);
    };
    /// Return object of correct type
    Storage& object(void) const;
    /// Set object to \a o
    void object(Storage& o);
    /// Update action value at position \a i
    void update(int i);
    /// Acquire mutex
    void acquire(void);
    /// Release mutex
    void release(void);
  public:
    /// \name Constructors and initialization
    //@{
    /**
     * \brief Construct as not yet intialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized action storage is init and the assignment operator.
     *
     */
    VAFC(void);
    /// Copy constructor
    GECODE_KERNEL_EXPORT
    VAFC(const VAFC& a);
    /// Assignment operator
    GECODE_KERNEL_EXPORT
    VAFC& operator =(const VAFC& a);
    /// Initialize for views \a x and decay factor \a d and action as defined by \a bm
    template<class View>
    VAFC(Home home, ViewArray<View>& x, double d,
         typename BranchTraits<typename View::VarType>::Merit bm);
    /// Initialize for views \a x and decay factor \a d and action as defined by \a bm
    template<class View>
    void init(Home home, ViewArray<View>& x, double d,
              typename BranchTraits<typename View::VarType>::Merit bm);
    /// Default (empty) action information
    GECODE_KERNEL_EXPORT static const VAFC def;
    //@}

    /// Destructor
    GECODE_KERNEL_EXPORT
    ~VAFC(void);

    /// \name Information access
    //@{
    /// Return action value at position \a i
    double operator [](int i) const;
    /// Return number of action values
    int size(void) const;
    //@}

    /// \name Decay factor for aging
    //@{
    /// Set decay factor to \a d
    GECODE_KERNEL_EXPORT
    void decay(Space& home, double d);
    /// Return decay factor
    GECODE_KERNEL_EXPORT
    double decay(const Space& home) const;
    //@}
  };

  /// Propagator for recording action information
  template<class View>
  class VAFC::Recorder : public NaryPropagator<View,PC_GEN_NONE> {
  protected:
    using NaryPropagator<View,PC_GEN_NONE>::x;
    /// Advisor with index and change information
    class Idx : public Advisor {
    protected:
      /// Index and mark information
      int _idx;
    public:
      /// Constructor for creation
      Idx(Space& home, Propagator& p, Council<Idx>& c, int i);
      /// Constructor for cloning \a a
      Idx(Space& home, Idx& a);
      /// Get index of view
      int idx(void) const;
    };
    /// Access to action information
    VAFC a;
    /// The advisor council
    Council<Idx> c;
    /// Constructor for cloning \a p
    Recorder(Space& home, Recorder<View>& p);
  public:
    /// Constructor for creation
    Recorder(Home home, ViewArray<View>& x, VAFC& a);
    /// Copy propagator during cloning
    virtual Propagator* copy(Space& home);
    /// Cost function (record so that propagator runs last)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Schedule function
    virtual void reschedule(Space& home);
    /// Give advice to propagator
    virtual ExecStatus advise(Space& home, Advisor& a, const Delta& d);
    /// Give advice to propagator when \a home has failed
    virtual void advise(Space& home, Advisor& a);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
    /// Post action recorder propagator
    static ExecStatus post(Home home, ViewArray<View>& x, VAFC& a);
  };

  /**
   * \brief Print action values enclosed in curly brackets
   * \relates VAFC
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const VAFC& a);


  /*
   * Advisor for action recorder
   *
   */
  template<class View>
  forceinline
  VAFC::Recorder<View>::Idx::Idx(Space& home, Propagator& p,
                                   Council<Idx>& c, int i)
    : Advisor(home,p,c), _idx(i) {}
  template<class View>
  forceinline
  VAFC::Recorder<View>::Idx::Idx(Space& home, Idx& a)
    : Advisor(home,a), _idx(a._idx) {
  }
  template<class View>
  forceinline int
  VAFC::Recorder<View>::Idx::idx(void) const {
    return _idx;
  }


  /*
   * Posting of action recorder propagator
   *
   */
  template<class View>
  forceinline
  VAFC::Recorder<View>::Recorder(Home home, ViewArray<View>& x,
                                   VAFC& a0)
    : NaryPropagator<View,PC_GEN_NONE>(home,x), a(a0), c(home) {
    home.notice(*this,AP_DISPOSE);
    for (int i=0; i<x.size(); i++)
      if (!x[i].assigned())
        x[i].subscribe(home,*new (home) Idx(home,*this,c,i), true);
  }

  template<class View>
  forceinline ExecStatus
  VAFC::Recorder<View>::post(Home home, ViewArray<View>& x, VAFC& a) {
    (void) new (home) Recorder<View>(home,x,a);
    return ES_OK;
  }


  /*
   * VAFC value storage
   *
   */

  template<class View>
  forceinline
  VAFC::Storage::Storage(Home home, ViewArray<View>& x, double d,
                         typename
                         BranchTraits<typename View::VarType>::Merit bm)
    : n(x.size()), invd(1.0 / d), a(heap.alloc<double>(x.size())) {
    if (bm)
      for (int i=0; i<n; i++) {
        typename View::VarType xi(x[i].varimp());
        a[i] = bm(home,xi,i);
      }
    else
      for (int i=0; i<n; i++)
        a[i] = x[i].degree();
  }
  forceinline void
  VAFC::Storage::update(int i) {
    /*
     * The trick to inverse decay is from: An Extensible SAT-solver,
     * Niklas Eén, Niklas Sörensson, SAT 2003.
     */
    assert((i >= 0) && (i < n));
    a[i] = invd * (a[i] + 1.0);
    if (a[i] > Kernel::Config::rescale_limit)
      for (int j=0; j<n; j++)
        a[j] *= Kernel::Config::rescale;
  }


  /*
   * VAFC
   *
   */

  forceinline VAFC::Storage&
  VAFC::object(void) const {
    return static_cast<VAFC::Storage&>(*SharedHandle::object());
  }

  forceinline void
  VAFC::object(VAFC::Storage& o) {
    SharedHandle::object(&o);
  }

  forceinline void
  VAFC::update(int i) {
    object().update(i);
  }
  forceinline double
  VAFC::operator [](int i) const {
    assert((i >= 0) && (i < object().n));
    return object().a[i];
  }
  forceinline int
  VAFC::size(void) const {
    return object().n;
  }
  forceinline void
  VAFC::acquire(void) {
    object().m.acquire();
  }
  forceinline void
  VAFC::release(void) {
    object().m.release();
  }


  forceinline
  VAFC::VAFC(void) {}

  template<class View>
  forceinline
  VAFC::VAFC(Home home, ViewArray<View>& x, double d,
             typename BranchTraits<typename View::VarType>::Merit bm) {
    assert(!*this);
    object(*new Storage(home,x,d,bm));
    (void) Recorder<View>::post(home,x,*this);
  }
  template<class View>
  forceinline void
  VAFC::init(Home home, ViewArray<View>& x, double d,
             typename BranchTraits<typename View::VarType>::Merit bm) {
    assert(!*this);
    object(*new Storage(home,x,d,bm));
    (void) Recorder<View>::post(home,x,*this);
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const VAFC& a) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << '{';
    if (a.size() > 0) {
      s << a[0];
      for (int i=1; i<a.size(); i++)
        s << ", " << a[i];
    }
    s << '}';
    return os << s.str();
  }


  /*
   * Propagation for action recorder
   *
   */
  template<class View>
  forceinline
  VAFC::Recorder<View>::Recorder(Space& home, Recorder<View>& p)
    : NaryPropagator<View,PC_GEN_NONE>(home,p), a(p.a) {
    c.update(home, p.c);
  }

  template<class View>
  Propagator*
  VAFC::Recorder<View>::copy(Space& home) {
    return new (home) Recorder<View>(home, *this);
  }

  template<class View>
  inline size_t
  VAFC::Recorder<View>::dispose(Space& home) {
    // Delete access to action information
    home.ignore(*this,AP_DISPOSE);
    a.~VAFC();
    // Cancel remaining advisors
    for (Advisors<Idx> as(c); as(); ++as)
      x[as.advisor().idx()].cancel(home,as.advisor(),true);
    c.dispose(home);
    (void) NaryPropagator<View,PC_GEN_NONE>::dispose(home);
    return sizeof(*this);
  }

  template<class View>
  PropCost
  VAFC::Recorder<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::record();
  }

  template<class View>
  void
  VAFC::Recorder<View>::reschedule(Space& home) {
    View::schedule(home,*this,ME_GEN_ASSIGNED);
  }

  template<class View>
  ExecStatus
  VAFC::Recorder<View>::advise(Space&, Advisor& a, const Delta&) {
    Idx& i = static_cast<Idx&>(a);
    if (x[i.idx()].assigned())
      return ES_FIX_DISPOSE(c,a);
    else
      ES_FIX;
  }

  template<class View>
  void
  VAFC::Recorder<View>::advise(Space&, Advisor& a) {
    Idx& i = static_cast<Idx&>(a);
    a.acquire();
    a.update(i.idx());
    a.release();
  }

  template<class View>
  ExecStatus
  VAFC::Recorder<View>::propagate(Space& home, const ModEventDelta&) {
    return c.empty() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


}

// STATISTICS: kernel-branch
