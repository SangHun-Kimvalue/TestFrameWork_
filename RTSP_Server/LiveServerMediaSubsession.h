//
//  LiveServerMediaSubsession.h
//  FFmpegRTSPServer
//
//  Created by Mina Saad on 9/22/15.
//  Copyright (c) 2015 Mina Saad. All rights reserved.
//

//#ifndef MESAI_Live_SERVER_MEDIA_SUBSESSION_HH
//#define MESAI_Live_SERVER_MEDIA_SUBSESSION_HH

#include <H264VideoFileServerMediaSubsession.hh>
#include <StreamReplicator.hh>
#include <H264VideoRTPSink.hh>
#include <H264VideoStreamFramer.hh>
#include <H264VideoStreamDiscreteFramer.hh>
#include <UsageEnvironment.hh>
#include <Groupsock.hh>

class LiveServerMediaSubsession: public H264VideoFileServerMediaSubsession
{
public:
    static LiveServerMediaSubsession* createNew(UsageEnvironment& env, StreamReplicator* replicator);
    static LiveServerMediaSubsession* createNew(UsageEnvironment& env);
    static LiveServerMediaSubsession* createNew(UsageEnvironment& env, char const* fileName, Boolean reuseFirstSource);
    
protected:

    LiveServerMediaSubsession(UsageEnvironment& env)
        : H264VideoFileServerMediaSubsession(env, "", True), m_replicator(nullptr) {};

	 LiveServerMediaSubsession(UsageEnvironment& env, StreamReplicator* replicator)
		 : H264VideoFileServerMediaSubsession(env, "", False), m_replicator(replicator) {};
      
	 LiveServerMediaSubsession(UsageEnvironment& env, char const* fileName, Boolean reuseFirstSource)
		 : H264VideoFileServerMediaSubsession(env, fileName, reuseFirstSource), m_replicator(nullptr)
	 {};//,fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL) {};


    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);    

    StreamReplicator * m_replicator;
};


//#endif