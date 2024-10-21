#include "message.h"

char * message_serialize(const uint8_t id, const char *payload, const size_t payloadlen)
{
    char *buf = (char *)malloc(payloadlen + 1 + 4);
    if (buf == NULL)
    {
        return 0;
    }
    uint32_t length = payloadlen + 1;
    buf[0] = (length >> 24) & 0xff;
    buf[1] = (length >> 16) & 0xff;
    buf[2] = (length >> 8) & 0xff;
    buf[3] = length & 0xff;
    buf[4] = id;
    memcpy(buf + 5, payload, payloadlen);
    return buf;
}


message * message_deserialize(char *buf, int *msglen)
{
    message *msg = (message *)malloc(sizeof(message));
    int32_t length = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    *msglen = length + 4;
    msg->id = (uint8_t)buf[4];
    msg->payload = buf + 5;
    return msg;
}