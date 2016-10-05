/*
 * Copyright (c) 2011-2016 George Ungureanu <ugeorge@kth.se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OF THIS SOFTWARE NOR THE
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FORSYDE_TOOLS_CONTAINERS_H_
#define FORSYDE_TOOLS_CONTAINERS_H_

/**
 * @file
 * @author  George Ungureanu <ugeorge@kth.se>
 * @version 0.1
 *
 * @brief Contains various tools used by the classes.
 *
 * This file contains functions which are used by various classes and thus
 * do not belong to any particular class. Typically these include functions
 * which operate on dates, times and strings.
 */

#include <list>
#include <vector>
#include <iterator>

namespace tools {

/**
 * Copies and appends the content of one list to the end of another.
 *
 * @tparam T
 *         List element class.
 * @param to
 *        List to which to append the elements.
 * @param from
 *        List from which to copy the elements.
 */
/*template <typename T>
void append(std::list<T>& to, const std::list<T>& from) throw() {
    to.insert(to.end(), from.begin(), from.end());
}*/

template<template<class, class> class TContainer, class TObject>
void append(TContainer<TObject, std::allocator<TObject>> & to, const TContainer<TObject, std::allocator<TObject>> & from) throw () {
  to.insert(to.end(), from.begin(), from.end());
}

template<template<class, class> class TContainer, class TObject>
std::pair<TContainer<TObject, std::allocator<TObject>>, TContainer<TObject, std::allocator<TObject>>>
  split(const TContainer<TObject, std::allocator<TObject>> & container, size_t n ) throw () {

  size_t delim = std::min(n, container.size());
  auto it = container.begin();
  std::advance(it, delim);
  TContainer<TObject, std::allocator<TObject>> first(container.begin(), it);
  TContainer<TObject, std::allocator<TObject>> second(it, container.end());
  return std::pair<TContainer<TObject, std::allocator<TObject>>, TContainer<TObject, std::allocator<TObject>>>(first,second);
}

template<template<class, class> class TContainer, class TObject>
TContainer<TObject, std::allocator<TObject>>
  firstN(const TContainer<TObject, std::allocator<TObject>> & container, size_t n ) throw () {

  size_t delim = std::min(n, container.size());
  auto it = container.begin();
  std::advance(it, delim);
  TContainer<TObject, std::allocator<TObject>> first(container.begin(), it);
  return first;
}

}

#endif
