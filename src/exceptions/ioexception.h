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

#ifndef EXCEPTIONS_IOEXCEPTION_H_
#define EXCEPTIONS_IOEXCEPTION_H_

#include "runtimeexception.h"
#include <string>

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines exceptions for I/O errors.
 */

namespace DeSyDe {

/**
 * @brief Used to indicate that an I/O error has occurred when operating on
 *        a file.
 */
class IOException : public Exception {
  public:
    /**
     * Creates an exception with no message.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     * @param file
     *        File where the I/O error occurred.
     */
    IOException(const std::string& source_file, int source_line,
                const std::string& file) throw();

    /**
     * Same as IOException(const std::string&) but with a specified error
     * message.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     * @param file
     *        File path where the I/O error occurred.
     * @param message
     *        Error message.
     */
    IOException(const std::string& source_file, int source_line,
                const std::string& file, const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~IOException() throw();

    /**
     * Gets the file of this exception.
     *
     * @returns File path.
     */
    std::string getFile() const throw();

    /**
     * See Exception::getMessage().
     *
     * @returns Error message.
     */
    virtual std::string getMessage() const throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();

  protected:
    /**
     * File where the I/O error occurred.
     */
    const std::string file_;
};


/**
 * @brief Used to indicate that a particular file was not found when it was
 *        expected to.
 */
class FileNotFoundException : public IOException {
  public:
    /**
     * Creates an exception with no message.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     * @param file
     *        File which was not found.
     */
    FileNotFoundException(const std::string& source_file, int source_line,
                          const std::string& file) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~FileNotFoundException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};


/**
 * @brief Used when input data is of invalid format.
 */
class InvalidFormatException : public IOException {
  public:
    /**
     * @copydoc Exception::Exception(const std::string&, int, const std::string&)
     */
    InvalidFormatException(const std::string& source_file, int source_line,
                          const std::string& message) throw();
    /**
     * @copydoc Exception::Exception(const std::string&, int, const std::string&)
     */
    InvalidFormatException(const std::string& source_file, int source_line,
        const std::string& cause, const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~InvalidFormatException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};


/**
 * @brief Used when a parse method fails.
 */
class ParseException : public IOException {
  public:
    /**
     * Creates an exception.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line in the source file from where the exception was thrown.
     * @param file
     *        Parsed file.
     * @param message
     *        Error message.
     */
    ParseException(const std::string& source_file, int source_line,
                   const std::string& file, const std::string& message) throw();

    /**
     * Creates an exception.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     * @param file
     *        Parsed file.
     * @param line
     *        Line  where the parsing failed.
     * @param message
     *        Error message.
     */
    ParseException(const std::string& source_file, int source_line,
                   const std::string& file, int line,
                   const std::string& message) throw();

    /**
     * Creates an exception.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     * @param file
     *        Parsed file.
     * @param line
     *        Line where the parsing failed.
     * @param column
     *        Column  where the parsing failed.
     * @param message
     *        Error message.
     */
    ParseException(const std::string& source_file, int source_line,
                   const std::string& file, int line, int column,
                   const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~ParseException() throw();

    /**
     * @copydoc Exception::getMessage()
     */
    virtual std::string getMessage() const throw();

    /**
     * Gets the parsed file.
     *
     * @returns Parsed file.
     */
    std::string getFile() const throw();

    /**
     * Gets the line  where the parsing failed. If no line  is
     * available \c -1 is returned.
     *
     * @returns Line , if available; otherwise \c -1.
     */
    int getLine() const throw();

    /**
     * Gets the column  where the parsing failed. If no column  is
     * available \c -1 is returned.
     *
     * @returns Column , if available; otherwise \c -1.
     */
    int getColumn() const throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();

  protected:
    /**
     * Parsed file.
     */
    const std::string file_;

    /**
     * Line where the parsing failed.
     */
    int line_;

    /**
     * Column where the parsing failed.
     */
    int column_;
};

}



#endif
