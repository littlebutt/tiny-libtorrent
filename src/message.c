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
    if (msg == NULL)
    {
        return NULL;
    }
    int32_t length = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    *msglen = length + 4;
    msg->id = (uint8_t)buf[4];
    msg->payload = buf + 5;
    return msg;
}

int message_parse_have(message *msg, int msglen)
{
    if (msg->id != MSG_HAVE)
    {
        printf("[message] Expected id 4, but got id: %d", msg->id);
        return 0;
    }
    if (msglen != 5/*message head*/ + 4)
    {
        printf("[message] Expected message length 8, but got length: %d", msglen);
        return 0;
    }
    int32_t index = msg->payload[0] << 24 | msg->payload[1] << 16 | msg->payload[2] << 8 | msg->payload[3];
    return index;
}

int message_parse_piece(message *msg, int msglen, char **buf, int buflen, int index)
{
    int parsed_index, begin;
    char *data;
    if (msg->id != MSG_PIECE)
    {
        printf("[message] Expected id 7, but got id: %d", msg->id);
        return 0;
    }
    if (msglen < 5/*message head*/ + 8)
    {
        printf("[message] Message is too short: %d", msglen);
        return 0;
    }
    parsed_index = msg->payload[0] << 24 | msg->payload[1] << 16 | msg->payload[2] << 8 | msg->payload[3];
    if (parsed_index != index)
    {
        printf("[message] Expected index %d, but got index: %d", index, parsed_index);
        return 0;
    }
    begin = msg->payload[4] << 24 | msg->payload[5] << 16 | msg->payload[6] << 8 | msg->payload[7];
    if (begin > buflen)
    {
        printf("[message] Begin offset is too high: %d > %d", begin, buflen);
        return 0;
    }
    data = msg->payload + 8;
    if (msglen - 13 + begin > buflen)
    {
        printf("[message] Data is too long for offset %d with length %d", begin, buflen);
        return 0;
    }
    memcpy(*buf + begin, data, msglen - 8/*payload head*/ - 5/*message head*/);
    // TODO: sf Recieved: \x00\x00\x40\x09\x07\x00\x00\x00\x00\x00\x00\xc0\x00
    return msglen - 13;
}