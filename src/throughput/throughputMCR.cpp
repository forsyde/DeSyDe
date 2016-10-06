#include "throughputMCR.hpp"

using namespace Gecode;
using namespace Int;
using namespace std;

ThroughputMCR::ThroughputMCR(Space& home, ViewArray<IntView> p_latency,
        ViewArray<IntView> p_period, ViewArray<IntView> p_iterations,
        ViewArray<IntView> p_iterationsCh, ViewArray<IntView> p_sendbufferSz,
        ViewArray<IntView> p_recbufferSz,
        ViewArray<IntView> p_next, ViewArray<IntView> p_wcet,
        ViewArray<IntView> p_sendingTime, ViewArray<IntView> p_sendingLatency,
        ViewArray<IntView> p_sendingNext, ViewArray<IntView> p_receivingTime,
        ViewArray<IntView> p_receivingNext,
        ViewArray<IntView> p_timedSched_start,
        ViewArray<IntView> p_timedSched_end,
        ViewArray<IntView> p_timedSched_IC_start,
        ViewArray<IntView> p_timedSched_IC_end,
        ViewArray<IntView> p_periodicSched_start,
        ViewArray<IntView> p_periodicSched_end,
        ViewArray<IntView> p_periodicSched_IC_start,
        ViewArray<IntView> p_periodicSched_IC_end, IntArgs p_ch_src,
        IntArgs p_ch_dst, IntArgs p_tok, IntArgs p_apps, IntArgs p_minIndices,
        IntArgs p_maxIndices) :
        Propagator(home), latency(p_latency), period(p_period), iterations(
                p_iterations), iterationsCh(p_iterationsCh), sendbufferSz(
                p_sendbufferSz), recbufferSz(p_recbufferSz), next(p_next), wcet(
                p_wcet), sendingTime(p_sendingTime), sendingLatency(
                p_sendingLatency), sendingNext(p_sendingNext), receivingTime(
                p_receivingTime), receivingNext(p_receivingNext), timedSched_start(
                p_timedSched_start), timedSched_end(p_timedSched_end), timedSched_IC_start(
                p_timedSched_IC_start), timedSched_IC_end(p_timedSched_IC_end), periodicSched_start(
                p_periodicSched_start), periodicSched_end(p_periodicSched_end), periodicSched_IC_start(
                p_periodicSched_IC_start), periodicSched_IC_end(
                p_periodicSched_IC_end), ch_src(p_ch_src), ch_dst(p_ch_dst), tok(
                p_tok), apps(p_apps), minIndices(p_minIndices), maxIndices(
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

    calls = 0;
    total_time = 0;

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

    msaGraph.~unordered_map<int, vector<SuccessorNode>>();
    channelMapping.~vector<int>();
    receivingActors.~vector<int>();
    ch_state.~vector<int>();
    actor_delay.~vector<int>();

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
        Propagator(home, share, p), ch_src(p.ch_src), ch_dst(p.ch_dst), tok(
                p.tok), apps(p.apps), minIndices(p.minIndices), maxIndices(
                p.maxIndices), n_actors(p.n_actors), n_channels(p.n_channels), n_procs(
                p.n_procs), n_msagActors(p.n_msagActors), msaGraph(p.msaGraph), channelMapping(
                p.channelMapping), receivingActors(p.receivingActors), ch_state(
                p.ch_state), actor_delay(p.actor_delay), wc_latency(
                p.wc_latency), wc_period(p.wc_period), calls(p.calls), total_time(
                p.total_time), printDebug(p.printDebug) {
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
    timedSched_start.update(home, share, p.timedSched_start);
    timedSched_end.update(home, share, p.timedSched_end);
    timedSched_IC_start.update(home, share, p.timedSched_IC_start);
    timedSched_IC_end.update(home, share, p.timedSched_IC_end);
    periodicSched_start.update(home, share, p.periodicSched_start);
    periodicSched_end.update(home, share, p.periodicSched_end);
    periodicSched_IC_start.update(home, share, p.periodicSched_IC_start);
    periodicSched_IC_end.update(home, share, p.periodicSched_IC_end);
}

Propagator* ThroughputMCR::copy(Space& home, bool share) {
    return new (home) ThroughputMCR(home, share, *this);
}

//TODO: do this right
PropCost ThroughputMCR::cost(const Space& home,
        const ModEventDelta& med) const {
    return PropCost::linear(PropCost::HI, next.size());
}

void ThroughputMCR::debug_constructMSAG() {
//if(printDebug) cout << "\tThroughputMCR::debug_constructMSAG()" << endl;
//first, figure out how many actors there will be in the MSAG, in order to
    //initialize channel-matrix and actor-vector for the state of MCR
    n_msagActors = n_actors;
    for(int i = 0; i < sendingTime.size(); i++){
        if(sendingTime[i].min() > 0){ //=> channel on interconnect
            n_msagActors += 3; //one blocking, one sending and one receiving actor
        }
    }

    msaGraph.clear();
    ch_state.clear();
    actor_delay.clear();
    receivingActors.clear();
    channelMapping.clear();

    ch_state.insert(ch_state.begin(), n_msagActors * n_msagActors, -1);
    actor_delay.insert(actor_delay.begin(), n_msagActors, 0);
    receivingActors.insert(receivingActors.begin(), n_actors, -1); //pre-fill with -1

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

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(ch_src[i]);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(ch_src[i]).push_back(succB);
                }else{      //no entry for ch_src[i] yet
                    vector<SuccessorNode> succBv;
                    succBv.push_back(succB);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(ch_src[i],
                                    succBv));
                }
            }
            //add ch_src[i]->block_actor to state of SSE
            ch_state[ch_src[i] * n_msagActors + block_actor] = succB.max_tok;
            actor_delay[ch_src[i]] = wcet[ch_src[i]].min();
            actor_delay[block_actor] = sendingLatency[i].min();

            //add ch_src[i] as successor of the block actor, with buffer size as tokens
            SuccessorNode srcCh;
            srcCh.successor_key = ch_src[i];
            srcCh.delay =
                    wcet[ch_src[i]].assigned() ?
                            wcet[ch_src[i]].val() : wcet[ch_src[i]].min();
            srcCh.min_tok = sendbufferSz[i].min();
            srcCh.max_tok = sendbufferSz[i].max();

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(block_actor);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(block_actor).push_back(srcCh);
                }else{      //no entry for block_actor yet
                    vector<SuccessorNode> srcChv;
                    srcChv.push_back(srcCh);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(block_actor,
                                    srcChv));
                }
            }
            //add block_actor->ch_src[i] to state of SSE
            ch_state[block_actor * n_msagActors + ch_src[i]] = srcCh.max_tok;
            actor_delay[block_actor] = sendingLatency[i].min();
            actor_delay[ch_src[i]] = wcet[ch_src[i]].min();
//###
            //add the send actor as a successor of the block actor
            SuccessorNode succS;
            succS.successor_key = send_actor;
            succS.delay = sendingTime[i].min();
            succS.min_tok = 0;
            succS.max_tok = 0;
            succS.channel = i;

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(block_actor);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(block_actor).push_back(succS);
                }else{      //no entry for block_actor yet
                    vector<SuccessorNode> succSv;
                    succSv.push_back(succS);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(block_actor,
                                    succSv));
                }
            }
            //add block_actor->send_actor to state of SSE
            ch_state[block_actor * n_msagActors + send_actor] = succS.max_tok;
            actor_delay[block_actor] = sendingLatency[i].min();
            actor_delay[send_actor] = sendingTime[i].min();

            //add the block actor as successor of the send actor, with one token (serialization)
            SuccessorNode succBS;
            succBS.successor_key = block_actor;
            succBS.delay = sendingLatency[i].min();
            succBS.min_tok = 1;
            succBS.max_tok = 1;
            succBS.channel = i;

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(send_actor);
                if(it != msaGraph.end()){ //send actor already has an entry in the map
                    msaGraph.at(send_actor).push_back(succBS);
                }else{      //no entry for send_actor yet
                    vector<SuccessorNode> succBSv;
                    succBSv.push_back(succBS);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(send_actor,
                                    succBSv));
                }
            }
            //add send_actor -> block_actor to state of SSE
            ch_state[send_actor * n_msagActors + block_actor] = succBS.max_tok;
            actor_delay[send_actor] = sendingTime[i].min();
            actor_delay[block_actor] = sendingLatency[i].min();
