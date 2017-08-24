#include "platform.hpp"

using namespace std;

Platform::Platform(size_t p_nodes, int p_cycle, size_t p_memSize, int p_buffer, enum InterconnectType p_type, int p_dps, int p_tdma, int p_roundLength){
  for (size_t i=0; i<p_nodes; i++){
    compNodes.push_back(new PE("pe"+i,"gp",vector<double>(1,p_cycle),
                               vector<int>(1,p_memSize), vector<int>(1,10), vector<int>(1,10),
                               vector<int>(1,10),vector<int>(1,1),p_buffer));
  }

  interconnect = Interconnect(p_type, "", p_dps, p_tdma, p_roundLength, p_nodes, 1,0,0);
}

Platform::Platform(std::vector<PE*> p_nodes, enum InterconnectType p_type, int p_dps, int p_tdma, int p_roundLength){
  compNodes = p_nodes;
  interconnect = Interconnect(p_type, "", p_dps, p_tdma, p_roundLength, (int)p_nodes.size(), 1,0,0);
}

Platform::Platform(std::vector<PE*> p_nodes, enum InterconnectType p_type, int p_dps, int p_tdma, int p_roundLength, int p_col, int p_row){
  compNodes = p_nodes;
  if(p_type == NOC){
    interconnect = Interconnect(p_type, "NoC", p_dps, p_tdma, p_roundLength, p_col, p_row,0,0);
  }else{
    interconnect = Interconnect(p_type, "TDMA-bus", p_dps, p_tdma, p_roundLength, (int)p_nodes.size(), 1,0,0);
  }
}

Platform::Platform(XMLdoc& xml) throw (InvalidArgumentException)
{
    const char* my_xpathString = "///@name";
	LOG_DEBUG("running xpathString  " + tools::toString(my_xpathString) + " on platform xml ...");
	for (const auto& name : xml.xpathStrings(my_xpathString)){
		LOG_DEBUG("Platform class is parsing the following platform: " + name + "...");
	}
	load_xml(xml);
    ///Assigning default interconnect
    if(interconnect.type == UNASSIGNED){
      interconnect = Interconnect(TDMA_BUS, "TDMA-bus",32, (int) compNodes.size(), 1, (int)compNodes.size(), 1,0,0);    
      LOG_DEBUG("No interconnect found in platform XML. Assigning default: TDMA_BUS");
    }
}

void Platform::load_xml(XMLdoc& xml) throw (InvalidArgumentException)
{
	const char* my_xpathString = "///platform/processor";
	LOG_DEBUG("running xpathString  " + tools::toString(my_xpathString) + " platform xml ...");
	auto xml_procs = xml.xpathNodes(my_xpathString);
	int proc_id = 0;
	for (const auto& proc : xml_procs)
	{
		string proc_model = xml.getProp(proc, "model");
		string proc_number = xml.getProp(proc, "number");
		LOG_DEBUG("Reading processor model: " + proc_model + "...");		
		for(int i=0; i<atoi(proc_number.c_str()); i++)
		{
			PE* pe = new PE(proc_model, i);
            /// Parsing the modes
            string query = "///platform/processor[@model=\'" + proc_model + "\']/mode";
            auto   modes = xml.xpathNodes(query.c_str());
            for (auto mode : modes) {
              string mode_name = xml.getProp(mode, "name");
              string mode_cycle = xml.getProp(mode, "cycle");
              string mode_mem = xml.getProp(mode, "mem");
              string mode_dynPower = xml.getProp(mode, "dynPower");
              string mode_staticPower = xml.getProp(mode, "staticPower");
              string mode_area = xml.getProp(mode, "area");
              string mode_monetary = xml.getProp(mode, "monetary");
              pe->AddMode(atof(mode_cycle.c_str()), atoi(mode_mem.c_str()),
                        atoi(mode_dynPower.c_str()), atoi(mode_staticPower.c_str()), atoi(mode_area.c_str()),
                        atoi(mode_monetary.c_str()));
              LOG_DEBUG("Reading processor mode: " + mode_name + "...");		
            }
            
			proc_id++;
			compNodes.push_back(pe);
		}
	}
  my_xpathString = "///platform/interconnect";
	LOG_DEBUG("running xpathString  " + tools::toString(my_xpathString) + " platform xml ...");
	auto xml_interconnects = xml.xpathNodes(my_xpathString);
  LOG_DEBUG(tools::toString(xml_interconnects.size()) + " interconnect(s) found. \n");
  //for (const auto& ic : xml_interconnects){
  string query = "///platform/interconnect/TDN_NoC";
  auto tdn_noc = xml.xpathNodes(query.c_str());
  for(auto ic_settings : tdn_noc) {
    string noc_name = xml.getProp(ic_settings, "name");
    string noc_topology = xml.getProp(ic_settings, "topology");
    string noc_columns = xml.getProp(ic_settings, "x-dimension");
    string noc_rows = xml.getProp(ic_settings, "y-dimension");
    string noc_routing = xml.getProp(ic_settings, "routing");
    string noc_flitSize = xml.getProp(ic_settings, "flitSize");
    string noc_tdnCycles = xml.getProp(ic_settings, "cycles");
    //string noc_cycleLength = xml.getProp(ic_settings, "cycleLength");
  
    interconnect = Interconnect(TDN_NOC, noc_name, 0, 0, 0, atoi(noc_columns.c_str()), 
                                atoi(noc_rows.c_str()), atoi(noc_flitSize.c_str()), 
                                atoi(noc_tdnCycles.c_str()));
                                //atoi(noc_cycleLength.c_str()));  
                                
    query = "///platform/interconnect/TDN_NoC/mode";
    auto ic_modes = xml.xpathNodes(query.c_str());
    for(auto ic_mode : ic_modes) {
      string mode_name = xml.getProp(ic_mode, "name");
      string mode_cycleLength = xml.getProp(ic_mode, "cycleLength");
      string mode_dynPowerCons_link = xml.getProp(ic_mode, "dynPower_link");
      string mode_dynPowerCons_NI = xml.getProp(ic_mode, "dynPower_NI");
      string mode_dynPowerCons_switch = xml.getProp(ic_mode, "dynPower_switch");
      string mode_staticPow_link = xml.getProp(ic_mode, "staticPower_link");
      string mode_staticPow_NI = xml.getProp(ic_mode, "staticPower_NI");
      string mode_staticPow_switch = xml.getProp(ic_mode, "staticPower_switch");
      string mode_area_link = xml.getProp(ic_mode, "area_link");
      string mode_area_NI = xml.getProp(ic_mode, "area_NI");
      string mode_area_switch = xml.getProp(ic_mode, "area_switch");
      string mode_monetary_link = xml.getProp(ic_mode, "monetary_link");
      string mode_monetary_NI = xml.getProp(ic_mode, "monetary_NI");
      string mode_monetary_switch = xml.getProp(ic_mode, "monetary_switch");
      
      interconnect.addMode(mode_name, atoi(mode_cycleLength.c_str()),
                            atoi(mode_dynPowerCons_link.c_str()),
                            atoi(mode_dynPowerCons_NI.c_str()),
                            atoi(mode_dynPowerCons_switch.c_str()),
                            atoi(mode_staticPow_link.c_str()),
                            atoi(mode_staticPow_NI.c_str()),
                            atoi(mode_staticPow_switch.c_str()),
                            atoi(mode_area_link.c_str()),
                            atoi(mode_area_NI.c_str()),
                            atoi(mode_area_switch.c_str()),
                            atoi(mode_monetary_link.c_str()),
                            atoi(mode_monetary_NI.c_str()),
                            atoi(mode_monetary_switch.c_str()));
    }
                                
    LOG_DEBUG("Found a " + noc_name + " with " + noc_topology + " topology, "
                         + noc_routing + " routing, "	
                         + noc_columns + " columns, "	
                         + noc_rows + " rows, "	
                         + noc_flitSize + " bit flit size, "	
                         + noc_tdnCycles + " tdnCycles, and "	
                         + tools::toString(interconnect.modes.size()) + " modes.");	
                                
    int procsInNoC = atoi(noc_rows.c_str()) * atoi(noc_columns.c_str());
    if(procsInNoC != proc_id){
       THROW_EXCEPTION(InvalidArgumentException, "size of TDN-Noc does not match with number of processors");
    }                            


    createTDNGraph();
    createRouteTable();
    getchar();
  }
    
  //}
  
  	
}
Platform::~Platform(){
  //compNodes (they were potentially created with 'new'')
  for (size_t i=0; i<compNodes.size(); i++){
    delete compNodes[i];
  }
}

