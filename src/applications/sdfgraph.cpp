#include "sdfgraph.hpp"

#include <math.h>
#include <algorithm>

SDFGraph::SDFGraph(XMLdoc& xmlAppGraph) : xml(xmlAppGraph) {
  
  //Initializations
  parentActors 	     = 0;
  period_constraint  = 0;
  latency_constraint = 0;

  graphName = xml.xpathStrings("///sdf/@name")[0];
  LOG_INFO("   ...application " + graphName );

  _d = new dictionaries();
  buildDictionaries();
  transform();
  createPathMatrix();
  if (_d) delete _d;

  LOG_DEBUG("SDFGraph intermediate representation built successfully");
  //LOG_DEBUG(getString());

}

SDFGraph::SDFGraph(Platform* platform, XMLdoc& doc): xml(doc) {
  graphName = "TDNconfigG";
  parentActors 	     = platform->nodes();
  period_constraint  = 0;
  latency_constraint = 0;
  
  //generate actors
  for(size_t a=0; a<platform->nodes(); a++){
    actors.push_back(new SDFActor());
    actors[a]->id = a;
    actors[a]->name = "actor_"+tools::toString(a);
    actors[a]->parent_id = a;
    actors[a]->parent_name = "parent_"+tools::toString(a);
    actors[a]->codeSize = 0;
    actors[a]->dataSize = 0;
  }
  
  //create path matrix
  pathMatrix.assign(actors.size() * actors.size(), simplePath());
  
  //generate channels between all actors
  size_t id = 0;
  for(size_t src=0; src<actors.size(); src++){
    for(size_t dst=0; dst<actors.size(); dst++){
      channels.push_back(new SDFChannel());
      channels[id]->id = id;
      channels[id]->name = "ch_"+tools::toString(id);
      channels[id]->source = src;
      channels[id]->src_name = actors[src]->name;
      channels[id]->prod = 1;
      channels[id]->destination = dst;
      channels[id]->dst_name = actors[dst]->name;
      channels[id]->cons = 1;
      channels[id]->initTokens = (src<dst) ? 0 : 1;
      channels[id]->tokenSize = platform->getFlitSize();
      channels[id]->messageSize = channels[id]->tokenSize;
      
      pathMatrix[src * actors.size() + dst].exists = 1;
      pathMatrix[src * actors.size() + dst].initTokens = channels[id]->initTokens;
      
      id++;
    }
  }
  LOG_DEBUG("Path matrix of generated SDFG for TDN configuration:\n"+printPathMatrix());
}

SDFGraph::~SDFGraph(){
  //delete actors and channels (they were created with 'new'')
  for (size_t i=0; i<actors.size(); i++){
    delete actors[i];
  }
  for (size_t j=0; j<channels.size(); j++){
    delete channels[j];
  }
}

void SDFGraph::buildDictionaries() {

  // extract port rates from the XMLdoc
  auto acts = xml.xpathNodes("///sdf/actor");
  for (size_t i=0; i<acts.size(); i++){
    string name  = xml.getProp(acts[i], "name");
    string query = "///sdf/actor[@name=\'" + name + "\']/port";
    auto   ports = xml.xpathNodes(query.c_str());
    for (auto port : ports) {
      string port_name = xml.getProp(port, "name");
      string port_rate = xml.getProp(port, "rate");
      _d->rate[name][port_name]=atoi(port_rate.c_str());
    }
    _d->actor_id[name] = i;

    string size_query = "///sdfProperties/actorProperties[@actor=\'" + name + "\']/processor/memory/stateSize/@max";
    auto codeSizeValue = xml.xpathStrings(size_query.c_str());
    // TODO: if state size are not found, they are replaced with 0;
    _d->actor_sz[name] = (codeSizeValue.size() > 0) ? atoi(codeSizeValue[0].c_str()) : 0;
  }

  auto chans = xml.xpathNodes("///sdf/channel");
  for (size_t i=0; i<chans.size(); i++){
    string ch_name = xml.getProp(chans[i], "name");
    string src_act = xml.getProp(chans[i], "srcActor");
    string dst_act = xml.getProp(chans[i], "dstActor");
    string src_prt = xml.getProp(chans[i], "srcPort");
    string dst_prt = xml.getProp(chans[i], "dstPort");
    _d->channel[src_act][src_prt] = ch_name;
    _d->channel[dst_act][dst_prt] = ch_name;
    _d->chan_con[ch_name] = {src_act, src_prt, dst_act, dst_prt};
    string size_query = "///sdfProperties/channelProperties[@channel=\'" + ch_name + "\']/tokenSize/@sz";
    auto tokenSizeValue = xml.xpathStrings(size_query.c_str());
    // TODO: if token sizes or channel sizes are not found, they are replaced with 0;
    _d->chan_sz[ch_name] = (tokenSizeValue.size() > 0) ? atoi(tokenSizeValue[0].c_str()) : 0;
    _d->init_tok[ch_name] = xml.hasProp(chans[i], "initialTokens") ? atoi(xml.getProp(chans[i], "initialTokens").c_str()) : 0;
  }
}

