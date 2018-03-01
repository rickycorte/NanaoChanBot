FROM debian

RUN apt-get update -y

#installa programmi requisiti
RUN apt-get install -y git-core build-essential cmake automake autoconf libtool

#scarica librerie necessarie
RUN apt-get install -y libssl-dev curl libcurl4-gnutls-dev

#scarica e compila mongoc
WORKDIR /mongoc 
RUN git clone https://github.com/mongodb/mongo-c-driver.git
RUN cd mongo-c-driver && ./autogen.sh --with-libbson=bundled && make && make install

WORKDIR /mongocxx

#scarica e compila mongocxx
RUN git clone https://github.com/mongodb/mongo-cxx-driver.git --branch releases/stable --depth 1
RUN cd mongo-cxx-driver/build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr .. && \
    make EP_mnmlstc_core && \
    make && make install

WORKDIR /nanao

#copia i sorgenti di nanao
ADD Data Data
ADD language language
ADD lib lib
ADD MalParser MalParser
ADD src src
ADD Makefile Makefile

#builda nanao
RUN make 

EXPOSE 1997

RUN cd build && ./nanaoBot