void Platform::createTDNGraph() throw (InvalidArgumentException){
  LOG_DEBUG("Creating the TDN graph / table: ");
  int nodes = interconnect.rows * interconnect.columns;
  
  int tdn_nodeId = 0;
  int corner = 0;
  int edge = 0;
  int middle = 0;
  int totalTDN_nodes;
  
  for(int i=0; i<nodes; i++){
    int yLoc_i = i/interconnect.columns;
    int xLoc_i = i%interconnect.columns;
    
    //cout << "I am processor/NoC-node " << i << ", located at (" << xLoc_i << ", " << yLoc_i << "):";
    if(xLoc_i % (interconnect.columns-1) == 0 && yLoc_i % (interconnect.rows-1) == 0){
      //cout << " I am a CORNER node" << endl;
      corner++;
    }else if(xLoc_i % (interconnect.columns-1) == 0 || yLoc_i % (interconnect.rows-1) == 0){
      //cout << " I am an EDGE node" << endl;
      edge++;
    }else{
      //cout << " I am a MIDDLE node" << endl;
      middle++;
    }
  }
    
  totalTDN_nodes = (corner*4 + edge*5 + middle*6) * interconnect.tdnCycles;
  
  //vector<tdn_graphNode> tdn_graph(totalTDN_nodes);
  tdn_graph.assign(totalTDN_nodes, tdn_graphNode());
  LOG_DEBUG("Initialized TDN graph with " +  tools::toString(tdn_graph.size()) + " nodes. Now filling with info...");    
  
  LOG_DEBUG("#### OUT FROM NI ###########");  
  for(int i=0; i<nodes; i++){
    //int yLoc_i = i/interconnect.columns;
    //int xLoc_i = i%interconnect.columns;
    
    //root nodes of the TDN graph: all links from NIs to switch, for each TDN cycle
    for(int k=0; k<interconnect.tdnCycles; k++){
      //cout << "I am node " << tdn_nodeId << " of the TDN graph." << endl;
      //cout << "\t I represent the link from NI to switch at NoC-node " << i << " (" << xLoc_i << ", " << yLoc_i << ")";
      //cout << " at TDN cycle " << k << endl;
      //cout << "==========" << endl;
      
      //add node to graph
      tdn_graph[tdn_nodeId].passingProcs.emplace(i);
      tdn_graph[tdn_nodeId].link = {-1, i, k};
      
      tdn_nodeId++;
    }
    //cout << "###############" << endl;
  }
  
  //"middle" nodes of the TDN graph: all links from switch to switch in all cycles
  //first upwards...
  LOG_DEBUG("#### UP ###########");
  for(int col=0; col<interconnect.columns; col++){
    for(int row=0; row<interconnect.rows-1; row++){
      int from = row*interconnect.columns+col;
      int to = (row+1)*interconnect.columns+col;
      
      for(int k=0; k<interconnect.tdnCycles; k++){
        //cout << "I am node " << tdn_nodeId << " of the TDN graph." << endl;
        //cout << "\t I represent the link from switch " << from << " (" << col << ", " << row << ")" <<  " to switch " << to << " (" << col << ", " << (row+1) << ")";
        //cout << " at TDN cycle " << k << endl;
        //cout << "==========" << endl;
        
        //add node to graph
        tdn_graph[tdn_nodeId].link = {from, to, k};
        
        tdn_nodeId++;
      }
      //cout << "###############" << endl;
      
    }
  }
  //then downwards...
  LOG_DEBUG("#### DOWN ###########");
  for(int col=0; col<interconnect.columns; col++){
    for(int row=interconnect.rows-1; row>0; row--){
      int from = row*interconnect.columns+col;
      int to = (row-1)*interconnect.columns+col;
      
      for(int k=0; k<interconnect.tdnCycles; k++){
        //cout << "I am node " << tdn_nodeId << " of the TDN graph." << endl;
        //cout << "\t I represent the link from switch " << from << " (" << col << ", " << row << ")" <<  " to switch " << to << " (" << col << ", " << (row-1) << ")";
        //cout << " at TDN cycle " << k << endl;
        //cout << "==========" << endl;
        
        //add node to graph
        tdn_graph[tdn_nodeId].link = {from, to, k};
        
        tdn_nodeId++;
      }
      //cout << "###############" << endl;
      
    }
  }
  //...then right
  LOG_DEBUG("#### RIGHT ###########");
  for(int row=0; row<interconnect.rows; row++){
    for(int col=0; col<interconnect.columns-1; col++){
      int from = row*interconnect.columns+col;
      int to = row*interconnect.columns+col+1;
      
      for(int k=0; k<interconnect.tdnCycles; k++){
        //cout << "I am node " << tdn_nodeId << " of the TDN graph." << endl;
        //cout << "\t I represent the link from switch " << from << " (" << col << ", " << row << ")" <<  " to switch " << to << " (" << (col+1) << ", " << row << ")";
        //cout << " at TDN cycle " << k << endl;
        //cout << "==========" << endl;
        
        //add node to graph
        tdn_graph[tdn_nodeId].link = {from, to, k};
        
        tdn_nodeId++;
      }
      //cout << "###############" << endl;
      
    }
  }
  //...and then left
  LOG_DEBUG("#### LEFT ###########");
  for(int row=0; row<interconnect.rows; row++){
    for(int col=interconnect.columns-1; col>0; col--){
      int from = row*interconnect.columns+col;
      int to = row*interconnect.columns+col-1;
      
      for(int k=0; k<interconnect.tdnCycles; k++){
        //cout << "I am node " << tdn_nodeId << " of the TDN graph." << endl;
        //cout << "\t I represent the link from switch " << from << " (" << col << ", " << row << ")" <<  " to switch " << to << " (" << (col-1) << ", " << row << ")";
        //cout << " at TDN cycle " << k << endl;
        //cout << "==========" << endl;
        
        //add node to graph
        tdn_graph[tdn_nodeId].link = {from, to, k};
        
        tdn_nodeId++;
      }
      //cout << "###############" << endl;
    }
  }
  //...and finally back into the NI
  
  LOG_DEBUG("#### TO NI ###########");
  for(int i=0; i<nodes; i++){
    //int yLoc_i = i/interconnect.columns;
    //int xLoc_i = i%interconnect.columns;
    
    //leaf nodes of the TDN graph: all links from switch to NI, for each TDN cycle
    for(int k=0; k<interconnect.tdnCycles; k++){
      //cout << "I am node " << tdn_nodeId << " of the TDN graph." << endl;
      //cout << "\t I represent the link from switch to NI at NoC-node " << i << " (" << xLoc_i << ", " << yLoc_i << ")";
      //cout << " at TDN cycle " << k << endl;
      //cout << "==========" << endl;
        
        //add node to graph
        tdn_graph[tdn_nodeId].link = {i, -1, k};
        
      tdn_nodeId++;
    }
      //cout << "###############" << endl;
  }
  if(tdn_nodeId != totalTDN_nodes){
    THROW_EXCEPTION(InvalidArgumentException, (tools::toString(tdn_nodeId) + " nodes are in the graph. Should be " + tools::toString(totalTDN_nodes)));
  }
    
  tdn_nodeId = 0;
  LOG_DEBUG("\n Now, let's travel through the entire NoC...");
  for(int i=0; i<nodes; i++){
    int yLoc_i = i/interconnect.columns;
    int xLoc_i = i%interconnect.columns;
    
    
    for(int k=0; k<interconnect.tdnCycles; k++){
      
      tdn_nodeId = i * interconnect.tdnCycles + k;
      int tdn_srcNodeId = tdn_nodeId;
      
      
      
      //cout << "\n\t starting from NI to switch at NoC-node " << i << " at (" << xLoc_i << ", " << yLoc_i << ")";
      //cout << " in cycle " << k << " (TDN-Node: "<< tdn_nodeId << ")" << endl;
    
      for(int j=0; j<nodes; j++){
        int yLoc_j = j/interconnect.columns;
        int xLoc_j = j%interconnect.columns;
        
        int tmp_cycle = k;
        tmp_cycle = (tmp_cycle+1)%interconnect.tdnCycles;
        
        shared_ptr<tdn_route> tmp_tdn_route = make_shared<tdn_route>();
        
        int y_inc = (yLoc_i == yLoc_j) ? 0 : ((yLoc_i < yLoc_j) ? 1 : -1);
        int x_inc = (xLoc_i == xLoc_j) ? 0 : ((xLoc_i < xLoc_j) ? 1 : -1);
        
        if(x_inc != 0 || y_inc != 0){
          tmp_tdn_route->srcProc = i;
          tmp_tdn_route->dstProc = j; //{j, vector<int>()};
          tmp_tdn_route->tdn_nodePath.push_back(tdn_srcNodeId);
        }
      
        int tmp_yLoc = yLoc_i;
        int tmp_xLoc = xLoc_i;
        //cout << "\t\t via switch (" << tmp_xLoc << ", " << tmp_yLoc << ")";
        //cout << " at cycle " << tmp_cycle << endl;
        while(tmp_yLoc != yLoc_j){
          //cout << "\t    ...travelling " << ((y_inc == -1) ? "DOWN  " : "UP    ");
          //cout << " via TDN-node ";
          if(y_inc == 1){
            tdn_nodeId = nodes*interconnect.tdnCycles + (tmp_xLoc*interconnect.tdnCycles*(interconnect.rows-1)) + (tmp_yLoc*interconnect.tdnCycles) + tmp_cycle;
          }else if(y_inc == -1){
            tdn_nodeId = (2*nodes-interconnect.columns)*interconnect.tdnCycles + (tmp_xLoc*interconnect.tdnCycles*(interconnect.rows-1)) + (((interconnect.rows-1)-tmp_yLoc)*interconnect.tdnCycles) + tmp_cycle;
          }
          
          tdn_graph[tdn_nodeId].passingProcs.emplace(i);
          tmp_tdn_route->tdn_nodePath.push_back(tdn_nodeId);
          tdn_graph[tdn_nodeId].tdn_routes.push_back(tmp_tdn_route);
          
          //cout << tdn_nodeId << ": from (" <<  tmp_xLoc << ", " << tmp_yLoc << ") to (";
          tmp_yLoc += y_inc;
          //cout <<  tmp_xLoc << ", " << tmp_yLoc << "); cycle " << tmp_cycle << endl;
          tmp_cycle = (tmp_cycle+1)%interconnect.tdnCycles;
        }
        
        while(tmp_xLoc != xLoc_j){
          //cout << "\t    ...travelling " << ((x_inc == -1) ? "LEFT  " : "RIGHT ");
          //cout << " via TDN-node ";
          if(x_inc == 1){
            tdn_nodeId = (3*nodes-2*interconnect.columns)*interconnect.tdnCycles + (tmp_yLoc*interconnect.tdnCycles*(interconnect.columns-1)) + (tmp_xLoc*interconnect.tdnCycles) + tmp_cycle;
          }else if(x_inc == -1){
            tdn_nodeId = (4*nodes-2*interconnect.columns-interconnect.rows)*interconnect.tdnCycles + (tmp_yLoc*interconnect.tdnCycles*(interconnect.columns-1)) + (((interconnect.columns-1)-tmp_xLoc)*interconnect.tdnCycles) + tmp_cycle;
          }
          
          tdn_graph[tdn_nodeId].passingProcs.emplace(i);
          tmp_tdn_route->tdn_nodePath.push_back(tdn_nodeId);
          tdn_graph[tdn_nodeId].tdn_routes.push_back(tmp_tdn_route);
          
          //cout << tdn_nodeId << ": from (" <<  tmp_xLoc << ", " << tmp_yLoc << ") to (";
          tmp_xLoc += x_inc;
          //cout <<  tmp_xLoc << ", " << tmp_yLoc << "); cycle " << tmp_cycle << endl;
          tmp_cycle = (tmp_cycle+1)%interconnect.tdnCycles;
          //cout << "(" << tmp_xLoc << ", " << tmp_yLoc << ");";
          //cout << " cycle " << tmp_cycle << endl;
        }
        
        //from switch to NI at destination NoC-node
        if(x_inc != 0 || y_inc !=0){
          tdn_nodeId = (5*nodes-2*interconnect.columns-2*interconnect.rows)*interconnect.tdnCycles + j * interconnect.tdnCycles + tmp_cycle;
          //cout << "\t arriving at NI from switch at NoC-node " << j << " at (" << tmp_xLoc << ", " << tmp_yLoc << ")";
          //cout << " in cycle " << tmp_cycle << " (TDN-Node: "<< tdn_nodeId << ")" << endl;
          
          tdn_graph[tdn_nodeId].passingProcs.emplace(i);
          tmp_tdn_route->tdn_nodePath.push_back(tdn_nodeId);
          tdn_graph[tdn_nodeId].tdn_routes.push_back(tmp_tdn_route);
          tdn_graph[tdn_srcNodeId].tdn_routes.push_back(tmp_tdn_route);
          
          LOG_DEBUG(tools::toString(i) + " to " + tools::toString(j) + " in cycle " + tools::toString(k) + ":  "
                     + tools::toString(tmp_tdn_route->tdn_nodePath));

        }
      }
    }
  }
  
  for (size_t i=0; i<tdn_graph.size(); i++)
    LOG_DEBUG("NoC-nodes " + tools::toString(vector<int>(tdn_graph[i].passingProcs.begin(), tdn_graph[i].passingProcs.end())) + " can pass through link (" + tools::toString(tdn_graph[i].link.from)
               + ", " + tools::toString(tdn_graph[i].link.to) + ")");
               
  LOG_DEBUG("### Done with TDN table. ###");

}

