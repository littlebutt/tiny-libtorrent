#include "peers.h"

size_t _read_peers(char **peers, const char *buf, size_t buflen)
{
    size_t peerslen = 0;
    struct bencode ctx[1];
    int res = 0;
    // A flag for memoizing keys in bencode:
    // 1 for peers
    int flag = 0;
    bencode_init(ctx, buf, buflen);
    // XXX: Assume all data we need is in a dict.
    while ((res = bencode_next(ctx)) > 0)
    {
        switch (res)
        {
            case BENCODE_STRING:
            {
                char *key = (char *)malloc((ctx->toklen + 1) * sizeof(char));
                if (key == NULL)
                {
                    return 0;
                }
                memcpy(key, ctx->tok, ctx->toklen);
                key[ctx->toklen] = '\0';
                switch (flag)
                {
                    case 0:
                    {
                        if (strcmp(key, "peers") == 0)
                        {
                            flag = 1;
                        }
                        break;
                    }
                    case 1:
                    {
                        flag = 0;
                        char *value = (char *)malloc(ctx->toklen * sizeof(char));
                        if (value == NULL) {
                            return 0;
                        }
                        memcpy(value, key, ctx->toklen * sizeof(char));
                        *peers = value;
                        peerslen = ctx->toklen;
                        break;   
                    }
                }
                free(key);
            }
        }
    }
    bencode_free(ctx);
    return peerslen;    
}

