FROM gcc:9

WORKDIR /app

RUN apt-get update
RUN apt-get install -y cmake

COPY ./CMakeLists.txt /app/
COPY ./Source /app/Source
COPY ./Dependencies /app/Dependencies

RUN ls -alR /app

RUN cd /app/; \
    cmake -S . -B build; \
    cmake --build build 

EXPOSE 8000
ENTRYPOINT [ "/app/Distribution/SolarToLunar" ]