void Platform::createRouteTable(){
  int nodes = interconnect.rows * interconnect.columns;
  
  for(int i=0; i<nodes; i++){
    int yLoc_i = i/interconnect.columns;
    int xLoc_i = i%interconnect.columns;
    
    int srcLinkId = i;
    int linkId;
        
    for(int j=0; j<nodes; j++){
      int yLoc_j = j/interconnect.columns;
      int xLoc_j = j%interconnect.columns;
      
      tdn_route tmp_route;
      
      int y_inc = (yLoc_i == yLoc_j) ? 0 : ((yLoc_i < yLoc_j) ? 1 : -1);
      int x_inc = (xLoc_i == xLoc_j) ? 0 : ((xLoc_i < xLoc_j) ? 1 : -1);
      
      if(x_inc != 0 || y_inc != 0){
        tmp_route.srcProc = i;
        tmp_route.dstProc = j; //{j, vector<int>()};
        tmp_route.tdn_nodePath.push_back(srcLinkId);
        
        
        //cout << "\t starting from NI to switch at NoC-node " << i << " at (" << xLoc_i << ", " << yLoc_i << ")";
        //cout << " (linkId: "<< srcLinkId << ")" << endl;
        cout << srcLinkId << ": from NI to (" <<  xLoc_i << ", " << yLoc_i << ")" << endl;
      }
    
      int tmp_yLoc = yLoc_i;
      int tmp_xLoc = xLoc_i;
      //cout << "\t\t via switch (" << tmp_xLoc << ", " << tmp_yLoc << ")" << endl;
      while(tmp_yLoc != yLoc_j){
        //cout << "\t    ...travelling " << ((y_inc == -1) ? "DOWN  " : "UP    ");
        //cout << " via TDN-node ";
        if(y_inc == 1){
          linkId = nodes + (tmp_xLoc*(interconnect.rows-1)) + (tmp_yLoc);
        }else if(y_inc == -1){
          linkId = (2*nodes-interconnect.columns) + (tmp_xLoc*(interconnect.rows-1)) + (((interconnect.rows-1)-tmp_yLoc));
        }
        
        tmp_route.tdn_nodePath.push_back(linkId);
        
        cout << linkId << ": from (" <<  tmp_xLoc << ", " << tmp_yLoc << ") to (";
        tmp_yLoc += y_inc;
        cout <<  tmp_xLoc << ", " << tmp_yLoc << ")" << endl;
      }
      
      while(tmp_xLoc != xLoc_j){
        //cout << "\t    ...travelling " << ((x_inc == -1) ? "LEFT  " : "RIGHT ");
        //cout << " via TDN-node ";
        if(x_inc == 1){
          linkId = (3*nodes-2*interconnect.columns) + (tmp_yLoc*(interconnect.columns-1)) + (tmp_xLoc);
        }else if(x_inc == -1){
          linkId = (4*nodes-2*interconnect.columns-interconnect.rows) + (tmp_yLoc*(interconnect.columns-1)) + (((interconnect.columns-1)-tmp_xLoc));
        }
        
        tmp_route.tdn_nodePath.push_back(linkId);
        
        cout << linkId << ": from (" <<  tmp_xLoc << ", " << tmp_yLoc << ") to (";
        tmp_xLoc += x_inc;
        cout <<  tmp_xLoc << ", " << tmp_yLoc << ")" << endl;
      }
      
      //from switch to NI at destination NoC-node
      if(x_inc != 0 || y_inc !=0){
        linkId = (5*nodes-2*interconnect.columns-2*interconnect.rows) + j ;
        //cout << "\t arriving at NI from switch at NoC-node " << j << " at (" << tmp_xLoc << ", " << tmp_yLoc << ")";
        //cout << " (linkId: "<< linkId << ")" << endl;
        cout << linkId << ": from (" <<  tmp_xLoc << ", " << tmp_yLoc << ") to NI" << endl << endl;
        
        tmp_route.tdn_nodePath.push_back(linkId);
        interconnect.all_routes.push_back(tmp_route);
        
      }
    }
  }
  
  for(int i=0; i<interconnect.all_routes.size(); i++){
    cout << i << " (from " << interconnect.all_routes[i].srcProc << " to " << interconnect.all_routes[i].dstProc;
    cout << ") " << tools::toString(interconnect.all_routes[i].tdn_nodePath) << endl;
  }
  
}

