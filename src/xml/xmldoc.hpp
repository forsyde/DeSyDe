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
#ifndef XML_XMLDOC_H_
#define XML_XMLDOC_H_

#include <cstring>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>

#include "../settings/config.hpp"
#include "../logger/logger.hpp"
#include "../exceptions/ioexception.h"

using namespace DeSyDe;

/**
 * @brief XML parser parent class
 */
class XMLdoc {
public:

  /** @brief Empty constructor */
  XMLdoc(const std::string& filepath) throw ();

  /** @brief Copy constructor */
  XMLdoc(const XMLdoc &lhs) throw ();

  /** @brief Destructor */
  virtual ~XMLdoc() throw();


  /** 
   * @brief Reads an XML file
   *
   * Checks and XML file and creates the DOM tree
   *
   * @param filepath
   *        XML file path
   * @param dtd
   *        (optional) DTD validation
   * @throws IOException
   *         When there is something wrong with the file.
   */
  void read(bool dtd = false) throw (IOException);

  /**
   * @brief Reads an XML file and validate agains XSD schema
   *
   * Checks and XML file based on a schema in the root node
   * and creates the DOM tree
   *
   * @param filepath
   *        XML file path
   * @param path_node
   *        node containing URI to XSD schema
   * @param path_node
   *        attribute name with URI to XSD schema
   * @throws IOException
   *         When there is something wrong with the file.
   */
  void readXSD(const char* path_node, const char* path_attribute) throw (IOException, DeSyDe::ParseException);

  /**
   * @brief Reads an XML file and validate agains XSD schema
   *
   * Checks and XML file based on a schema in the root node
   * and creates the DOM tree
   *
   * @param filepath
   *        XML file path
   * @param xsd_uri
   *        path to XSD schema
   * @throws IOException
   *         When there is something wrong with the file.
   */
  void readXSD(const char* xsd_uri) throw (IOException);


  /** @brief Dumps this document to a file */
  void dump(std::string filepath) throw (IOException);

  std::vector<std::string> xpathStrings (const char* xpath) throw (InvalidArgumentException);
  std::vector<xmlNodePtr> xpathNodes (const char* xpath) throw (InvalidArgumentException);

  bool hasProp(xmlNodePtr node, const std::string attr) throw ();
  std::string getProp(xmlNodePtr, const std::string name) throw (InvalidArgumentException);

protected:  
  xmlDocPtr        doc;    //!< XML document 
  xmlNodePtr       root;   //!< Root XML node  

  /** @brief Lightweight node getter only for root */
  xmlNodePtr getFirstChild(const xmlChar * nodename) throw ();

private:
  bool validateXSD(const char *schema_path);

  template<typename T>
  void safeXmlFree(T t){  if ( t ) { xmlFree(t); t = NULL; } }

  void safeXmlFreeDoc(xmlDocPtr d){  if ( d ) { xmlFreeDoc(d); d = NULL; } }


  std::string path_;

};

#endif