//###
            //add receiving actor as successor of the send actor, with potential initial tokens
            SuccessorNode dstCh;
            dstCh.successor_key = rec_actor;
            dstCh.delay = receivingTime[i].min();
            dstCh.min_tok = tok[i];
            dstCh.max_tok = tok[i];
            dstCh.channel = i;
            dstCh.recOrder = receivingNext[i].min();

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(send_actor);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(send_actor).push_back(dstCh);
                }else{      //no entry for i yet
                    vector<SuccessorNode> dstChv;
                    dstChv.push_back(dstCh);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(send_actor,
                                    dstChv));
                }
            }
            //add send_actor->rec_actor to state of SSE
            ch_state[send_actor * n_msagActors + rec_actor] = dstCh.max_tok;
            actor_delay[send_actor] = sendingTime[i].min();
            actor_delay[rec_actor] = receivingTime[i].min();

            //save the receiving actors for each actor (for next order)
            if(receivingActors[ch_dst[i]] == -1){ //first rec_actor for the dst
                receivingActors[ch_dst[i]] = rec_actor;
            }else{
                int curRec_actor_ch = channelMapping[receivingActors[ch_dst[i]]
                        - n_actors];

                //debug
                if(curRec_actor_ch < 0
                        || curRec_actor_ch >= receivingNext.size()){
                    cout
                            << "receivingNext[curRec_actor_ch] with curRec_actor_ch = "
                            << curRec_actor_ch << endl;
                }

                if(receivingNext[curRec_actor_ch].assigned()){
                    if(receivingNext[curRec_actor_ch].val() < n_channels){
                        if(ch_dst[receivingNext[curRec_actor_ch].val()]
                                != ch_dst[i]){ //last rec_actor for this dst
                            receivingActors[ch_dst[i]] = rec_actor;
                        } //else
                    }else{ //last rec_actor for this dst
                        receivingActors[ch_dst[i]] = rec_actor;
                    }
                }

                //debug
                if(channelMapping[rec_actor - n_actors] < 0
                        || channelMapping[rec_actor - n_actors]
                                >= receivingNext.size()){
                    cout
                            << "receivingNext[channelMapping[rec_actor-n_actors]] with"
                            << endl;
                    cout << "  channelMapping[rec_actor-n_actors] = "
                            << channelMapping[rec_actor - n_actors] << endl;
                    cout << "  rec_actor-n_actors = " << rec_actor - n_actors
                            << endl;
                }

                if(receivingNext[channelMapping[rec_actor - n_actors]].assigned()){
                    if(receivingNext[channelMapping[rec_actor - n_actors]].val()
                            == receivingActors[ch_dst[i]]){
                        receivingActors[ch_dst[i]] = rec_actor;
                    }
                }
            }

            //add the send actor as a successor node of the receiving actor, with rec. buffer size - initial tokens
            SuccessorNode succRec;
            succRec.successor_key = send_actor;
            succRec.delay = sendingLatency[i].min() + sendingTime[i].min();
            succRec.min_tok = recbufferSz[i].min() - tok[i];
            succRec.max_tok = recbufferSz[i].max() - tok[i];
            succRec.channel = i;

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(rec_actor);
                if(it != msaGraph.end()){ //i already has an entry in the map
                    msaGraph.at(rec_actor).push_back(succRec);
                }else{ //no entry for i yet
                    vector<SuccessorNode> succRecv;
                    succRecv.push_back(succRec);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(rec_actor,
                                    succRecv));
                }
            }
            //add rec_actor->send_actor to state of SSE
            ch_state[rec_actor * n_msagActors + send_actor] = succRec.max_tok;
            actor_delay[rec_actor] = receivingTime[i].min();
            actor_delay[send_actor] = sendingTime[i].min();

            channel_count += 3;
        }else if(sendingTime[i].min() == 0){ //Step 1b: add all edges from G to the MSAG
            if(!sendingTime[i].assigned()
                    || (sendingTime[i].assigned() && tok[i] > 0)
                    || (sendingTime[i].assigned() && !next[ch_src[i]].assigned())){
                //ch_src[i] -> ch_dst[i]: add channel destination as successor node of the channel source
                SuccessorNode dst;
                dst.successor_key = ch_dst[i];
                dst.delay = wcet[ch_dst[i]].min();
                dst.min_tok = tok[i];
                dst.max_tok = tok[i];
                dst.channel = i;

                n_msagChannels++;
                if(printDebug){
                    unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                            msaGraph.find(ch_src[i]);
                    if(it != msaGraph.end()){ //i already has an entry in the map
                        msaGraph.at(ch_src[i]).push_back(dst);
                    }else{ //no entry for i yet
                        vector<SuccessorNode> dstv;
                        dstv.push_back(dst);
                        msaGraph.insert(
                                pair<int, vector<SuccessorNode>>(ch_src[i],
                                        dstv));
                    }
                }
                //add ch_src[i]->ch_dst[i] to state of SSE
                ch_state[ch_src[i] * n_msagActors + ch_dst[i]] = tok[i];
                actor_delay[ch_src[i]] = wcet[ch_src[i]].min();
                actor_delay[ch_dst[i]] = wcet[ch_dst[i]].min();
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

            //debug
            if(x < 0 || x >= sendingNext.size()){
                cout << "sendingNext[x] with x = " << x << endl;
            }

            if(sendingNext[x].assigned()){
                nextCh = sendingNext[x].val();
                if(nextCh >= n_channels){ //end of chain found
                    if(nextCh > n_channels){
                        nextCh = n_channels
                                + ((nextCh - n_channels - 1) % n_procs);
                    }else{
                        nextCh = n_channels + n_procs - 1;
                    }
                    tokens = 1;

                    //debug
                    if(nextCh < 0 || nextCh >= sendingNext.size()){
                        cout << "sendingNext[nextCh] with nextCh = " << nextCh
                                << endl;
                    }
                    if(sendingNext[nextCh].assigned()){
                        nextCh = sendingNext[nextCh].val();

                        //debug
                        if(nextCh < 0 || nextCh >= sendingTime.size()){
                            cout << "sendingTime[nextCh] with nextCh = "
                                    << nextCh << endl;
                        }
                        if(sendingTime[nextCh].min() > 0){
                            nextFound = true;
                        }else{
                            x = nextCh;
                        }
                    }else{
                        continues = false;
                    }
                }else{ //not end of chain (nextCh < n_channels)

                    //debug
                    if(nextCh < 0 || nextCh >= sendingTime.size()){
                        cout << "sendingTime[nextCh] with nextCh = " << nextCh
                                << endl;
                    }
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

                //debug
                if(nextCh < 0 || nextCh >= sendingLatency.size()){
                    cout << "sendingLatency[nextCh] with nextCh = " << nextCh
                            << endl;
                }
                succBS.delay = sendingLatency[nextCh].min();
                succBS.min_tok = tokens;
                succBS.max_tok = tokens;
                succBS.channel = nextCh;

                n_msagChannels++;
                if(printDebug){
                    unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                            msaGraph.find(i + n_actors);
                    if(it != msaGraph.end()){ //send actor already has an entry in the map
                        msaGraph.at(i + n_actors).push_back(succBS);
                    }else{ //no entry for send_actor i yet
                        vector<SuccessorNode> succBSv;
                        succBSv.push_back(succBS);
                        msaGraph.insert(
                                pair<int, vector<SuccessorNode>>(i + n_actors,
                                        succBSv));
                    }
                }
                //add i -> block_actor to state of SSE
                ch_state[(i + n_actors) * n_msagActors + block_actor] =
                        succBS.max_tok;
                actor_delay[i + n_actors] =
                        sendingTime[channelMapping[i]].min();
                actor_delay[block_actor] = sendingLatency[nextCh].min();
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
            //debug
            if(x < 0 || x >= receivingNext.size()){
                cout << "receivingNext[x] with x = " << x << endl;
            }

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

                        //debug
                        if(nextCh < 0 || nextCh >= sendingTime.size()){
                            cout << "sendingTime[nextCh] with nextCh = "
                                    << nextCh << endl;
                        }
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
        //add rec_actor of channel i -> nextCh

        //cout << "  found " << nextCh;

        SuccessorNode succRec;
        succRec.successor_key =
                nextCh == -1 ? ch_dst[channelMapping[i]] : getRecActor(nextCh);
        succRec.delay =
                nextCh == -1 ?
                        wcet[ch_dst[channelMapping[i]]].min() :
                        receivingTime[nextCh].min();
        succRec.min_tok = 0;
        succRec.max_tok = 0;
        if(nextCh != -1)
            succRec.channel = nextCh;

        //cout << " ( "<< succRec.successor_key <<")" << endl;

        n_msagChannels++;
        if(printDebug){
            unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                    msaGraph.find(i + n_actors);
            if(it != msaGraph.end()){ //send actor already has an entry in the map
                msaGraph.at(i + n_actors).push_back(succRec);
            }else{ //no entry for send_actor i yet
                vector<SuccessorNode> succRecv;
                succRecv.push_back(succRec);
                msaGraph.insert(
                        pair<int, vector<SuccessorNode>>(i + n_actors,
                                succRecv));
            }
        }
        //add i -> block_actor to state of SSE
        ch_state[(i + n_actors) * n_msagActors + succRec.successor_key] =
                succRec.max_tok;
        actor_delay[i + n_actors] = receivingTime[channelMapping[i]].min();
        actor_delay[succRec.successor_key] = succRec.delay;
    }

    for(int i = 0; i < n_actors; i++){
        actor_delay[i] = wcet[i].min();
        //Step 2
        if(next[i].assigned() && next[i].val() < n_actors){ //if next[i] is decided, the forward edge goes from i to next[i]
            int nextActor = next[i].val();

            //check whether nextActor has preceding rec_actor
            SuccessorNode nextA;
            if(receivingActors[nextActor] == -1){
                //add edge i -> nextActor
                nextA.successor_key = nextActor;

                //debug
                if(nextActor < 0 || nextActor >= wcet.size()){
                    cout << "wcet[nextActor] with nextActor = " << nextActor
                            << endl;
                }
                nextA.delay = wcet[nextActor].min();
                nextA.min_tok = 0;
                nextA.max_tok = 0;
            }else{
                //add edge i -> receivingActor[nextActor]
                nextA.successor_key = receivingActors[nextActor];

                //debug
                if(channelMapping[receivingActors[nextActor] - n_actors] < 0
                        || channelMapping[receivingActors[nextActor] - n_actors]
                                >= receivingTime.size()){
                    cout
                            << "receivingTime[channelMapping[receivingActors[nextActor]-n_actors]] with channelMapping[receivingActors[nextActor]-n_actors] = "
                            << channelMapping[receivingActors[nextActor]
                                    - n_actors] << endl;
                }

                nextA.delay =
                        receivingTime[channelMapping[receivingActors[nextActor]
                                - n_actors]].min();
                nextA.min_tok = 0;
                nextA.max_tok = 0;
                nextA.channel = channelMapping[receivingActors[nextActor]
                        - n_actors];
            }

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(i);
                if(it != msaGraph.end()){ //i already has an entry in the map
                    msaGraph.at(i).push_back(nextA);
                }else{ //no entry for i yet
                    vector<SuccessorNode> nextAv;
                    nextAv.push_back(nextA);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(i, nextAv));
                }
            }
            //add i->nextA to state of SSE
            ch_state[i * n_msagActors + nextA.successor_key] = nextA.max_tok;
            actor_delay[i] = wcet[i].min();
            actor_delay[nextA.successor_key] = nextA.delay;

        }else if(next[i].assigned() && next[i].val() >= n_actors){ //next[i]>=n_actors
        //Step 3: add cycle-closing edge on each proc
            int firstActor = next[i].val();
            if(firstActor > n_actors){
                firstActor = n_actors + ((firstActor - n_actors - 1) % n_procs);
            }else{
                firstActor = n_actors + n_procs - 1;
            }

            //debug
            if(firstActor < 0 || firstActor >= next.size()){
                cout << "next[firstActor] with firstActor = " << firstActor
                        << endl;
            }
            if(next[firstActor].assigned()){

                int oldFirstActor = firstActor;
                firstActor = next[firstActor].val();

                //check whether firstActor has preceding rec_actor
                SuccessorNode first;
                if(receivingActors[firstActor] == -1){
                    //add edge i -> firstActor
                    first.successor_key = firstActor;

                    //debug
                    if(firstActor < 0 || firstActor >= wcet.size()){
                        cout << "wcet[firstActor] with firstActor = "
                                << firstActor << endl;
                    }
                    first.delay = wcet[firstActor].min();
                    first.min_tok = 1;
                    first.max_tok = 1;
                }else{

                    //add edge i -> receivingActor[firstActor]
                    first.successor_key = receivingActors[firstActor];

                    //debug
                    if(channelMapping[receivingActors[firstActor] - n_actors]
                            < 0
                            || channelMapping[receivingActors[firstActor]
                                    - n_actors] >= receivingTime.size()){
                        cout << "i: " << i << " next[i].val(): "
                                << next[i].val() << " firstActor: "
                                << oldFirstActor << endl;
                        cout
                                << "receivingTime[channelMapping[receivingActors[firstActor]-n_actors]] "
                                << endl;
                        cout
                                << "  with channelMapping[receivingActors[firstActor]-n_actors] = "
                                << channelMapping[receivingActors[firstActor]
                                        - n_actors] << endl;
                        cout << "  and receivingActors[firstActor] = "
                                << receivingActors[firstActor] << endl;
                        cout << "  and firstActor = " << firstActor << endl;

                        cout << "  and receivingActors = ";
                        for(int s = 0; s < next.size(); s++){
                            cout << "next[" << s << "] = " << next[s] << endl;
                        }
                        for(unsigned s = 0; s < receivingActors.size(); s++){
                            cout << receivingActors[s] << " ";
                        }
                        cout << endl;
                    }
                    first.delay =
                            receivingTime[channelMapping[receivingActors[firstActor]
                                    - n_actors]].min();
                    first.min_tok = 1;
                    first.max_tok = 1;
                    first.channel = channelMapping[receivingActors[firstActor]
                            - n_actors];
                }

                n_msagChannels++;
                if(printDebug){
                    unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                            msaGraph.find(i);
                    if(it != msaGraph.end()){ //i already has an entry in the map
                        msaGraph.at(i).push_back(first);
                    }else{    //no entry for i yet
                        vector<SuccessorNode> firstv;
                        firstv.push_back(first);
                        msaGraph.insert(
                                pair<int, vector<SuccessorNode>>(i, firstv));
                    }
                }
                //add i->ch_first to state of SSE
                ch_state[i * n_msagActors + first.successor_key] =
                        first.max_tok;
                actor_delay[i] = wcet[i].min();
                actor_delay[first.successor_key] = first.delay;
            }
        }
    }

    if(printDebug){
        /*cout << "initial state for SSE: " << endl;
         for(int i=0; i<n_msagActors; i++){
         for(int j=0; j<n_msagActors; j++){
         if(ch_state[i*n_msagActors+j] != -1)
         if(ch_state[i*n_msagActors+j]>10)
         cout << "B ";
         else
         cout << ch_state[i*n_msagActors+j] << " ";
         else
         cout << "X ";
         }
         cout << endl;
         }*/
        //printThroughputGraphAsDot(".");
    }
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

    msaGraph.clear();
    ch_state.clear();
    actor_delay.clear();
    receivingActors.clear();
    channelMapping.clear();

    ch_state.insert(ch_state.begin(), n_msagActors * n_msagActors, -1);
    actor_delay.insert(actor_delay.begin(), n_msagActors, 0);
    receivingActors.insert(receivingActors.begin(), n_actors, -1); //pre-fill with -1

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

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(ch_src[i]);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(ch_src[i]).push_back(succB);
                }else{      //no entry for ch_src[i] yet
                    vector<SuccessorNode> succBv;
                    succBv.push_back(succB);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(ch_src[i],
                                    succBv));
                }
            }
            //add ch_src[i]->block_actor to state of SSE
            ch_state[ch_src[i] * n_msagActors + block_actor] = succB.max_tok;
            actor_delay[ch_src[i]] = wcet[ch_src[i]].min();
            actor_delay[block_actor] = sendingLatency[i].min();

            //add ch_src[i] as successor of the block actor, with buffer size as tokens
            SuccessorNode srcCh;
            srcCh.successor_key = ch_src[i];
            srcCh.delay =
                    wcet[ch_src[i]].assigned() ?
                            wcet[ch_src[i]].val() : wcet[ch_src[i]].min();
            srcCh.min_tok = sendbufferSz[i].min();
            srcCh.max_tok = sendbufferSz[i].max();

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(block_actor);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(block_actor).push_back(srcCh);
                }else{      //no entry for block_actor yet
                    vector<SuccessorNode> srcChv;
                    srcChv.push_back(srcCh);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(block_actor,
                                    srcChv));
                }
            }
            //add block_actor->ch_src[i] to state of SSE
            ch_state[block_actor * n_msagActors + ch_src[i]] = srcCh.max_tok;
            actor_delay[block_actor] = sendingLatency[i].min();
            actor_delay[ch_src[i]] = wcet[ch_src[i]].min();
