// A test and an example usage of the socket
// interface. 
// Before running the test, "make ping" 
// in the build directory.

#include <net/socket.h>
#include <kernel/printk.h>

void socket_test() 
{
    // create a socket, the dest ip address is 10.0.2.2
    // when we use the socket to send a packet to 10.0.2.2,
    // qemu delivers it to the application(ie. make ping) 
    // on the host computer running qemu.
    Socket *sock = sock_open(MAKE_IP_ADDR(10, 0, 2, 2), 2000, 22307);
    ASSERT(sock != NULL);

    // write a message to ping server. 
    sock_write(sock, "This is qemu!\n", 15);

    static char buf[256];
    for (int i = 0; i < 20; i++) {
        // receive the message from ping server.
        sock_read(sock, buf, sizeof(buf));
        // should print "This is a ping!"
        printk("%s", buf);
    }

    // close the socket. OK.
    sock_close(sock);
    printk("socket test (may be) OK.\n");
}
