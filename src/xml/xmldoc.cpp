#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "xmldoc.hpp"

using namespace std;
using namespace DeSyDe;
namespace fs = boost::filesystem;

XMLdoc::XMLdoc(const string& filepath) throw()
    : doc(NULL), root(NULL), path_(filepath) {
}

XMLdoc::XMLdoc(const XMLdoc &lhs) throw()
    : doc(lhs.doc), root(lhs.root), path_(lhs.path_) { }

XMLdoc::~XMLdoc() throw() {
  safeXmlFreeDoc(doc);
}

void XMLdoc::read (bool dtd) throw (IOException) {
  LIBXML_TEST_VERSION;

  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  int opts = dtd ? XML_PARSE_DTDVALID : 0;
  doc = xmlCtxtReadFile(ctxt, path_.c_str(), NULL, opts);

  if (doc == NULL ) 
    THROW_EXCEPTION (IOException, path_, string("file not found"));
  root = xmlDocGetRootElement(doc);
	
  if (root == NULL) {
    THROW_EXCEPTION (IOException, path_, string("empty document"));
    safeXmlFreeDoc(doc);
  }

  if (!ctxt->valid) {
    safeXmlFreeDoc(doc);
    THROW_EXCEPTION (IOException, path_, string("failed to validate document"));
  }

  //TODO: logger debug
}

void XMLdoc::readXSD (const char* xsd_node, const char* xsd_attr) throw (IOException, ParseException) {
  LIBXML_TEST_VERSION;

  LOG_DEBUG("Parsing " + path_ + "...");

  doc = xmlReadFile(path_.c_str(), NULL, 0);
  if (doc == NULL)
    THROW_EXCEPTION (IOException, path_, string("failed to parse"));

  root = xmlDocGetRootElement(doc);
  if (root == NULL) {
      safeXmlFreeDoc(doc);
      THROW_EXCEPTION (IOException, path_, string("empty document"));
  }

  xmlNodePtr sdf_node = getFirstChild((const xmlChar *)xsd_node);
  if (sdf_node == NULL) {
    safeXmlFreeDoc(doc);
    THROW_EXCEPTION (ParseException,path_, string()
        + "failed to find \'" + xsd_node + "\' node");
  }

  xmlChar*   xsd_uri;
  xmlNsPtr* name_spc =  xmlGetNsList(doc,sdf_node);
  if (name_spc == NULL) {
    xsd_uri = xmlGetProp(sdf_node,(const xmlChar *)xsd_attr);
  }
  else {
    xsd_uri = xmlGetNsProp(sdf_node, (const xmlChar *)xsd_attr, (*name_spc)->href);
  }
  if (xsd_uri == NULL) {
    safeXmlFreeDoc(doc);
    THROW_EXCEPTION (ParseException,path_, string()
        + "failed to find \'" + xsd_attr
        + "\' inside the \'" + xsd_node + "\' node.");
  }
  LOG_DEBUG(string() + "Found XSD schema at " + (char *)xsd_uri);

  /*if (!validateXSD((char *)xsd_uri))
    THROW_EXCEPTION (IOException, path_, string () +
        "could not validate path against schema at " + (char *)xsd_uri);*/

  safeXmlFree(xsd_uri);
  //xmlFreeNsList(*name_spc);
  LOG_DEBUG("Document valid");
  LOG_DEBUG("Parsed " + path_ );
}

void XMLdoc::readXSD (const char* xsd_uri) throw (IOException) {
  LIBXML_TEST_VERSION;

  doc = xmlReadFile(path_.c_str(), NULL, 0);
  if (doc == NULL)
    THROW_EXCEPTION (IOException, path_, string("failed to parse"));

  root = xmlDocGetRootElement(doc);
  if (root == NULL) {
    THROW_EXCEPTION (IOException, path_, string("empty document"));
    safeXmlFreeDoc(doc);
  }
  if (!validateXSD(xsd_uri))
    THROW_EXCEPTION (IOException, path_, string("failed to parse"));

  //TODO: logger debug
}


void XMLdoc::dump (string filepath) throw (IOException) {
  fs::path p (filepath);
  if (!fs::exists(p.parent_path()) || !fs::is_directory(p.parent_path()))
    THROW_EXCEPTION (IOException, filepath, string("path does not exist"));

  FILE* output  = fopen (filepath.c_str(), "w");
  if (output == NULL)
    THROW_EXCEPTION (IOException, filepath, string("cannot open path"));

  if (xmlDocDump(output, doc) == -1) 
    THROW_EXCEPTION (IOException, filepath, string("failed to dump document"));

  fclose (output);
}

vector<string> XMLdoc::xpathStrings (const char* xpath) throw (InvalidArgumentException){
  vector<string> result_strings;

  for (auto& node : xpathNodes(xpath)) {
    xmlChar* keyword = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    result_strings.push_back(string((char *)keyword));
    safeXmlFree(keyword);
  }
  return result_strings;
}

