/*
 * solvers.cpp
 *
 *  Created on: Sep 29, 2016
 *      Author: george
 */

#include "solvers.hpp"

#include <gecode/int.hh>
#include <gecode/float.hh>
#include <gecode/search.hh>
#include <gecode/gist.hh>


#include <boost/numeric/ublas/io.hpp>

using namespace Gecode;

class LinearEqSysSolver : public Space {
protected:
  IntVarArray x;
public:
  LinearEqSysSolver(
      const std::vector<std::vector<int>>& A,
      const std::vector<int>& y) throw (InvalidArgumentException)
    : x(*this, y.size(), 1, Int::Limits::max) {

    const size_t n_actors = y.size();
    const size_t n_channels = A.size();

    for (size_t i=0; i<n_channels; i++ ) {
      IntArgs c(n_actors);
      IntVarArgs xs(n_actors);
      for (size_t j=0; j<n_actors; j++ ) {
        c[j]  = A[i][j];
        xs[j] = x[j];
      }

      linear(*this, c, xs, IRT_EQ, y[i]);
    }
    branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
  }

  LinearEqSysSolver(bool share, LinearEqSysSolver& s) : Space(share, s) {
    x.update(*this, share, s.x);
  }

  virtual Space* copy(bool share) {
    return new LinearEqSysSolver(share,*this);
  }

  void print(void) const {
    std::cout << x << std::endl;
  }

  vector<int> getSolution(void) const {
    vector<int> tmp_sol;
    for (int i=0; i<x.size(); i++) {
      //if x[i].assigned()
      tmp_sol.push_back(x[i].val());
      //...else something is wrong
    }
    return tmp_sol;
  }

  // constrain function
  virtual void constrain(const Space& _b) {
    const LinearEqSysSolver& b = static_cast<const LinearEqSysSolver&>(_b);

    int sum = 0;
    IntVarArgs xs(b.x.size());
    for (int i=0; i<b.x.size(); i++) {
      sum += b.x[i].val();
      xs[i] = x[i];
    }
    linear(*this, xs, IRT_LE, sum);
  }
};


class FloatSysSolver : public Space {
protected:
  FloatVarArray x;
public:
  FloatSysSolver(
      const std::vector<std::vector<int>>& A,
      const std::vector<int>& y) throw (InvalidArgumentException)
    : x(*this, y.size(), 1, Float::Limits::max) {

    const size_t n_actors = y.size();
    const size_t n_channels = A.size();

    for (size_t i=0; i<n_channels; i++ ) {
      FloatValArgs c(n_actors);
      FloatVarArgs xs(n_actors);
      for (size_t j=0; j<n_actors; j++ ) {
        c[j]  = A[i][j];
        xs[j] = x[j];
      }

      linear(*this, c, xs, FRT_EQ, y[i]);
    }
    branch(*this, x, FLOAT_VAR_SIZE_MIN(), FLOAT_VAL_SPLIT_MIN());
  }

  FloatSysSolver(bool share, FloatSysSolver& s) : Space(share, s) {
    x.update(*this, share, s.x);
  }

  virtual Space* copy(bool share) {
    return new FloatSysSolver(share,*this);
  }

  void print(void) const {
    std::cout << x << std::endl;
  }

  vector<double> getSolution(void) const {
    vector<double> tmp_sol;
    for (int i=0; i<x.size(); i++) {
      //if x[i].assigned()
      tmp_sol.push_back(x[i].val().min());
      //...else something is wrong
    }
    return tmp_sol;
  }

  // constrain function
  virtual void constrain(const Space& _b) {
    const FloatSysSolver& b = static_cast<const FloatSysSolver&>(_b);

    double sum = 0;
    FloatVarArgs xs(b.x.size());
    for (int i=0; i<b.x.size(); i++) {
      sum += b.x[i].val().max();
      xs[i] = x[i];
    }
    linear(*this, xs, FRT_LE, sum);
  }
};



vector<int> tools::solveLinearEqSys(
    const std::vector<std::vector<int>>& A,
    const std::vector<int>& y) throw (InvalidArgumentException) {

  LinearEqSysSolver* m = new LinearEqSysSolver(A,y);
  //Gist::bab(m);
  BAB<LinearEqSysSolver> e(m);
  delete m;

  std::vector<int> solutions;
  while (LinearEqSysSolver* s = e.next()) {
    s->print();
    solutions = s->getSolution();
    delete s;
  }
  //if solutions.size() = 0, SDFG is probably not consistent!
  return solutions;
}

vector<double> tools::solveFloatSys(
    const std::vector<std::vector<int>>& A,
    const std::vector<int>& y) throw (InvalidArgumentException) {

  FloatSysSolver* m = new FloatSysSolver(A,y);
  //Gist::bab(m);
  BAB<FloatSysSolver> e(m);
  delete m;

  std::vector<double> solutions;
  while (FloatSysSolver* s = e.next()) {
    s->print();
    solutions = s->getSolution();
    delete s;
  }
  //if solutions.size() = 0, SDFG is probably not consistent!
  return solutions;
}

