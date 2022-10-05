FROM alpine:3.16.2 as build-env
RUN apk add --no-cache build-base
WORKDIR /app
COPY . .
RUN gcc -o memfag src/memfag.c

FROM alpine:3.16.2
COPY --from=build-env /app/memfag /app/memfag
WORKDIR /app
ENTRYPOINT ["/app/memfag"] 

