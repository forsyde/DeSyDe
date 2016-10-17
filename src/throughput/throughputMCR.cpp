#include "throughputMCR.hpp"
#include "../tools/stringtools.hpp"

using namespace Gecode;
using namespace Int;
using namespace std;

ThroughputMCR::ThroughputMCR(Space& home, ViewArray<IntView> p_latency, ViewArray<IntView> p_period, ViewArray<IntView> p_iterations,
        ViewArray<IntView> p_iterationsCh, ViewArray<IntView> p_sendbufferSz, ViewArray<IntView> p_recbufferSz, ViewArray<IntView> p_next,
        ViewArray<IntView> p_wcet, ViewArray<IntView> p_sendingTime, ViewArray<IntView> p_sendingLatency, ViewArray<IntView> p_sendingNext,
        ViewArray<IntView> p_receivingTime, ViewArray<IntView> p_receivingNext, IntArgs p_ch_src, IntArgs p_ch_dst, IntArgs p_tok, IntArgs p_apps,
        IntArgs p_minIndices, IntArgs p_maxIndices) :
        Propagator(home), latency(p_latency), period(p_period), iterations(p_iterations), iterationsCh(p_iterationsCh), sendbufferSz(p_sendbufferSz), recbufferSz(
                p_recbufferSz), next(p_next), wcet(p_wcet), sendingTime(p_sendingTime), sendingLatency(p_sendingLatency), sendingNext(p_sendingNext), receivingTime(
                p_receivingTime), receivingNext(p_receivingNext), ch_src(p_ch_src), ch_dst(p_ch_dst), tok(p_tok), apps(p_apps), minIndices(p_minIndices), maxIndices(
                p_maxIndices) {

    sendingTime.subscribe(home, *this, Int::PC_INT_BND);
    sendingNext.subscribe(home, *this, Int::PC_INT_VAL);
    next.subscribe(home, *this, Int::PC_INT_VAL);
    /*latency.subscribe(home, *this, Int::PC_INT_BND);
     period.subscribe(home, *this, Int::PC_INT_BND);
     sendbufferSz.subscribe(home, *this, Int::PC_INT_BND);
     recbufferSz.subscribe(home, *this, Int::PC_INT_BND);
     //next.subscribe(home, *this, Int::PC_INT_VAL);
     sendingTime.subscribe(home, *this, Int::PC_INT_BND);
     sendingLatency.subscribe(home, *this, Int::PC_INT_BND);
     sendingNext.subscribe(home, *this, Int::PC_INT_VAL);
     receivingNext.subscribe(home, *this, Int::PC_INT_VAL);*/

    printDebug = true;

    n_actors = p_wcet.size();
    n_channels = p_ch_src.size();
    n_procs = p_next.size() - n_actors;
    wc_latency.insert(wc_latency.begin(), p_apps.size(), vector<int>());
    wc_period.insert(wc_period.begin(), p_apps.size(), 0);

    home.notice(*this, AP_DISPOSE);
}

size_t ThroughputMCR::dispose(Space& home) {
    sendingTime.cancel(home, *this, Int::PC_INT_BND);
    sendingNext.cancel(home, *this, Int::PC_INT_VAL);
    next.cancel(home, *this, Int::PC_INT_VAL);
    /*latency.cancel(home, *this, Int::PC_INT_BND);
     period.cancel(home, *this, Int::PC_INT_BND);
     sendbufferSz.cancel(home, *this, Int::PC_INT_BND);
     recbufferSz.cancel(home, *this, Int::PC_INT_BND);
     next.cancel(home, *this, Int::PC_INT_VAL);
     sendingTime.cancel(home, *this, Int::PC_INT_BND);
     sendingLatency.cancel(home, *this, Int::PC_INT_BND);
     sendingNext.cancel(home, *this, Int::PC_INT_VAL);
     receivingNext.cancel(home, *this, Int::PC_INT_VAL);*/

    //b_msag.~adj_list_impl();
    b_msags.~vector<boost_msag*>();
    msaGraph.~unordered_map<int, vector<SuccessorNode>>();
    channelMapping.~vector<int>();
    receivingActors.~vector<int>();

    wc_latency.~vector<vector<int>>();
    wc_period.~vector<int>();
    max_start.~vector<vector<int>>();
    max_end.~vector<vector<int>>();
    min_start.~vector<vector<int>>();
    min_end.~vector<vector<int>>();
    start_pp.~vector<int>();
    end_pp.~vector<int>();
    min_iterations.~vector<int>();
    max_iterations.~vector<int>();
    min_send_buffer.~vector<int>();
    max_send_buffer.~vector<int>();
    min_rec_buffer.~vector<int>();
    max_rec_buffer.~vector<int>();

    home.ignore(*this, AP_DISPOSE);
    (void) Propagator::dispose(home);
    return sizeof(*this);
}

ThroughputMCR::ThroughputMCR(Space& home, bool share, ThroughputMCR& p) :
        Propagator(home, share, p), ch_src(p.ch_src), ch_dst(p.ch_dst), tok(p.tok), apps(p.apps), minIndices(p.minIndices), maxIndices(p.maxIndices), n_actors(
                p.n_actors), n_channels(p.n_channels), n_procs(p.n_procs), n_msagActors(p.n_msagActors), msaGraph(p.msaGraph),
                b_msag(p.b_msag), b_msags(p.b_msags), channelMapping(p.channelMapping), receivingActors(
                p.receivingActors), wc_latency(p.wc_latency), wc_period(p.wc_period), printDebug(p.printDebug) {
    latency.update(home, share, p.latency);
    period.update(home, share, p.period);
    iterations.update(home, share, p.iterations);
    iterationsCh.update(home, share, p.iterationsCh);
    sendbufferSz.update(home, share, p.sendbufferSz);
    recbufferSz.update(home, share, p.recbufferSz);
    next.update(home, share, p.next);
    wcet.update(home, share, p.wcet);
    sendingTime.update(home, share, p.sendingTime);
    sendingLatency.update(home, share, p.sendingLatency);
    sendingNext.update(home, share, p.sendingNext);
    receivingTime.update(home, share, p.receivingTime);
    receivingNext.update(home, share, p.receivingNext);
}

Propagator* ThroughputMCR::copy(Space& home, bool share) {
    return new (home) ThroughputMCR(home, share, *this);
}

//TODO: do this right
PropCost ThroughputMCR::cost(const Space& home, const ModEventDelta& med) const {
    return PropCost::linear(PropCost::HI, next.size());
}

void ThroughputMCR::constructMSAG() {
    if(printDebug)
        cout << "\tThroughputMCR::constructMSAG()" << endl;

    //first, figure out how many actors there will be in the MSAG, in order to
    //initialize channel-matrix and actor-vector for the state of SSE
    n_msagActors = n_actors;
    for(int i = 0; i < sendingTime.size(); i++){
        if(sendingTime[i].min() > 0){ //=> channel on interconnect
            n_msagActors += 3; //one blocking, one sending and one receiving actor
        }
    }

    b::graph_traits<boost_msag>::vertex_descriptor src, dst;
    b::graph_traits<boost_msag>::edge_descriptor _e;

    msaGraph.clear();
    receivingActors.clear();
    channelMapping.clear();
    receivingActors.insert(receivingActors.begin(), n_actors, -1); //pre-fill with -1

    //add all actors as vertices, and self-loops
    bool found;
    for(int n = 0; n < n_msagActors; n++){
        add_vertex(n, b_msag);
        //add self-edges
        src = vertex(n, b_msag);
        tie(_e, found) = add_edge(src, src, b_msag);
        if(n < n_actors){
            b::put(b::edge_weight, b_msag, _e, wcet[n].min());
        } //else{}: delay for communication actors are added further down
        b::put(b::edge_weight2, b_msag, _e, 1);
    }
    //next: add edges to boost-msag

    channel_count = 0;
    n_msagChannels = 0; //to count the number of channels in the MSAG

    //building the throughput analysis graph
    /* Step 1a: check sendingTime-array for all messages and add block-, send- & receive-"actors" with back-edges (buffering)
     Step 1b: check for dependencies in application graph that are not covered in 1a
     Step 2: check for decided forward-path in next-array
     Step 3: close execution cycles with back-edges found in next-array (next[i], i>=n_actors)
     */
    for(int i = 0; i < sendingTime.size(); i++){
        if(sendingTime[i].min() > 0){ //Step1a: => channel on interconnect

            int block_actor = n_actors + channel_count;
            int send_actor = block_actor + 1;
            int rec_actor = send_actor + 1;
            //store mapping between block/send/rec_actor and channel i
            channelMapping.push_back(i); //[block_actor] = i;
            channelMapping.push_back(i); //[send_actor] = i;
            channelMapping.push_back(i); //[rec_actor] = i;
            //add the block actor as a successor of ch_src[i]
            SuccessorNode succB;
            succB.successor_key = block_actor;
            succB.delay = sendingLatency[i].min();
            succB.min_tok = 0;
            succB.max_tok = 0;
            succB.channel = i;

            //add to boost-msag
            src = b::vertex(ch_src[i], b_msag);
            dst = b::vertex(block_actor, b_msag);
            b::tie(_e, found) = b::add_edge(src, dst, b_msag);
            b::put(b::edge_weight, b_msag, _e, sendingLatency[i].min());
            b::put(b::edge_weight2, b_msag, _e, 0);
            //delay-weight for self-loop on block-actor:
            tie(_e, found) = edge(dst, dst, b_msag);
            b::put(b::edge_weight, b_msag, _e, sendingLatency[i].min());

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(ch_src[i]);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(ch_src[i]).push_back(succB);
                }else{      //no entry for ch_src[i] yet
                    vector<SuccessorNode> succBv;
                    succBv.push_back(succB);
                    msaGraph.insert(pair<int, vector<SuccessorNode>>(ch_src[i], succBv));
                }
            }

            //add ch_src[i] as successor of the block actor, with buffer size as tokens
            SuccessorNode srcCh;
            srcCh.successor_key = ch_src[i];
            srcCh.delay = wcet[ch_src[i]].assigned() ? wcet[ch_src[i]].val() : wcet[ch_src[i]].min();
            srcCh.min_tok = sendbufferSz[i].min();
            srcCh.max_tok = sendbufferSz[i].max();

            //add to boost-msag
            src = b::vertex(block_actor, b_msag);
            dst = b::vertex(ch_src[i], b_msag);
            b::tie(_e, found) = b::add_edge(src, dst, b_msag);
            b::put(b::edge_weight, b_msag, _e, wcet[ch_src[i]].min());
            b::put(b::edge_weight2, b_msag, _e, sendbufferSz[i].max());

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(block_actor);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(block_actor).push_back(srcCh);
                }else{      //no entry for block_actor yet
                    vector<SuccessorNode> srcChv;
                    srcChv.push_back(srcCh);
                    msaGraph.insert(pair<int, vector<SuccessorNode>>(block_actor, srcChv));
                }
            }
