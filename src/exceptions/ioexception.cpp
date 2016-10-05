/*
 * Copyright (c) 2011-2013
 *     Gabriel Hjort Blindell <ghb@kth.se>
 *     George Ungureanu <ugeorge@kth.se>
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

#include "ioexception.h"
#include <string>


using namespace DeSyDe;
using std::string;

IOException::IOException(const string& source_file, int source_line,
                         const string& file) throw()
        : Exception(source_file, source_line, "I/O error"),
          file_(file) {}

IOException::IOException(const string& source_file, int source_line,
                         const string& file, const string& message)
        throw() : Exception(source_file, source_line, message),
                  file_(file) {}

IOException::~IOException() throw() {}

string IOException::getFile() const throw() {
    return file_;
}

string IOException::getMessage() const throw() {
    return message_ + " (\"" + file_ + "\")";
}

string IOException::type() const throw() {
    return "IOException";
}

FileNotFoundException::FileNotFoundException(
    const string& source_file, int source_line,
    const string& file) throw()
        : IOException(source_file, source_line, file, "file not found") {}

FileNotFoundException::~FileNotFoundException() throw() {}

string FileNotFoundException::type() const throw() {
    return "FileNotFoundException";
}


InvalidFormatException::InvalidFormatException(
    const string& source_file, int source_line, const string& message)
        throw() : IOException(source_file, source_line, message) {}

InvalidFormatException::InvalidFormatException(
    const string& source_file, int source_line, const string& cause, const string& message)
        throw() : IOException(source_file, source_line,
            cause, message )  {}


InvalidFormatException::~InvalidFormatException() throw() {}

string InvalidFormatException::type() const throw() {
    return "InvalidFormatException";
}

ParseException::ParseException(
    const string& source_file, int source_line, const string& file,
    const string& message)
        throw() : IOException(source_file, source_line, message), file_(file),
                  line_(-1), column_(-1) {}

ParseException::ParseException(
    const string& source_file, int source_line, const string& file, int line,
    const string& message)
        throw() : IOException(source_file, source_line, message), file_(file),
                  line_(line), column_(-1) {}

ParseException::ParseException(
    const string& source_file, int source_line, const string& file, int line,
    int column, const string& message)
        throw() : IOException(source_file, source_line, message), file_(file),
                  line_(line), column_(column) {}

ParseException::~ParseException() throw() {}

string ParseException::getMessage() const throw() {
    string str;
    str += "\"";
    str += file_;
    str += "\"";
    if (line_ >= 0) {
        str += " at line ";
        str += std::to_string(line_);
    }
    str += ": ";
    str += message_;
    return str;
}

string ParseException::getFile() const throw() {
    return file_;
}

int ParseException::getLine() const throw() {
    return line_;
}

int ParseException::getColumn() const throw() {
    return column_;
}

string ParseException::type() const throw() {
    return "ParseException";
}
