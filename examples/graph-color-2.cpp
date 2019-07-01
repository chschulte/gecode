/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2019
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

// Instance data
namespace {

  // Instances
  extern const int* data[];
  // Instance names
  extern const char* name[];

  /// A wrapper class for instance data
  class Spec {
  protected:
    /// Raw instance data
    const int* data;
    /// Name
    const char* n;
    /// Number of edges
    int _n_e;
  public:
    /// Whether a valid specification has been found
    bool valid(void) const {
      return data != nullptr;
    }
  protected:
    /// Find instance by name \a s
    static const int* find(const char* s) {
      std::cout << "find: " << s << std::endl;
      for (int i=0; ::name[i] != nullptr; i++)
        if (!strcmp(s,::name[i]))
          return ::data[i];
      std::cout << "NOT FOUND: " << s << std::endl;
      return nullptr;
    }
  public:
    /// Initialize
    Spec(const char* s) : data(find(s)), n(s), _n_e(0) {
      std::cout << "FIND" << std::endl;
      if (valid()) {
        // Compute number of edges
        const int* d = data + 1;
        while (*d != -1) {
          _n_e++; d += 2;
        }
        std::cout << "VALID" << std::endl;
      }
    }
    /// Return number of nodes
    int nodes(void) const {
      return data[0];
    }
    /// Return number of edges
    int edges(void) const {
      return _n_e;
    }
    /// Return name
    const char* name(void) const {
      return n;
    }
    /// Return edge
    const int edge(int i, int p) const {
      return data[1 + 2*i + p]-1;
    }
  };

}

/**
 * \brief %Example: Graph coloring
 *
 * \ingroup Example
 *
 */
class GraphColor : public IntMinimizeScript {
private:
  /// Specification
  const Spec spec;
  /// Color of nodes
  IntVarArray v;
  /// Number of colors
  IntVar m;
public:
  /// Branching to use for model
  enum {
    BRANCH_DEGREE,         ///< Choose variable with largest degree
    BRANCH_SIZE,           ///< Choose variablee with smallest size
    BRANCH_DEGREE_SIZE,    ///< Choose variable with largest degree/size
    BRANCH_AFC_SIZE,       ///< Choose variable with largest afc/size
    BRANCH_ACTION_SIZE  ///< Choose variable with smallest size/action
  };
  /// The actual model
  GraphColor(const InstanceOptions& opt)
    : IntMinimizeScript(opt), spec(opt.instance()),
      v(*this,spec.nodes(),0,spec.nodes()-1),
      m(*this,1,spec.nodes()-1) {

    std::cout << "Name:  " << spec.name() << std::endl
              << "Nodes: " << spec.nodes() << std::endl
              << "Edges: " << spec.edges() << std::endl;

    rel(*this, v, IRT_LQ, m);

    for (int i=0; i<spec.edges(); i++)
      rel(*this, v[spec.edge(i,0)], IRT_NQ, v[spec.edge(i,1)]);

    // Branch on colors
    switch (opt.branching()) {
    case BRANCH_SIZE:
      branch(*this, v, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      break;
    case BRANCH_DEGREE:
      branch(*this, v, tiebreak(INT_VAR_DEGREE_MAX(),INT_VAR_SIZE_MIN()),
             INT_VAL_MIN());
      break;
    case BRANCH_DEGREE_SIZE:
      branch(*this, v, INT_VAR_DEGREE_SIZE_MAX(), INT_VAL_MIN());
      break;
    case BRANCH_AFC_SIZE:
      branch(*this, v, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_MIN());
      break;
    case BRANCH_ACTION_SIZE:
      branch(*this, v, INT_VAR_ACTION_SIZE_MAX(opt.decay()), INT_VAL_MIN());
      break;
    }

    // Branching on the number of colors
    assign(*this, m, INT_ASSIGN_MIN());
  }
  /// Cost function
  virtual IntVar cost(void) const {
    return m;
  }
  /// Constructor for cloning \a s
  GraphColor(GraphColor& s) : IntMinimizeScript(s), spec(s.spec) {
    v.update(*this, s.v);
    m.update(*this, s.m);
  }
  /// Copying during cloning
  virtual Space*
  copy(void) {
    return new GraphColor(*this);
  }
  /// Print the solution
  virtual void
  print(std::ostream& os) const {
    os << "\tm = " << m << std::endl
       << "\tv[] = {";
    for (int i = 0; i < v.size(); i++) {
      os << v[i] << ", ";
      if ((i+1) % 15 == 0)
        os << std::endl << "\t       ";
    }
    os << "};" << std::endl;
  }
};


/** \brief Main-function
 *  \relates GraphColor
 */
int
main(int argc, char* argv[]) {
  InstanceOptions opt("GraphColor");
  opt.solutions(0);

  opt.instance("small1");

  opt.branching(GraphColor::BRANCH_AFC_SIZE);
  opt.branching(GraphColor::BRANCH_DEGREE,      "degree");
  opt.branching(GraphColor::BRANCH_SIZE,        "size");
  opt.branching(GraphColor::BRANCH_DEGREE_SIZE, "degree-size");
  opt.branching(GraphColor::BRANCH_AFC_SIZE,    "afc-size");
  opt.branching(GraphColor::BRANCH_ACTION_SIZE, "action-size");

  opt.parse(argc,argv);
  Script::run<GraphColor,BAB,InstanceOptions>(opt);
  return 0;
}

#include "examples/graph-color-instances.hpp"

// STATISTICS: example-any

