//
//  LiveServerMediaSubsession.cpp
//  FFmpegRTSPServer
//
//  Created by Mina Saad on 9/22/15.
//  Copyright (c) 2015 Mina Saad. All rights reserved.
//

#include "LiveServerMediaSubsession.h"
#include "H264VideoFileServerMediaSubsession.hh"
#include "H264VideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "H264VideoStreamFramer.hh"

LiveServerMediaSubsession * LiveServerMediaSubsession::createNew(UsageEnvironment& env)
//LiveServerMediaSubsession * LiveServerMediaSubsession::createNew(UsageEnvironment& env, StreamReplicator* replicator)
{ 
	//return new LiveServerMediaSubsession(env,replicator);
	return new LiveServerMediaSubsession(env);
}

LiveServerMediaSubsession* LiveServerMediaSubsession::
createNew(UsageEnvironment& env, char const* fileName, Boolean reuseFirstSource){
	return new LiveServerMediaSubsession(env, fileName, reuseFirstSource);
}

LiveServerMediaSubsession * LiveServerMediaSubsession::
createNew(UsageEnvironment& env, StreamReplicator* replicator)
{
	return new LiveServerMediaSubsession(env,replicator);
}
					
FramedSource* LiveServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
	estBitrate = 500; // kbps, estimate

	//FramedSource* source = m_replicator->createStreamReplica();
	//FramedSource* source;
	ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(envir(), fFileName);
	if (fileSource == NULL) return NULL;
	fFileSize = fileSource->fileSize();
	return H264VideoStreamDiscreteFramer::createNew(envir(), fileSource);
	//return source;



}
		
RTPSink* LiveServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	return H264VideoRTPSink::createNew(envir(), rtpGroupsock,rtpPayloadTypeIfDynamic);
}