char * _build_ip(char *buf)
{
    uint8_t ip[4];
    char *res;
    for (int i = 0; i < 4; i++)
    {
        ip[i] = (uint8_t)buf[i];
    }
    res = (char *)malloc(16);
    snprintf(res, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return res;
}

int _build_port(char *buf)
{
    return (uint16_t)(buf[0] << 8 | buf[1]);
}


int _make_peers(peer **p, const char *buf, size_t buflen)
{
    if (buflen % 6 != 0)
    {
        printf("[peer] Poor format peers");
        return 0;
    }

    peer *head = NULL, *tail = NULL;
    char *pb = (char *)buf;
    
    for (size_t i = 0; i < buflen; i += 6)
    {
        char *ip = _build_ip(pb + i);
        int port = _build_port(pb + i + 4);
        peer *pt = (peer *)malloc(sizeof(peer));
        pt->ip = ip;
        pt->port = port;
        pt->next = NULL;
        
        if (head == NULL)
        {
            head = pt;
        }
        else
        {
            tail->next = pt;
        }
        tail = pt;
    }
    *p = head;
    printf("\n[peer] Find %lld peers!\n", buflen / 6);
    return 1;
}


int peer_init(peer **p, const char *buf, size_t buflen)
{
    char *peers;
    size_t peerslen;
    peerslen = _read_peers(&peers, buf, buflen);
    if (peerslen == 0)
    {
        return 0;
    }
    return _make_peers(p, peers, peerslen);
}

void peer_free(peer *p)
{
    while (p != NULL)
    {
        peer *pp = p;
        p = p->next;
        free(pp->ip);
        free(pp);
    }
}

char * _build_handshake(const char *info_hash, const char *peerid)
{
    char *hs_msg = (char *)malloc(sizeof(char) * 68);
    memset(hs_msg, 0, 68);
    memcpy(hs_msg, (char []){0x13}, 1);
    memcpy(hs_msg + 1, "BitTorrent protocol", 19);
    memcpy(hs_msg + 28, info_hash, 20);
    memcpy(hs_msg + 48, peerid, 20);
    return hs_msg;
}

char * _handshake(peer *p, const char *info_hash, const char *peerid, int *socket)
{
    int sock;
    char *handshake;
    char *recv;
    int pstrlen;
    if ((sock = tcp_connect(p->ip, p->port)) <= 0)
    {
        return NULL;
    }
    
    handshake = _build_handshake(info_hash, peerid);
    if (tcp_send(sock, handshake, 68, &recv) == 0)
    {
        return NULL;
    }
    pstrlen = (int)recv[0];
    if (recv == 0)
    {
        return NULL;
    }
    if (memcmp(recv + 1 + pstrlen + 8, info_hash, 20) != 0)
    {
        printf("[peer] Fail to handshake with %s:%d\n", p->ip, p->port);
        return NULL;
    }
    *socket = sock;
    return recv + 1 + pstrlen + 48;
}


int _build_bitfield(char *buf, char **bitfield)
{
    int msglen;
    message *msg;
    msg = message_deserialize(buf, &msglen);
    *bitfield = msg->payload;
    return msglen - 5;
}

int _send_unchoke(int sock, char **recvs) 
{
    char * messagebuf = message_serialize(MSG_UNCHOKE, NULL, 0);
    return tcp_send(sock, messagebuf, 5, recvs);
}

int _send_interested(int sock, char **recvs)
{
    char * messagebuf = message_serialize(MSG_INTERESTED, NULL, 0);
    return tcp_send(sock, messagebuf, 5, recvs);
}

_peer_context * _peer_context_init(int sock, char *bitfield, int bitfieldlen, char *info_hash)
{
    _peer_context *ctx = (_peer_context *)malloc(sizeof(_peer_context));
    ctx->sock = sock;
    ctx->bitfield = bitfield;
    ctx->bitfieldlen = bitfieldlen;
    ctx->info_hash = info_hash;
    ctx->choked = 1;
    ctx->state = NULL;
    return ctx;
}

_peer_state * _peer_state_init(int index, size_t buflen)
{
    _peer_state *state = (_peer_state *)malloc(sizeof(_peer_state));
    state->index = index;
    state->buflen = buflen;
    state->requested = 0;
    state->downloaded = 0;
    state->backlog = 0;
    return state;
}

int _read_message(char *buf, int buflen, _peer_context *ctx)
{
    int msglen;
    message *msg;
    if (buf == NULL)
    {
        return 1;
    }
    msg = message_deserialize(buf, &msglen);
    if (msglen == 5)
    {
        return 0;
    }
    switch (msg->id)
    {
        case MSG_UNCHOKE:
        {
            ctx->choked = 0;
            break;
        }
        case MSG_CHOKE:
        {
            ctx->choked = 1;
            break;
        }
        case MSG_HAVE:
        {
            int index = message_parse_have(msg, msglen);
            if (index == 0)
            {
                return 0;
            }
            piecework_set_piece(ctx->bitfield, ctx->bitfieldlen, index);
            break;
        }
        case MSG_PIECE:
        {
            int n = message_parse_piece(msg, msglen, &(ctx->state->buf), ctx->state->buflen, ctx->state->index);
            if (n == 0)
            {
                return 0;
            }
            ctx->state->downloaded += n;
            ctx->state->backlog --;
            break;
        }
    }
    return 1;
}

int _peer_send_request(int sock, int32_t index, int32_t requested, int32_t block_size, char **recvs)
{
    char *payload = (char *)malloc(sizeof(char) * 12);
    payload[0] = (index >> 24) & 0xff;
    payload[1] = (index >> 16) & 0xff;
    payload[2] = (index >> 8) & 0xff;
    payload[3] = index & 0xff;
    payload[4] = (requested >> 24) & 0xff;
    payload[5] = (requested >> 16) & 0xff;
    payload[6] = (requested >> 8) & 0xff;
    payload[7] = requested & 0xff;
    payload[8] = (block_size >> 24) & 0xff;
    payload[9] = (block_size >> 16) & 0xff;
    payload[10] = (block_size >> 8) & 0xff;
    payload[11] = block_size & 0xff;
    char *msg = message_serialize(MSG_REQUEST, payload, 12);
    return tcp_send(sock, msg, 12 + 5, recvs);
}

int _peer_download(_peer_context *ctx, const piecework *pw, char *buf, int buflen)
{
    _peer_state *state = _peer_state_init(pw->index, pw->length);
    ctx->state = state;
    char *reply = buf;
    size_t replylen = buflen;

    while (state->downloaded < pw->length)
    {
        if (!ctx->choked)
        {
            while (state->backlog < 5 && state->requested < pw->length)
            {
                int block_size = 16384;
                if (pw->length - state->requested < block_size)
                {
                    block_size = pw->length - state->requested;
                }
                char *recvs;
                int recvslen;
                recvslen = _peer_send_request(ctx->sock, pw->index, state->requested, block_size, &recvs);
                state->backlog ++;
				state->requested += block_size;
                replylen += recvslen;
                reply = (char *)realloc(reply, replylen);
                if (reply == NULL)
                {
                    goto ERROR_RETURN;
                }
                memcpy(reply + replylen - recvslen, recvs, recvslen);
            }
            
        }

        if (_read_message(reply, replylen, ctx) == 1) // TODO: FIX here
        {
            goto ERROR_RETURN;
        }
        reply += 5;
        replylen -= 5;

    }
    return 1;
ERROR_RETURN:
    free(state);
    return 0;

}

int peer_download(peer *p, char *info_hash, const char *peerid, piecework *pw)
{
    char *recv;
    char *bitfield;
    int bitfieldlen;
    int sock;
    _peer_context *ctx;
    if ((recv = _handshake(p, info_hash, peerid, &sock)) == NULL)
    {
        return 0;
    }
    bitfieldlen = _build_bitfield(recv, &bitfield);
    ctx = _peer_context_init(sock, bitfield, bitfieldlen, info_hash);

    char *unchoke_recvs = NULL;
    int unchoke_recvslen = 0;
    unchoke_recvslen = _send_unchoke(sock, &unchoke_recvs);
    char *interested_recvs = NULL;
    int interested_recvslen = 0;
    interested_recvslen = _send_interested(sock, &interested_recvs);
    char *recv2 = (char *)malloc(sizeof(char) * (unchoke_recvslen + interested_recvslen));
    memcpy(recv2, unchoke_recvs, unchoke_recvslen);
    memcpy(recv2 + unchoke_recvslen, interested_recvs, interested_recvslen);
    free(unchoke_recvs);
    free(interested_recvs);
    piecework *ppw;
    for (ppw = pw; ppw != NULL; ppw = ppw->next)
    {
        if (!piecework_has_piece(bitfield, bitfieldlen, ppw->index))
        {
            continue;
        }
        _peer_download(ctx, ppw, recv2, unchoke_recvslen + interested_recvslen);

    }
    
    return 0;

}