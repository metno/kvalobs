#include <boost/thread/xtime.hpp>

template <class MessageType>
MessageQue<MessageType>::MessageQue(int maxSize):
  mutex(),  que_(), maxSize_(maxSize),
  cancel_(false)
{
  if(maxSize_<0)
    maxSize_=0;
}

template <class MessageType>
MessageQue<MessageType>::~MessageQue()
{
  lock lck(mutex);
}

template <class MessageType> bool 
MessageQue<MessageType>::post(MessageType message)
{
  boost::xtime xt;
  
  lock lck(mutex);
  
  try{
    while(true){
      std::cerr << "MessageQue::post: Size: " << que_.size() << std::endl;

      if(maxSize_<1 || 
	 que_.size()<static_cast<typename Que::size_type>(maxSize_)){
	que_.push_back(message);

	
	emptyCond.notify_all();
	return true;
      }else{ //Køen er full
	boost::xtime_get(&xt, boost::TIME_UTC);
	xt.sec+=1;
	fullCond.timed_wait(lck, xt);
	
	if(cancel_){
	  throw MessageQueCancel();
	}
      }
    }
  }
  catch(...){
    return false;
  }
  
}

template <class MessageType> MessageType
MessageQue<MessageType>::get(int timeout)
{
  boost::xtime xt;
  lock lck(mutex);

  std::cerr << "MessageQue::get: Size: " << que_.size() << std::endl;
  
  if(que_.empty()){
    if(timeout<1){
      
      while(que_.empty()){
	boost::xtime_get(&xt, boost::TIME_UTC);
	xt.sec+=1;
	emptyCond.timed_wait(lck, xt);
	
	if(cancel_){
	  throw MessageQueCancel();
	}
      }
    }else{
      boost::xtime_get(&xt, boost::TIME_UTC);
      xt.sec+=timeout;
      
      emptyCond.timed_wait(lck, xt);
      
      if(que_.empty()){
	throw MessageQueEmpty();
      }
    }
  }
    
  MessageType tmp=que_.front();
  que_.pop_front();
  
  if(maxSize_>0)
    fullCond.notify_all(); 
  
  return tmp;
  }

template <class MessageType> bool      
MessageQue<MessageType>::isEmpty()const
{
  lock lck(mutex);
  
  return que_.empty();
}

template <class MessageType> int       
MessageQue<MessageType>::size()const
{
  lock lck(mutex);

return que_.size();
}



template <class MessageType>bool      
MessageQue<MessageType>::cancel()const
{
  lock lck(mutex);
  return cancel_;   
}

template <class MessageType> void     
MessageQue<MessageType>::cancel(bool flag)
{
   lock lck(mutex);

   cancel_=flag;
}




