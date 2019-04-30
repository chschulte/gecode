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

using namespace Gecode;

int rnd(int n) {
  long long int r = static_cast<long long int>(Support::hwrnd()) * n;
  return r / UINT_MAX;
}

int factorial(int n) {
  int f = 1;
  for (int i=2; i<=n; i++)
    f *= i;
  return f;
}

void fill2(char f, int n) {
  if (n < 10) std::cout << f;
  std::cout << n;
}
void fill4(char f, int n) {
  if (n < 10) std::cout << f;
  if (n < 100) std::cout << f;
  if (n < 1000) std::cout << f;
  std::cout << n;
}

class Permutation : public Space {
public:
  IntVarArray x;
  Permutation(int n) : x(*this, n, 0,n-1) {
    distinct(*this, x);
    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }
  Permutation(Permutation& p) : Space(p) {
    x.update(*this, p.x);
  }
  virtual Space* copy(void) {
    return new Permutation(*this);
  }
};

int
main(int argc, char* argv[]) {
  int k = 10000; // Number of instances
  int m = 9; // Machines
  int n = 9; // Jobs
  int d = 99; // Maximal duration
  
  int fm = factorial(m);

  int* p = new int[m * fm];

  Rnd rnd(0);

  {
    Permutation* g = new Permutation(m);
    DFS<Permutation> e(g);
    delete g;
    int q=0;
    while (Permutation* s = e.next()) {
      for (int j=0; j<m; j++)
        p[q++] = s->x[j].val();
      delete s;
    }
  }

  for (int r=0; r<k; r++) {
    std::cout << "  const int q";
    fill4('0',r);
    std::cout << "[] = {" << std::endl;
    std::cout << "    " << n << ", " << m << ", "
              << "// Number of jobs and machines"
              << std::endl;
    for (int j=0; j<n; j++) {
      int q = rnd(fm);
      std::cout << "    ";
      for (int i=0; i<m; i++) {
        std::cout << p[m*q+i] << ", ";
        fill2(' ',rnd(d)+1);
        if ((i != n-1) || (j != m-1))
          std::cout << ", ";
      }
      std::cout << std::endl;
    }
    std::cout << "  };" << std::endl;
  }

  std::cout << std::endl;

  {
  int l=8;
  for (int r=0; r<k; r++) {
    if (l == 8) {
      std::cout << std::endl << "    ";
      l=0;
    }
    l++;
    std::cout << "&q";
    fill4('0',r);
    std::cout << "[0], ";
  }

  std::cout << std::endl;
  l=8;

  for (int r=0; r<k; r++) {
    if (l == 8) {
      std::cout << std::endl << "    ";
      l=0;
    }
    l++;
    std::cout << "\"q";
    fill4('0',r);
    std::cout << "\", ";
  }

  std::cout << std::endl;
  std::cout << std::endl;
  }

  return 0;
}

// STATISTICS: example-any