void SDFGraph::calculateRepetitionVector(vector<rational<int>>& firing, string a) {
  auto& rate     = _d->rate;
  auto& actor_id = _d->actor_id;
  auto& channel  = _d->channel;
  auto& chan_con = _d->chan_con;

  for (auto& p_c : channel[a]) {
    string pA = p_c.first;
    vector<string> c  = chan_con[p_c.second];
    string b  = c[0];
    string pB = c[1];
    if (a == b) {
      b  = c[2];
      pB = c[3];
    }
    rational<int> ratioAB(rate[a][pA], rate[b][pB]);
    rational<int> &rateA = firing[actor_id[a]];
    rational<int> &rateB = firing[actor_id[b]];
    if (rateB == 0) {
      rateB = rateA * ratioAB;
      calculateRepetitionVector(firing, b);
    }
    else if (rateB != rateA * ratioAB)
       THROW_EXCEPTION(RuntimeException, string() +
           "SDF graph is inconsistent!\n" +
           "\trate(" + b + ") : " + tools::toString(rateB) +
           " != " + tools::toString(rateA * ratioAB));
  }
}

void SDFGraph::transform(){
  auto& actor_id = _d->actor_id;

  LOG_DEBUG("Calculating repetition vector for " + graphName);

  using boost::lcm;
  using boost::gcd;
  vector<rational<int>> firing(actor_id.size(), rational<int>(0));

  auto acts = xml.xpathNodes("///sdf/actor");
  for (auto& a : acts){
    string name  = xml.getProp(a, "name");
    if (firing[actor_id[name]] == 0) {
      firing[actor_id[name]] = 1;
      calculateRepetitionVector(firing, name);
    }
  }

  // normalize fractions to their LCM
  int lcm_den = 1;
  vector<int> rep_vec;
  for (auto& f : firing) lcm_den = lcm(lcm_den, f.denominator());
  for (auto& f : firing) rep_vec.push_back(f.numerator() * (lcm_den / f.denominator()));
  int gcd_num = rep_vec[0];
  for (auto& r : rep_vec) gcd_num = boost::gcd(gcd_num, r);
  for (auto& r : rep_vec) r /= gcd_num;

  LOG_DEBUG("Found repetition vector: " + tools::toString(rep_vec));

  //Initiate corresponding transformation method
  bool isHSDF = all_of(rep_vec.begin(), rep_vec.end(), [](int i){return i==1;});
  isHSDF ? transformFromHSDF() : transformFromSDF(rep_vec);
  
  //print graph into debug log file
  LOG_DEBUG("All " + tools::toString(actors.size()) + " actors of graph " + graphName +":");
  for(auto i : actors){
    LOG_DEBUG("actor " + tools::toString(i->id) + ": " + i->name
              + ", parent(id): " + i->parent_name + "(" + tools::toString(i->parent_id) +")"
              + "; code size: " + tools::toString(i->codeSize) 
              + "; data size: " + tools::toString(i->dataSize));
  }
  
  LOG_DEBUG("All " + tools::toString(channels.size()) + " channels of graph " + graphName +":");
  for(auto i: channels){
    LOG_DEBUG("channel " + tools::toString(i->id) + ": " + i->name
              +" from " + i->src_name + "(" + tools::toString(i->source) + ")"
              + " [prod: " + tools::toString(i->prod) + "]"
              +" to " + i->dst_name + "(" + tools::toString(i->destination) + ")"
              + " [cons: " + tools::toString(i->cons) + "]"
              + "; initial tokens: " + tools::toString(i->initTokens)
              + "; token size: " + tools::toString(i->tokenSize)
              + "; message size: "+ tools::toString(i->messageSize));
  }
}

