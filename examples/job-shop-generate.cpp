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

void permute(int* p, int n) {
  for (int i=0; i<n; i++)
    p[i]=i;
  for (int k=n*n; k--; )
    std::swap(p[rnd(n)],p[rnd(n)]);
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
  //  if (n < 1000) std::cout << f;
  std::cout << n;
}

  const int abz5[] = {
    10, 10, // Number of jobs and machines
    4, 88, 8, 68, 6, 94, 5, 99, 1, 67, 2, 89, 9, 77, 7, 99, 0, 86, 3, 92, 
    5, 72, 3, 50, 6, 69, 4, 75, 2, 94, 8, 66, 0, 92, 1, 82, 7, 94, 9, 63, 
    9, 83, 8, 61, 0, 83, 1, 65, 6, 64, 5, 85, 7, 78, 4, 85, 2, 55, 3, 77, 
    7, 94, 2, 68, 1, 61, 4, 99, 3, 54, 6, 75, 5, 66, 0, 76, 9, 63, 8, 67, 
    3, 69, 4, 88, 9, 82, 8, 95, 0, 99, 2, 67, 6, 95, 5, 68, 7, 67, 1, 86, 
    1, 99, 4, 81, 5, 64, 6, 66, 8, 80, 2, 80, 7, 69, 9, 62, 3, 79, 0, 88, 
    7, 50, 1, 86, 4, 97, 3, 96, 0, 95, 8, 97, 2, 66, 5, 99, 6, 52, 9, 71, 
    4, 98, 6, 73, 3, 82, 2, 51, 1, 71, 5, 94, 7, 85, 0, 62, 8, 95, 9, 79, 
    0, 94, 6, 71, 3, 81, 7, 85, 1, 66, 2, 90, 4, 76, 5, 58, 8, 93, 9, 97, 
    3, 50, 0, 59, 1, 82, 8, 67, 7, 56, 9, 96, 6, 58, 4, 81, 5, 59, 2, 96
  };

int
main(int argc, char* argv[]) {
  int k = 1000; // Number of instances
  int m = 10; // Machines
  int n = 10; // Jobs
  int d = 50; // Maximal duration
  
  int* p = new int[m];

  for (int r=0; r<k; r++) {
    std::cout << "  const int r";
    fill4('0',r);
    std::cout << "[] = {" << std::endl;
    std::cout << "    " << n << ", " << m << ", "
              << "// Number of jobs and machines"
              << std::endl;
    for (int j=0; j<n; j++) {
      permute(p,m);
      std::cout << "    ";
      for (int i=0; i<m; i++) {
        std::cout << p[i] << ", ";
        fill2(' ',rnd(d)+1);
        if ((i != n-1) || (j != m-1))
          std::cout << ", ";
      }
      std::cout << std::endl;
    }
    std::cout << "  };" << std::endl;
  }

  std::cout << std::endl;

  int l=8;
  for (int r=0; r<k; r++) {
    if (l == 8) {
      std::cout << std::endl << "    ";
      l=0;
    }
    l++;
    std::cout << "&r";
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
    std::cout << "\"r";
    fill4('0',r);
    std::cout << "\", ";
  }

  std::cout << std::endl;
  std::cout << std::endl;

  return 0;
}

// STATISTICS: example-any

