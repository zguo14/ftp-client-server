CXX = g++
CXXFLAGS = -std=c++11
all : client server

client : client.cpp network.cpp
	${CXX} $(CXXFLAGS) client.cpp network.cpp -o $@

server: server.cpp network.cpp
	${CXX} $(CXXFLAGS) server.cpp network.cpp -o $@