size_t newIndex(size_t actor, const vector<int> &repVector){
  int sum=0;
  for (size_t i=0; i<actor; i++){
    sum += repVector[i];
  } 
  return sum;
}

void SDFGraph::transformFromSDF(const vector<int>& rp) {
  auto& rate     = _d->rate;
  auto& actor_id = _d->actor_id;
  auto& actor_sz = _d->actor_sz;
  auto& chan_sz  = _d->chan_sz;
  auto& init_tok = _d->init_tok;

  LOG_DEBUG("Transforming graph " + graphName + " from SDF...");

  unsigned int newActors = newIndex(rp.size(), rp);
  vector<int> curInports(newActors, 0);
  vector<int> curOutports(newActors, 0);

  //initialize actors
  for (size_t i = 0; i < newActors; i++)
    actors.push_back(new SDFActor());

  //keep number of original (parent) actors
  parentActors = actor_id.size();

  auto chan_nodes = xml.xpathNodes("///sdf/channel");
  for (auto& gC : chan_nodes) {
    string ch_name = xml.getProp(gC, "name");
    string src_act = xml.getProp(gC, "srcActor");
    string dst_act = xml.getProp(gC, "dstActor");
    string src_prt = xml.getProp(gC, "srcPort");
    string dst_prt = xml.getProp(gC, "dstPort");

    int srcId   = actor_id[src_act];
    int dstId   = actor_id[dst_act];
    int p       = rate[src_act][src_prt];
    int q       = rate[dst_act][dst_prt];
    int initTok = init_tok[ch_name];
    int tokSz   = chan_sz[ch_name];

    int i = newIndex(srcId, rp);
    int j = curOutports[i];
    // for index of dst node, +((initTok/q)%rp[dstId]) for "shifting" due to initial tokens
    int k = newIndex(dstId, rp) + ((initTok / q) % rp[dstId]);
    int l = curInports[k];
    int x = rp[srcId] * p;
    int r = 0;
    int tok = 0;
    while (x > 0) {
      int newRate;
      if (r == 0) {
        newRate = min(p, q);
        x -= newRate;
        r = abs(p - q);
      } else { //r!=0
        newRate = min(r, p);
        newRate = min(newRate, q);
        x -= newRate;
        r = max((x % p), (x % q));
      }
      if (initTok > 0 && initTok > x)
        tok = newRate;
      SDFChannel* ch = new SDFChannel();
      ch->id          = channels.size();
      ch->name        = ch_name;
      ch->source      = i;
      ch->src_name    = src_act + "_" + tools::toString(i - newIndex(srcId, rp));
      ch->destination = k;
      ch->dst_name    = dst_act + "_" + tools::toString(k - newIndex(dstId, rp)) ;
      ch->prod        = newRate;
      ch->cons        = newRate;
      ch->initTokens  = tok;
      ch->tokenSize   = tokSz;
      ch->messageSize = newRate * ch->tokenSize;

      //add actors
      channels.push_back(ch);
      actors[i]->id          = i;
      actors[i]->name        = ch->src_name;
      actors[i]->parent_id   = srcId;
      actors[i]->parent_name = src_act;
      actors[i]->codeSize    = actor_sz[src_act];
      actors[i]->dataSize    = actor_sz[src_act];

      actors[k]->id          = k;
      actors[k]->name        = ch->dst_name;
      actors[k]->parent_id   = dstId;
      actors[k]->parent_name = dst_act;
      actors[k]->codeSize    = actor_sz[dst_act];
      actors[k]->dataSize    = actor_sz[dst_act];

      //cout << "[" << ch->source  << ch->prod << ch->destination << ch->cons << ch->initTokens << "]" << endl;
      curOutports[i]++;
      curInports[k]++;

      if ((rp[srcId] * p - x) % p == 0) {
        i = newIndex(srcId, rp) + ((i - newIndex(srcId, rp) + 1) % rp[srcId]);
        j = curOutports[i];
      } else { //if(rp[srcId]*p-x < (i-newIndex(srcId, rp)+1)*p){
        j++;
      }
      if ((rp[dstId] * q - x) % q == 0) {
        k = newIndex(dstId, rp) + ((k - newIndex(dstId, rp) + 1) % rp[dstId]);
        l = curInports[k];
      } else { //if(rp[dstId]*q-x < (k-newIndex(dstId, rp)+1)*q){
        l++;
      }
    }
  }
}

