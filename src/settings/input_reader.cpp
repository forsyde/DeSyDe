#include "input_reader.hpp"

#include "../tools/systools.hpp"

using namespace std;
InputReader::InputReader(std::string _inputsPath):
  inputsPath(_inputsPath)
{
};
int InputReader::count_element_children( xmlNodePtr node )
{
  int n = 0;
  if ( !node || !node->children ) return n;
  xmlNodePtr child = node->children;
  for( xmlNodePtr curr = child ; curr ; curr = curr->next )
    if ( curr->type == XML_ELEMENT_NODE )
      n++;
  return n;
}

void InputReader::walk_tree( xmlNodePtr node )
{
  vector<char*> elements;
  vector<char*> values;
  if (!node) 
    return;
  for( xmlNodePtr curr = node ; curr ; curr = curr->next )
    {
      if ( curr->type == XML_ELEMENT_NODE )
        {
          char* currElement = (char *) curr->name;
          int nchild = count_element_children(curr);
          if ( curr->children && nchild == 0 )
            {
              char* currValue = (char *) curr->children->content ;
              elements.push_back(currElement);
              values.push_back(currValue);
            }                               
        }
      if ( curr->children )
        walk_tree( curr->children );
    }
  switch(xmlReadingMode)
    {
    case(TASK):
      parsTaskXMLNode(elements, values, node);
      break;
    case(PLATFORM):
      parsPlatformXMLNode(elements, values, node);
      break;
    case(WCET):
      parsWCETXMLNode(elements, values, node);
      break;
    case(CONSTRAINTS):
      parsConstXMLNode(elements, values, node);
      break;
    }
                
}
        
void InputReader::parsTaskXMLNode(vector<char*> elements, vector<char*> values, xmlNodePtr node)
{
  const xmlChar* taskNodeName = (xmlChar*) "periodicTask";
  /**
   * if the node is a pr task, then create one and push it in the tasks vactor
   */ 
  if ( node->parent && xmlStrEqual(node->parent->name, taskNodeName) == 1)
    {
      size_t taskNumber = atoi(getValueOfElement(elements, values, (char *)"number"));
      for (size_t i=0;i<taskNumber;i++)
        tasks.push_back(new PeriodicTask(elements, values, i));
      return;
    }
}
void InputReader::parsPlatformXMLNode(vector<char*> elements, vector<char*> values, xmlNodePtr node)
{
  const xmlChar* procNodeName = (xmlChar*) "processor";
  /**
   * if the node is a processor, then create one PE, push it in the PE vector and
   * find its corresponding modes from modes vector
   */ 
  if ( node->parent && xmlStrEqual(node->parent->name, procNodeName) == 1)
    {
      size_t procNumber = atoi(getValueOfElement(elements, values, (char *)"number"));
      char* model = getValueOfElement(node, (char *)"model");
      for (size_t i=0;i<procNumber;i++)
        {
          PE* proc =  new PE(elements, values, i);
          /// Find the corresponding modes
          for (size_t j=0; j<modes.size();j++)
            {
              if(strcmp(modes[j].model, model) == 0)
                proc->AddMode(modes[j].elements, modes[j].values);
            }
          processors.push_back(proc);
        }
      return;
    }
                        
  const xmlChar* modeNodeName = (xmlChar*) "mode";
  /**
   * if the node is a processor mode, then add the mode to 
   * the processor modes lis with the parent type
   */ 
  if ( node->parent && xmlStrEqual(node->parent->name, modeNodeName) == 1)
    {
      char* model = getValueOfElement(node->parent, (char *)"model");
      modes.push_back(ProcessorMode(model, elements, values));
    }
}
void InputReader::parsWCETXMLNode(vector<char*> elements, vector<char*> values, xmlNodePtr node)
{               
  const xmlChar* wcetNodeName = (xmlChar*) "mapping";
  /**
   * if the node is a wcet mapping, then assign corresponding values in the mapping
   */ 
  if ( node->parent && xmlStrEqual(node->parent->name, wcetNodeName) == 1)
    {
      mapping->setWCETs(elements, values);
      return;
    }
}
        