//###
            //add the send actor as a successor of the block actor
            SuccessorNode succS;
            succS.successor_key = send_actor;
            succS.delay = sendingTime[i].min();
            succS.min_tok = 0;
            succS.max_tok = 0;
            succS.channel = i;

            //add to boost-msag
            src = b::vertex(block_actor, b_msag);
            dst = b::vertex(send_actor, b_msag);
            b::tie(_e, found) = b::add_edge(src, dst, b_msag);
            b::put(b::edge_weight, b_msag, _e, sendingTime[i].min());
            b::put(b::edge_weight2, b_msag, _e, 0);
            //delay-weight for self-loop on send-actor:
            tie(_e, found) = edge(dst, dst, b_msag);
            b::put(b::edge_weight, b_msag, _e, sendingTime[i].min());

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(block_actor);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(block_actor).push_back(succS);
                }else{      //no entry for block_actor yet
                    vector<SuccessorNode> succSv;
                    succSv.push_back(succS);
                    msaGraph.insert(pair<int, vector<SuccessorNode>>(block_actor, succSv));
                }
            }

            //add the block actor as successor of the send actor, with one token (serialization)
            SuccessorNode succBS;
            succBS.successor_key = block_actor;
            succBS.delay = sendingLatency[i].min();
            succBS.min_tok = 1;
            succBS.max_tok = 1;
            succBS.channel = i;

            //add to boost-msag
            src = b::vertex(send_actor, b_msag);
            dst = b::vertex(block_actor, b_msag);
            b::tie(_e, found) = b::add_edge(src, dst, b_msag);
            b::put(b::edge_weight, b_msag, _e, sendingLatency[i].min());
            b::put(b::edge_weight2, b_msag, _e, 1);

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(send_actor);
                if(it != msaGraph.end()){ //send actor already has an entry in the map
                    msaGraph.at(send_actor).push_back(succBS);
                }else{      //no entry for send_actor yet
                    vector<SuccessorNode> succBSv;
                    succBSv.push_back(succBS);
                    msaGraph.insert(pair<int, vector<SuccessorNode>>(send_actor, succBSv));
                }
            }

            //add receiving actor as successor of the send actor, with potential initial tokens
            SuccessorNode dstCh;
            dstCh.successor_key = rec_actor;
            dstCh.delay = receivingTime[i].min();
            dstCh.min_tok = tok[i];
            dstCh.max_tok = tok[i];
            dstCh.channel = i;
            dstCh.recOrder = receivingNext[i].min();


            //add to boost-msag
            src = b::vertex(send_actor, b_msag);
            dst = b::vertex(rec_actor, b_msag);
            b::tie(_e, found) = b::add_edge(src, dst, b_msag);
            b::put(b::edge_weight, b_msag, _e, receivingTime[i].min());
            b::put(b::edge_weight2, b_msag, _e, tok[i]);
            //delay-weight for self-loop on rec-actor:
            tie(_e, found) = edge(dst, dst, b_msag);
            b::put(b::edge_weight, b_msag, _e, receivingTime[i].min());

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(send_actor);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(send_actor).push_back(dstCh);
                }else{      //no entry for i yet
                    vector<SuccessorNode> dstChv;
                    dstChv.push_back(dstCh);
                    msaGraph.insert(pair<int, vector<SuccessorNode>>(send_actor, dstChv));
                }
            }

            //save the receiving actors for each actor (for next order)
            if(receivingActors[ch_dst[i]] == -1){ //first rec_actor for the dst
                receivingActors[ch_dst[i]] = rec_actor;
            }else{
                int curRec_actor_ch = channelMapping[receivingActors[ch_dst[i]] - n_actors];
                if(receivingNext[curRec_actor_ch].assigned()){
                    if(receivingNext[curRec_actor_ch].val() < n_channels){
                        if(ch_dst[receivingNext[curRec_actor_ch].val()] != ch_dst[i]){ //last rec_actor for this dst
                            receivingActors[ch_dst[i]] = rec_actor;
                        } //else
                    }else{ //last rec_actor for this dst
                        receivingActors[ch_dst[i]] = rec_actor;
                    }
                }
                if(receivingNext[channelMapping[rec_actor - n_actors]].assigned()){
                    if(receivingNext[channelMapping[rec_actor - n_actors]].val() == receivingActors[ch_dst[i]]){
                        receivingActors[ch_dst[i]] = rec_actor;
                    }
                }
            }

            //add the send actor as a successor node of the receiving actor, with rec. buffer size - initial tokens
            SuccessorNode succRec;
            succRec.successor_key = send_actor;
            succRec.delay = sendingTime[i].min();
            succRec.min_tok = recbufferSz[i].min() - tok[i];
            succRec.max_tok = recbufferSz[i].max() - tok[i];
            succRec.channel = i;

            //add to boost-msag
            src = b::vertex(rec_actor, b_msag);
            dst = b::vertex(send_actor, b_msag);
            b::tie(_e, found) = b::add_edge(src, dst, b_msag);
            b::put(b::edge_weight, b_msag, _e, sendingTime[i].min());
            b::put(b::edge_weight2, b_msag, _e, recbufferSz[i].max() - tok[i]);

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(rec_actor);
                if(it != msaGraph.end()){ //i already has an entry in the map
                    msaGraph.at(rec_actor).push_back(succRec);
                }else{ //no entry for i yet
                    vector<SuccessorNode> succRecv;
                    succRecv.push_back(succRec);
                    msaGraph.insert(pair<int, vector<SuccessorNode>>(rec_actor, succRecv));
                }
            }

            channel_count += 3;
        }else if(sendingTime[i].min() == 0){ //Step 1b: add all edges from G to the MSAG
            if(!sendingTime[i].assigned() || (sendingTime[i].assigned() && tok[i] > 0) || (sendingTime[i].assigned() && !next[ch_src[i]].assigned())){
                //ch_src[i] -> ch_dst[i]: add channel destination as successor node of the channel source
                SuccessorNode _dst;
                _dst.successor_key = ch_dst[i];
                _dst.delay = wcet[ch_dst[i]].min();
                _dst.min_tok = tok[i];
                _dst.max_tok = tok[i];
                _dst.channel = i;

                //add to boost-msag
                src = b::vertex(ch_src[i], b_msag);
                dst = b::vertex(ch_dst[i], b_msag);
                b::tie(_e, found) = b::add_edge(src, dst, b_msag);
                b::put(b::edge_weight, b_msag, _e, wcet[ch_dst[i]].min());
                b::put(b::edge_weight2, b_msag, _e, tok[i]);

                n_msagChannels++;
                if(printDebug){
                    unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(ch_src[i]);
                    if(it != msaGraph.end()){ //i already has an entry in the map
                        msaGraph.at(ch_src[i]).push_back(_dst);
                    }else{ //no entry for i yet
                        vector<SuccessorNode> dstv;
                        dstv.push_back(_dst);
                        msaGraph.insert(pair<int, vector<SuccessorNode>>(ch_src[i], dstv));
                    }
                }
            }
        }
    }
    /*
     for(unsigned int i=0; i<receivingActors.size(); i++){
     cout << receivingActors[i] << " ";
     }
     cout << endl;*/

    /*
     cout << "channelMapping.size() = " << channelMapping.size() << endl;
     for(unsigned int i=0; i<channelMapping.size(); i+=3){
     cout << channelMapping[i] << " ";
     }
     cout << endl;*/

    //put sendNext relations into the MSAG
    for(unsigned int i = 1; i < channelMapping.size(); i += 3){ //for all sending actors
        bool continues = true;
        bool nextFound = false;
        int nextCh;
        int x = channelMapping[i];
        int tokens = 0; //is channel to add a cycle-closing back-edge?
        while(!nextFound && continues){
            if(sendingNext[x].assigned()){
                nextCh = sendingNext[x].val();
                if(nextCh >= n_channels){ //end of chain found
                    if(nextCh > n_channels){
                        nextCh = n_channels + ((nextCh - n_channels - 1) % n_procs);
                    }else{
                        nextCh = n_channels + n_procs - 1;
                    }
                    tokens = 1;
                    if(sendingNext[nextCh].assigned()){
                        nextCh = sendingNext[nextCh].val();
                        if(sendingTime[nextCh].min() > 0){
                            nextFound = true;
                        }else{
                            x = nextCh;
                        }
                    }else{
                        continues = false;
                    }
                }else{ //not end of chain (nextCh < n_channels)
                    if(sendingTime[nextCh].min() > 0){
                        nextFound = true;
                        if(tokens != 1)
                            tokens = 0;
                    }else{
                        x = nextCh; //nextCh is not on interconnect. Continue with nextSend[nextCh].
                    }
                }
            }else{
                continues = false;
            }
        }
        if(nextFound){
            //add send_actor of channel i -> block_actor of nextCh
            if(channelMapping[i] != nextCh){ //if found successor is not the channel's own block_actor (then it is already in the graph)
                int block_actor = getBlockActor(nextCh);

                //cout << "Next channel (send_actor of channel " << i << "): " << nextCh << endl;

                SuccessorNode succBS;
                succBS.successor_key = block_actor;
                succBS.delay = sendingLatency[nextCh].min();
                succBS.min_tok = tokens;
                succBS.max_tok = tokens;
                succBS.channel = nextCh;

                //add to boost-msag
                src = b::vertex(i + n_actors, b_msag);
                dst = b::vertex(block_actor, b_msag);
                b::tie(_e, found) = b::add_edge(src, dst, b_msag);
                b::put(b::edge_weight, b_msag, _e, sendingLatency[nextCh].min());
                b::put(b::edge_weight2, b_msag, _e, tokens);

                n_msagChannels++;
                if(printDebug){
                    unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(i + n_actors);
                    if(it != msaGraph.end()){ //send actor already has an entry in the map
                        msaGraph.at(i + n_actors).push_back(succBS);
                    }else{ //no entry for send_actor i yet
                        vector<SuccessorNode> succBSv;
                        succBSv.push_back(succBS);
                        msaGraph.insert(pair<int, vector<SuccessorNode>>(i + n_actors, succBSv));
                    }
                }
            }
        }
    }

    //put recNext relations into the MSAG
    for(unsigned int i = 2; i < channelMapping.size(); i += 3){ //for all receiving actors
        bool nextFound = false;
        int nextCh;
        int x = channelMapping[i];

        //cout << "looking for recNext, channel " << i << endl;

        while(!nextFound){
            if(receivingNext[x].assigned()){
                nextCh = receivingNext[x].val();
                if(nextCh >= n_channels){ //end of chain found
                    nextCh = -1; //nextCh = ch_dst[channelMapping[i]];
                    nextFound = true;
                }else{ //not end of chain (nextCh < n_channels)
                    if(ch_dst[nextCh] != ch_dst[channelMapping[i]]){ //next rec actor belongs to other dst
                        nextCh = -1; //nextCh = ch_dst[channelMapping[i]];
                        nextFound = true;
                    }else{ //same dst
                        if(sendingTime[nextCh].min() > 0){
                            nextFound = true;
                        }else{
                            x = nextCh; //nextCh is not on interconnect. Continue with nextSend[nextCh].
                        }
                    }
                }
            }else{
                nextCh = -1; //nextCh = ch_dst[channelMapping[i]];
                nextFound = true;
            }
        }

        //cout << "  found " << nextCh;

        SuccessorNode succRec;
        succRec.successor_key = nextCh == -1 ? ch_dst[channelMapping[i]] : getRecActor(nextCh);
        succRec.delay = nextCh == -1 ? wcet[ch_dst[channelMapping[i]]].min() : receivingTime[nextCh].min();
        succRec.min_tok = 0;
        succRec.max_tok = 0;
        if(nextCh != -1)
            succRec.channel = nextCh;

        //cout << " ( "<< succRec.successor_key <<")" << endl;

        //add to boost-msag
        src = b::vertex(i + n_actors, b_msag);
        dst = b::vertex(nextCh == -1 ? ch_dst[channelMapping[i]] : getRecActor(nextCh), b_msag);
        b::tie(_e, found) = b::add_edge(src, dst, b_msag);
        b::put(b::edge_weight, b_msag, _e, nextCh == -1 ? wcet[ch_dst[channelMapping[i]]].min() : receivingTime[nextCh].min());
        b::put(b::edge_weight2, b_msag, _e, 0);

        n_msagChannels++;
        if(printDebug){
            unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(i + n_actors);
            if(it != msaGraph.end()){ //send actor already has an entry in the map
                msaGraph.at(i + n_actors).push_back(succRec);
            }else{ //no entry for send_actor i yet
                vector<SuccessorNode> succRecv;
                succRecv.push_back(succRec);
                msaGraph.insert(pair<int, vector<SuccessorNode>>(i + n_actors, succRecv));
            }
        }
    }

    for(int i = 0; i < n_actors; i++){
        //Step 2
        if(next[i].assigned() && next[i].val() < n_actors){ //if next[i] is decided, the forward edge goes from i to next[i]
            int nextActor = next[i].val();

            //check whether nextActor has preceding rec_actor
            SuccessorNode nextA;
            if(receivingActors[nextActor] == -1){
                //add edge i -> nextActor
                nextA.successor_key = nextActor;
                nextA.delay = wcet[nextActor].min();
                nextA.min_tok = 0;
                nextA.max_tok = 0;

                //add to boost-msag
                src = b::vertex(i, b_msag);
                dst = b::vertex(nextActor, b_msag);
                b::tie(_e, found) = b::add_edge(src, dst, b_msag);
                b::put(b::edge_weight, b_msag, _e, wcet[nextActor].min());
                b::put(b::edge_weight2, b_msag, _e, 0);
            }else{
                //add edge i -> receivingActor[nextActor]
                nextA.successor_key = receivingActors[nextActor];
                nextA.delay = receivingTime[channelMapping[receivingActors[nextActor] - n_actors]].min();
                nextA.min_tok = 0;
                nextA.max_tok = 0;
                nextA.channel = channelMapping[receivingActors[nextActor] - n_actors];

                //add to boost-msag
                src = b::vertex(i, b_msag);
                dst = b::vertex(receivingActors[nextActor], b_msag);
                b::tie(_e, found) = b::add_edge(src, dst, b_msag);
                b::put(b::edge_weight, b_msag, _e, receivingTime[channelMapping[receivingActors[nextActor] - n_actors]].min());
                b::put(b::edge_weight2, b_msag, _e, 0);
            }

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(i);
                if(it != msaGraph.end()){ //i already has an entry in the map
                    msaGraph.at(i).push_back(nextA);
                }else{ //no entry for i yet
                    vector<SuccessorNode> nextAv;
                    nextAv.push_back(nextA);
                    msaGraph.insert(pair<int, vector<SuccessorNode>>(i, nextAv));
                }
            }

        }else if(next[i].assigned() && next[i].val() >= n_actors){ //next[i]>=n_actors
        //Step 3: add cycle-closing edge on each proc
            int firstActor = next[i].val();
            if(firstActor > n_actors){
                firstActor = n_actors + ((firstActor - n_actors - 1) % n_procs);
            }else{
                firstActor = n_actors + n_procs - 1;
            }
            if(next[firstActor].assigned()){
                firstActor = next[firstActor].val();

                //check whether firstActor has preceding rec_actor
                SuccessorNode first;
                if(receivingActors[firstActor] == -1){
                    //add edge i -> firstActor
                    first.successor_key = firstActor;
                    first.delay = wcet[firstActor].min();
                    first.min_tok = 1;
                    first.max_tok = 1;

                    //add to boost-msag
                    src = b::vertex(i, b_msag);
                    dst = b::vertex(firstActor, b_msag);
                    b::tie(_e, found) = b::add_edge(src, dst, b_msag);
                    b::put(b::edge_weight, b_msag, _e, wcet[firstActor].min());
                    b::put(b::edge_weight2, b_msag, _e, 1);
                }else{
                    //add edge i -> receivingActor[firstActor]
                    first.successor_key = receivingActors[firstActor];
                    first.delay = receivingTime[channelMapping[receivingActors[firstActor] - n_actors]].min();
                    first.min_tok = 1;
                    first.max_tok = 1;
                    first.channel = channelMapping[receivingActors[firstActor] - n_actors];

                    //add to boost-msag
                    src = b::vertex(i, b_msag);
                    dst = b::vertex(receivingActors[firstActor], b_msag);
                    b::tie(_e, found) = b::add_edge(src, dst, b_msag);
                    b::put(b::edge_weight, b_msag, _e, receivingTime[channelMapping[receivingActors[firstActor] - n_actors]].min());
                    b::put(b::edge_weight2, b_msag, _e, 1);
                }

                n_msagChannels++;
                if(printDebug){
                    unordered_map<int, vector<SuccessorNode>>::const_iterator it = msaGraph.find(i);
                    if(it != msaGraph.end()){ //i already has an entry in the map
                        msaGraph.at(i).push_back(first);
                    }else{    //no entry for i yet
                        vector<SuccessorNode> firstv;
                        firstv.push_back(first);
                        msaGraph.insert(pair<int, vector<SuccessorNode>>(i, firstv));
                    }
                }
            }
        }
    }

    if(printDebug){
        //printThroughputGraphAsDot(".");
    }
}

