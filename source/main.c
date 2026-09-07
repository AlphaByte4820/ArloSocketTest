#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOC_BUFFER_SIZE 0x100000
#define TEST_PORT 8001

static u32 *soc_buffer = NULL;

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("\x1b[2;1H=== ARLO SOCKET TEST ===\n\n");
    printf("Testing TCP port %d\n\n", TEST_PORT);

    soc_buffer = memalign(0x1000, SOC_BUFFER_SIZE);

    if (!soc_buffer) {
        printf("FAIL: memalign()\n");
        printf("\nPress START to exit.\n");

        while (aptMainLoop()) {
            hidScanInput();
            if (hidKeysDown() & KEY_START)
                break;
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        }

        gfxExit();
        return 0;
    }

    Result rc = socInit(soc_buffer, SOC_BUFFER_SIZE);

    printf("socInit: 0x%08lX\n", (unsigned long)rc);

    if (R_FAILED(rc)) {
        printf("FAIL: socInit\n");
        goto finished;
    }

    errno = 0;
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    printf("socket: %d\n", server);
    printf("errno : %d (%s)\n\n", errno, strerror(errno));

    if (server < 0)
        goto cleanup_soc;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(TEST_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    errno = 0;
    int br = bind(
        server,
        (struct sockaddr *)&addr,
        sizeof(addr)
    );

    printf("bind:   %d\n", br);
    printf("errno:  %d (%s)\n\n", errno, strerror(errno));

    if (br < 0)
        goto cleanup_socket;

    errno = 0;
    int lr = listen(server, 1);

    printf("listen: %d\n", lr);
    printf("errno:  %d (%s)\n\n", errno, strerror(errno));

    if (lr < 0)
        goto cleanup_socket;

    printf("==========================\n");
    printf("LISTENER SHOULD BE ACTIVE\n");
    printf("192.168.1.254:%d\n", TEST_PORT);
    printf("==========================\n\n");

    printf("Test port 8001 from Termux NOW.\n");
    printf("Do not press START yet.\n");

    while (aptMainLoop()) {
        hidScanInput();

        if (hidKeysDown() & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

cleanup_socket:
    closesocket(server);

cleanup_soc:
    socExit();

finished:
    if (soc_buffer)
        free(soc_buffer);

    printf("\nPress START to exit.\n");

    while (aptMainLoop()) {
        hidScanInput();

        if (hidKeysDown() & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