size_t Platform::nodes() const {
  return compNodes.size();
}

InterconnectType Platform::getInterconnectType() const{
  return interconnect.type;
}

// Gives the TDN Graph / Table
vector<tdn_graphNode> Platform::getTDNGraph() const{
  return tdn_graph;  
}

int Platform::getTDNCycles() const{
  return interconnect.tdnCycles;
}

size_t Platform::getInterconnectModes() const{
  return interconnect.modes.size();
}

//TODO fix for modes
int Platform::getTDNCycleLength() const{
  return interconnect.modes[0].cycleLength;
}

/*! Gets the cycle length, depending on the NoC mode. */
vector<int> Platform::getTDNCycleLengths() const{
  vector<int> tmp;
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].cycleLength);
  }
  
  return tmp;
}

/*! Gets the dynamic power consumption of a link */
vector<int> Platform::getDynPowerCons_link() const{
  vector<int> tmp; 
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].dynPower_link);
  }
  
  return tmp;  
}
/*! Gets the dynamic power consumption of a link */
vector<int> Platform::getDynPowerCons_NI() const{
  vector<int> tmp; 
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].dynPower_NI);
  }
  
  return tmp;  
}
/*! Gets the dynamic power consumption of a link */
vector<int> Platform::getDynPowerCons_switch() const{
  vector<int> tmp; 
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].dynPower_switch);
  }
  
  return tmp;  
}