void InputReader::parsConstXMLNode(vector<char*> elements, vector<char*> values, xmlNodePtr node)
{               
  const xmlChar* constNodeName = (xmlChar*) "constraint";
  /**
   * if the node is a constraint, then assign it to the corresponding sdf app
   */ 
  if ( node->parent && xmlStrEqual(node->parent->name, constNodeName) == 1)
    {
      char* entityName = getValueOfElement(node, (char *)"entityName");
      for (size_t i=0; i<(*sdfApps).size(); i++)
        {
          if((*sdfApps)[i]->getName().compare(entityName) == 0)
            {
              (*sdfApps)[i]->setDesignConstraints(elements, values);
            }
        }
      return;
    }
}
        
char* InputReader::getValueOfElement(xmlNodePtr node, char* inElement)
{
  if (!node) 
    return '\0';
  /// go to the begining of the node
  while( node->prev )
    node = node->prev;
  for( xmlNodePtr curr = node ; curr ; curr = curr->next )
    {
      if ( curr->type == XML_ELEMENT_NODE )
        {
          char* currElement = (char *) curr->name;
          if ( curr->children && strcmp(currElement, inElement) == 0)
            {
              return (char *) curr->children->content;
            }                               
        }
    }
  return '\0';
}
        
        
char* InputReader::getValueOfElement(vector<char*> elements, vector<char*> values, char* inElement)
{
  for (size_t i=0; i<elements.size();i++)
    {
      if(strcmp(elements[i], inElement) == 0)
        {
          return values[i];
        }
    }
                
  return '\0';
}
        
vector<PeriodicTask*> InputReader::ReadTaskset(const string fileName)
{
                
  string filePath = inputsPath;
  filePath += fileName;
  xmlReadingMode = TASK;
  readXML(filePath);
                
  return tasks;
}
        
vector<PE*> InputReader::ReadPlatform(const string fileName)
{
                
  string filePath = inputsPath;
  filePath += fileName;
  xmlReadingMode = PLATFORM;
  readXML(filePath);
                
  return processors;
}
        
void InputReader::readXML(string filePath)
{
                
  char myArray[filePath.size()+1];///Converting to char array
  strcpy(myArray, filePath.c_str());
                
  xmlDocPtr doc;
  doc = xmlParseFile(myArray);
  if (doc == NULL) 
    {
      fprintf(stderr, "Failed to parse document: ");
      cout << filePath << endl;
      return;
    }
  xmlNodePtr root_node = xmlDocGetRootElement(doc);

  walk_tree(root_node);
                
  xmlFreeDoc(doc);
                
  return;
}
void InputReader::ReadWCETS(const string fileName, Mapping* _mapping)
{
  string filePath = inputsPath;
  filePath += fileName;
  mapping = _mapping;
  xmlReadingMode = WCET;
  readXML(filePath);
                
}
        
void InputReader::ReadConstraints(const string fileName, vector<SDFGraph*>* _sdfApps)
{
  string filePath = inputsPath;
  filePath += fileName;
  sdfApps = _sdfApps;
  xmlReadingMode = CONSTRAINTS;
  readXML(filePath);
  return;
}

std::ostream& operator<< (std::ostream &out, const InputReader &inputReader)
{
  out << "inputsPath:" << inputReader.inputsPath          << endl;
         
  return out;
}

vector<vector<string>> InputReader::ReadCSV(string filePath)
{
  char myArray[filePath.size()+1];///Converting to char array
  strcpy(myArray, filePath.c_str());
        
  ifstream file(myArray);
  string line;
  const char sep = ',';
  vector<vector<string>> table;
  while(getline(file,line))
    {
      vector<string> row;

      stringstream lineStream(line);
      string cell;

      while(getline(lineStream, cell, sep))
        {
          row.push_back(cell);              
        }
                
      table.push_back(row);
    }
  cout << "read " <<      filePath        <<      endl;
  return table;
}
