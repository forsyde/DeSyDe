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
 * pathtools.hpp
 *
 *  Created on: Sep 22, 2016
 *      Author: George Ungureanu <ugeorge@kth.se>
 */

#ifndef TOOLS_SYSTOOLS_HPP_
#define TOOLS_SYSTOOLS_HPP_

#include <list>
#include <vector>
#include "../exceptions/ioexception.h"


using namespace DeSyDe;

namespace tools {

/**
 * @brief Returns a list of files in a path
 *
 * Checks if a path exists and returns a vector of files in a
 * path. If the path is a file itself, then it returns a
 * signleton list.
 *
 * @param inpath
 *        File or directory path.
 * @param extension
 *        file types to look out for
 * @throws IOException
 *         When there is something wrong with the file.
 * @throws IOException
 *         When there is something wrong with the file.
 */
std::vector<std::string> getFileNames(std::string inpath, std::string extension = "")
    throw (IOException);

bool isAccessible(const std::string &inpath) throw (IOException);

bool isValidFilePath(const std::string &inpath) throw (IOException);


/**
 * Gets the current system timestamp in the form of "YYYY-MM-DD hh:mm:ss".
 *
 * @returns Current timestamp.
 */
std::string getCurrentTimestamp() throw();


}

#endif