/*! Gets the static power consumption of the entire NoC for each mode. */
vector<int> Platform::getStaticPowerCons() const{
  vector<int> tmp; 
  int nodes = interconnect.rows * interconnect.columns;
  size_t corner, edge, middle = 0;
  
  for(size_t i=0; i<nodes; i++){
    int yLoc_i = i/interconnect.columns;
    int xLoc_i = i%interconnect.columns;
    
    //cout << "I am processor/NoC-node " << i << ", located at (" << xLoc_i << ", " << yLoc_i << "):";
    if(xLoc_i % (interconnect.columns-1) == 0 && yLoc_i % (interconnect.rows-1) == 0){
      //cout << " I am a CORNER node" << endl;
      corner++;
    }else if(xLoc_i % (interconnect.columns-1) == 0 || yLoc_i % (interconnect.rows-1) == 0){
      //cout << " I am an EDGE node" << endl;
      edge++;
    }else{
      //cout << " I am a MIDDLE node" << endl;
      middle++;
    }
    
  }
    
  for(size_t i=0; i<interconnect.modes.size(); i++){
    int staticPower_total = nodes*interconnect.modes[i].staticPow_NI +
                            nodes*interconnect.modes[i].staticPow_switch +
                            (((corner*4 + edge*5 + middle*6))*interconnect.modes[i].staticPow_link);
    tmp.push_back(staticPower_total);
  }
  
  return tmp;
}


  
/*! Gets the dynamic power consumption of the link at node node for each mode. */
vector<int> Platform::getStaticPowerCons_link(size_t node) const{
  vector<int> tmp; 
  
  int yLoc_node = node/interconnect.columns;
  int xLoc_node = node%interconnect.columns;
  
  //cout << "I am processor/NoC-node " << i << ", located at (" << xLoc_node << ", " << yLoc_node << "):";
  if(xLoc_node % (interconnect.columns-1) == 0 && yLoc_node % (interconnect.rows-1) == 0){
    //cout << " I am a CORNER node" << endl;
    for(size_t i=0; i<interconnect.modes.size(); i++){
      tmp.push_back(4*interconnect.modes[i].staticPow_link);
    }
  }else if(xLoc_node % (interconnect.columns-1) == 0 || yLoc_node % (interconnect.rows-1) == 0){
    //cout << " I am an EDGE node" << endl;
    for(size_t i=0; i<interconnect.modes.size(); i++){
      tmp.push_back(5*interconnect.modes[i].staticPow_link);
    }
  }else{
    //cout << " I am a MIDDLE node" << endl;
    for(size_t i=0; i<interconnect.modes.size(); i++){
      tmp.push_back(6*interconnect.modes[i].staticPow_link);
    }
  }
  
  return tmp;
}

