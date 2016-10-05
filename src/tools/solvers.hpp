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
 * solvers.hpp
 *
 *  Created on: Sep 29, 2016
 *      Author: George Ungureanu <ugeorge@kth.se>
 */

#ifndef SRC_TOOLS_SOLVERS_HPP_
#define SRC_TOOLS_SOLVERS_HPP_

#include "../exceptions/runtimeexception.h"

#include <vector>

using namespace DeSyDe;
using namespace std;

namespace tools {

vector<int> solveLinearEqSys(
    const std::vector<std::vector<int>>& A,
    const std::vector<int>& y
    ) throw (InvalidArgumentException);

vector<double> solveFloatSys(
    const std::vector<std::vector<int>>& A,
    const std::vector<int>& y
    ) throw (InvalidArgumentException);


}


#endif /* SRC_TOOLS_SOLVERS_HPP_ */
