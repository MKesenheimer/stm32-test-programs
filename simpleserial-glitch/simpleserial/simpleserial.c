// simpleserial.c

#include "simpleserial.h"
#include <stdint.h>
#include "hal.h"

#define MAX_SS_CMDS 16
static int num_commands = 0;

#define MAX_SS_LEN 256

// 0xA6 formerly 
#define CW_CRC 0x4D 
uint8_t ss_crc(uint8_t *buf, uint8_t len)
{
    unsigned int k = 0;
    uint8_t crc = 0x00;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++) {
            crc = crc & 0x80 ? (crc << 1) ^ CW_CRC: crc << 1;
        }
    }
    return crc;

}

// [B_STUFF, CMD, SCMD, LEN, B_STUFF, DATA..., CRC, TERM]

int hex_decode(int len, char* ascii_buf, uint8_t* data_buf)
{
    for(int i = 0; i < len; i++)
    {
        char n_hi = ascii_buf[2*i];
        char n_lo = ascii_buf[2*i+1];

        if(n_lo >= '0' && n_lo <= '9')
            data_buf[i] = n_lo - '0';
        else if(n_lo >= 'A' && n_lo <= 'F')
            data_buf[i] = n_lo - 'A' + 10;
        else if(n_lo >= 'a' && n_lo <= 'f')
            data_buf[i] = n_lo - 'a' + 10;
        else
            return 1;

        if(n_hi >= '0' && n_hi <= '9')
            data_buf[i] |= (n_hi - '0') << 4;
        else if(n_hi >= 'A' && n_hi <= 'F')
            data_buf[i] |= (n_hi - 'A' + 10) << 4;
        else if(n_hi >= 'a' && n_hi <= 'f')
            data_buf[i] |= (n_hi - 'a' + 10) << 4;
        else
            return 1;
    }

    return 0;
}

typedef struct ss_cmd
{
    char c;
    unsigned int len;
    uint8_t (*fp)(uint8_t, uint8_t, uint8_t, uint8_t *);
} ss_cmd;
static ss_cmd commands[MAX_SS_CMDS];

void ss_puts(char *x)
{
    do {
        putch(*x);
    } while (*++x);
}

#define FRAME_BYTE 0x00

uint8_t ss_get_commands(uint8_t cmd, uint8_t scmd, uint8_t len, uint8_t *data)
{
    uint8_t cmd_chars[MAX_SS_CMDS];
    for (uint8_t i = 0; i < (num_commands & 0xFF); i++) {
        cmd_chars[i] = commands[i].c;
    }

    simpleserial_put('r', num_commands & 0xFF, (void *)cmd_chars);
    return 0x00;

}

uint8_t stuff_data(uint8_t *buf, uint8_t len)
{
    uint8_t i = 1;
    uint8_t last = 0;
    for (; i < len; i++) {
        if (buf[i] == FRAME_BYTE) {
            buf[last] = i - last;
            last = i;
        }
    }
    return 0x00;
}

uint8_t unstuff_data(uint8_t *buf, uint8_t len)
{
    uint8_t next = buf[0];
    buf[0] = 0x00;
    //len -= 1;
    uint8_t tmp = next;
    while ((next < len) && tmp != 0) {
        tmp = buf[next];
        buf[next] = FRAME_BYTE;
        next += tmp;
    }
    return next;
}

// Set up the SimpleSerial module by preparing internal commands
// This just adds the "v" command for now...
void simpleserial_init()
{
    simpleserial_addcmd('w', 0, ss_get_commands);
}

int simpleserial_addcmd(char c, unsigned int len, uint8_t (*fp)(uint8_t, uint8_t, uint8_t, uint8_t*))
{
    if(num_commands >= MAX_SS_CMDS) {
        putch('a');
        return 1;
    }

    if(len >= MAX_SS_LEN) {
        putch('b');
        return 1;
    }

    commands[num_commands].c   = c;
    commands[num_commands].len = len;
    commands[num_commands].fp  = fp;
    num_commands++;

    return 0;
}

void simpleserial_get(void)
{
    uint8_t data_buf[MAX_SS_LEN];
    uint8_t err = 0;

    for (int i = 0; i < 4; i++) {
        data_buf[i] = getch(); //PTR, cmd, scmd, len
        if (data_buf[i] == FRAME_BYTE) {
            err = SS_ERR_FRAME_BYTE;
            goto ERROR;
        }
    }
    uint8_t next_frame = unstuff_data(data_buf, 4);

    // check for valid command
    uint8_t c = 0;
    for(c = 0; c < num_commands; c++)
    {
        if(commands[c].c == data_buf[1])
            break;
    }

    if (c == num_commands) {
        err = SS_ERR_CMD;
        goto ERROR;
    }

    //check that next frame not beyond end of message
    // account for cmd, scmd, len, data, crc, end of frame
    if ((data_buf[3] + 5) < next_frame) {
        err = SS_ERR_LEN;
        goto ERROR;
    }

    // read in data
    // eq to len + crc + frame end
    int i = 4;
    for (; i < data_buf[3] + 5; i++) {
        data_buf[i] = getch();
        if (data_buf[i] == FRAME_BYTE) {
            err = SS_ERR_FRAME_BYTE;
            goto ERROR;
        }
    }

    //check that final byte is the FRAME_BYTE
    data_buf[i] = getch();
    if (data_buf[i] != FRAME_BYTE) {
        err = SS_ERR_LEN;
        goto ERROR;
    }

    //fully unstuff data now
    unstuff_data(data_buf + next_frame, i - next_frame + 1);

    //calc crc excluding original frame offset and frame end and crc
    uint8_t crc = ss_crc(data_buf+1, i-2);
    if (crc != data_buf[i-1]) {
        err = SS_ERR_CRC;
        goto ERROR;
    }

    err = commands[c].fp(data_buf[1], data_buf[2], data_buf[3], data_buf+4);

ERROR:
    simpleserial_put('e', 0x01, &err);
    return;
}

void simpleserial_put(char c, uint8_t size, uint8_t* output)
{
    uint8_t data_buf[MAX_SS_LEN];
    data_buf[0] = 0x00;
    data_buf[1] = c;
    data_buf[2] = size;
    int i = 0;
    for (; i < size; i++) {
        data_buf[i + 3] = output[i];
    }
    data_buf[i + 3] = ss_crc(data_buf+1, size+2);
    data_buf[i + 4] = 0x00;
    stuff_data(data_buf, i + 5);
    for (int i = 0; i < size + 5; i++) {
        putch(data_buf[i]);
    }
}