struct G {
  typedef typename b::property_map<boost_msag, vertex_actorid_t>::const_type IdMap;
  map<int, b::graph_traits<boost_msag>::vertex_descriptor> vertices;

  void addVertex(int id, b::graph_traits<boost_msag>::vertex_descriptor vertex){
    vertices[id] = vertex;
  }
  int getId(b::graph_traits<boost_msag>::vertex_descriptor vertex, const boost_msag& graph) {
    IdMap  id = b::get(vertex_actorid, graph);
    auto id_ = b::get(id, vertex);
    return id_;
  }
  b::graph_traits<boost_msag>::vertex_descriptor getVertex(int id) {
    return vertices[id];
  }
};

void ThroughputMCR::constructMSAG(vector<int> &msagMap) {


  if (printDebug)
    cout << "\tThroughputMCR::constructMSAG(vector<int> &msagMap)" << endl;

  msaGraph.clear();
  receivingActors.clear();
  channelMapping.clear();
  receivingActors.insert(receivingActors.begin(), n_actors, -1); //pre-fill with -1
  //to identify for each msag-actor, which msag it belongs to
  vector<int> msagId;

  //first, figure out how many actors there will be in the MSAG
  n_msagActors = n_actors;
  for (int i = 0; i < sendingTime.size(); i++) {
    if (sendingTime[i].min() > 0) { //=> channel on interconnect
      n_msagActors += 3; //one blocking, one sending and one receiving actor
      //store mapping between block/send/rec_actor and channel i
      channelMapping.push_back(i); //[block_actor] = i;
      channelMapping.push_back(i); //[send_actor] = i;
      channelMapping.push_back(i); //[rec_actor] = i;
    }
  }

  for (int i = 0; i < n_msagActors; i++) {
    msagId.push_back(msagMap[getApp(i)]);
  }

  b::graph_traits<boost_msag>::vertex_descriptor src, dst;
  b::graph_traits<boost_msag>::edge_descriptor _e;
  G g;

  //add all actors as vertices, and self-loops
  bool found;
  for (int n = 0; n < n_msagActors; n++) {
    boost_msag& curr_graph = *b_msags[msagId[n]];

    cout << n << " is part of msag " << msagId[n] << " ... ";

    src = add_vertex(curr_graph);
    b::put(vertex_actorid, curr_graph, src, n);
    g.addVertex(n,src);
    //add self-edges
    tie(_e, found) = add_edge(src, src, curr_graph);
    cout << "vertex = " << src << "(id:" << g.getId(src, curr_graph) << "); found: " << found << endl;
    if (n < n_actors) {
      b::put(b::edge_weight, curr_graph, _e, wcet[n].min());
    } //else{}: delay for communication actors are added further down
    b::put(b::edge_weight2, curr_graph, _e, 1);
  }
  //next: add edges to boost-msag

  for (size_t t = 0; t < b_msags.size(); t++) {
    string graphName = "1_boost_msag" + to_string(t);
    ofstream out;
    string outputFile = ".";
    outputFile +=
        (outputFile.back() == '/') ?
            (graphName + ".dot") : ("/" + graphName + ".dot");
    out.open(outputFile.c_str());
    write_graphviz(out, *b_msags[t]);
    out.close();
  }

  channel_count = 0;
  n_msagChannels = 0; //to count the number of channels in the MSAG

  //building the throughput analysis graph
  /* Step 1a: check sendingTime-array for all messages and add block-, send- & receive-"actors" with back-edges (buffering)
   Step 1b: check for dependencies in application graph that are not covered in 1a
   Step 2: check for decided forward-path in next-array
   Step 3: close execution cycles with back-edges found in next-array (next[i], i>=n_actors)
   */
  for (int i = 0; i < sendingTime.size(); i++) {
    if (sendingTime[i].min() > 0) { //Step1a: => channel on interconnect

      int block_actor = n_actors + channel_count;
      int send_actor = block_actor + 1;
      int rec_actor = send_actor + 1;
      //add the block actor as a successor of ch_src[i]
      SuccessorNode succB;
      succB.successor_key = block_actor;
      succB.delay = sendingLatency[i].min();
      succB.min_tok = 0;
      succB.max_tok = 0;
      succB.channel = i;

      //add to boost-msag
      boost_msag& curr_graph = *b_msags[msagId[ch_src[i]]];
      src = g.getVertex(ch_src[i]);    //b::vertex(ch_src[i], *b_msags[msagId[ch_src[i]]]);
      dst = g.getVertex(block_actor);  //b::vertex(block_actor, *b_msags[msagId[ch_src[i]]]);
      b::tie(_e, found) = b::add_edge(src, dst, curr_graph);
      b::put(b::edge_weight,  curr_graph, _e, sendingLatency[i].min());
      b::put(b::edge_weight2, curr_graph, _e, 0);
      //delay-weight for self-loop on block-actor:
      curr_graph = *b_msags[msagId[block_actor]];
      tie(_e, found) = edge(dst, dst, curr_graph);
      b::put(b::edge_weight, curr_graph, _e, sendingLatency[i].min());

      n_msagChannels++;
      if (printDebug) {
        unordered_map<int, vector<SuccessorNode>>::const_iterator it =
            msaGraph.find(ch_src[i]);
        if (it != msaGraph.end()) {    //i already has an entry in the map
          msaGraph.at(ch_src[i]).push_back(succB);
        } else {      //no entry for ch_src[i] yet
          vector<SuccessorNode> succBv;
          succBv.push_back(succB);
          msaGraph.insert(pair<int, vector<SuccessorNode>>(ch_src[i], succBv));
        }
      }

      //add ch_src[i] as successor of the block actor, with buffer size as tokens
      SuccessorNode srcCh;
      srcCh.successor_key = ch_src[i];
      srcCh.delay = wcet[ch_src[i]].assigned() ? wcet[ch_src[i]].val() : wcet[ch_src[i]].min();
      srcCh.min_tok = sendbufferSz[i].min();
      srcCh.max_tok = sendbufferSz[i].max();

      //add to boost-msag
      boost_msag& curr_graph1 = *b_msags[msagId[block_actor]];
      src = g.getVertex(block_actor);  //b::vertex(block_actor, *b_msags[msagId[block_actor]]);
      dst = g.getVertex(ch_src[i]);    //b::vertex(ch_src[i], *b_msags[msagId[block_actor]]);
      b::tie(_e, found) = b::add_edge(src, dst, *b_msags[msagId[block_actor]]);
      b::put(b::edge_weight, curr_graph1, _e, wcet[ch_src[i]].min());
      b::put(b::edge_weight2, curr_graph1, _e, sendbufferSz[i].max());

      n_msagChannels++;
      if (printDebug) {
        unordered_map<int, vector<SuccessorNode>>::const_iterator it =  msaGraph.find(block_actor);
        if (it != msaGraph.end()) {    //i already has an entry in the map
          msaGraph.at(block_actor).push_back(srcCh);
        } else {      //no entry for block_actor yet
          vector<SuccessorNode> srcChv;
          srcChv.push_back(srcCh);
          msaGraph.insert(pair<int, vector<SuccessorNode>>(block_actor, srcChv));
        }
      }
//###
      //add the send actor as a successor of the block actor
      SuccessorNode succS;
      succS.successor_key = send_actor;
      succS.delay = sendingTime[i].min();
      succS.min_tok = 0;
      succS.max_tok = 0;
      succS.channel = i;

      //add to boost-msag
      boost_msag& curr_graph2 = *b_msags[msagId[block_actor]];
      src = g.getVertex(block_actor);  //b::vertex(block_actor, *b_msags[msagId[block_actor]]);
      dst = g.getVertex(send_actor);   //b::vertex(send_actor, *b_msags[msagId[block_actor]]);
      b::tie(_e, found) = b::add_edge(src, dst, curr_graph2);
      b::put(b::edge_weight,  curr_graph2, _e, sendingTime[i].min());
      b::put(b::edge_weight2, curr_graph2, _e, 0);
      //delay-weight for self-loop on send-actor:
      tie(_e, found) = edge(dst, dst, curr_graph2);
      b::put(b::edge_weight, curr_graph2, _e, sendingTime[i].min());

      n_msagChannels++;
      if (printDebug) {
        unordered_map<int, vector<SuccessorNode>>::const_iterator it =
            msaGraph.find(block_actor);
        if (it != msaGraph.end()) {    //i already has an entry in the map
          msaGraph.at(block_actor).push_back(succS);
        } else {      //no entry for block_actor yet
          vector<SuccessorNode> succSv;
          succSv.push_back(succS);
          msaGraph.insert(
              pair<int, vector<SuccessorNode>>(block_actor, succSv));
        }
      }

      //add the block actor as successor of the send actor, with one token (serialization)
      SuccessorNode succBS;
      succBS.successor_key = block_actor;
      succBS.delay = sendingLatency[i].min();
      succBS.min_tok = 1;
      succBS.max_tok = 1;
      succBS.channel = i;

      //add to boost-msag
      boost_msag& curr_graph3 = *b_msags[msagId[send_actor]];
      src = g.getVertex(send_actor);   //b::vertex(send_actor, *b_msags[msagId[send_actor]]);
      dst = g.getVertex(block_actor);  //b::vertex(block_actor, *b_msags[msagId[send_actor]]);
      b::tie(_e, found) = b::add_edge(src, dst, *b_msags[msagId[send_actor]]);
      b::put(b::edge_weight,  curr_graph3, _e,sendingLatency[i].min());
      b::put(b::edge_weight2, curr_graph3, _e, 1);

      n_msagChannels++;
      if (printDebug) {
        unordered_map<int, vector<SuccessorNode>>::const_iterator it =
            msaGraph.find(send_actor);
        if (it != msaGraph.end()) { //send actor already has an entry in the map
          msaGraph.at(send_actor).push_back(succBS);
        } else {      //no entry for send_actor yet
          vector<SuccessorNode> succBSv;
          succBSv.push_back(succBS);
          msaGraph.insert(
              pair<int, vector<SuccessorNode>>(send_actor, succBSv));
        }
      }

      //add receiving actor as successor of the send actor, with potential initial tokens
      SuccessorNode dstCh;
      dstCh.successor_key = rec_actor;
      dstCh.delay = receivingTime[i].min();
      dstCh.min_tok = tok[i];
      dstCh.max_tok = tok[i];
      dstCh.channel = i;
      dstCh.recOrder = receivingNext[i].min();

      //add to boost-msag
      boost_msag& curr_graph4 = *b_msags[msagId[send_actor]];
      src = g.getVertex(send_actor);   //b::vertex(send_actor, *b_msags[msagId[send_actor]]);
      dst = g.getVertex(rec_actor);    //b::vertex(rec_actor, *b_msags[msagId[send_actor]]);
      b::tie(_e, found) = b::add_edge(src, dst, curr_graph4);
      b::put(b::edge_weight, curr_graph4, _e, receivingTime[i].min());
      b::put(b::edge_weight2, curr_graph4, _e, tok[i]);
      //delay-weight for self-loop on rec-actor:
      tie(_e, found) = edge(dst, dst, curr_graph4);
      b::put(b::edge_weight, curr_graph4, _e, receivingTime[i].min());

      n_msagChannels++;
      if (printDebug) {
        unordered_map<int, vector<SuccessorNode>>::const_iterator it =
            msaGraph.find(send_actor);
        if (it != msaGraph.end()) {    //i already has an entry in the map
          msaGraph.at(send_actor).push_back(dstCh);
        } else {      //no entry for i yet
          vector<SuccessorNode> dstChv;
          dstChv.push_back(dstCh);
          msaGraph.insert(pair<int, vector<SuccessorNode>>(send_actor, dstChv));
        }
      }

      //save the receiving actors for each actor (for next order)
      if (receivingActors[ch_dst[i]] == -1) { //first rec_actor for the dst
        receivingActors[ch_dst[i]] = rec_actor;
      } else {
        int curRec_actor_ch = channelMapping[receivingActors[ch_dst[i]]
            - n_actors];
        if (receivingNext[curRec_actor_ch].assigned()) {
          if (receivingNext[curRec_actor_ch].val() < n_channels) {
            if (ch_dst[receivingNext[curRec_actor_ch].val()] != ch_dst[i]) { //last rec_actor for this dst
              receivingActors[ch_dst[i]] = rec_actor;
            } //else
          } else { //last rec_actor for this dst
            receivingActors[ch_dst[i]] = rec_actor;
          }
        }
        if (receivingNext[channelMapping[rec_actor - n_actors]].assigned()) {
          if (receivingNext[channelMapping[rec_actor - n_actors]].val()
              == receivingActors[ch_dst[i]]) {
            receivingActors[ch_dst[i]] = rec_actor;
          }
        }
      }

      //add the send actor as a successor node of the receiving actor, with rec. buffer size - initial tokens
      SuccessorNode succRec;
      succRec.successor_key = send_actor;
      succRec.delay = sendingTime[i].min();
      succRec.min_tok = recbufferSz[i].min() - tok[i];
      succRec.max_tok = recbufferSz[i].max() - tok[i];
      succRec.channel = i;

      //add to boost-msag
      boost_msag& curr_graph5 = *b_msags[msagId[rec_actor]];
      src = g.getVertex(rec_actor);   //b::vertex(rec_actor, *b_msags[msagId[rec_actor]]);
      dst = g.getVertex(send_actor);   //b::vertex(send_actor, *b_msags[msagId[rec_actor]]);
      b::tie(_e, found) = b::add_edge(src, dst, curr_graph5);
      b::put(b::edge_weight,  curr_graph5, _e, sendingTime[i].min());
      b::put(b::edge_weight2, curr_graph5, _e, recbufferSz[i].max() - tok[i]);

      n_msagChannels++;
      if (printDebug) {
        unordered_map<int, vector<SuccessorNode>>::const_iterator it =
            msaGraph.find(rec_actor);
        if (it != msaGraph.end()) { //i already has an entry in the map
          msaGraph.at(rec_actor).push_back(succRec);
        } else { //no entry for i yet
          vector<SuccessorNode> succRecv;
          succRecv.push_back(succRec);
          msaGraph.insert(
              pair<int, vector<SuccessorNode>>(rec_actor, succRecv));
        }
      }

      channel_count += 3;
    } else if (sendingTime[i].min() == 0) { //Step 1b: add all edges from G to the MSAG
      if (!sendingTime[i].assigned()
          || (sendingTime[i].assigned() && tok[i] > 0)
          || (sendingTime[i].assigned() && !next[ch_src[i]].assigned())) {
        //ch_src[i] -> ch_dst[i]: add channel destination as successor node of the channel source
        SuccessorNode _dst;
        _dst.successor_key = ch_dst[i];
        _dst.delay = wcet[ch_dst[i]].min();
        _dst.min_tok = tok[i];
        _dst.max_tok = tok[i];
        _dst.channel = i;

        //add to boost-msag
        boost_msag& curr_graph5 = *b_msags[msagId[ch_src[i]]];
        src = g.getVertex(ch_src[i]);   //b::vertex(ch_src[i], *b_msags[msagId[ch_src[i]]]);
        dst = g.getVertex(ch_dst[i]);   //b::vertex(ch_dst[i], *b_msags[msagId[ch_src[i]]]);
        b::tie(_e, found) = b::add_edge(src, dst, curr_graph5);
        b::put(b::edge_weight,  curr_graph5, _e, wcet[ch_dst[i]].min());
        b::put(b::edge_weight2, curr_graph5, _e, tok[i]);

        n_msagChannels++;
        if (printDebug) {
          unordered_map<int, vector<SuccessorNode>>::const_iterator it =
              msaGraph.find(ch_src[i]);
          if (it != msaGraph.end()) { //i already has an entry in the map
            msaGraph.at(ch_src[i]).push_back(_dst);
          } else { //no entry for i yet
            vector<SuccessorNode> dstv;
            dstv.push_back(_dst);
            msaGraph.insert(pair<int, vector<SuccessorNode>>(ch_src[i], dstv));
          }
        }
      }
    }
  }
  /*
   for(unsigned int i=0; i<receivingActors.size(); i++){
   cout << receivingActors[i] << " ";
   }
   cout << endl;*/

  /*
   cout << "channelMapping.size() = " << channelMapping.size() << endl;
   for(unsigned int i=0; i<channelMapping.size(); i+=3){
   cout << channelMapping[i] << " ";
   }
   cout << endl;*/

  //put sendNext relations into the MSAG
  for (unsigned int i = 1; i < channelMapping.size(); i += 3) { //for all sending actors
    bool continues = true;
    bool nextFound = false;
    int nextCh;
    int x = channelMapping[i];
    int tokens = 0; //is channel to add a cycle-closing back-edge?
    while (!nextFound && continues) {
      if (sendingNext[x].assigned()) {
        nextCh = sendingNext[x].val();
        if (nextCh >= n_channels) { //end of chain found
          if (nextCh > n_channels) {
            nextCh = n_channels + ((nextCh - n_channels - 1) % n_procs);
          } else {
            nextCh = n_channels + n_procs - 1;
          }
          tokens = 1;
          if (sendingNext[nextCh].assigned()) {
            nextCh = sendingNext[nextCh].val();
            if (sendingTime[nextCh].min() > 0) {
              nextFound = true;
            } else {
              x = nextCh;
            }
          } else {
            continues = false;
          }
        } else { //not end of chain (nextCh < n_channels)
          if (sendingTime[nextCh].min() > 0) {
            nextFound = true;
            if (tokens != 1)
              tokens = 0;
          } else {
            x = nextCh; //nextCh is not on interconnect. Continue with nextSend[nextCh].
          }
        }
      } else {
        continues = false;
      }
    }
    if (nextFound) {
      //add send_actor of channel i -> block_actor of nextCh
      if (channelMapping[i] != nextCh) { //if found successor is not the channel's own block_actor (then it is already in the graph)
        int block_actor = getBlockActor(nextCh);

        //cout << "Next channel (send_actor of channel " << i << "): " << nextCh << endl;

        SuccessorNode succBS;
        succBS.successor_key = block_actor;
        succBS.delay = sendingLatency[nextCh].min();
        succBS.min_tok = tokens;
        succBS.max_tok = tokens;
        succBS.channel = nextCh;

        //add to boost-msag
        boost_msag& curr_graph6 = *b_msags[msagId[block_actor]];
        src = g.getVertex(i + n_actors);   //b::vertex(i + n_actors, *b_msags[msagId[block_actor]]);
        dst = g.getVertex(block_actor);    //b::vertex(block_actor, *b_msags[msagId[block_actor]]);
        b::tie(_e, found) = b::add_edge(src, dst,  curr_graph6);
        b::put(b::edge_weight,  curr_graph6, _e, sendingLatency[nextCh].min());
        b::put(b::edge_weight2, curr_graph6, _e, tokens);

        n_msagChannels++;
        if (printDebug) {
          unordered_map<int, vector<SuccessorNode>>::const_iterator it =
              msaGraph.find(i + n_actors);
          if (it != msaGraph.end()) { //send actor already has an entry in the map
            msaGraph.at(i + n_actors).push_back(succBS);
          } else { //no entry for send_actor i yet
            vector<SuccessorNode> succBSv;
            succBSv.push_back(succBS);
            msaGraph.insert(
                pair<int, vector<SuccessorNode>>(i + n_actors, succBSv));
          }
        }
      }
    }
  }

  //put recNext relations into the MSAG
  for (unsigned int i = 2; i < channelMapping.size(); i += 3) { //for all receiving actors
    bool nextFound = false;
    int nextCh;
    int x = channelMapping[i];

    //cout << "looking for recNext, channel " << i << endl;

    while (!nextFound) {
      if (receivingNext[x].assigned()) {
        nextCh = receivingNext[x].val();
        if (nextCh >= n_channels) { //end of chain found
          nextCh = -1; //nextCh = ch_dst[channelMapping[i]];
          nextFound = true;
        } else { //not end of chain (nextCh < n_channels)
          if (ch_dst[nextCh] != ch_dst[channelMapping[i]]) { //next rec actor belongs to other dst
            nextCh = -1; //nextCh = ch_dst[channelMapping[i]];
            nextFound = true;
          } else { //same dst
            if (sendingTime[nextCh].min() > 0) {
              nextFound = true;
            } else {
              x = nextCh; //nextCh is not on interconnect. Continue with nextSend[nextCh].
            }
          }
        }
      } else {
        nextCh = -1; //nextCh = ch_dst[channelMapping[i]];
        nextFound = true;
      }
    }

    //cout << "  found " << nextCh;

    SuccessorNode succRec;
    succRec.successor_key =
        nextCh == -1 ? ch_dst[channelMapping[i]] : getRecActor(nextCh);
    succRec.delay =
        nextCh == -1 ?
            wcet[ch_dst[channelMapping[i]]].min() : receivingTime[nextCh].min();
    succRec.min_tok = 0;
    succRec.max_tok = 0;
    if (nextCh != -1)
      succRec.channel = nextCh;

    //cout << " ( "<< succRec.successor_key <<")" << endl;

    //add to boost-msag
    int tmp = i + n_actors;
    boost_msag& curr_graph7 = *b_msags[msagId[tmp]];
    src = g.getVertex(tmp);            //b::vertex(tmp, *b_msags[msagId[tmp]]);
    dst = g.getVertex(nextCh == -1 ? ch_dst[channelMapping[i]] : getRecActor(nextCh));   //b::vertex(nextCh == -1 ? ch_dst[channelMapping[i]] : getRecActor(nextCh),*b_msags[msagId[tmp]]);
    b::tie(_e, found) = b::add_edge(src, dst, curr_graph7);
    b::put(b::edge_weight,  curr_graph7, _e,nextCh == -1 ?wcet[ch_dst[channelMapping[i]]].min() : receivingTime[nextCh].min());
    b::put(b::edge_weight2, curr_graph7, _e, 0);

    n_msagChannels++;
    if (printDebug) {
      unordered_map<int, vector<SuccessorNode>>::const_iterator it =
          msaGraph.find(i + n_actors);
      if (it != msaGraph.end()) { //send actor already has an entry in the map
        msaGraph.at(i + n_actors).push_back(succRec);
      } else { //no entry for send_actor i yet
        vector<SuccessorNode> succRecv;
        succRecv.push_back(succRec);
        msaGraph.insert(
            pair<int, vector<SuccessorNode>>(i + n_actors, succRecv));
      }
    }
  }

  for (int i = 0; i < n_actors; i++) {
    //Step 2
    if (next[i].assigned() && next[i].val() < n_actors) { //if next[i] is decided, the forward edge goes from i to next[i]
      int nextActor = next[i].val();

      //check whether nextActor has preceding rec_actor
      SuccessorNode nextA;
      if (receivingActors[nextActor] == -1) {
        //add edge i -> nextActor
        nextA.successor_key = nextActor;
        nextA.delay = wcet[nextActor].min();
        nextA.min_tok = 0;
        nextA.max_tok = 0;

        //add to boost-msag
        boost_msag& curr_graph8 = *b_msags[msagId[i]];
        src = g.getVertex(i);         //b::vertex(i, *b_msags[msagId[i]]);
        dst = g.getVertex(nextActor); //b::vertex(nextActor, *b_msags[msagId[i]]);
        b::tie(_e, found) = b::add_edge(src, dst, curr_graph8);
        b::put(b::edge_weight,  curr_graph8, _e, wcet[nextActor].min());
        b::put(b::edge_weight2, curr_graph8, _e, 0);
      } else {
        //add edge i -> receivingActor[nextActor]
        nextA.successor_key = receivingActors[nextActor];
        nextA.delay = receivingTime[channelMapping[receivingActors[nextActor]
            - n_actors]].min();
        nextA.min_tok = 0;
        nextA.max_tok = 0;
        nextA.channel = channelMapping[receivingActors[nextActor] - n_actors];

        //add to boost-msag
        boost_msag& curr_graph9 = *b_msags[msagId[i]];
        src = g.getVertex(i);                          //b::vertex(i, *b_msags[msagId[i]]);
        dst = g.getVertex(receivingActors[nextActor]); //b::vertex(receivingActors[nextActor], *b_msags[msagId[i]]);
        b::tie(_e, found) = b::add_edge(src, dst, curr_graph9);
        b::put(b::edge_weight,  curr_graph9, _e, receivingTime[channelMapping[receivingActors[nextActor] - n_actors]].min());
        b::put(b::edge_weight2, curr_graph9, _e, 0);
      }

      n_msagChannels++;
      if (printDebug) {
        unordered_map<int, vector<SuccessorNode>>::const_iterator it =
            msaGraph.find(i);
        if (it != msaGraph.end()) { //i already has an entry in the map
          msaGraph.at(i).push_back(nextA);
        } else { //no entry for i yet
          vector<SuccessorNode> nextAv;
          nextAv.push_back(nextA);
          msaGraph.insert(pair<int, vector<SuccessorNode>>(i, nextAv));
        }
      }

    } else if (next[i].assigned() && next[i].val() >= n_actors) { //next[i]>=n_actors
    //Step 3: add cycle-closing edge on each proc
      int firstActor = next[i].val();
      if (firstActor > n_actors) {
        firstActor = n_actors + ((firstActor - n_actors - 1) % n_procs);
      } else {
        firstActor = n_actors + n_procs - 1;
      }
      if (next[firstActor].assigned()) {
        firstActor = next[firstActor].val();

        //check whether firstActor has preceding rec_actor
        SuccessorNode first;
        if (receivingActors[firstActor] == -1) {
          //add edge i -> firstActor
          first.successor_key = firstActor;
          first.delay = wcet[firstActor].min();
          first.min_tok = 1;
          first.max_tok = 1;

          //add to boost-msag
          boost_msag& curr_graph10 = *b_msags[msagId[i]];
          src = g.getVertex(i);          //b::vertex(i, *b_msags[msagId[i]]);
          dst = g.getVertex(firstActor); //b::vertex(firstActor, *b_msags[msagId[i]]);
          b::tie(_e, found) = b::add_edge(src, dst, curr_graph10);
          b::put(b::edge_weight,  curr_graph10, _e, wcet[firstActor].min());
          b::put(b::edge_weight2, curr_graph10, _e, 1);
        } else {
          //add edge i -> receivingActor[firstActor]
          first.successor_key = receivingActors[firstActor];
          first.delay = receivingTime[channelMapping[receivingActors[firstActor]
              - n_actors]].min();
          first.min_tok = 1;
          first.max_tok = 1;
          first.channel =
              channelMapping[receivingActors[firstActor] - n_actors];

          //add to boost-msag
          boost_msag& curr_graph11 = *b_msags[msagId[i]];
          src = g.getVertex(i);                           //b::vertex(i, *b_msags[msagId[i]]);
          dst = g.getVertex(receivingActors[firstActor]); //b::vertex(receivingActors[firstActor], *b_msags[msagId[i]]);
          b::tie(_e, found) = b::add_edge(src, dst, curr_graph11);
          b::put(b::edge_weight,  curr_graph11, _e, receivingTime[channelMapping[receivingActors[firstActor] - n_actors]].min());
          b::put(b::edge_weight2, curr_graph11, _e, 1);
        }

        n_msagChannels++;
        if (printDebug) {
          unordered_map<int, vector<SuccessorNode>>::const_iterator it =
              msaGraph.find(i);
          if (it != msaGraph.end()) { //i already has an entry in the map
            msaGraph.at(i).push_back(first);
          } else {    //no entry for i yet
            vector<SuccessorNode> firstv;
            firstv.push_back(first);
            msaGraph.insert(pair<int, vector<SuccessorNode>>(i, firstv));
          }
        }
      }
    }
  }

  if (printDebug) {
    //printThroughputGraphAsDot(".");
  }
}