/*! Gets the dynamic power consumption of the NI at node node for each mode. */
vector<int> Platform::getStaticPowerCons_NI() const{
  vector<int> tmp; 
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].staticPow_NI);
  }
  
  return tmp;  
}

/*! Gets the dynamic power consumption of the switch at node node for each mode. */
vector<int> Platform::getStaticPowerCons_switch() const{
  vector<int> tmp; 
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].staticPow_switch);
  }
  
  return tmp;  
}
  

/*! Gets the area cost of the NoC, depending on the mode. */
vector<int> Platform::interconnectAreaCost() const{
  vector<int> tmp; 
  int nodes = interconnect.rows * interconnect.columns;
  size_t corner, edge, middle = 0;
  
  for(size_t i=0; i<nodes; i++){
    int yLoc_i = i/interconnect.columns;
    int xLoc_i = i%interconnect.columns;
    
    //cout << "I am processor/NoC-node " << i << ", located at (" << xLoc_i << ", " << yLoc_i << "):";
    if(xLoc_i % (interconnect.columns-1) == 0 && yLoc_i % (interconnect.rows-1) == 0){
      //cout << " I am a CORNER node" << endl;
      corner++;
    }else if(xLoc_i % (interconnect.columns-1) == 0 || yLoc_i % (interconnect.rows-1) == 0){
      //cout << " I am an EDGE node" << endl;
      edge++;
    }else{
      //cout << " I am a MIDDLE node" << endl;
      middle++;
    }
    
  }
    
  for(size_t i=0; i<interconnect.modes.size(); i++){
    int area_total = nodes*interconnect.modes[i].area_NI +
                            nodes*interconnect.modes[i].area_switch +
                            (((corner*4 + edge*5 + middle*6))*interconnect.modes[i].area_link);
    tmp.push_back(area_total);
  }
  
  return tmp;
}
  
/*! Gets the area cost of the links at node node for each mode. */
vector<int> Platform::interconnectAreaCost_link(size_t node) const{
  vector<int> tmp; 
  
  int yLoc_node = node/interconnect.columns;
  int xLoc_node = node%interconnect.columns;
  
  //cout << "I am processor/NoC-node " << i << ", located at (" << xLoc_node << ", " << yLoc_node << "):";
  if(xLoc_node % (interconnect.columns-1) == 0 && yLoc_node % (interconnect.rows-1) == 0){
    //cout << " I am a CORNER node" << endl;
    for(size_t i=0; i<interconnect.modes.size(); i++){
      tmp.push_back(4*interconnect.modes[i].area_link);
    }
  }else if(xLoc_node % (interconnect.columns-1) == 0 || yLoc_node % (interconnect.rows-1) == 0){
    //cout << " I am an EDGE node" << endl;
    for(size_t i=0; i<interconnect.modes.size(); i++){
      tmp.push_back(5*interconnect.modes[i].area_link);
    }
  }else{
    //cout << " I am a MIDDLE node" << endl;
    for(size_t i=0; i<interconnect.modes.size(); i++){
      tmp.push_back(6*interconnect.modes[i].area_link);
    }
  }
  
  return tmp;
}