void SDFGraph::transformFromHSDF() {
    auto& rate = _d->rate;
    auto& actor_id = _d->actor_id;
    auto& actor_sz = _d->actor_sz;
    auto& chan_sz = _d->chan_sz;
    auto& init_tok = _d->init_tok;

    LOG_DEBUG("Transforming graph " + graphName + " from HSDF...");

    //Step 1: Copy the actors
    int parentActorCount = 0;
    unordered_map<string, int> parentActorIds;

    auto actor_nodes = xml.xpathNodes("///sdf/actor");
    for(auto& gF : actor_nodes){
        SDFActor* a = new SDFActor();
        string a_name = xml.getProp(gF, "name");

        a->name = a_name;
        a->id = actor_id[a_name];
        a->codeSize = actor_sz[a_name]; //TODO: combine with data size?
        a->dataSize = actor_sz[a_name];

        //find the name and id of the parent actor (in original SDF)
        unsigned pos = a->name.find_last_of("_");
        a->parent_name = a->name.substr(0, pos);

        auto it = parentActorIds.find(a->parent_name);
        if(it != parentActorIds.end()){ //actor has been found before
            a->parent_id = it->second;
        }else{ //actor is found for the first time
            parentActorIds.insert(
                    pair<string, int>(a->parent_name, parentActorCount));
            a->parent_id = parentActorCount;
            parentActorCount++;
        }
        //insert into actors vector
        actors.push_back(a);
    }

    parentActors = parentActorCount;
    parentActorIds.clear();

    //Step 2: Check for and combine parallel channels
    unordered_map<int, vector<SDFChannel*>> channelMap;
    int old = 0;
    int newCh = 0;

    auto chan_nodes = xml.xpathNodes("///sdf/channel");
    for(size_t i = 0; i < chan_nodes.size(); i++){
        auto gC = chan_nodes[i];
        string ch_name = xml.getProp(gC, "name");
        string src_act = xml.getProp(gC, "srcActor");
        string dst_act = xml.getProp(gC, "dstActor");
        string src_prt = xml.getProp(gC, "srcPort");
        string dst_prt = xml.getProp(gC, "dstPort");

        SDFChannel* c = new SDFChannel();
        c->id = i;
        c->name = ch_name;
        c->source = actor_id[src_act];
        c->src_name = src_act;
        c->prod = rate[src_act][src_prt];
        c->destination = actor_id[dst_act];
        c->dst_name = dst_act;
        c->cons = rate[dst_act][dst_prt];
        c->initTokens = init_tok[ch_name];
        c->tokenSize = chan_sz[ch_name];
        c->messageSize = c->tokenSize; //update if combined with other channel
        c->oldIds.push_back(c->id);

        unordered_map<int, vector<SDFChannel*>>::const_iterator it =
                channelMap.find(c->source);
        if(it != channelMap.end()){
            //c->source already has an entry in the map, check whether a channel with same dest. exists
            vector<SDFChannel*> srcChannels = channelMap.at(c->source);
            bool sameChannel = false;
            int tmpCh = 0;
            int oldCh = 0;
            for(auto it = srcChannels.begin(); it != srcChannels.end(); ++it){
                if(c->destination == (*it)->destination){ //same destination
                    sameChannel = true;
                    oldCh = tmpCh;
                    if((*it)->initTokens != c->initTokens)
                        LOG_WARNING("(*it)->initTokens != c->initTokens");
                    c->initTokens = max(c->initTokens, (*it)->initTokens);
                    //check how the channels are to be combined. Same or different token size?
                    //TODO: this identification could be done using the channel names
                    //      same name = increase rates, different name = concatenate names & add token sizes
                    if(c->tokenSize == (*it)->tokenSize){
                        c->prod += (*it)->prod;
                        c->cons += (*it)->cons;
                        c->messageSize = c->prod * c->tokenSize;
                    }else{ //different token size
                        c->tokenSize += (*it)->tokenSize;
                        c->messageSize = c->prod * c->tokenSize;
                    }
                    //append all so-far-collected ids of combined channels
                    c->oldIds.insert(c->oldIds.end(), (*it)->oldIds.begin(),
                            (*it)->oldIds.end());
                    c->id = (*it)->id;
                }
                tmpCh++; //save the index of the found channel (for later update)
            }
            if(!sameChannel){ //there was no channel with same destination, so add it
                c->id = newCh; //TODO: update channel name as well?
                newCh++;
                channelMap.at(c->source).push_back(c);
            }else{ //channel already existed -> replace with new, combined channel
                delete channelMap.at(c->source)[oldCh];
                channelMap.at(c->source)[oldCh] = c;
            }
        }else{ //no entry for c->source yet
            vector<SDFChannel*> newChannel;
            c->id = newCh;
            newCh++;
            newChannel.push_back(c);
            channelMap.insert(make_pair(c->source, newChannel));
        }

        old++; //count original channels
    }

    for(size_t i = 0; i < actors.size(); i++){
        unordered_map<int, vector<SDFChannel*>>::const_iterator it =
                channelMap.find(i);
        if(it != channelMap.end()){
            //channels.insert(channels.end(), channelMap.at(i).begin(), channelMap.at(i).end()); //append vector to channels vector
            for(vector<SDFChannel*>::iterator y = channelMap.at(i).begin();
                    y != channelMap.at(i).end(); ++y){
                channels.push_back(*y);
            }
        }
    }
    for(size_t k = 0; k < channels.size(); k++){
        channels[k]->id = k;
    }
}

