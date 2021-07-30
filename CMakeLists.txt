cmake_minimum_required(VERSION 3.17)
project(Btmeteor C)

set(CMAKE_C_STANDARD 11)

add_executable(Btmeteor main.c parse_metafile.h parse_metafile.c bitfield.c bitfield.h bterror.c
        bterror.h log.c log.h signal_handler.c signal_handler.h peer.c peer.h message.c message.h data.c data.h)