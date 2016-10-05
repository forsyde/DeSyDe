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

#ifndef EXCEPTIONS_EXCEPTION_H_
#define EXCEPTIONS_EXCEPTION_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines the base class for exceptions.
 */


#include <exception>
#include <string>


namespace DeSyDe {

  /**
   * @brief Base class for exceptions.
   *
   * This base class serves as base from which other exceptions derive . The \c
   * Exception class itself derives from \c std::exception in order to be
   * throwable, but hides the methods provided by the standard exception class
   * since it provides equivalent methods of its own.
   *
   * The macro #THROW_EXCEPTION(exception_class, ...) can be used to simplify
   * exception throwing.
   */
  class Exception : private std::exception {
  public:
    /**
     * Creates an exception with no message.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line from where the exception was thrown.
     */
    Exception(const std::string& source_file, int source_line) throw();

    /**
     * Same as Exception() but with an error message.
     *
     * @param source_file
     *        Name of source file from where the exception was thrown.
     * @param source_line
     *        Line in the source file from where the exception was thrown.
     * @param message
     *        Message of what caused this exception.
     */
    explicit Exception(const std::string& source_file, int source_line,
                       const std::string& message) throw();

    /**
     * Destroys this exception.
     */
    virtual ~Exception() throw();

    /**
     * Gets the name of the source file from where the exception was thrown.
     *
     * @returns Source file.
     */
    std::string getSourceFile() const throw();

    /**
     * Gets the line from where the exception was thrown.
     *
     * @returns Line 
     */
    int getSourceLine() const throw();

    /**
     * Gets the error message of this exception.
     *
     * @returns Error message.
     */
    virtual std::string getMessage() const throw();

    /**
     * Gets a string representation of this exception.
     *
     * @returns This exception as a string.
     */
    std::string toString() const throw();

  protected:
    /**
     * Gets the name of this exception as a string (needed for toString()).
     *
     * @returns Exception name.
     */
    virtual std::string type() const throw();

  private:
    /**
     * Name of source file from where the exception was thrown.
     */
    const std::string source_file_;

    /**
     * Line from where the exception was thrown.
     */
    const int source_line_;

  protected:
    /**
     * Error message.
     */
    const std::string message_;
  };

}

/**
 * @file
 * @def THROW_EXCEPTION(exception_class, ...)
 *
 * @brief Throws an exception.
 *
 * Throws an exception.
 * 
 * All exceptions require that the name of the source file and the line from
 * which the exception is thrown. Although there are preleafor macros for
 * getting the file name and line (\c __FILE__ and \c __LINE__), this would be
 * tedious if they would have to be inserted manually into the constructor
 * arguments of the exception (using the macros inside the constructor doesn't
 * work). By using this macro, these required values are automatically added as
 * arguments to the exception so the programmer needn't bother.
 *
 * The macro takes an exception class as mandatory argument, along with a
 * variable list of additional arguments. Thus, an exception of type \c
 * ExampleException, which requires an additional \c int argument, can be thrown
 * as follows:
 * @code
 *    THROW_EXCEPTION(ExampleException, 10);
 * @endcode
 * This is equivalent to writing:
 * @code
 *    throw ExampleException(__FILE__, __LINE__, 10);
 * @endcode
 * If the exception has no additional arguments, the variable list of arguments
 * must be removed entirely, like so:
 * @code
 *    THROW_EXCEPTION(ExampleException);
 * @endcode
 * The following is an \em incorrect use of the macro and causes an syntax error
 * when compiled:
 * @code
 *    THROW_EXCEPTION(ExampleException, );
 * @endcode
 *
 * @param exception_class
 *        Name of the exception class to throw.
 * @param ...
 *        A variable list of additional arguments needed by the exception
 *        constructor. If there are no such arguments, remove this parameter
 *        entirely in the macro call (see description).
 */

#define THROW_EXCEPTION(exception_class, ...)                   \
  throw exception_class(__FILE__, __LINE__, ##__VA_ARGS__)
    

#endif
