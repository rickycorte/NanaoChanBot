FROM debian

RUN apt-get update -y && apt-get install -y git-core build-essential cmake && apt-get clean

RUN git clone https://github.com/rickycorte/NanaoChanBot.git --recurse-submodules --depth 1 && \
    cd NanaoChanBot && \
    cmake . && \
    make

EXPOSE 8080
WORKDIR NanaoChanBot
RUN ./NanaoChanBot -t
CMD ["./NanaoChanBot"]
