#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_CHOKE          0
#define MSG_UNCHOKE        1
#define MSG_INTERESTED     2
#define MSG_NOT_INTERESTED 3
#define MSG_HAVE           4
#define MSG_BITFIELD       5
#define MSG_REQUEST        6
#define MSG_PIECE          7
#define MSG_CANCEL         8

typedef struct {
    uint8_t id;
    char *payload;
} message;

char *message_serialize(const uint8_t id, const char *payload, const size_t payloadlen);

message *message_deserialize(char *buf, int *msglen);

int message_parse_have(message *msg, int msglen);

int message_parse_piece(message *msg, int msglen, char **buf, int buflen, int index);

#endif // MESSAGE_H