void checkApp(int app, unordered_map<int, set<int>>& coMappedApps, vector<int>& uncheckedApps, set<int>& res) {
  res.insert(app);
  uncheckedApps[app] = 0;
  for (auto& appl : coMappedApps[app]) checkApp(appl, coMappedApps, uncheckedApps, res);
}

ExecStatus ThroughputMCR::propagate(Space& home, const ModEventDelta&) {
  if (printDebug)
    cout << "\tThroughputMCR::propagate()" << endl;

  //auto _start = std::chrono::high_resolution_clock::now(); //timer
  //int time; //runtime of period calculation

  vector<int> msagMap(apps.size(), 0);

  cout << apps.size() << " applications." << endl;

  if (apps.size() > 1) {
    //check which application graphs are mapped to same processor (= combined into the same MSAG)
    vector<set<int>> result;
    unordered_map<int, set<int>> coMappedApps;
    vector<int> uncheckedApps(apps.size(), 1);
    for (int a = 0; a < apps.size(); a++) {
      coMappedApps.insert(pair<int, set<int>>(a, set<int>()));
    }
    for (int i = 0; i < n_actors; i++) {
      if (next[i].assigned() && next[i].val() < n_actors) { //next[i] is decided and points to an application actor
        int actor = i;
        int nextActor = next[i].val();
        if (getApp(actor) != getApp(nextActor)) { //from different applications
          unordered_map<int, set<int>>::const_iterator it = coMappedApps.find(getApp(actor));
          if (it != coMappedApps.end()) { //i already has an entry in the map
            coMappedApps.at(getApp(actor)).insert(getApp(nextActor));
          } else { //no entry for ch_src[i] yet
            set<int> coApp;
            coApp.insert(getApp(nextActor));
            coMappedApps.insert(pair<int, set<int>>(getApp(actor), coApp));
          }
          it = coMappedApps.find(getApp(nextActor));
          if (it != coMappedApps.end()) { //i already has an entry in the map
            coMappedApps.at(getApp(nextActor)).insert(getApp(actor));
          } else { //no entry for ch_src[i] yet
            set<int> coApp;
            coApp.insert(getApp(actor));
            coMappedApps.insert(pair<int, set<int>>(getApp(nextActor), coApp));
          }
        }
      }
    }
    cout << coMappedApps.size() << " entries in coMappedApps" << endl;
    if (coMappedApps.size() > 0) {

      cout << "unchecked apps:" << tools::toString(uncheckedApps) << endl;

      int sum_unchecked = 0;
      for (int x : uncheckedApps) sum_unchecked += x;
      while (sum_unchecked) {

        for (auto& mapp : coMappedApps) {
          cout << "App " << mapp.first << " is" << (mapp.second.empty() ? " not " : " ") << "co-mapped with ";
          cout << (mapp.second.empty() ? string(" any other app") : tools::toString(mapp.second)) << endl;

          if (uncheckedApps[mapp.first]) {
            set<int> res;
            result.push_back(res);
            checkApp(mapp.first, coMappedApps, uncheckedApps, result.back());
          }
        }

        sum_unchecked = 0;
        for (int x : uncheckedApps) sum_unchecked += x;


      }

      /*for (auto it = coMappedApps.begin(); it != coMappedApps.end(); it++) {
        cout << "App " << it->first << " is ";
        if (it->second.empty())
          cout << "not ";
        cout << "co-mapped with ";
        if (it->second.empty())
          cout << "any other application " << endl;
        else
          cout << endl << "    ";
        for (auto iy = it->second.begin(); iy != it->second.end(); iy++) {
          cout << *iy;
        }
        cout << endl;

        if (it->second.empty() && !checkedApps[it->first]) {
          set<int> res;
          res.insert(it->first);
          result.push_back(res);
          checkedApps[it->first] = true;
        } else if (!checkedApps[it->first]) {
          set<int> t_apps = it->second;
          checkedApps[it->first] = true;

        }

        //int app = it->first;
        //                while(!t_apps.empty()){
        //                    unordered_map<int, set<int>>::iterator iy = coMappedApps.find(*t_apps.begin());
        //                    if(iy != coMappedApps.end() && iy->second.size()>0){ // has an entry in the map
        //                        it->second.insert(iy->second.begin(), iy->second.end());
        //                        t_apps.insert(iy->second.begin(), iy->second.end());
        //                        iy->second.clear();
        //                        t_apps.erase(t_apps.begin());
        //                    }
        //
        //                }

      }
      for (auto it = coMappedApps.begin(); it != coMappedApps.end(); it++) {
        if (it->second.size() > 0) {
          set<int> res = it->second;
          res.insert(it->first);
          result.push_back(res);
        }
      }
      cout << result.size() << " resulting graphs." << endl;*/
    } else {
      for (size_t i = 0; i < wc_period.size(); i++) {
        set<int> res;
        res.insert(i);
        result.push_back(res);
      }
    }
    cout << "~~~" << endl;
    for (size_t i = 0; i < result.size(); i++) {
      b_msags.push_back(new boost_msag());
      for (std::set<int>::iterator it = result[i].begin();
          it != result[i].end(); ++it) {
        std::cout << ' ' << *it;
        msagMap[*it] = i;
      }
      std::cout << '\n';
    }
    cout << "~~~" << endl;
    constructMSAG(msagMap);

    for (size_t i = 0; i < b_msags.size(); i++) {
      cout << "Graph " << i << endl;
      cout << "  Vertices number: " << num_vertices(*b_msags[i]) << endl;
      cout << "  Edges number: " << num_edges(*b_msags[i]) << endl;
    }

    cout << "+~+~+~" << endl;
    if (printDebug) {
      //if(next.assigned() && wcet.assigned()){
      cout << "trying to print " << b_msags.size() << " boost-msags." << endl;
      for (size_t t = 0; t < b_msags.size(); t++) {
        string graphName = "boost_msag" + to_string(t);
        ofstream out;
        string outputFile = ".";
        outputFile +=
            (outputFile.back() == '/') ?
                (graphName + ".dot") : ("/" + graphName + ".dot");
        out.open(outputFile.c_str());
        write_graphviz(out, *b_msags[t]);
        out.close();
      }
      printThroughputGraphAsDot(".");
      //}
    }

  } else { //only a single application
    constructMSAG();
    using namespace boost;
    int max_cr; /// maximum cycle ratio
    typedef std::vector<graph_traits<boost_msag>::edge_descriptor> t_critCycl;
    t_critCycl cc; ///critical cycle

    property_map<boost_msag, vertex_index_t>::type vim = get(vertex_index,
        b_msag);
    property_map<boost_msag, edge_weight_t>::type ew1 = get(edge_weight,
        b_msag);
    property_map<boost_msag, edge_weight2_t>::type ew2 = get(edge_weight2,
        b_msag);
    //do MCR analysis
    max_cr = maximum_cycle_ratio(b_msag, vim, ew1, ew2, &cc);
    wc_period[0] = max_cr;

    if (printDebug) {
      if (next.assigned() && wcet.assigned()) {
        string graphName = "boost_msag";
        ofstream out;
        string outputFile = ".";
        outputFile +=
            (outputFile.back() == '/') ?
                (graphName + ".dot") : ("/" + graphName + ".dot");
        out.open(outputFile.c_str());
        write_graphviz(out, b_msag);
        out.close();
        printThroughputGraphAsDot(".");
      }

      cout << "Maximum cycle ratio is " << max_cr << endl;
      cout << "Critical cycle:\n";
      for (t_critCycl::iterator itr = cc.begin(); itr != cc.end(); ++itr) {
        cout << "(" << vim[source(*itr, b_msag)] << ","
            << vim[target(*itr, b_msag)] << ") ";
      }
      cout << endl;
    }
  }

  //constructMSAG();

//    if(next.assigned() && wcet.assigned()){
//        printThroughputGraphAsDot(".");
//        getchar();
//    }

  bool all_assigned = next.assigned() && wcet.assigned()
      && sendingTime.assigned() && sendingLatency.assigned()
      && sendingNext.assigned() && receivingTime.assigned(); //&&
      //receivingNext.assigned() &&
      //sendbufferSz.assigned() &&
      //recbufferSz.assigned();
//cout << "next.assigned(): " << next.assigned() << endl;
//cout << "wcet.assigned(): " << wcet.assigned() << endl;
//cout << "sendingTime.assigned(): " << sendingTime.assigned() << endl;
//cout << "sendingLatency.assigned(): " << sendingLatency.assigned() << endl;
//cout << "sendingNext.assigned(): " << sendingNext.assigned() << endl;
//cout << "receivingTime.assigned(): " << receivingTime.assigned() << endl;
//cout << "receivingNext.assigned(): " << receivingNext.assigned() << endl;

//cout << "all_assigned: " << all_assigned << " with period = " << wc_period << endl;

  bool all_ch_local = true;
  for (int i = 0; i < n_channels; i++) {
    if (!sendingTime[i].assigned() || sendingTime[i].min() > 0) {
      all_ch_local = false;
    }
  }

  //propagate latency and period (bounds)
  for (int i = 0; i < apps.size(); i++) {
    /*if(!all_assigned && all_ch_local && wcet.assigned() && next.assigned()){
     //GECODE_ME_CHECK(latency[i].eq(home,wc_latency[i][0]));
     GECODE_ME_CHECK(period[i].eq(home,wc_period[i]));
     }else*/
    if (!all_assigned && !all_ch_local) {
//          GECODE_ME_CHECK(latency[i].gq(home, wc_latency[i][0]));
      //GECODE_ME_CHECK(period[i].gq(home, wc_period[i]));
    } else if (all_assigned) {
//          GECODE_ME_CHECK(latency[i].eq(home, wc_latency[i][0]));
      //GECODE_ME_CHECK(period[i].eq(home, wc_period[i]));
    }
  }
  /*
   //propagate bounds on iterations of entities and channels in time-based schedule
   for(int i=0; i<n_actors; i++){
   //if(next.assigned())GECODE_ME_CHECK(iterations[i].gq(home,min_iterations[i]));
   //if(all_assigned)GECODE_ME_CHECK(iterations[i].gq(home,min_iterations[i]));
   if(all_assigned)GECODE_ME_CHECK(iterations[i].lq(home,max_iterations[i])); //upper bound can increase from "no mapping" until "fixed mapping" (due to added communication delays)
   }
   for(int i=n_actors; i<n_msagActors; i++){
   //for now, only consider the sending part for each channel
   if((i-n_actors)%2==0){
   //if(next.assigned())GECODE_ME_CHECK(iterationsCh[channelMapping[i]].gq(home,min_iterations[i]));
   //if(all_assigned)GECODE_ME_CHECK(iterationsCh[channelMapping[i]].gq(home,min_iterations[i]));
   if(all_assigned)GECODE_ME_CHECK(iterationsCh[channelMapping[i]].lq(home,max_iterations[i])); //upper bound can increase from "no mapping" until "fixed mapping" (due to added communication delays)
   }
   }*/

  b_msag.clear();
  for (size_t t = 0; t < b_msags.size(); t++) {
    //b_msags[t]->clear();
    delete b_msags[t];
  }
  b_msags.clear();
  msaGraph.clear();
  channelMapping.clear();
  receivingActors.clear();

  max_start.clear();
  max_end.clear();
  min_start.clear();
  min_end.clear();
  start_pp.clear();
  end_pp.clear();
  min_iterations.clear();
  max_iterations.clear();
  min_send_buffer.clear();
  max_send_buffer.clear();
  min_rec_buffer.clear();
  max_rec_buffer.clear();

  if (next.assigned() && wcet.assigned() && sendingTime.assigned()
      && sendingLatency.assigned() && receivingTime.assigned()
      && receivingNext.assigned() && sendbufferSz.assigned()
      && recbufferSz.assigned())
    return home.ES_SUBSUMED(*this);

  if (all_ch_local && wcet.assigned() && next.assigned()) {
    return home.ES_SUBSUMED(*this);
  }

  return ES_FIX;
}