void SDFGraph::createPathMatrix() {
    using namespace boost;
    typedef adjacency_list<vecS, vecS, bidirectionalS> Graph;
    typedef adjacency_list<> TcGraph;
    pathMatrix.assign(actors.size() * actors.size(), simplePath());
    const int num_vertices = actors.size();

    Graph g(num_vertices);
    TcGraph tc;
    Graph g_noTokens(num_vertices);
    TcGraph tc_noTokens;

    for(size_t k = 0; k < channels.size(); k++){
        int src = channels[k]->source;
        int dst = channels[k]->destination;
        int initTokens = channels[k]->initTokens;
        if(!initTokens){
            add_edge(src, dst, g_noTokens);
        }
        add_edge(src, dst, g);
    }

    transitive_closure(g, tc);
    transitive_closure(g_noTokens, tc_noTokens);

    graph_traits<TcGraph>::edge_iterator ei, ei_end;
    for(tie(ei, ei_end) = edges(tc); ei != ei_end; ++ei){
        auto src = boost::source(*ei, tc);
        auto dst = boost::target(*ei, tc);

        pathMatrix[src * actors.size() + dst].exists = 1;
        pathMatrix[src * actors.size() + dst].initTokens = 1;
    }
    for(tie(ei, ei_end) = edges(tc_noTokens); ei != ei_end; ++ei){
        auto src = boost::source(*ei, tc_noTokens);
        auto dst = boost::target(*ei, tc_noTokens);

        pathMatrix[src * actors.size() + dst].exists = 1;
        pathMatrix[src * actors.size() + dst].initTokens = 0;
    }
}

