/**
 * Copyright (c) 2013-2016, George Ungureanu <ugeorge@kth.se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * stringtools.hpp
 *
 *  Created on: Sep 25, 2016
 *      Author: george
 */

#ifndef TOOLS_STRINGTOOLS_HPP_
#define TOOLS_STRINGTOOLS_HPP_

#include <algorithm>
#include <sstream>
#include <fstream>
#include <istream>
#include <cstdlib>
#include <list>
#include <boost/algorithm/string/join.hpp>
using namespace std;

namespace tools {

/**
 * Converts an element of any type (or at least most) into a string.
 *
 * @tparam T
 *         Element type.
 * @param e
 *        Element to convert.
 * @returns String representation.
 */
template<typename T>
std::string toString(const T& e) throw () {
  std::stringstream ss;
  ss << e;
  return ss.str();
}

/*template<template<class, class> class TContainer>
std::string toString(const TContainer<std::string, std::allocator<std::string>> & container) throw () {
  std::ostringstream imploded;
  const char* const delim = ", ";
  std::copy(container.begin(), container.end(),
             std::ostream_iterator<std::string>(imploded, delim));
  return imploded.str();
}*/

template<template<class, class> class TContainer, class TObject>
std::string toString(const TContainer<TObject, std::allocator<TObject>> & container) throw () {
  string out = "[";
  for (const auto& el : container)
    out+=tools::toString(el) + " ";
  return out + "]";
}



/**
 * Searched a string for another string and replaces it with a third.
 *
 * @param str
 *        Search-in string
 * @param search
 *        Search-for string.
 * @param replace
 *        Replacement string.
 * @returns Modified string.
 */
std::string& searchReplace(std::string& str, const std::string& search,
                           const std::string& replace) throw();

/**
 * Creates a tabbed indentation corresponding to the current parser
 * level.
 *
 * @param level
 *        Parsing level.
 * @returns Modified string.
 */
std::string indent(int level) throw();

/**
 * Trims front and end of a string from whitespace.
 *
 * @param str
 *        String to trim.
 * @returns Modified string.
 */
std::string& trim(std::string& str) throw();

/**
 * Converts a string to lower case.
 *
 * @param str
 *        String to convert.
 * @returns Modified string.
 */
std::string& toLowerCase(std::string& str) throw();

/**
 * Converts a string to upper case.
 *
 * @param str
 *        String to convert.
 * @returns Modified string.
 */
std::string& toUpperCase(std::string& str) throw();

/**
 * Splits a string into a vector at a certain delimiter.
 *
 * @param str
 *        String to split.
 * @param delim
 *        Delimiter character to split at.
 * @returns Vector of split strings.
 */
std::vector<std::string> split(const std::string& str, char delim) throw();

/**
 * Breaks lines in a string which exceed the specified maximum length at word
 * positions. The remaining part of the line will be indented by a specified
 * amount.
 *
 * @param str
 *        String to break.
 * @param maximum_length
 *        Maximum line length before breaking.
 * @param indent_length
 *        Number of spaces to include following a broken line.
 */
void breakLongLines(std::string& str, size_t maximum_length,
                    size_t indent_length)
    throw();


}

#endif /* TOOLS_STRINGTOOLS_HPP_ */