/*! Gets the area cost of an NI for each mode. */
vector<int> Platform::interconnectAreaCost_NI() const{
  vector<int> tmp; 
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].area_NI);
  }
  
  return tmp;  
}

/*! Gets the area cost of a switch for each mode. */
vector<int> Platform::interconnectAreaCost_switch() const{
  vector<int> tmp; 
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].area_switch);
  }
  
  return tmp;  
}
/*! Gets the monetary cost of the NoC, depending on the mode. */
vector<int> Platform::interconnectMonetaryCost() const{
  vector<int> tmp; 
  int nodes = interconnect.rows * interconnect.columns;
  size_t corner, edge, middle = 0;
  
  for(size_t i=0; i<nodes; i++){
    int yLoc_i = i/interconnect.columns;
    int xLoc_i = i%interconnect.columns;
    
    //cout << "I am processor/NoC-node " << i << ", located at (" << xLoc_i << ", " << yLoc_i << "):";
    if(xLoc_i % (interconnect.columns-1) == 0 && yLoc_i % (interconnect.rows-1) == 0){
      //cout << " I am a CORNER node" << endl;
      corner++;
    }else if(xLoc_i % (interconnect.columns-1) == 0 || yLoc_i % (interconnect.rows-1) == 0){
      //cout << " I am an EDGE node" << endl;
      edge++;
    }else{
      //cout << " I am a MIDDLE node" << endl;
      middle++;
    }
    
  }
    
  for(size_t i=0; i<interconnect.modes.size(); i++){
    int monetary_total = nodes*interconnect.modes[i].monetary_NI +
                            nodes*interconnect.modes[i].monetary_switch +
                            (((corner*4 + edge*5 + middle*6))*interconnect.modes[i].monetary_link);
    tmp.push_back(monetary_total);
  }
  
  return tmp;
}
  
/*! Gets the monetary cost of the links at node node for each mode. */
vector<int> Platform::interconnectMonetaryCost_link(size_t node) const{
  vector<int> tmp; 
  
  int yLoc_node = node/interconnect.columns;
  int xLoc_node = node%interconnect.columns;
  
  //cout << "I am processor/NoC-node " << i << ", located at (" << xLoc_node << ", " << yLoc_node << "):";
  if(xLoc_node % (interconnect.columns-1) == 0 && yLoc_node % (interconnect.rows-1) == 0){
    //cout << " I am a CORNER node" << endl;
    for(size_t i=0; i<interconnect.modes.size(); i++){
      tmp.push_back(4*interconnect.modes[i].monetary_link);
    }
  }else if(xLoc_node % (interconnect.columns-1) == 0 || yLoc_node % (interconnect.rows-1) == 0){
    //cout << " I am an EDGE node" << endl;
    for(size_t i=0; i<interconnect.modes.size(); i++){
      tmp.push_back(5*interconnect.modes[i].monetary_link);
    }
  }else{
    //cout << " I am a MIDDLE node" << endl;
    for(size_t i=0; i<interconnect.modes.size(); i++){
      tmp.push_back(6*interconnect.modes[i].monetary_link);
    }
  }
  
  return tmp;
}

/*! Gets the monetary cost of a NI for each mode. */
vector<int> Platform::interconnectMonetaryCost_NI() const{
  vector<int> tmp; 
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].monetary_NI);
  }
  
  return tmp;  
}

/*! Gets the monetary cost of a switch for each mode. */
vector<int> Platform::interconnectMonetaryCost_switch() const{
  vector<int> tmp; 
  for(int i=0; i<interconnect.modes.size(); i++){
    tmp.push_back(interconnect.modes[i].monetary_switch);
  }
  
  return tmp;  
}

int Platform::getMaxNoCHops() const{
 return interconnect.columns + interconnect.rows;  
}

int Platform::getFlitSize() const{
  return interconnect.flitSize;
}

int Platform::tdmaSlots() const{
  return interconnect.tdmaSlots;
}

int Platform::dataPerSlot() const{
  return interconnect.dataPerSlot;
}

int Platform::dataPerRound() const{
  return interconnect.dataPerRound;
}

double Platform::speedUp(int node, int mode) const {
  return compNodes[node]->cycle_length[mode];
}

//maximum communication time (=blocking+sending) on the TDM bus for a token of size tokSize
//for different TDM slot allocations (index of vector = number of slots
//for use with the element constraint in the model
const vector<int> Platform::maxCommTimes(int tokSize) const{
  double slotLength = (double)interconnect.roundLength/interconnect.tdmaSlots;
  double slotsNeeded = ((double)tokSize/interconnect.dataPerSlot);
  double activeSendingTime = slotsNeeded * slotLength;

  std::vector<int> sendingTimes;
  sendingTimes.push_back(-1); //for tdma_alloc=0, sendingTime = -1 (used in CP model)
  for (auto i=1; i<=interconnect.tdmaSlots; i++){ //max sendingTime depends on allocated TDM slots
    double initBlock = interconnect.roundLength - (i-1)*slotLength;
    int roundsNeeded = ceil(slotsNeeded/i);
    double slotDelay = (roundsNeeded-1) * (interconnect.roundLength - (i*slotLength));
    //cout << "tdmAlloc = " << i << " , commTime = " << initBlock+activeSendingTime+slotDelay << endl;

    sendingTimes.push_back(ceil(initBlock+activeSendingTime+slotDelay));
  }
  return sendingTimes;
}

