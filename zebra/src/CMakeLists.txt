ADD_EXECUTABLE(zebra
  main.cpp
  EventMessageSender.cpp
  CommandMessageListener.cpp)

TARGET_LINK_LIBRARIES(zebra
  pthread
  gflags
  glog
  activemq-cpp
  boost_system
  boost_thread)