string SDFGraph::printPathMatrix() const {
  string str;
  str += "    ";
  for (size_t ii=0; ii<(actors.size()*4)-1; ii++){
    str += "-";
  }
  str += "\n";
  for (size_t ii=0; ii<actors.size(); ii++){
    str += "    ";
    for (size_t ij=0; ij<actors.size(); ij++){
      if(pathMatrix[ii*actors.size()+ij].exists){
        str += tools::toString(pathMatrix[ii*actors.size()+ij].initTokens);
        str +=  " | ";
      }else{
        str += "- | ";
      }
    }
    str += "\n";
  }
  str += "    ";
  for (size_t ii=0; ii<(actors.size()*4)-1; ii++){
    str += "-";
  }
  str += "\n";
  return str;
}


string SDFGraph::getString() const {
  string str;
  str += "* graphName : " + graphName;
  str += "\n* parentActors : " + tools::toString(parentActors);
  str += "\n* period_constraint : " + tools::toString(period_constraint);
  str += "\n* latency_constraint : " + tools::toString(latency_constraint);
  str += "\n* actors :";
  for (auto& a : actors) {
    str += "\n  + " + a->name + " (" + tools::toString(a->id) + ")";
    str += "\n    - parent : " + a->parent_name + " (" + tools::toString(a->parent_id) + ")";
    str += "\n    - codeSize : " + tools::toString(a->codeSize);
    str += "\n    - dataSize : " + tools::toString(a->dataSize);
  }
  str += "\n* channels :";
  for (auto& c : channels) {
    str += "\n  + " + c->name + " (" + tools::toString(c->id) + ")";
    str += "\n    - source : " + c->src_name + " (" + tools::toString(c->source) + ")";
    str += "\n    - dest : " + c->dst_name + " (" + tools::toString(c->destination) + ")";
    str += "\n    - prod : " + tools::toString(c->prod);
    str += "\n    - cons : " + tools::toString(c->cons);
    str += "\n    - initTokens : " + tools::toString(c->initTokens);
    str += "\n    - tokenSize : " + tools::toString(c->tokenSize);
    str += "\n    - messageSize : " + tools::toString(c->messageSize);
    str += "\n    - cost : " + tools::toString(c->cost);
    str += "\n    - oldIds : " + tools::toString(c->oldIds);
  }
#ifdef DEBUG
  str += "\n* path matrix :\n";
  str += printPathMatrix();
#endif
  return str;
}