//###
            //add the send actor as a successor of the block actor
            SuccessorNode succS;
            succS.successor_key = send_actor;
            succS.delay = sendingTime[i].min();
            succS.min_tok = 0;
            succS.max_tok = 0;
            succS.channel = i;

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(block_actor);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(block_actor).push_back(succS);
                }else{      //no entry for block_actor yet
                    vector<SuccessorNode> succSv;
                    succSv.push_back(succS);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(block_actor,
                                    succSv));
                }
            }
            //add block_actor->send_actor to state of SSE
            ch_state[block_actor * n_msagActors + send_actor] = succS.max_tok;
            actor_delay[block_actor] = sendingLatency[i].min();
            actor_delay[send_actor] = sendingTime[i].min();

            //add the block actor as successor of the send actor, with one token (serialization)
            SuccessorNode succBS;
            succBS.successor_key = block_actor;
            succBS.delay = sendingLatency[i].min();
            succBS.min_tok = 1;
            succBS.max_tok = 1;
            succBS.channel = i;

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(send_actor);
                if(it != msaGraph.end()){ //send actor already has an entry in the map
                    msaGraph.at(send_actor).push_back(succBS);
                }else{      //no entry for send_actor yet
                    vector<SuccessorNode> succBSv;
                    succBSv.push_back(succBS);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(send_actor,
                                    succBSv));
                }
            }
            //add send_actor -> block_actor to state of SSE
            ch_state[send_actor * n_msagActors + block_actor] = succBS.max_tok;
            actor_delay[send_actor] = sendingTime[i].min();
            actor_delay[block_actor] = sendingLatency[i].min();
