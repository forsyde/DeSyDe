/*
 * Copyright (c) 2011-2013
 *     Gabriel Hjort Blindell <ghb@kth.se>
 *  George Ungureanu <ugeorge@kth.se>
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

#ifndef EXCEPTIONS_RUNTIMEEXCEPTION_H_
#define EXCEPTIONS_RUNTIMEEXCEPTION_H_

/**
 * @file
 * @author  Gabriel Hjort Blindell <ghb@kth.se>
 * @version 0.1
 *
 * @brief Defines base classes for errors occurring during runtime, most likely
 *        due to programming errors.
 */

#include "exception.h"
#include <string>

namespace DeSyDe {

/**
 * @brief Base class for runtime error.
 *        
 * Runtime exceptions is a category which defines errors that occur during
 * execution. Most often they are unexpected from the program's point of view
 * and occur due to programming errors.
 */
class RuntimeException : public Exception {
  public:
    /**
     * @copydoc Exception::Exception(const std::string&, int)
     */
    RuntimeException(const std::string& source_file, int source_line) throw();

    /**
     * @copydoc Exception::Exception(const std::string&, int, const std::string&)
     */
    RuntimeException(const std::string& source_file, int source_line,
                          const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~RuntimeException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};



/**
 * @brief Used when a cast fails (mostly used for dynamic casts).
 */
class CastException : public RuntimeException {
  public:
    /**
     * @copydoc Exception(const std::string&, int)
     */
    CastException(const std::string& source_file, int source_line) throw();

    /**
     * @copydoc Exception::Exception(const std::string&, int, const std::string&)
     */
    CastException(const std::string& source_file, int source_line,
                  const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~CastException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};


/**
 * @brief Used when a method was called on an object which does not allow
 *        it, for whatever reason.
 */
class IllegalCallException : public RuntimeException {
  public:
    /**
     * @copydoc Exception::Exception(const std::string&, int, const std::string&)
     */
    IllegalCallException(const std::string& source_file, int source_line,
                          const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~IllegalCallException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};


/**
 * @brief Used when a method fails due to its object being in an illegal
 *        state. Illegal states just should not happen and probably indicate a
 *        bug.
 */
class IllegalStateException : public RuntimeException {
  public:
    /**
     * @copydoc Exception::Exception(const std::string&, int, const std::string&)
     */
    IllegalStateException(const std::string& source_file, int source_line,
                          const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~IllegalStateException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};

/**
 * @brief Used when an attempt access a container uses an index which is out of
 * bounds.
 */
class IndexOutOfBoundsException : public RuntimeException {
  public:
    /**
     * @copydoc Exception::Exception(const std::string&, int)
     */
    IndexOutOfBoundsException(const std::string& source_file, int source_line)
        throw();

    /**
     * @copydoc Exception::Exception(const std::string&, int, const std::string&)
     */
    IndexOutOfBoundsException(const std::string& source_file, int source_line,
                         const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~IndexOutOfBoundsException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};


/**
 * @brief Used when a method is invoked with invalid arguments which just should
 *        not happen. Throwing this exception is an indication of a bug.
 */
class InvalidArgumentException : public RuntimeException {
  public:
    /**
     * @copydoc RuntimeException::RuntimeException(const std::string&, int, const std::string&)
     */
    InvalidArgumentException(const std::string& source_file, int source_line,
                          const std::string& message) throw();

    /**
     * @copydoc RuntimeException::RuntimeException(const std::string&, int, const std::string&)
     */
    InvalidArgumentException(const std::string& source_file, int source_line,
        const std::string& arg,
        const std::string& cause) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~InvalidArgumentException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};


/**
 * @brief Used when a method was called on an object which does not support it
 *        it, for whatever reason.
 */
class NotSupportedException : public RuntimeException {
  public:
    /**
     * @copydoc Exception::Exception(const std::string&, int)
     */
    NotSupportedException(const std::string& source_file, int source_line)
        throw();

    /**
     * @copydoc Exception::Exception(const std::string&, int, const std::string&)
     */
    NotSupportedException(const std::string& source_file, int source_line,
                          const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~NotSupportedException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};


/**
 * @brief Used when an attempt to allocate new memory failed.
 */
class OutOfMemoryException : public RuntimeException {
  public:
    /**
     * @copydoc Exception::Exception(const std::string&, int)
     */
    OutOfMemoryException(const std::string& source_file, int source_line)
        throw();

    /**
     * @copydoc Exception::Exception(const std::string&, int, const std::string&)
     */
    OutOfMemoryException(const std::string& source_file, int source_line,
                         const std::string& message) throw();

    /**
     * @copydoc Exception::~Exception()
     */
    virtual ~OutOfMemoryException() throw();

  protected:
    /**
     * @copydoc Exception::type()
     */
    virtual std::string type() const throw();
};

}

#endif
