FROM alpine:latest

RUN apk add --no-cache gcc musl-dev curl-dev make cjson-dev

WORKDIR /usr/src/app

COPY Makefile .
COPY include/ include/
COPY src/ src/


RUN make

CMD ["./implant"]
