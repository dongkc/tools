ADD_EXECUTABLE(acceptor
  acceptor.cpp
  hhd_message_writer.cpp
  hhd_message_reader.cpp
  )

TARGET_LINK_LIBRARIES(acceptor
  PocoFoundation
  PocoNet
  glog)

ADD_EXECUTABLE(connector
  connector.cpp)

TARGET_LINK_LIBRARIES(connector
  PocoFoundation
  PocoNet)

ADD_EXECUTABLE(notification_test
  notification_test.cpp)

TARGET_LINK_LIBRARIES(notification_test
  PocoFoundation
  PocoNet)

ADD_EXECUTABLE(notification_queue
  notification_queue.cpp)

TARGET_LINK_LIBRARIES(notification_queue
  PocoFoundation
  PocoNet)
