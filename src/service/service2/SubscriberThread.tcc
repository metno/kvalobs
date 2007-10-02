
template <typename cmdType>
int 
SubscriberThread<cmdType>::
run()
{
  milog::LogContext cntxt(subscriberid());
  dnmi::thread::CommandBase *cmd=0;
  cmdType *theCmd;
  retry=0;
  maxRetry=0;


  time(&tick);

  while(!terminate_ && !shutdown() && maxRetry<tick){
    if(!cmd){
      cmd=getDataFromQue(2);
      
      if(!cmd)
	continue;
    }else{
      //We have a left over dataset that is not sendt to the subscriber
      sleep(2);
      
      time(&tick);
      
      if(tick<retry)
	continue;
    }
    
    theCmd=dynamic_cast<cmdType*>(cmd);

    if(!theCmd){
      LOGERROR("Unexpected command!");
      delete cmd;
      cmd=0;
      continue;
    }

    typename cmdType::_ptr_type sub=subscriber_;

    int ret=(*theCmd)(*static_cast<KvDataSubscriberInfo*>(this), sub);

    time(&tick);

    if(ret==0){
      maxRetry=0;
      delete theCmd;
      cmd=0;
    }else if(ret<0){
      maxRetry=tick;
    }else{
      if(maxRetry==0){
	maxRetry=tick+MAX_RETRY;
      }

      retry=tick+RETRY;
    }
  }
  
  inque_.suspend();
  inque_.clear();
  
  if(terminate_){
    LOGINFO("Terminate: Unsubscribed!");
    return 1;
  }

  if(maxRetry>0){
    LOGWARN("TERMINATE: The subscriber is NOT responding!");
    return 2;
  }
  
  LOGINFO("Terminate: Normal exit!");

  return 0;
}
