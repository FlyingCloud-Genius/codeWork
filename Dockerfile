FROM ubuntu:18.04 as base

RUN apt-get update
#install some basic packages
RUN apt-get install -y --no-install-recommends vim gcc g++ cmake

#install x11
RUN apt-get install -y --no-install-recommends libx11-dev

#install mpi
RUN apt-get install -y mpich




