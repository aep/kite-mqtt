CXXFLAGS=-std=c++11

main: main.cpp Kite/EventLoop.cpp Kite/SslSocket.cpp Kite/MqttClient.cpp Kite/Stdio.cpp
	$(CXX) -g $(CXXFLAGS) -o $@ $^ -lcrypto -lssl