/* next: |#actors|
 * execCycles: |#actors*#actors|
 * wcet: |#actors|
 * sendingTime: |#channels|
 * tok: |#channels|
 */
void throughputMCR(Space& home, const IntVar latency, const IntVar period,
        const IntVarArgs& iterations, //min/max
        const IntVarArgs& iterationsCh, //min/max
        const IntVarArgs& sendbufferSz, const IntVarArgs& recbufferSz, const IntVarArgs& next, const IntVarArgs& wcet, const IntVarArgs& sendingTime,
        const IntVarArgs& sendingLatency, const IntVarArgs& sendingNext, const IntVarArgs& receivingTime, const IntVarArgs& receivingNext,
        const IntArgs& ch_src, const IntArgs& ch_dst, const IntArgs& tok, const IntArgs& minIndices, const IntArgs& maxIndices) {
    if(iterations.size() != wcet.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, iterations & next");
    }
    if(iterationsCh.size() != sendingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, iterationsCh & sendingTime");
    }
    if(sendingTime.size() != sendingLatency.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & sendingLatency");
    }
    if(sendingTime.size() != receivingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & receivingTime");
    }
    if(sendingNext.size() != receivingNext.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & receivingNext");
    }
    if(sendingTime.size() != sendbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & sendbufferSz");
    }
    if(sendingTime.size() != recbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & recbufferSz");
    }
    if(sendingTime.size() != ch_src.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & ch_src");
    }
    if(sendingTime.size() != ch_dst.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & ch_dst");
    }
    if(sendingTime.size() != tok.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & tok");
    }
    if(minIndices.size() != maxIndices.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, minIndices & maxIndices");
    }

    if(home.failed())
        return;

//create variable views
    //Int::IntView _latency(latency);
    //Int::IntView _period(period);
    ViewArray<Int::IntView> _latency(home, 1);
    _latency[0] = latency;
    ViewArray<Int::IntView> _period(home, 1);
    _period[0] = period;
    ViewArray<Int::IntView> _iterations(home, iterations);
    ViewArray<Int::IntView> _iterationsCh(home, iterationsCh);
    ViewArray<Int::IntView> _sendbufferSz(home, sendbufferSz);
    ViewArray<Int::IntView> _recbufferSz(home, recbufferSz);
    ViewArray<Int::IntView> _next(home, next);
    ViewArray<Int::IntView> _wcet(home, wcet);
    ViewArray<Int::IntView> _sendingTime(home, sendingTime);
    ViewArray<Int::IntView> _sendingLatency(home, sendingLatency);
    ViewArray<Int::IntView> _sendingNext(home, sendingNext);
    ViewArray<Int::IntView> _receivingTime(home, receivingTime);
    ViewArray<Int::IntView> _receivingNext(home, receivingNext);
    IntArgs apps(1, wcet.size() - 1);
    if(ThroughputMCR::post(home, _latency, _period, _iterations, _iterationsCh, _sendbufferSz, _recbufferSz, _next, _wcet, _sendingTime, _sendingLatency,
            _sendingNext, _receivingTime, _receivingNext, ch_src, ch_dst, tok, apps, minIndices, maxIndices) != ES_OK){
        home.fail();
    }
}