std::vector<xmlNodePtr> XMLdoc::xpathNodes (const char* xpath) throw (InvalidArgumentException){
  xmlXPathContextPtr context;
  xmlXPathObjectPtr  result;
  vector<xmlNodePtr> result_nodes;

  context = xmlXPathNewContext(doc);
  if (context == NULL) {
    THROW_EXCEPTION (InvalidArgumentException,string()
        + "cannot create xpath context from " + xpath);
  }
  result = xmlXPathEvalExpression((xmlChar *)xpath, context);
  if (result == NULL) {
    xmlXPathFreeContext(context);
    THROW_EXCEPTION (InvalidArgumentException,string()
        + "cannot evaluae xpath " + xpath);
  }
  if ( result->nodesetval && result->nodesetval->nodeTab ) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    for (int i=0; i < nodeset->nodeNr; i++) {
      result_nodes.push_back(nodeset->nodeTab[i]);
    }
  }
  xmlXPathFreeObject(result);
  xmlXPathFreeContext(context);
  return result_nodes;
}

bool XMLdoc::hasProp(xmlNodePtr node, const std::string attr) throw () {
  xmlChar* val = xmlGetProp(node, (const xmlChar *)attr.c_str());
  bool has_prop = (val != NULL);
  safeXmlFree(val);
  return has_prop;
}


std::string XMLdoc::getProp(xmlNodePtr node, const std::string attr) throw (InvalidArgumentException) {
  xmlChar* val = xmlGetProp(node, (const xmlChar *)attr.c_str());
  if (val == NULL) {
    safeXmlFree(val);
    THROW_EXCEPTION (InvalidArgumentException,string()
        + "cannot get attribute \'" + attr
        + "\' from node \'" + (char*)node->name + "\'");
  }

  string ret = (char *)val;
  safeXmlFree(val);
  return tools::trim(ret);
}

xmlNodePtr XMLdoc::getFirstChild(const xmlChar* nodename)  throw () {
  xmlNodePtr node = root;
  while(node != NULL){
    if(!xmlStrcmp(node->name, nodename)) return node;
    node = node->next;
  }
  return NULL;
}

// first, two callbacks that display errors - you could probably use
// fprintf instead.

static void xmlSchemaValidityErrorFunc_impl(void __attribute__((unused)) *ctx, const char *msg, ...) {
  static char buffer[5000];
  va_list argp;
  va_start(argp, msg);
  vsprintf(buffer, msg, argp);
  va_end(argp);
  // elided logging of errors
}


static void xmlSchemaValidityWarningFunc_impl(void __attribute__((unused)) *ctx, const char *msg, ...) {
  static char buffer[5000];
  va_list argp;
  va_start(argp, msg);
  vsprintf(buffer, msg, argp);
  va_end(argp);
  // elided logging of warnings
}



bool XMLdoc::validateXSD(const char *schema_path) {
  xmlDocPtr schema_doc = NULL;
  xmlSchemaParserCtxtPtr parser_ctxt = NULL;
  xmlSchemaPtr schema = NULL;
  xmlSchemaValidCtxtPtr valid_ctxt = NULL;

  if (!(schema_doc = xmlReadFile(schema_path, NULL, 0))) {
    THROW_EXCEPTION (IOException, "", string("failed to read schema from ")
          + schema_path);
  }

  if (!(parser_ctxt = xmlSchemaNewDocParserCtxt(schema_doc))) {
    safeXmlFreeDoc(schema_doc);
    THROW_EXCEPTION (IOException, string("failed to create a schema context at ")
        + schema_path);
  }

  if (!(schema = xmlSchemaParse(parser_ctxt))) {
    safeXmlFreeDoc(schema_doc);
    xmlSchemaFreeParserCtxt(parser_ctxt);
    THROW_EXCEPTION (IOException, string("failed to parse schema at ")
        + schema_path);
  }

  if (!(valid_ctxt = xmlSchemaNewValidCtxt(schema))) {
    safeXmlFreeDoc(schema_doc);
    xmlSchemaFreeParserCtxt(parser_ctxt);
    xmlSchemaFree(schema);
    THROW_EXCEPTION (IOException, string("failed to create validation context from schema at ")
        + schema_path);
  }

  xmlSchemaSetValidErrors(valid_ctxt, &xmlSchemaValidityErrorFunc_impl,
      &xmlSchemaValidityWarningFunc_impl, NULL);

  int valid = xmlSchemaValidateDoc(valid_ctxt, doc);

  safeXmlFreeDoc(schema_doc);
  xmlSchemaFreeParserCtxt(parser_ctxt);
  xmlSchemaFree(schema);
  xmlSchemaFreeValidCtxt(valid_ctxt);
  if (valid == -1)
    THROW_EXCEPTION (IOException, schema_path, "internal error when validating against XSD schema at ");

  return (valid == 0);
}
