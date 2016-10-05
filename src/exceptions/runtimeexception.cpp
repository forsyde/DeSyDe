/*
 * Copyright (c) 2011-2013
 *     Gabriel Hjort Blindell <ghb@kth.se>
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

#include "runtimeexception.h"

using namespace DeSyDe;
using std::string;

RuntimeException::RuntimeException(const string& source_file, int source_line)
        throw() : Exception(source_file, source_line) {}

RuntimeException::RuntimeException(
    const string& source_file, int source_line, const string& message)
        throw() : Exception(source_file, source_line, message) {}

RuntimeException::~RuntimeException() throw() {}

string RuntimeException::type() const throw() {
    return "RuntimeException";
}


CastException::CastException(const string& source_file, int source_line)
        throw() : RuntimeException(source_file, source_line, "") {}

CastException::CastException(const string& source_file, int source_line,
                             const string& message)
        throw() : RuntimeException(source_file, source_line, message) {}

CastException::~CastException() throw() {}

string CastException::type() const throw() {
    return "CastException";
}

IllegalCallException::IllegalCallException(
    const string& source_file, int source_line, const string& message)
        throw() : RuntimeException(source_file, source_line, message) {}

IllegalCallException::~IllegalCallException() throw() {}

string IllegalCallException::type() const throw() {
    return "IllegalCallException";
}


IllegalStateException::IllegalStateException(
    const string& source_file, int source_line, const string& message)
        throw() : RuntimeException(source_file, source_line, message) {}

IllegalStateException::~IllegalStateException() throw() {}

string IllegalStateException::type() const throw() {
    return "IllegalStateException";
}

IndexOutOfBoundsException::IndexOutOfBoundsException(
    const string& source_file, int source_line) throw()
        : RuntimeException(source_file, source_line) {}

IndexOutOfBoundsException::IndexOutOfBoundsException(
    const string& source_file, int source_line, const string& message) throw()
        : RuntimeException(source_file, source_line, message) {}

IndexOutOfBoundsException::~IndexOutOfBoundsException() throw() {}

string IndexOutOfBoundsException::type() const throw() {
    return "IndexOutOfBoundsException";
}

InvalidArgumentException::InvalidArgumentException(
    const string& source_file, int source_line, const string& message)
        throw() : RuntimeException(source_file, source_line, message) {}

InvalidArgumentException::InvalidArgumentException(
    const string& source_file, int source_line, const string& arg,
    const string& cause)
        throw() : RuntimeException(source_file, source_line,
        string("\"") + arg + "\" must not be " + cause) {}

InvalidArgumentException::~InvalidArgumentException() throw() {}

string InvalidArgumentException::type() const throw() {
    return "InvalidArgumentException";
}

NotSupportedException::NotSupportedException(
    const string& source_file, int source_line)
        throw() : RuntimeException(source_file, source_line) {}

NotSupportedException::NotSupportedException(
    const string& source_file, int source_line, const string& message)
        throw() : RuntimeException(source_file, source_line, message) {}

NotSupportedException::~NotSupportedException() throw() {}

string NotSupportedException::type() const throw() {
    return "NotSupportedException";
}


OutOfMemoryException::OutOfMemoryException(
    const string& source_file, int source_line) throw()
        : RuntimeException(source_file, source_line) {}

OutOfMemoryException::OutOfMemoryException(
    const string& source_file, int source_line, const string& message) throw()
        : RuntimeException(source_file, source_line, message) {}

OutOfMemoryException::~OutOfMemoryException() throw() {}

string OutOfMemoryException::type() const throw() {
    return "OutOfMemoryException";
}