/* next: |#actors|+|#procs|
 * wcet: |#actors|
 * sendingTime: |#channels|
 * tok: |#channels|
 */
void throughputMCR(Space& home, const IntVar latency, const IntVar period,
        const IntVarArgs& iterations, //min/max
        const IntVarArgs& iterationsCh, //min/max
        const IntVarArgs& sendbufferSz, const IntVarArgs& recbufferSz, const IntVarArgs& next, const IntVarArgs& wcet, const IntVarArgs& sendingTime,
        const IntVarArgs& sendingLatency, const IntVarArgs& sendingNext, const IntVarArgs& receivingTime, const IntVarArgs& receivingNext,
        const IntArgs& ch_src, const IntArgs& ch_dst, const IntArgs& tok) {
    if(iterations.size() != wcet.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, iterations & next");
    }
    if(iterationsCh.size() != sendingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, iterationsCh & sendingTime");
    }
    if(sendingTime.size() != sendingLatency.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & sendingLatency");
    }
    if(sendingTime.size() != receivingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & receivingTime");
    }
    if(sendingNext.size() != receivingNext.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingNext & receivingNext");
    }
    if(sendingTime.size() != sendbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & sendbufferSz");
    }
    if(sendingTime.size() != recbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & recbufferSz");
    }
    if(sendingTime.size() != ch_src.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & ch_src");
    }
    if(sendingTime.size() != ch_dst.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & ch_dst");
    }
    if(sendingTime.size() != tok.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & tok");
    }

    if(home.failed())
        return;

//create variable views
    //Int::IntView _latency(latency);
    //Int::IntView _period(period);
    ViewArray<Int::IntView> _latency(home, 1);
    _latency[0] = latency;
    ViewArray<Int::IntView> _period(home, 1);
    _period[0] = period;
    ViewArray<Int::IntView> _iterations(home, iterations);
    ViewArray<Int::IntView> _iterationsCh(home, iterationsCh);
    ViewArray<Int::IntView> _sendbufferSz(home, sendbufferSz);
    ViewArray<Int::IntView> _recbufferSz(home, recbufferSz);
    ViewArray<Int::IntView> _next(home, next);
    ViewArray<Int::IntView> _wcet(home, wcet);
    ViewArray<Int::IntView> _sendingTime(home, sendingTime);
    ViewArray<Int::IntView> _sendingLatency(home, sendingLatency);
    ViewArray<Int::IntView> _sendingNext(home, sendingNext);
    ViewArray<Int::IntView> _receivingTime(home, receivingTime);
    ViewArray<Int::IntView> _receivingNext(home, receivingNext);
    IntArgs minIndices, maxIndices;
    IntArgs apps(1, wcet.size() - 1);
    if(ThroughputMCR::post(home, _latency, _period, _iterations, _iterationsCh, _sendbufferSz, _recbufferSz, _next, _wcet, _sendingTime, _sendingLatency,
            _sendingNext, _receivingTime, _receivingNext, ch_src, ch_dst, tok, apps, minIndices, maxIndices) != ES_OK){
        home.fail();
    }
}