//maximum blocking time on the TDM bus for a token
//for different TDM slot allocations (index of vector = number of slots
//for use with the element constraint in the model
const vector<int> Platform::maxBlockingTimes() const{
  double slotLength = (double)interconnect.roundLength/interconnect.tdmaSlots;

  //cout << "TMDA slot length = " << slotLength << endl;

  std::vector<int> blockingTimes;
  blockingTimes.push_back(0); //for tdma_alloc=0, blockingTime = 0 (used in CP model)
  for (auto i=1; i<=interconnect.tdmaSlots; i++){ //max blocking depends on allocated TDM slots
    double initBlock = interconnect.roundLength - (i-1)*slotLength;
    blockingTimes.push_back(ceil(initBlock));
  }
  return blockingTimes;
}

//maximum transfer time on the TDM bus for a token of size tokSize
//for different TDM slot allocations (index of vector = number of slots
//for use with the element constraint in the model
const vector<int> Platform::maxTransferTimes(int tokSize) const{
  double slotLength = (double)interconnect.roundLength/interconnect.tdmaSlots;
  double slotsNeeded = ((double)tokSize/interconnect.dataPerSlot);
  double activeSendingTime = slotsNeeded * slotLength;

//  cout << "token size = " << tokSize << endl;
//  cout << "TMDA slot length = " << slotLength << endl;
//  cout << "TMDA slots needed = " << slotsNeeded << endl;
//  cout << "active transfer time = " << activeSendingTime << endl;

  std::vector<int> transferTimes;
  transferTimes.push_back(0); //for tdma_alloc=0, transerTime = -1 (used in CP model)
  for (auto i=1; i<=interconnect.tdmaSlots; i++){ //max sendingTime depends on allocated TDM slots
    int roundsNeeded = ceil(slotsNeeded/i);
    double slotDelay = (roundsNeeded-1) * (interconnect.roundLength - (i*slotLength));

    transferTimes.push_back(ceil(activeSendingTime+slotDelay));
  }
  return transferTimes;
}


size_t Platform::memorySize(int node, int mode) const{
  return compNodes[node]->memorySize[mode];
}

size_t Platform::getModes(int node) const{
  return compNodes[node]->n_types;
}

size_t Platform::getMaxModes() const
{
  size_t max_mode = 0;
  for (size_t k = 0; k < nodes(); k++)
    {
      if(getModes(k) > max_mode)
        max_mode = getModes(k);
    }
  return max_mode;
}

vector<int> Platform::getMemorySize(int node) const{
  return compNodes[node]->memorySize;
}

vector<int> Platform::getPowerCons(int node) const{
  return compNodes[node]->dynPowerCons;
}

vector<int> Platform::getAreaCost(int node) const{
  return compNodes[node]->areaCost;
}

vector<int> Platform::getMonetaryCost(int node) const{
  return compNodes[node]->monetaryCost;
}

int Platform::bufferSize(int node) const{
  return compNodes[node]->NI_bufferSize;
}

bool Platform::isFixed() const{
  for (size_t j = 0; j < nodes(); j++){
    if(compNodes[j]->n_types > 1) return false;
  }

  return true;
}

// True if the nodes are homogeneous
bool Platform::homogeneousNodes(int node0, int node1) const{
  if(compNodes[node0]->n_types > 1 || compNodes[node1]->n_types > 1 ||
     compNodes[node0]->type != compNodes[node1]->type ||
     compNodes[node0]->cycle_length != compNodes[node1]->cycle_length ||
     compNodes[node0]->memorySize[0] != compNodes[node1]->memorySize[0]) {
    return false;
  }
  return true;
}


bool Platform::homogeneousModeNodes(int node0, int node1) const{
  if(compNodes[node0]->n_types != compNodes[node1]->n_types){
      return false;
  }

  for (auto m=0; m<compNodes[node0]->n_types; m++){
    if(compNodes[node0]->cycle_length[m] != compNodes[node1]->cycle_length[m] ||
       compNodes[node0]->memorySize[m] != compNodes[node1]->memorySize[m] ||
       compNodes[node0]->dynPowerCons[m] != compNodes[node1]->dynPowerCons[m] ||
       compNodes[node0]->areaCost[m] != compNodes[node1]->areaCost[m] ||
       compNodes[node0]->monetaryCost[m] != compNodes[node1]->monetaryCost[m]){
        return false;
    }
  }
  return true;
}

// True if the platform is homogeneous
bool Platform::homogeneous() const{
  for (size_t j = 1; j < nodes(); j++) {
    if(!homogeneousNodes(j-1, j)) return false;
  }
  return true;
}

// True if all processors only have one mode
bool Platform::allProcsFixed() const{
  for (size_t j = 0; j < nodes(); j++) {
    if(compNodes[j]->type == "flexProc" ) return false;
  }
  return true;
}

string Platform::getProcModel(size_t id)
{
  if(id > compNodes.size())
      THROW_EXCEPTION(InvalidArgumentException,"processor id out of bound\n");  
  return compNodes[id]->model;
}

std::ostream& operator<< (std::ostream &out, const Platform &p)
{
  for (size_t i=0;i<p.compNodes.size();i++)
    {
      out << "PE[" << i << "]: " << *p.compNodes[i] << endl;
    }

  return out;

}
