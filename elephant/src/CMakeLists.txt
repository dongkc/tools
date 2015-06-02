ADD_EXECUTABLE(acceptor
  acceptor.cpp)

TARGET_LINK_LIBRARIES(acceptor
  PocoFoundation
  PocoNet)

ADD_EXECUTABLE(connector
  connector.cpp)

TARGET_LINK_LIBRARIES(connector
  PocoFoundation
  PocoNet)