/* next: |#actors|+|#procs|
 * wcet: |#actors|
 * sendingTime: |#channels|
 * tok: |#channels|
 */
void throughputMCR(Space& home, const IntVarArgs& latency, const IntVarArgs& period,
        const IntVarArgs& iterations, //min/max
        const IntVarArgs& iterationsCh, //min/max
        const IntVarArgs& sendbufferSz, const IntVarArgs& recbufferSz, const IntVarArgs& next, const IntVarArgs& wcet, const IntVarArgs& sendingTime,
        const IntVarArgs& sendingLatency, const IntVarArgs& sendingNext, const IntVarArgs& receivingTime, const IntVarArgs& receivingNext,
        const IntArgs& ch_src, const IntArgs& ch_dst, const IntArgs& tok, const IntArgs& apps) {
    if(latency.size() != period.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, latency & period");
    }
    if(latency.size() != apps.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, latency & apps");
    }
    if(iterations.size() != wcet.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, iterations & next");
    }
    if(iterationsCh.size() != sendingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, iterationsCh & sendingTime");
    }
    if(sendingTime.size() != sendingLatency.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & sendingLatency");
    }
    if(sendingTime.size() != receivingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & receivingTime");
    }
    if(sendingNext.size() != receivingNext.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingNext & receivingNext");
    }
    if(sendingTime.size() != sendbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & sendbufferSz");
    }
    if(sendingTime.size() != recbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & recbufferSz");
    }
    if(sendingTime.size() != ch_src.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & ch_src");
    }
    if(sendingTime.size() != ch_dst.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & ch_dst");
    }
    if(sendingTime.size() != tok.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint, sendingTime & tok");
    }

    if(home.failed())
        return;

//create variable views
    ViewArray<Int::IntView> _latency(home, latency);
    ViewArray<Int::IntView> _period(home, period);
    ViewArray<Int::IntView> _iterations(home, iterations);
    ViewArray<Int::IntView> _iterationsCh(home, iterationsCh);
    ViewArray<Int::IntView> _sendbufferSz(home, sendbufferSz);
    ViewArray<Int::IntView> _recbufferSz(home, recbufferSz);
    ViewArray<Int::IntView> _next(home, next);
    ViewArray<Int::IntView> _wcet(home, wcet);
    ViewArray<Int::IntView> _sendingTime(home, sendingTime);
    ViewArray<Int::IntView> _sendingLatency(home, sendingLatency);
    ViewArray<Int::IntView> _sendingNext(home, sendingNext);
    ViewArray<Int::IntView> _receivingTime(home, receivingTime);
    ViewArray<Int::IntView> _receivingNext(home, receivingNext);
    IntArgs minIndices, maxIndices;
    if(ThroughputMCR::post(home, _latency, _period, _iterations, _iterationsCh, _sendbufferSz, _recbufferSz, _next, _wcet, _sendingTime, _sendingLatency,
            _sendingNext, _receivingTime, _receivingNext, ch_src, ch_dst, tok, apps, minIndices, maxIndices) != ES_OK){
        home.fail();
    }
}