void SDFGraph::outputGraphAsDot(const string &dir) const {
  
  ofstream out;
  string outputFile = dir;
  outputFile += (outputFile.back()=='/') ? (graphName+".dot") : ("/"+graphName+".dot");
  out.open(outputFile.c_str());
  
  out << "digraph " << graphName << " {" << endl;
  out << "    size=\"7,10\";" << endl;
  out << "    rankdir=\"LR\"" << endl;
  
  //Output actors
  for (size_t i=0; i<actors.size(); i++){
    SDFActor* a = actors[i];
    
    //32 colors to choose from
    int col = (a->parent_id + 1) % 32;
    
    out << "    " << a->name << " [ label=\"" << a->name << "\"";
    out << ", style=filled";
    out << ", fillcolor=\""; 
    if (parentActors > 32) { //if more than 32 colors needed, use white only
      out << "white";
    }else{
      if(col<13){
        // (See ColorBrewer license)
        out << "/set312/" << col;
      }else if(col>12&&col<24){
        // (See ColorBrewer license)
        out << "/spectral11/" << col-12;
      }else{ //col>23
        // (See ColorBrewer license)
        out << "/set19/" << col-23;
        
      }
    }
    out << "\"";
    out << "];" << endl;  
  }
  out << endl;
    
  //Output all channels
  for (size_t j=0; j<channels.size(); j++){
    SDFChannel* c = channels[j];
        
    // Initial tokens on channel?
    if (c->initTokens != 0){
#ifdef __DOT_EXPLICIT_INITIAL_TOKENS
      out << "    " << c->src_name << " -> " << c->name;
      out << " [ label=\"" << c->name << "\", taillabel=\"";
      out << c->prod << "\", arrowhead=dot, headlabel=\"";
      out << c->initTokens << "\" ];" << endl;

      out << "    " << c->name << " -> " << c->dst_name;
      out << " [ headlabel=\"" << c->cons << "\" ];" << endl;
      
      out << "    " << c->name;
      out << " [ label=\"\", width=0, height=0, fixedsize=true ];";
      out << endl;
#else // __DOT_EXPLICIT_INITIAL_TOKENS
      out << "    " << c->src_name << " -> " << c->dst_name;
      out << " [ label=\"" << "init(";
      out << c->initTokens<< ")" << "\", taillabel=\"";
      out << c->prod << "\", headlabel=\"";
      out << c->cons << "\" ];" << endl;
#endif // __DOT_EXPLICIT_INITIAL_TOKENS
    }else{
      out << "    " << c->src_name << " -> " << c->dst_name;
      out << " [ label=\"        \", taillabel=\"";
      out << c->prod << "\"";
      out << ",labeldistance=" << 1.8;
      out << ", headlabel=\"";
      out << c->cons << "\" ];" << endl;
    }
  }
  for (size_t i=0; i<parentActors; i++){
    out << "    { rank=same; ";
    for (size_t j=0; j<actors.size(); j++){
      SDFActor* a = actors[j];  
      if(a->parent_id == i) out << a->name << " ";
    }
    out << "}" << endl;
  }  
  
  out << "}" << endl;
  
  out.close();
  
  cout << "  Printed dot graph file " << outputFile << endl;
#ifdef DEBUG
  string cmd = "cat "+outputFile+" | dot -Tpdf > "+outputFile+".pdf";
  cout << "  " << cmd << endl;
  system(cmd.c_str());
#endif
}

int SDFGraph::n_actors() const{
  return actors.size();
}

int SDFGraph::n_channels() const{
  return channels.size();
}

size_t SDFGraph::n_parentActors() const{
  return parentActors;
}

string SDFGraph::getName() const{
  return graphName;  
}

string SDFGraph::getActorName(size_t p_actor) const{
  if(p_actor<actors.size()){
    return actors[p_actor]->name;
  }  
  return "";
}

string SDFGraph::getParentName(size_t p_actor) const{
  if(p_actor<actors.size()){
    return actors[p_actor]->parent_name;
  }  
  return "";
}

int SDFGraph::getParentId(size_t p_actor) const{
  if(p_actor<actors.size()){
    return actors[p_actor]->parent_id;
  }  
  return -1;
}

size_t SDFGraph::getCodeSize(size_t p_actor) const{
  if(p_actor<actors.size()){
    return actors[p_actor]->codeSize;
  }  
  return 0;
}

size_t SDFGraph::getDataSize(size_t p_actor) const{
  if(p_actor<actors.size()){
    return actors[p_actor]->dataSize;
  }  
  return 0;
}

size_t SDFGraph::getMaxCodeSize() const{
  size_t maxCodeSize = 0;
  for (size_t i=0; i<actors.size(); i++){
    if(maxCodeSize < actors[i]->codeSize) 
      maxCodeSize =  actors[i]->codeSize;
  }  
  return maxCodeSize;
}

size_t SDFGraph::getTokenSize(size_t ch_id) const{
  if(ch_id<channels.size()){
    return channels[ch_id]->tokenSize;
  }  
  return 0;
}

int SDFGraph::lastActorInstanceId(size_t p_parentActor) const{
  for (size_t i=actors.size()-1; i>=0; i--){
    if(actors[i]->parent_id == p_parentActor)
      return i;
  }  
  return -1;
}

//get the list of channels
vector<SDFChannel*> SDFGraph::getChannels() const{
  return channels;  
}

