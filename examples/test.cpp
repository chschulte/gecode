/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
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

#include <gecode/driver.hh>

#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: %Alpha puzzle
 *
 * Well-known cryptoarithmetic puzzle of unknown origin.
 *
 * \ingroup Example
 *
 */
class Alpha : public Script {
protected:
  /// Alphabet has 26 letters
  static const int n = 26;
  /// Array for letters
  IntVarArray b;
  IntVarArray c;
  IntVarArray l;
public:
  /// Actual model
  Alpha(const Options& opt)
    : Script(opt) {
    /*
    dom(*this, l[0], IntSet({8,10}));
    dom(*this, l[1], IntSet({3,5}));
    dom(*this, c[0], IntSet(3,4));
    dom(*this, c[1], IntSet(2,3));
    //    rel(*this, b[3] == 1);
    rel(*this, b[4] == 0);
    rel(*this, b[5] == 1);
    print(std::cout);
    status();
    print(std::cout);
    rel(*this, b[0] == 0);
    status();
    print(std::cout);
    */
    /*
    dom(*this, l[0], IntSet({5,7}));
    dom(*this, l[1], IntSet(6,8));
    dom(*this, c[0], IntSet(1,3));
    dom(*this, c[1], IntSet(3,5));
    rel(*this, b[0] == 0);
    rel(*this, b[3] == 0);
    rel(*this, b[4] == 1);
    rel(*this, b[5] == 1);
    std::cout << "A" << std::endl;
    print(std::cout);
    status();
    std::cout << "B" << std::endl;
    print(std::cout);
    rel(*this, l[0] == 5);
    status();
    std::cout << "C" << std::endl;
    print(std::cout);
    rel(*this, b[1] == 1);
    status();
    std::cout << "D" << std::endl;
    print(std::cout);
    std::cout << "AFTER" << std::endl;
    */

    b = IntVarArray(*this,6,0,10);
    c = IntVarArray(*this,2,0,60);
    l = IntVarArray(*this,2,0,100);

    IntArgs s({1,2,4,8,16,32});

    binpacking(*this, l, c, b, s);

    rel(*this, l[0] == 33);
    rel(*this, l[1] == 30);
    rel(*this, c[0] == 2);
    rel(*this, c[1] == 4);

    rel(*this, b[0] == 0);
    rel(*this, b[2] == 1);
    rel(*this, b[3] == 1);
    rel(*this, b[4] == 1);
    rel(*this, b[5] == 0);
  }

  /// Constructor for cloning \a s
  Alpha(Alpha& s) : Script(s) {
    b.update(*this, s.b);
    c.update(*this, s.c);
    l.update(*this, s.l);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new Alpha(*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t" << "b = " << b << std::endl;
    os << "\t" << "c = " << c << std::endl;
    os << "\t" << "l = " << l << std::endl;
  }
};

/** \brief Main-function
 *  \relates Alpha
 */
int
main(int argc, char* argv[]) {
  Options opt("Alpha");
  opt.parse(argc,argv);
  Script::run<Alpha,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any