//###
            //add receiving actor as successor of the send actor, with potential initial tokens
            SuccessorNode dstCh;
            dstCh.successor_key = rec_actor;
            dstCh.delay = receivingTime[i].min();
            dstCh.min_tok = tok[i];
            dstCh.max_tok = tok[i];
            dstCh.channel = i;
            dstCh.recOrder = receivingNext[i].min();

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(send_actor);
                if(it != msaGraph.end()){    //i already has an entry in the map
                    msaGraph.at(send_actor).push_back(dstCh);
                }else{      //no entry for i yet
                    vector<SuccessorNode> dstChv;
                    dstChv.push_back(dstCh);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(send_actor,
                                    dstChv));
                }
            }
            //add send_actor->rec_actor to state of SSE
            ch_state[send_actor * n_msagActors + rec_actor] = dstCh.max_tok;
            actor_delay[send_actor] = sendingTime[i].min();
            actor_delay[rec_actor] = receivingTime[i].min();

            //save the receiving actors for each actor (for next order)
            if(receivingActors[ch_dst[i]] == -1){ //first rec_actor for the dst
                receivingActors[ch_dst[i]] = rec_actor;
            }else{
                int curRec_actor_ch = channelMapping[receivingActors[ch_dst[i]]
                        - n_actors];
                if(receivingNext[curRec_actor_ch].assigned()){
                    if(receivingNext[curRec_actor_ch].val() < n_channels){
                        if(ch_dst[receivingNext[curRec_actor_ch].val()]
                                != ch_dst[i]){ //last rec_actor for this dst
                            receivingActors[ch_dst[i]] = rec_actor;
                        } //else
                    }else{ //last rec_actor for this dst
                        receivingActors[ch_dst[i]] = rec_actor;
                    }
                }
                if(receivingNext[channelMapping[rec_actor - n_actors]].assigned()){
                    if(receivingNext[channelMapping[rec_actor - n_actors]].val()
                            == receivingActors[ch_dst[i]]){
                        receivingActors[ch_dst[i]] = rec_actor;
                    }
                }
            }

            //add the send actor as a successor node of the receiving actor, with rec. buffer size - initial tokens
            SuccessorNode succRec;
            succRec.successor_key = send_actor;
            succRec.delay = sendingLatency[i].min() + sendingTime[i].min();
            succRec.min_tok = recbufferSz[i].min() - tok[i];
            succRec.max_tok = recbufferSz[i].max() - tok[i];
            succRec.channel = i;

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(rec_actor);
                if(it != msaGraph.end()){ //i already has an entry in the map
                    msaGraph.at(rec_actor).push_back(succRec);
                }else{ //no entry for i yet
                    vector<SuccessorNode> succRecv;
                    succRecv.push_back(succRec);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(rec_actor,
                                    succRecv));
                }
            }
            //add rec_actor->send_actor to state of SSE
            ch_state[rec_actor * n_msagActors + send_actor] = succRec.max_tok;
            actor_delay[rec_actor] = receivingTime[i].min();
            actor_delay[send_actor] = sendingTime[i].min();

            channel_count += 3;
        }else if(sendingTime[i].min() == 0){ //Step 1b: add all edges from G to the MSAG
            if(!sendingTime[i].assigned()
                    || (sendingTime[i].assigned() && tok[i] > 0)
                    || (sendingTime[i].assigned() && !next[ch_src[i]].assigned())){
                //ch_src[i] -> ch_dst[i]: add channel destination as successor node of the channel source
                SuccessorNode dst;
                dst.successor_key = ch_dst[i];
                dst.delay = wcet[ch_dst[i]].min();
                dst.min_tok = tok[i];
                dst.max_tok = tok[i];
                dst.channel = i;

                n_msagChannels++;
                if(printDebug){
                    unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                            msaGraph.find(ch_src[i]);
                    if(it != msaGraph.end()){ //i already has an entry in the map
                        msaGraph.at(ch_src[i]).push_back(dst);
                    }else{ //no entry for i yet
                        vector<SuccessorNode> dstv;
                        dstv.push_back(dst);
                        msaGraph.insert(
                                pair<int, vector<SuccessorNode>>(ch_src[i],
                                        dstv));
                    }
                }
                //add ch_src[i]->ch_dst[i] to state of SSE
                ch_state[ch_src[i] * n_msagActors + ch_dst[i]] = tok[i];
                actor_delay[ch_src[i]] = wcet[ch_src[i]].min();
                actor_delay[ch_dst[i]] = wcet[ch_dst[i]].min();
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
                        nextCh = n_channels
                                + ((nextCh - n_channels - 1) % n_procs);
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

                n_msagChannels++;
                if(printDebug){
                    unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                            msaGraph.find(i + n_actors);
                    if(it != msaGraph.end()){ //send actor already has an entry in the map
                        msaGraph.at(i + n_actors).push_back(succBS);
                    }else{ //no entry for send_actor i yet
                        vector<SuccessorNode> succBSv;
                        succBSv.push_back(succBS);
                        msaGraph.insert(
                                pair<int, vector<SuccessorNode>>(i + n_actors,
                                        succBSv));
                    }
                }
                //add i -> block_actor to state of SSE
                ch_state[(i + n_actors) * n_msagActors + block_actor] =
                        succBS.max_tok;
                actor_delay[i + n_actors] =
                        sendingTime[channelMapping[i]].min();
                actor_delay[block_actor] = sendingLatency[nextCh].min();
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
        //add rec_actor of channel i -> nextCh

        //cout << "  found " << nextCh;

        SuccessorNode succRec;
        succRec.successor_key =
                nextCh == -1 ? ch_dst[channelMapping[i]] : getRecActor(nextCh);
        succRec.delay =
                nextCh == -1 ?
                        wcet[ch_dst[channelMapping[i]]].min() :
                        receivingTime[nextCh].min();
        succRec.min_tok = 0;
        succRec.max_tok = 0;
        if(nextCh != -1)
            succRec.channel = nextCh;

        //cout << " ( "<< succRec.successor_key <<")" << endl;

        n_msagChannels++;
        if(printDebug){
            unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                    msaGraph.find(i + n_actors);
            if(it != msaGraph.end()){ //send actor already has an entry in the map
                msaGraph.at(i + n_actors).push_back(succRec);
            }else{ //no entry for send_actor i yet
                vector<SuccessorNode> succRecv;
                succRecv.push_back(succRec);
                msaGraph.insert(
                        pair<int, vector<SuccessorNode>>(i + n_actors,
                                succRecv));
            }
        }
        //add i -> block_actor to state of SSE
        ch_state[(i + n_actors) * n_msagActors + succRec.successor_key] =
                succRec.max_tok;
        actor_delay[i + n_actors] = receivingTime[channelMapping[i]].min();
        actor_delay[succRec.successor_key] = succRec.delay;
    }

    for(int i = 0; i < n_actors; i++){
        actor_delay[i] = wcet[i].min();
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
            }else{
                //add edge i -> receivingActor[nextActor]
                nextA.successor_key = receivingActors[nextActor];
                nextA.delay =
                        receivingTime[channelMapping[receivingActors[nextActor]
                                - n_actors]].min();
                nextA.min_tok = 0;
                nextA.max_tok = 0;
                nextA.channel = channelMapping[receivingActors[nextActor]
                        - n_actors];
            }

            n_msagChannels++;
            if(printDebug){
                unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                        msaGraph.find(i);
                if(it != msaGraph.end()){ //i already has an entry in the map
                    msaGraph.at(i).push_back(nextA);
                }else{ //no entry for i yet
                    vector<SuccessorNode> nextAv;
                    nextAv.push_back(nextA);
                    msaGraph.insert(
                            pair<int, vector<SuccessorNode>>(i, nextAv));
                }
            }
            //add i->nextA to state of SSE
            ch_state[i * n_msagActors + nextA.successor_key] = nextA.max_tok;
            actor_delay[i] = wcet[i].min();
            actor_delay[nextA.successor_key] = nextA.delay;

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
                }else{
                    //add edge i -> receivingActor[firstActor]
                    first.successor_key = receivingActors[firstActor];
                    first.delay =
                            receivingTime[channelMapping[receivingActors[firstActor]
                                    - n_actors]].min();
                    first.min_tok = 1;
                    first.max_tok = 1;
                    first.channel = channelMapping[receivingActors[firstActor]
                            - n_actors];
                }

                n_msagChannels++;
                if(printDebug){
                    unordered_map<int, vector<SuccessorNode>>::const_iterator it =
                            msaGraph.find(i);
                    if(it != msaGraph.end()){ //i already has an entry in the map
                        msaGraph.at(i).push_back(first);
                    }else{    //no entry for i yet
                        vector<SuccessorNode> firstv;
                        firstv.push_back(first);
                        msaGraph.insert(
                                pair<int, vector<SuccessorNode>>(i, firstv));
                    }
                }
                //add i->ch_first to state of SSE
                ch_state[i * n_msagActors + first.successor_key] =
                        first.max_tok;
                actor_delay[i] = wcet[i].min();
                actor_delay[first.successor_key] = first.delay;
            }
        }
    }

    if(printDebug){
        /*cout << "initial state for SSE: " << endl;
         for(int i=0; i<n_msagActors; i++){
         for(int j=0; j<n_msagActors; j++){
         if(ch_state[i*n_msagActors+j] != -1)
         if(ch_state[i*n_msagActors+j]>10)
         cout << "B ";
         else
         cout << ch_state[i*n_msagActors+j] << " ";
         else
         cout << "X ";
         }
         cout << endl;
         }*/
        //printThroughputGraphAsDot(".");
    }
}

