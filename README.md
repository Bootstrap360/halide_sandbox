# Halide Sandbox

This project is to learn how to use halide

## Setup

1. Install Docker

        $ sudo apt-get install docker.io

1. Install halide docker image

        # docker pull mbuckler/halide

1. Run the docker image with terminal. Also mount repositories folder

        docker run -v /home/glen/repositories:/root/repositories -it mbuckler/halide

1. Launch another terminal into the same docker

        docker exec -it <hash> bash


## TODO

add the following to the image

        apt-get install libav-tools
        apt-get install imagemagick
        apt-get install gifsicle

    