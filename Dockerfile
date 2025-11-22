FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libsqlite3-dev \
    libasio-dev \
    nlohmann-json3-dev \
    wget \
    pkg-config 

RUN wget https://github.com/CrowCpp/Crow/releases/download/v1.0+5/crow_all.h -O /usr/local/include/crow_all.h

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. && \
    make


FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libsqlite3-0 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/scheduler .
COPY --from=builder /app/templates ./templates

EXPOSE 18080

CMD ["./scheduler"]