//get the list of direct predecessors of actor id
vector<SDFActor*> SDFGraph::getPredecessors(int p_actor) const{
  vector<SDFActor*> pred;
  
  for (size_t k=0; k<channels.size(); k++){
    SDFChannel* ch = channels[k];
    if(ch->destination==p_actor && ch->initTokens==0){
      pred.push_back(actors[ch->source]);
    } 
  } 
  return pred;
}

//get the list of direct successors of actor id
vector<SDFActor*> SDFGraph::getSuccessors(int p_actor) const{
  vector<SDFActor*> succ;
  
  for (size_t k=0; k<channels.size(); k++){
    SDFChannel* ch = channels[k];
    if(ch->source==p_actor && ch->initTokens==0){
      succ.push_back(actors[ch->destination]);
    } 
  } 
  return succ;
}

// Checks whether a channel between src and dst exists in the graph,
// with or without initial tokens
bool SDFGraph::channelExists(int p_src, int p_dst) const{
    
  for (size_t k=0; k<channels.size(); k++){
    SDFChannel* ch = channels[k];
    if(ch->source==p_src && ch->destination==p_dst){
      return true;
    } 
  } 
  return false;
}

// Gives the number of initial tokens on an edge from src to dst
// in case there is no edge from src to dst, it returns -1
int SDFGraph::tokensOnChannel(int p_src, int p_dst) const{
  for (size_t k=0; k<channels.size(); k++){
    SDFChannel* ch = channels[k];
    if(ch->source==p_src && ch->destination==p_dst){
      return ch->initTokens;
    } 
  } 
  return -1;
}

// Checks whether a path between src and dst exists in the graph
bool SDFGraph::pathExists(int p_src, int p_dst) const{
  return pathMatrix[p_src*actors.size()+p_dst].exists;
}

// Are there any initial tokens on the path from src to dst?
bool SDFGraph::tokensOnPath(int p_src, int p_dst) const{
  return pathMatrix[p_src*actors.size()+p_dst].initTokens;
}

//does actorI precede actor J in G (i.e. there is a path from i to j with no tokens)?
bool SDFGraph::precedes(int p_actorI, int p_actorJ) const{
  return (pathExists(p_actorI, p_actorJ) && !tokensOnPath(p_actorI, p_actorJ));  
}

//Are firings firingI and firingI independent in G?
bool SDFGraph::independent(int p_actorI, int p_actorJ) const{
  return ((!pathExists(p_actorI,p_actorJ) && !pathExists(p_actorJ,p_actorI)) || 
          ((pathExists(p_actorI,p_actorJ) && tokensOnPath(p_actorI, p_actorJ)>0) && 
           (!pathExists(p_actorJ,p_actorI) || (pathExists(p_actorJ,p_actorI) && tokensOnPath(p_actorJ, p_actorI)>0))) ||
          ((pathExists(p_actorJ, p_actorI) && tokensOnPath(p_actorJ, p_actorI)>0) && 
           (!pathExists(p_actorI,p_actorJ) || (pathExists(p_actorI,p_actorJ) && tokensOnPath(p_actorI,p_actorJ)>0))));
}

int SDFGraph::getPeriodConstraint() const {
  return period_constraint;
}
  
void SDFGraph::setPeriodConstraint(int _period_constraint) {
  period_constraint = _period_constraint;
}
  
int SDFGraph::getLatencyConstraint() const {
  return latency_constraint;
}	
  
void SDFGraph::setLatencyConstraint(int _latency_constraint) {
  latency_constraint = _latency_constraint;
}
void SDFGraph::setDesignConstraints(vector<char*> elements, vector<char*> values) {
  for (size_t i=0;i< elements.size();i++) {
    try {						
      if(strcmp(elements[i], "period_constraint") == 0) 
        period_constraint 	= atoi(values[i]);
      if(strcmp(elements[i], "latency_constraint") == 0)
        latency_constraint 	= atoi(values[i]);	
    }			
    catch(std::exception const& e) {
      cout << "reading design constraints xml file error : " << e.what() << endl;
    }
  }
}
