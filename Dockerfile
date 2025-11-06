FROM public.ecr.aws/docker/library/golang:1.25

WORKDIR /app

RUN apt-get update && apt-get install -y cmake build-essential

COPY . .