ExecStatus ThroughputMCR::propagate(Space& home, const ModEventDelta&) {
    if(printDebug)
        cout << "\tThroughputMCR::propagate()" << endl;

    //auto _start = std::chrono::high_resolution_clock::now(); //timer
    //int time; //runtime of period calculation

    //check which application graphs are mapped to same processor (= combined into the same MSAG)
    unordered_map<int, set<int>> coMappedApps;
    vector<set<int>> result;
    for(int i = 0; i < n_actors; i++){
        if(next[i].assigned() && next[i].val() < n_actors){ //next[i] is decided and points to an application actor
            int actor = i;
            int nextActor = next[i].val();
            if(getApp(actor) != getApp(nextActor)){ //from different applications
                unordered_map<int, set<int>>::const_iterator it =
                        coMappedApps.find(getApp(actor));
                if(it != coMappedApps.end()){ //i already has an entry in the map
                    coMappedApps.at(getApp(actor)).insert(getApp(nextActor));
                }else{ //no entry for ch_src[i] yet
                    set<int> coApp;
                    coApp.insert(getApp(nextActor));
                    coMappedApps.insert(
                            pair<int, set<int>>(getApp(actor), coApp));
                }
            }
        }
    }
    if(coMappedApps.size() > 0){
        for(auto it = coMappedApps.begin(); it != coMappedApps.end(); it++){
            //int app = it->first;
            set<int> t_apps = it->second;
            while(!t_apps.empty()){
                unordered_map<int, set<int>>::iterator iy = coMappedApps.find(*t_apps.begin());
                if(iy != coMappedApps.end()){ // has an entry in the map
                    it->second.insert(iy->second.begin(), iy->second.end());
                    t_apps.insert(iy->second.begin(), iy->second.end());
                    iy->second.clear();
                }

                t_apps.erase(t_apps.begin());
            }

        }
        for(auto it = coMappedApps.begin(); it != coMappedApps.end(); it++){
            if(it->second.size() > 0){
                set<int> res = it->second;
                res.insert(it->first);
                result.push_back(res);
            }
        }

    }


    constructMSAG();

    //debug_constructMSAG();

    //printThroughputGraphAsDot(".");
    //getchar();

    //printThroughputGraph();
    if(next.assigned() && wcet.assigned()){
        printThroughputGraphAsDot(".");
        getchar();
    }

    /*  if(printDebug){
     cout << "initial state for SSE: " << endl;
     for(int i=0; i<n_msagActors; i++){
     for(int j=0; j<n_msagActors; j++){
     if(ch_state[i*n_msagActors+j] != -1)
     if(ch_state[i*n_msagActors+j]>10)
     cout << "B ";
     else
     cout << ch_state[i*n_msagActors+j] << " ";
     else
     cout << "X ";
     }
     cout << endl;
     }
     }*/
    /*  for(int i=0; i<2*n_msagActors; i++){
     cout << "-";
     }*/
    /*cout << endl;
     for(int i=0; i<n_msagActors; i++){
     cout << actor_delay[i] << " ";
     }
     cout << endl;*/

    //stateSpaceExploration();

    //cout << "\t...done." << endl;
    /*
     cout  << "\twc_latency: ";
     for(int i=0; i<apps.size()-1; i++){
     cout << wc_latency[i] << ", ";
     }
     cout << wc_latency[apps.size()-1] << endl;
     cout  << "\twc_period: ";
     for(int i=0; i<apps.size()-1; i++){
     cout << wc_period[i] << ", ";
     }
     cout << wc_period[apps.size()-1] << endl;
     cout << "min/max iterations: ";
     for(int i=0; i<n_actors-1; i++){
     cout << min_iterations[i] << "/" << max_iterations[i] << ", ";

     }
     cout << min_iterations[n_actors-1] << "/" << max_iterations[n_actors-1] << endl;
     cout << "max timed schedule: " << endl;
     for(int i=0; i<n_actors; i++){
     cout << i << ": ";
     for(unsigned int j=0; j<max_end[i].size(); j++){
     cout << "[" << max_start[i][j] << ", " << max_end[i][j] << "] ";
     }
     //cout << "||  [" << start_pp[i] << ", " << end_pp[i] << "] ";
     cout << endl;
     }
     cout << "max timed schedule for channels: " << endl;
     for(unsigned int i=n_actors; i<max_end.size(); i++){
     cout << i << ": ";
     for(unsigned int j=0; j<max_end[i].size(); j++){
     cout << "[" << max_start[i][j] << ", " << max_end[i][j] << "] ";
     }
     //cout << "||  [" << start_pp[i] << ", " << end_pp[i] << "] ";
     cout << endl;
     }

     cout << "min timed schedule: " << endl;
     for(int i=0; i<n_actors; i++){
     cout << i << ": ";
     for(unsigned int j=0; j<min_end[i].size(); j++){
     cout << "[" << min_start[i][j] << ", " << min_end[i][j] << "] ";
     }
     //cout << "||  [" << start_pp[i] << ", " << end_pp[i] << "] ";
     cout << endl;
     }
     cout << "min timed schedule for channels: " << endl;
     for(unsigned int i=n_actors; i<min_end.size(); i++){
     cout << i << ": ";
     for(unsigned int j=0; j<min_end[i].size(); j++){
     cout << "[" << min_start[i][j] << ", " << min_end[i][j] << "] ";
     }
     //cout << "||  [" << start_pp[i] << ", " << end_pp[i] << "] ";
     cout << endl;
     }
     */
    /*time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-_start).count();
     total_time += time;
     calls++;*/
    //cout  << "\twc_period: ";
    //for(int i=0; i<apps.size()-1; i++){
    //  cout << wc_period[i] << ", ";
    //}
    //cout << wc_period[apps.size()-1] << endl;
    //cout  << "\t        (" << (time) << " ms)" << endl;
    //cout  << "\t        (AVG " << ((float)total_time/calls) << " ms for " << calls << " calls)" << endl;
    //documenting experimental results
    /*stringstream out;
     out << total_time << ", " << time << ", " << n_msagChannels << ", " << n_msagActors << ", " << calls << endl;
     std::ofstream fout;
     fout.open ("runtimes.csv", ios_base::app);
     fout << out.str();
     fout.close();*/

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
    for(int i = 0; i < n_channels; i++){
        if(!sendingTime[i].assigned() || sendingTime[i].min() > 0){
            all_ch_local = false;
        }
    }

    //propagate latency and period (bounds)
    for(int i = 0; i < apps.size(); i++){
        /*if(!all_assigned && all_ch_local && wcet.assigned() && next.assigned()){
         //if(wc_latency[i].size()==1)
         //GECODE_ME_CHECK(latency[i].eq(home,wc_latency[i][0]));
         GECODE_ME_CHECK(period[i].eq(home,wc_period[i]));
         }else*/if(!all_assigned && !all_ch_local){
            if(wc_latency[i].size() == 1)
                GECODE_ME_CHECK(latency[i].gq(home, wc_latency[i][0]));
            GECODE_ME_CHECK(period[i].gq(home, wc_period[i]));
        }else if(all_assigned){
            if(wc_latency[i].size() == 1)
                GECODE_ME_CHECK(latency[i].eq(home, wc_latency[i][0]));
            GECODE_ME_CHECK(period[i].eq(home, wc_period[i]));
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
     }

     //propagating the time-based schedule bounds
     if(next.assigned() && periodicSched_start.size()>0){
     for(int i=0; i<n_msagActors; i++){
     if(i<n_actors){
     //transient phase
     vector<int>::iterator its_min = min_start[i].begin();
     vector<int>::iterator ite_min = min_end[i].begin();
     vector<int>::iterator its_max = max_start[i].begin();
     vector<int>::iterator ite_max = max_end[i].begin();
     for(int j=minIndices[i]; j<=maxIndices[i]; j++){
     if(its_min!=min_start[i].end()){
     if(latency.assigned())GECODE_ME_CHECK(timedSched_start[j].lq(home, *its_min));
     if(sendingLatency.assigned())GECODE_ME_CHECK(timedSched_start[j].gq(home, *its_max));
     its_min++;
     its_max++;
     }else if(its_max!=max_start[i].end()){
     if(latency.assigned())GECODE_ME_CHECK(timedSched_start[j].lq(home, wc_latency-wcet[i].min()));
     IntVarImp _range(home, 0, (*its_max)-1);
     IntView rangeView(&_range);
     ViewRanges<IntView> range(rangeView);
     GECODE_ME_CHECK(timedSched_start[j].minus_r(home, range, false));
     its_max++;
     }else{
     if(all_assigned)GECODE_ME_CHECK(timedSched_start[j].eq(home, -1));
     }
     if(ite_min!=min_end[i].end()){
     if(latency.assigned())GECODE_ME_CHECK(timedSched_end[j].lq(home, *ite_min));
     if(sendingLatency.assigned())GECODE_ME_CHECK(timedSched_end[j].gq(home, *ite_max));
     ite_min++;
     ite_max++;
     }else if(ite_max!=max_end[i].end()){
     if(latency.assigned())GECODE_ME_CHECK(timedSched_end[j].lq(home, wc_latency));
     IntVarImp _range(home, 0, (*ite_max)-1);
     IntView rangeView(&_range);
     ViewRanges<IntView> range(rangeView);
     GECODE_ME_CHECK(timedSched_end[j].minus_r(home, range, false));
     ite_max++;
     }else{
     if(all_assigned)GECODE_ME_CHECK(timedSched_end[j].eq(home, -1));
     }
     }
     //periodic phase
     GECODE_ME_CHECK(periodicSched_start[i].gq(home, start_pp[i]));
     if(latency.assigned() && period.assigned())GECODE_ME_CHECK(periodicSched_start[i].lq(home, end_pp[i]-wcet[i].min()));
     if(latency.assigned() && period.assigned())GECODE_ME_CHECK(periodicSched_end[i].lq(home, end_pp[i]));
     GECODE_ME_CHECK(periodicSched_end[i].gq(home, start_pp[i]+wcet[i].min()));
     }else if(i>=n_actors && (i-n_actors)%2==0){ //sending node
     int ch = channelMapping[i]; //id of channel
     //transient phase
     vector<int>::iterator its_min = min_start[i].begin();
     vector<int>::iterator ite_min = min_end[i].begin();
     vector<int>::iterator its_max = max_start[i].begin();
     vector<int>::iterator ite_max = max_end[i].begin();
     for(int j=minIndices[n_actors+ch]; j<=maxIndices[n_actors+ch]; j++){
     if(its_min!=min_start[i].end()){
     if(latency.assigned())GECODE_ME_CHECK(timedSched_IC_start[j].lq(home, *its_min));
     if(sendingLatency.assigned())GECODE_ME_CHECK(timedSched_IC_start[j].gq(home, *its_max));
     its_min++;
     its_max++;
     }else if(its_max!=max_start[i].end()){
     if(latency.assigned())GECODE_ME_CHECK(timedSched_IC_start[j].lq(home, wc_latency-sendingTime[ch].min()-sendingLatency[ch].min()));
     IntVarImp _range(home, 0, (*its_max)-1);
     IntView rangeView(&_range);
     ViewRanges<IntView> range(rangeView);
     GECODE_ME_CHECK(timedSched_IC_start[j].minus_r(home, range, false));
     its_max++;
     }else{
     if(all_assigned)GECODE_ME_CHECK(timedSched_IC_start[j].eq(home, -1));
     if(latency.assigned())GECODE_ME_CHECK(timedSched_IC_start[j].lq(home, wc_latency-sendingTime[ch].min()-sendingLatency[ch].min()));
     }
     if(ite_min!=min_end[i].end()){
     if(latency.assigned())GECODE_ME_CHECK(timedSched_IC_end[j].lq(home, *ite_min));
     if(sendingLatency.assigned())GECODE_ME_CHECK(timedSched_IC_end[j].gq(home, *ite_max));
     ite_min++;
     ite_max++;
     }else if(ite_max!=max_end[i].end()){
     if(latency.assigned())GECODE_ME_CHECK(timedSched_IC_end[j].lq(home, wc_latency));
     IntVarImp _range(home, 0, (*ite_max)-1);
     IntView rangeView(&_range);
     ViewRanges<IntView> range(rangeView);
     GECODE_ME_CHECK(timedSched_IC_end[j].minus_r(home, range, false));
     ite_max++;
     }else{
     if(all_assigned)GECODE_ME_CHECK(timedSched_IC_end[j].eq(home, -1));
     if(latency.assigned())GECODE_ME_CHECK(timedSched_IC_end[j].lq(home, wc_latency));
     }
     }
     //periodic phase
     GECODE_ME_CHECK(periodicSched_IC_start[ch].gq(home, start_pp[i]));
     if(latency.assigned() && period.assigned())GECODE_ME_CHECK(periodicSched_IC_start[ch].lq(home, end_pp[i]-sendingTime[ch].min()-sendingLatency[ch].min()));
     if(latency.assigned() && period.assigned())GECODE_ME_CHECK(periodicSched_IC_end[ch].lq(home, end_pp[i]));
     GECODE_ME_CHECK(periodicSched_IC_end[ch].gq(home, start_pp[i]+sendingTime[ch].min()+sendingLatency[ch].min()));
     }
     }
     }

     if(timedSched_start.size()>0){
     //propagate time-based schedule for channels with no comm. delay (local channels)
     for(int i=0; i<ch_src.size(); i++){
     if(sendingTime[i].max() + sendingLatency[i].max() == 0){
     //transient phase
     int src_j = minIndices[ch_src[i]];
     for(int j=minIndices[n_actors+i]; j<=maxIndices[n_actors+i]; j++){
     GECODE_ME_CHECK(timedSched_IC_start[j].gq(home, timedSched_end[src_j].min()));
     GECODE_ME_CHECK(timedSched_IC_end[j].gq(home, timedSched_end[src_j].min()));
     GECODE_ME_CHECK(timedSched_IC_start[j].lq(home, timedSched_end[src_j].max()));
     GECODE_ME_CHECK(timedSched_IC_end[j].lq(home, timedSched_end[src_j].max()));
     src_j++;
     }
     //periodic phase
     GECODE_ME_CHECK(periodicSched_IC_start[i].gq(home, periodicSched_end[ch_src[i]].min()));
     GECODE_ME_CHECK(periodicSched_IC_end[i].gq(home, periodicSched_end[ch_src[i]].min()));
     GECODE_ME_CHECK(periodicSched_IC_start[i].lq(home, periodicSched_end[ch_src[i]].max()));
     GECODE_ME_CHECK(periodicSched_IC_end[i].lq(home, periodicSched_end[ch_src[i]].max()));
     //propagate iterations
     GECODE_ME_CHECK(iterationsCh[i].lq(home,1));
     }
     }
     }*/

    /*  if(timedSched_start.size()>0){
     bool schedFixed = true;
     for(int i=0; i<ch_src.size(); i++){
     if(sendingTime[i].max() + sendingLatency[i].max() != 0 &&
     (min_send_buffer[i] < max_send_buffer[i] ||
     min_rec_buffer[i] < max_rec_buffer[i])){
     int bufferSz_send = 0;
     int bufferSz_rec = 0;
     int max_send = 0;
     int max_rec = 0;
     int src = ch_src[i];
     if(!periodicSched_start[src].assigned() || !periodicSched_IC_end[i].assigned()){
     schedFixed = false;
     }
     int ch = minIndices[n_actors+i];
     int src_j = minIndices[src];
     int time = timedSched_start[src_j].min();
     vector<int> nextTime;
     nextTime.push_back(timedSched_IC_end[ch].min());
     while(time <= wc_latency+wc_period && schedFixed){
     if(!timedSched_start[src_j].assigned() || !timedSched_IC_end[ch].assigned()){
     schedFixed = false; break;
     }
     if(time == timedSched_start[src_j].val()){
     bufferSz_send++;
     if(src_j < maxIndices[src]){
     src_j++;
     nextTime.push_back(timedSched_start[src_j].min());
     }else{
     if(periodicSched_start[src].val() < periodicSched_IC_end[i].val())
     nextTime.push_back(periodicSched_start[src].val());
     }
     }else if(time == periodicSched_start[src].val()){
     bufferSz_send++;
     }
     if(time == timedSched_IC_end[ch].val()){
     bufferSz_send--;
     if(max_send < bufferSz_send) max_send = bufferSz_send;
     if(ch < maxIndices[n_actors+i]){
     ch++;
     nextTime.push_back(timedSched_IC_end[ch].min());
     }else{
     nextTime.push_back(periodicSched_IC_end[i].val());
     }
     }else if(time == periodicSched_IC_end[ch].val()){
     bufferSz_send--;
     if(max_send < bufferSz_send) max_send = bufferSz_send;
     }
     //let time pass
     if(nextTime.empty()) break;
     sort(nextTime.begin(), nextTime.end());
     nextTime.erase(unique(nextTime.begin(), nextTime.end()), nextTime.end());
     time = nextTime[0];
     nextTime.erase(nextTime.begin());
     }
     if(schedFixed) cout << "found buffer size for ch" << i <<": " << max_send << endl;
     }
     }
     //cout << "schedFixed: " << schedFixed << endl;
     }*/
    /*
     if(timedSched_start.size()>0){
     for(int i=0; i<ch_src.size(); i++){
     GECODE_ME_CHECK(sendbufferSz[i].gq(home, min_send_buffer[i]));
     GECODE_ME_CHECK(recbufferSz[i].gq(home, min_rec_buffer[i]));
     //if(sendingTime.assigned() && sendingLatency.assigned() &&
     //   receivingTime.assigned() && receivingNext.assigned()){*/
    /*if(all_assigned){
     GECODE_ME_CHECK(sendbufferSz[i].lq(home, max_send_buffer[i]));
     GECODE_ME_CHECK(recbufferSz[i].lq(home, max_rec_buffer[i]));
     }*/
    /*    }
     }
     */

    msaGraph.clear();
    channelMapping.clear();
    receivingActors.clear();
    ch_state.clear();
    actor_delay.clear();

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

    if(next.assigned() && wcet.assigned() && sendingTime.assigned()
            && sendingLatency.assigned() && receivingTime.assigned()
            && receivingNext.assigned() && sendbufferSz.assigned()
            && recbufferSz.assigned())
        return home.ES_SUBSUMED(*this);

    if(all_ch_local && wcet.assigned() && next.assigned()){
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
        const IntVarArgs& sendbufferSz, const IntVarArgs& recbufferSz,
        const IntVarArgs& next, const IntVarArgs& wcet,
        const IntVarArgs& sendingTime, const IntVarArgs& sendingLatency,
        const IntVarArgs& sendingNext, const IntVarArgs& receivingTime,
        const IntVarArgs& receivingNext, const IntVarArgs& timedSched_start,
        const IntVarArgs& timedSched_end, const IntVarArgs& timedSched_IC_start,
        const IntVarArgs& timedSched_IC_end,
        const IntVarArgs& periodicSched_start,
        const IntVarArgs& periodicSched_end,
        const IntVarArgs& periodicSched_IC_start,
        const IntVarArgs& periodicSched_IC_end, const IntArgs& ch_src,
        const IntArgs& ch_dst, const IntArgs& tok, const IntArgs& minIndices,
        const IntArgs& maxIndices) {
    if(iterations.size() != wcet.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, iterations & next");
    }
    if(iterationsCh.size() != sendingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, iterationsCh & sendingTime");
    }
    if(sendingTime.size() != sendingLatency.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & sendingLatency");
    }
    if(sendingTime.size() != receivingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & receivingTime");
    }
    if(sendingNext.size() != receivingNext.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & receivingNext");
    }
    if(sendingTime.size() != sendbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & sendbufferSz");
    }
    if(sendingTime.size() != recbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & recbufferSz");
    }
    if(timedSched_start.size() != timedSched_end.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, timedSched_start & timedSched_end");
    }
    if(timedSched_IC_start.size() != timedSched_IC_end.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint");
    }
    if(periodicSched_start.size() != wcet.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint");
    }
    if(periodicSched_start.size() != periodicSched_end.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint");
    }
    if(periodicSched_IC_start.size() != periodicSched_IC_end.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint");
    }
    if(periodicSched_IC_start.size() != ch_src.size()){
        throw Gecode::Int::ArgumentSizeMismatch("Throughput constraint");
    }
    if(sendingTime.size() != ch_src.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & ch_src");
    }
    if(sendingTime.size() != ch_dst.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & ch_dst");
    }
    if(sendingTime.size() != tok.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & tok");
    }
    if(minIndices.size() != maxIndices.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, minIndices & maxIndices");
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
    ViewArray<Int::IntView> _timedSched_start(home, timedSched_start);
    ViewArray<Int::IntView> _timedSched_end(home, timedSched_end);
    ViewArray<Int::IntView> _timedSched_IC_start(home, timedSched_IC_start);
    ViewArray<Int::IntView> _timedSched_IC_end(home, timedSched_IC_end);
    ViewArray<Int::IntView> _periodicSched_start(home, periodicSched_start);
    ViewArray<Int::IntView> _periodicSched_end(home, periodicSched_end);
    ViewArray<Int::IntView> _periodicSched_IC_start(home,
            periodicSched_IC_start);
    ViewArray<Int::IntView> _periodicSched_IC_end(home, periodicSched_IC_end);
    IntArgs apps(1, wcet.size() - 1);
    if(ThroughputMCR::post(home, _latency, _period, _iterations, _iterationsCh,
            _sendbufferSz, _recbufferSz, _next, _wcet, _sendingTime,
            _sendingLatency, _sendingNext, _receivingTime, _receivingNext,
            _timedSched_start, _timedSched_end, _timedSched_IC_start,
            _timedSched_IC_end, _periodicSched_start, _periodicSched_end,
            _periodicSched_IC_start, _periodicSched_IC_end, ch_src, ch_dst, tok,
            apps, minIndices, maxIndices) != ES_OK){
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
        const IntVarArgs& sendbufferSz, const IntVarArgs& recbufferSz,
        const IntVarArgs& next, const IntVarArgs& wcet,
        const IntVarArgs& sendingTime, const IntVarArgs& sendingLatency,
        const IntVarArgs& sendingNext, const IntVarArgs& receivingTime,
        const IntVarArgs& receivingNext, const IntArgs& ch_src,
        const IntArgs& ch_dst, const IntArgs& tok) {
    if(iterations.size() != wcet.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, iterations & next");
    }
    if(iterationsCh.size() != sendingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, iterationsCh & sendingTime");
    }
    if(sendingTime.size() != sendingLatency.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & sendingLatency");
    }
    if(sendingTime.size() != receivingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & receivingTime");
    }
    if(sendingNext.size() != receivingNext.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingNext & receivingNext");
    }
    if(sendingTime.size() != sendbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & sendbufferSz");
    }
    if(sendingTime.size() != recbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & recbufferSz");
    }
    if(sendingTime.size() != ch_src.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & ch_src");
    }
    if(sendingTime.size() != ch_dst.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & ch_dst");
    }
    if(sendingTime.size() != tok.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & tok");
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
    ViewArray<Int::IntView> _timedSched_start;
    ViewArray<Int::IntView> _timedSched_end;
    ViewArray<Int::IntView> _timedSched_IC_start;
    ViewArray<Int::IntView> _timedSched_IC_end;
    ViewArray<Int::IntView> _periodicSched_start;
    ViewArray<Int::IntView> _periodicSched_end;
    ViewArray<Int::IntView> _periodicSched_IC_start;
    ViewArray<Int::IntView> _periodicSched_IC_end;
    IntArgs minIndices, maxIndices;
    IntArgs apps(1, wcet.size() - 1);
    if(ThroughputMCR::post(home, _latency, _period, _iterations, _iterationsCh,
            _sendbufferSz, _recbufferSz, _next, _wcet, _sendingTime,
            _sendingLatency, _sendingNext, _receivingTime, _receivingNext,
            _timedSched_start, _timedSched_end, _timedSched_IC_start,
            _timedSched_IC_end, _periodicSched_start, _periodicSched_end,
            _periodicSched_IC_start, _periodicSched_IC_end, ch_src, ch_dst, tok,
            apps, minIndices, maxIndices) != ES_OK){
        home.fail();
    }
}

/* next: |#actors|+|#procs|
 * wcet: |#actors|
 * sendingTime: |#channels|
 * tok: |#channels|
 */
void throughputMCR(Space& home, const IntVarArgs& latency,
        const IntVarArgs& period,
        const IntVarArgs& iterations, //min/max
        const IntVarArgs& iterationsCh, //min/max
        const IntVarArgs& sendbufferSz, const IntVarArgs& recbufferSz,
        const IntVarArgs& next, const IntVarArgs& wcet,
        const IntVarArgs& sendingTime, const IntVarArgs& sendingLatency,
        const IntVarArgs& sendingNext, const IntVarArgs& receivingTime,
        const IntVarArgs& receivingNext, const IntArgs& ch_src,
        const IntArgs& ch_dst, const IntArgs& tok, const IntArgs& apps) {
    if(latency.size() != period.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, latency & period");
    }
    if(latency.size() != apps.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, latency & apps");
    }
    if(iterations.size() != wcet.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, iterations & next");
    }
    if(iterationsCh.size() != sendingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, iterationsCh & sendingTime");
    }
    if(sendingTime.size() != sendingLatency.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & sendingLatency");
    }
    if(sendingTime.size() != receivingTime.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & receivingTime");
    }
    if(sendingNext.size() != receivingNext.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingNext & receivingNext");
    }
    if(sendingTime.size() != sendbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & sendbufferSz");
    }
    if(sendingTime.size() != recbufferSz.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & recbufferSz");
    }
    if(sendingTime.size() != ch_src.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & ch_src");
    }
    if(sendingTime.size() != ch_dst.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & ch_dst");
    }
    if(sendingTime.size() != tok.size()){
        throw Gecode::Int::ArgumentSizeMismatch(
                "Throughput constraint, sendingTime & tok");
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
    ViewArray<Int::IntView> _timedSched_start;
    ViewArray<Int::IntView> _timedSched_end;
    ViewArray<Int::IntView> _timedSched_IC_start;
    ViewArray<Int::IntView> _timedSched_IC_end;
    ViewArray<Int::IntView> _periodicSched_start;
    ViewArray<Int::IntView> _periodicSched_end;
    ViewArray<Int::IntView> _periodicSched_IC_start;
    ViewArray<Int::IntView> _periodicSched_IC_end;
    IntArgs minIndices, maxIndices;
    if(ThroughputMCR::post(home, _latency, _period, _iterations, _iterationsCh,
            _sendbufferSz, _recbufferSz, _next, _wcet, _sendingTime,
            _sendingLatency, _sendingNext, _receivingTime, _receivingNext,
            _timedSched_start, _timedSched_end, _timedSched_IC_start,
            _timedSched_IC_end, _periodicSched_start, _periodicSched_end,
            _periodicSched_IC_start, _periodicSched_IC_end, ch_src, ch_dst, tok,
            apps, minIndices, maxIndices) != ES_OK){
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

void ThroughputMCR::printThroughputGraph() {
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
                if(((((SuccessorNode) (*itV)).successor_key - n_actors) % 3)
                        == 0){
                    cout << "b" << src << "-" << dst;
                }else if(((((SuccessorNode) (*itV)).successor_key - n_actors)
                        % 3) == 1){
                    cout << "s" << src << "-" << dst;
                }else{
                    cout << "r" << src << "-" << dst;
                }
                cout << "[" << ((SuccessorNode) (*itV)).successor_key << "]; ";
            }
            //cout << ((SuccessorNode)(*itV)).delay << "; ";
            cout << ((SuccessorNode) (*itV)).min_tok << "; ";
            cout << ((SuccessorNode) (*itV)).max_tok << ")";
        }
        cout << endl;
    }
}

void ThroughputMCR::printThroughputGraphAsDot(const string &dir) const {

    string graphName = "throughputGraph";
    ofstream out;
    string outputFile = dir;
    outputFile +=
            (outputFile.back() == '/') ?
                    (graphName + ".dot") : ("/" + graphName + ".dot");
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
                dstName = "block_ch"
                        + to_string(channelMapping[node2 - n_actors]);
            }else if(node2 >= n_actors && (node2 - n_actors) % 3 == 1){ //sending node
                dstName = "send_ch"
                        + to_string(channelMapping[node2 - n_actors]);
            }else if(node2 >= n_actors && (node2 - n_actors) % 3 == 2){ //receiving node
                dstName = "rec_ch"
                        + to_string(channelMapping[node2 - n_actors]);
            }
            int tok = ((SuccessorNode) (*itV)).max_tok;
            // Initial tokens on channel?
            if(tok != 0){
                string label;
                out << "    " << srcName << " -> " << dstName;
                if(node >= n_actors && (node - n_actors) % 3 == 0
                        && node2 < n_actors){
                    label = "send_buff (ch"
                            + to_string(channelMapping[node - n_actors]) + ")";
                    out << " [ label=\"" << label << "\"];" << endl;
                }else if(node >= n_actors && (node - n_actors) % 3 == 2
                        && node2 >= n_actors && (node2 - n_actors) % 3 == 1){
                    label = "rec_buff (ch"
                            + to_string(channelMapping[node2 - n_actors]) + ")";
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

    string cmd = "cat " + outputFile + " | dot -Tpdf > " + outputFile + ".pdf";
    cout << "  " << cmd << endl;
    system(cmd.c_str());

}

void ThroughputMCR::printSchedule(string type, int length) {
    //TODO: this has to be achieved some other way, but I need to see the schedules
    //for debugging purposes right now...
    vector<int> proc(n_msagActors, 0);
    proc[0] = 0;
    proc[1] = 0;
    proc[2] = 1;
    proc[3] = 2;
    proc[4] = 3;
    proc[5] = 4;
    //proc[6]=2;
    //proc[7]=2;
    //proc[8]=3;
    //proc[9]=3;
    //proc[10]=3;
    //proc[11]=4;
    //proc[12]=4;
    //proc[13]=4;
    //proc[14]=5;
    //proc[15]=5;
    //proc[16]=6;
    //proc[17]=6;
    //proc[18]=6;
    //proc[19]=6;
    //proc[20]=6;
    //proc[21]=6;
    //proc[22]=6;
    //proc[23]=6;
    //proc[24]=6;
    //proc[25]=6;
    //proc[26]=7;
    //proc[27]=7;
    //proc[28]=7;
    //proc[29]=7;
    //proc[30]=7;
    //proc[31]=7;
    for(int i = n_actors; i < n_msagActors; i++){
        proc[i] = proc[ch_src[channelMapping[i - n_actors]]] + n_procs;
    }

    double tikzLength = (double) length;
    int scaleFactor = 1;
    while(tikzLength > 50){
        tikzLength = tikzLength / 5;
        scaleFactor *= 5;
    }

    string dir = ".";
    if(type.compare("min") == 0){ //print min schedule
        string schedName = "minSchedule";
        ofstream out;
        string outputFile = dir;
        outputFile +=
                (outputFile.back() == '/') ?
                        (schedName + ".tex") : ("/" + schedName + ".tex");
        out.open(outputFile.c_str());

        out << "\\documentclass{standalone}\n\\usepackage{tikz}\n";
        out
                << "\\usetikzlibrary[arrows,decorations.pathmorphing,backgrounds,positioning]\n\n";
        out << "\\begin{document}\n\\begin{tikzpicture}\n";
        out
                << "[operand/.style={circle,draw=blue!50,fill=blue!20,thick,inner sep=0pt,minimum size=6mm},\n";
        out
                << "value/.style={rectangle,draw=black!50,fill=black!20,thick,inner sep=0pt,minimum size=6mm}]\n\n";

        out << "\\def \\tasks{" << n_procs * 2 << "}\n";
        out << "\\def \\time{" << tikzLength << "}\n\n";

        out << "\\foreach \\t in {";
        for(int i = 0; i < n_procs - 1; i++){
            out << i << ", ";
        }
        out << n_procs - 1 << "}\n";
        out << "{\n";
        out << "\t\\pgfmathparse{(\\t+1)*-1.5}\n";
        out << "\t\\let \\y \\pgfmathresult\n";
        out << "\t\\node at (-0.5,\\y) {\\Large $P_\\t$};\n";
        out << "\t\\draw [->] (0,\\y) -- (\\time,\\y) -- +(0.5,0);\n";
        out << "\t\\foreach \\x in {0,1,...,{\\time}}\n";
        out << "\t{\n";
        out << "\t\\draw (\\x, \\y-0.1) -- (\\x, \\y+0.1);\n";
        out << "\t\\pgfmathparse{\\x*" << scaleFactor << "}\n";
        out << "\t\\let \\printX \\pgfmathresult\n";
        out
                << "\t\\node at (\\x, \\y-0.25) {\\small\\pgfmathprintnumber{\\printX}};\n";
        out << "\t}\n";
        out << "}\n";

        for(int i = 0; i < n_actors; i++){
            double y = (double) (proc[i] + 1) * -1.5;
            for(unsigned int j = 0; j < min_end[i].size(); j++){
                double s = (double) min_start[i][j] / scaleFactor;
                double l = (double) (min_end[i][j] - min_start[i][j])
                        / scaleFactor;
                out << "\\draw  [fill=black!10] (" << s << "," << y
                        << ") rectangle +(";
                out << l << ",0.5);" << endl;
                out << "\\node at (" << s + 0.5 * l << "," << y + 0.25;
                out << ") {\\small\\pgfmathprintnumber{" << i << "}};\n";
            }
        }
        for(int i = n_actors; i < n_msagActors; i++){
            if((i - n_actors) % 3 != 2){ //don't print rec actors
                double y = (double) (proc[ch_src[channelMapping[i - n_actors]]]
                        + 1) * -1.5 + 0.5;
                for(unsigned int j = 0; j < min_end[i].size(); j++){
                    double s = (double) min_start[i][j] / scaleFactor;
                    double l = (double) (min_end[i][j] - min_start[i][j])
                            / scaleFactor;
                    if((i - n_actors) % 3 == 0){
                        out << "\\draw  [fill=black!60] (";
                    }else{
                        out << "\\draw  [fill=black!40] (";
                    }
                    out << s << "," << y << ") rectangle +(";
                    out << l << ",0.5);" << endl;
                    out << "\\node at (" << s + 0.5 * l << "," << y + 0.25;
                    out << ") {\\footnotesize\\pgfmathprintnumber{" << i
                            << "}};\n";
                }
            }
        }
        /*int y = n_procs*(-2);
         for(int i=0; i<apps.size(); i++){
         double lat = (double)wc_latency[i][0]/scaleFactor;
         double per = (double)(wc_period[i]+wc_latency[i][0])/scaleFactor;
         out << "\\draw ("<< lat << "," << 0 <<") -- (" << lat << "," << y << ");\n";
         //\node[anchor=north west, rotate=-45] at (4,-4) {latency 0};
         out << "\\node[anchor=north west, rotate=-45] at ("<< lat<<","<<y<<") {latency "<< i <<"};\n";
         out << "\\draw ("<< per << "," << 0 <<") -- (" << per << "," << y << ");\n";
         out << "\\node[anchor=north west, rotate=-45] at ("<< per<<","<<y<<") {period "<< i <<"};\n";
         }*/

        out << "\n\\end{tikzpicture}\n\\end{document}";

        out.close();
        cout << "  Printed min schedule " << outputFile << endl;
    }else{ //print max schedule
        string schedName = "maxSchedule";
        ofstream out;
        string outputFile = dir;
        outputFile +=
                (outputFile.back() == '/') ?
                        (schedName + ".tex") : ("/" + schedName + ".tex");
        out.open(outputFile.c_str());

        out << "\\documentclass{standalone}\n\\usepackage{tikz}\n";
        out
                << "\\usetikzlibrary[arrows,decorations.pathmorphing,backgrounds,positioning]\n\n";
        out << "\\begin{document}\n\\begin{tikzpicture}\n";
        out
                << "[operand/.style={circle,draw=blue!50,fill=blue!20,thick,inner sep=0pt,minimum size=6mm},\n";
        out
                << "value/.style={rectangle,draw=black!50,fill=black!20,thick,inner sep=0pt,minimum size=6mm}]\n\n";

        out << "\\def \\tasks{" << n_procs * 2 << "}\n";
        out << "\\def \\time{" << tikzLength << "}\n\n";

        out << "\\foreach \\t in {";
        for(int i = 0; i < n_procs - 1; i++){
            out << i << ", ";
        }
        out << n_procs - 1 << "}\n";
        out << "{\n";
        out << "\t\\pgfmathparse{(\\t+1)*-1.5}\n";
        out << "\t\\let \\y \\pgfmathresult\n";
        out << "\t\\node at (-0.5,\\y) {\\Large $P_\\t$};\n";
        out << "\t\\draw [->] (0,\\y) -- (\\time,\\y) -- +(0.5,0);\n";
        out << "\t\\foreach \\x in {0,1,...,{\\time}}\n";
        out << "\t{\n";
        out << "\t\\draw (\\x, \\y-0.1) -- (\\x, \\y+0.1);\n";
        out << "\t\\pgfmathparse{\\x*" << scaleFactor << "}\n";
        out << "\t\\let \\printX \\pgfmathresult\n";
        out
                << "\t\\node at (\\x, \\y-0.25) {\\small\\pgfmathprintnumber{\\printX}};\n";
        out << "\t}\n";
        out << "}\n";

        for(int i = 0; i < n_actors; i++){
            double y = (double) (proc[i] + 1) * -1.5;
            for(unsigned int j = 0; j < max_end[i].size(); j++){
                double s = (double) max_start[i][j] / scaleFactor;
                double l = (double) (max_end[i][j] - max_start[i][j])
                        / scaleFactor;
                out << "\\draw  [fill=black!10] (" << s << "," << y
                        << ") rectangle +(";
                out << l << ",0.5);" << endl;
                out << "\\node at (" << s + 0.5 * l << "," << y + 0.25;
                out << ") {\\small\\pgfmathprintnumber{" << i << "}};\n";
            }
        }
        for(int i = n_actors; i < n_msagActors; i++){
            if((i - n_actors) % 3 != 2){ //don't print rec actors
                double y = (double) (proc[ch_src[channelMapping[i - n_actors]]]
                        + 1) * -1.5 + 0.5;
                for(unsigned int j = 0; j < max_end[i].size(); j++){
                    double s = (double) max_start[i][j] / scaleFactor;
                    double l = (double) (max_end[i][j] - max_start[i][j])
                            / scaleFactor;
                    if((i - n_actors) % 3 == 0){
                        out << "\\draw  [fill=black!60] (";
                    }else{
                        out << "\\draw  [fill=black!40] (";
                    }
                    out << s << "," << y << ") rectangle +(";
                    out << l << ",0.5);" << endl;
                    out << "\\node at (" << s + 0.5 * l << "," << y + 0.25;
                    out << ") {\\footnotesize\\pgfmathprintnumber{" << i
                            << "}};\n";
                }
            }
        }
        int y = n_procs * (-2);
        for(int i = 0; i < apps.size(); i++){
            double lat = (double) wc_latency[i][0] / scaleFactor;
            double per = (double) (wc_period[i] + wc_latency[i][0])
                    / scaleFactor;
            out << "\\draw (" << lat << "," << 0 << ") -- (" << lat << "," << y
                    << ");\n";
            //\node[anchor=north west, rotate=-45] at (4,-4) {latency 0};
            out << "\\node[anchor=north west, rotate=-45] at (" << lat << ","
                    << y << ") {latency " << i << "};\n";
            out << "\\draw (" << per << "," << 0 << ") -- (" << per << "," << y
                    << ");\n";
            out << "\\node[anchor=north west, rotate=-45] at (" << per << ","
                    << y << ") {period " << i << "};\n";
        }

        out << "\n\\end{tikzpicture}\n\\end{document}";
        out.close();
        cout << "  Printed max schedule " << outputFile << endl;
    }

}

