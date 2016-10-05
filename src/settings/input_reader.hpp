/**
 * Copyright (c) 2013-2016, Nima Khalilzad   <nkhal@kth.se>
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
#pragma once
#include <math.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cassert>

#include <libxml/xmlreader.h>
#include "system/mapping.hpp"
#include "xml/xmldoc.hpp"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>


namespace fs = boost::filesystem;

using namespace std;

/**
 * A class for saving the modes found in the XML file
 * since we pars the file from the deepest child 
 * and modes params are deeper than processor params
 */ 
class ProcessorMode
{
public:
    ProcessorMode(char* _model, vector<char*> _elements, vector<char*> _values):
    model(_model),
    elements(_elements),
    values(_values)
    {};
    char* model;
    vector<char*> elements;
    vector<char*> values;
};

class InputReader
{
public:
    InputReader(std::string _inputsPath);;
    /**
     * reads a file named taskset.xml from the input directory and
     * returns a TaskSet object
     */ 
    vector<PeriodicTask*> ReadTaskset(const string fileName);
    
    /**
     * reads a file named platform.xml from the input directory and
     * returns a vector of PEs
     */ 
    vector<PE*> ReadPlatform(const string fileName);
    
    /**
     * reads a file named WCETs.xml from the input directory and
     * sets the corresponding values to its input mapping
     */ 
    void ReadWCETS(const string fileName, Mapping* _mapping);
    /**
     * reads csv files and 
     * returns a two dimentional vector of string
     */ 
    vector<vector<string>> ReadCSV(string file);
    
    
    void ReadConstraints(const string fileName, vector<SDFGraph*>* sdfApps);
        
    friend std::ostream& operator<< (std::ostream &out, const InputReader &inputReader);
    
private:
    enum                         ReadingModes{TASK, PLATFORM, WCET, CONSTRAINTS};
    ReadingModes                xmlReadingMode;
    string                         inputsPath;
    vector<PeriodicTask*>         tasks;
    vector<PE*>                    processors;
    vector<ProcessorMode >        modes;
    Mapping*                     mapping;
    
    vector<SDFGraph*>* sdfApps;

    void     walk_tree( xmlNodePtr node );
    int     count_element_children( xmlNodePtr node );
    void     parsTaskXMLNode(vector<char*> elements, vector<char*> values, xmlNodePtr node);
    void     parsPlatformXMLNode(vector<char*> elements, vector<char*> values, xmlNodePtr node);
    void     parsWCETXMLNode(vector<char*> elements, vector<char*> values, xmlNodePtr node);
    void     parsConstXMLNode(vector<char*> elements, vector<char*> values, xmlNodePtr node);
    void     readXML(string filePath);
    char*     getValueOfElement(vector<char*> elements, vector<char*> values, char* inElement);
    char*     getValueOfElement(xmlNodePtr node, char* inElement);
    
};