int ThroughputMCR::getBlockActor(int ch_id) const {
    auto it = find(channelMapping.begin(), channelMapping.end(), ch_id);
    if(it != channelMapping.end())
        return distance(channelMapping.begin(), it) + n_actors;

    return -1;
}

int ThroughputMCR::getSendActor(int ch_id) const {
    auto it = find(channelMapping.begin(), channelMapping.end(), ch_id);
    if(it != channelMapping.end())
        return distance(channelMapping.begin(), it) + n_actors + 1;

    return -1;
}

int ThroughputMCR::getRecActor(int ch_id) const {
    auto it = find(channelMapping.begin(), channelMapping.end(), ch_id);
    if(it != channelMapping.end())
        return distance(channelMapping.begin(), it) + n_actors + 2;

    return -1;
}

int ThroughputMCR::getApp(int msagActor_id) const {
    int id = msagActor_id;
    if(msagActor_id >= n_actors){
        id = ch_dst[channelMapping[msagActor_id - n_actors]];
    }
    for(int i = 0; i < apps.size(); i++){
        if(id <= apps[i])
            return i;
    }
    return -1;
}

void ThroughputMCR::printThroughputGraph() const {
    cout << "-------------------------------------------------" << endl;
    for(auto it = msaGraph.begin(); it != msaGraph.end(); ++it){
        int node = it->first;
        cout << node << ": ";
        vector<SuccessorNode> succs = (vector<SuccessorNode> ) (it->second);
        for(auto itV = succs.begin(); itV != succs.end(); ++itV){
            cout << "(";
            if(((SuccessorNode) (*itV)).successor_key < n_actors){
                cout << ((SuccessorNode) (*itV)).successor_key << "; ";
            }else{
                int src = ch_src[((SuccessorNode) (*itV)).channel];
                int dst = ch_dst[((SuccessorNode) (*itV)).channel];
                if(((((SuccessorNode) (*itV)).successor_key - n_actors) % 3) == 0){
                    cout << "b" << src << "-" << dst;
                }else if(((((SuccessorNode) (*itV)).successor_key - n_actors) % 3) == 1){
                    cout << "s" << src << "-" << dst;
                }else{
                    cout << "r" << src << "-" << dst;
                }
                cout << "[" << ((SuccessorNode) (*itV)).successor_key << "]; ";
            }
            cout << ((SuccessorNode)(*itV)).delay << "; ";
            //cout << ((SuccessorNode) (*itV)).min_tok << "; ";
            cout << ((SuccessorNode) (*itV)).max_tok << ")";
        }
        cout << endl;
    }
}

void ThroughputMCR::printThroughputGraphAsDot(const string &dir) const {

    string graphName = "throughputGraph";
    ofstream out;
    string outputFile = dir;
    outputFile += (outputFile.back() == '/') ? (graphName + ".dot") : ("/" + graphName + ".dot");
    out.open(outputFile.c_str());

    out << "digraph " << graphName << " {" << endl;
    out << "    size=\"7,10\";" << endl;
    //out << "    rankdir=\"LR\"" << endl;

    //Output actors
    for(int i = 0; i < n_msagActors; i++){
        string actorName;
        int col = 0;
        if(i < n_actors){
            actorName = "actor_" + to_string(i);
            col = (getApp(i) + 1) % 32;
        }else if(i >= n_actors && (i - n_actors) % 3 == 0){ //blocking node
            actorName = "block_ch" + to_string(channelMapping[i - n_actors]);
            col = -1;
        }else if(i >= n_actors && (i - n_actors) % 3 == 1){ //sending node
            actorName = "send_ch" + to_string(channelMapping[i - n_actors]);
            col = -2;
        }else if(i >= n_actors && (i - n_actors) % 3 == 2){ //receiving node
            actorName = "rec_ch" + to_string(channelMapping[i - n_actors]);
            col = -3;
        }

        out << "    " << actorName << " [ label=\"" << actorName << "\"";
        out << ", style=filled";
        out << ", fillcolor=\"";
        if(col == -1){
            out << "#999999";
        }else if(col == -2){
            out << "#CCCCCC";
        }else if(col == -3){
            out << "#FFFFFF";
        }else if(col > 0 && col < 13){
            // (See ColorBrewer license)
            out << "/set312/" << col;
        }else if(col > 12 && col < 24){
            // (See ColorBrewer license)
            out << "/spectral11/" << col - 12;
        }else{ //col>23
               // (See ColorBrewer license)
            out << "/set19/" << col - 23;
        }
        out << "\"";
        out << "];" << endl;
    }
    out << endl;

    for(auto it = msaGraph.begin(); it != msaGraph.end(); ++it){
        string srcName;
        int node = it->first;
        if(node < n_actors){
            srcName = "actor_" + to_string(node);
        }else if(node >= n_actors && (node - n_actors) % 3 == 0){ //blocking node
            srcName = "block_ch" + to_string(channelMapping[node - n_actors]);
        }else if(node >= n_actors && (node - n_actors) % 3 == 1){ //sending node
            srcName = "send_ch" + to_string(channelMapping[node - n_actors]);
        }else if(node >= n_actors && (node - n_actors) % 3 == 2){ //receiving node
            srcName = "rec_ch" + to_string(channelMapping[node - n_actors]);
        }
        vector<SuccessorNode> succs = (vector<SuccessorNode> ) (it->second);
        for(auto itV = succs.begin(); itV != succs.end(); ++itV){
            int node2 = ((SuccessorNode) (*itV)).successor_key;
            string dstName;
            if(node2 < n_actors){
                dstName = "actor_" + to_string(node2);
            }else if(node2 >= n_actors && (node2 - n_actors) % 3 == 0){ //blocking node
                dstName = "block_ch" + to_string(channelMapping[node2 - n_actors]);
            }else if(node2 >= n_actors && (node2 - n_actors) % 3 == 1){ //sending node
                dstName = "send_ch" + to_string(channelMapping[node2 - n_actors]);
            }else if(node2 >= n_actors && (node2 - n_actors) % 3 == 2){ //receiving node
                dstName = "rec_ch" + to_string(channelMapping[node2 - n_actors]);
            }
            int tok = ((SuccessorNode) (*itV)).max_tok;
            // Initial tokens on channel?
            if(tok != 0){
                string label;
                out << "    " << srcName << " -> " << dstName;
                if(node >= n_actors && (node - n_actors) % 3 == 0 && node2 < n_actors){
                    label = "send_buff (ch" + to_string(channelMapping[node - n_actors]) + ")";
                    out << " [ label=\"" << label << "\"];" << endl;
                }else if(node >= n_actors && (node - n_actors) % 3 == 2 && node2 >= n_actors && (node2 - n_actors) % 3 == 1){
                    label = "rec_buff (ch" + to_string(channelMapping[node2 - n_actors]) + ")";
                    out << " [ label=\"" << label << "\"];" << endl;
                }else{
                    label = to_string(tok);
                    out << " [ label=\"" << "init(";
                    out << label << ")" << "\"];" << endl;
                }

            }else{
                out << "    " << srcName << " -> " << dstName << ";" << endl;
            }
        }
    }

    for(int i = 0; i < n_actors; i++){
        bool close = false;
        if(next[i].assigned() && next[i].val() < n_actors){
            out << "    { rank=same; ";
            out << "actor_" + to_string(i) << " ";
            out << "actor_" + to_string(next[i].val()) << " ";
            close = true;
        }
        for(int j = 0; j < n_channels; j++){
            if(i == ch_dst[j] && getRecActor(j) != -1){
                if(!close){
                    out << "    { rank=same; ";
                    out << "actor_" + to_string(i) << " ";
                }
                out << "rec_ch" + to_string(j) << " ";
                close = true;
            }
        }
        if(close)
            out << "}" << endl;
    }

    out << "}" << endl;

    out.close();

    cout << "  Printed dot graph file " << outputFile << endl;

}
