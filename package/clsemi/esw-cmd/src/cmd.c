#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>

static int send_nl_msg(int sd, uint8_t *hdr, int len)
{
	struct iovec iov;
	struct sockaddr_nl dest_addr;
	struct msghdr msg;
	struct nlmsghdr *nlh;

	nlh = (struct nlmsghdr *)(hdr - NLMSG_HDRLEN);
	memset(nlh, 0, NLMSG_HDRLEN);
	nlh->nlmsg_len = NLMSG_LENGTH(len);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_type = 1;

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = 0;

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	return sendmsg(sd, &msg, 0);
}

int main(int argc, const char *argv[])
{
	int sd, offset = 0, i;
	struct sockaddr_nl local;
	char buf[2000] = {0};
	struct sockaddr_nl kernel_addr;
	struct iovec iov;
	struct msghdr rcv_msg;
	struct nlmsghdr *nlh;
	char rcv_buf[4096];
	ssize_t recv_len;
	struct nlmsgerr *err;
	char *payload;
	int payload_len;

	sd = socket(PF_NETLINK, SOCK_RAW, 30);
	if (sd < 0) {
		printf("Can't create netlink socket: %s\n", strerror(errno));
		return -errno;
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_pid = getpid();
	local.nl_groups = 0;

	if (bind(sd, (struct sockaddr *)&local, sizeof(local))) {
		close(sd);
		printf("netlink: can't bind port: %s\n", strerror(errno));
		return -errno;
	}

	uint8_t *msg_data = (uint8_t *)buf + NLMSG_SPACE(0);
	for (i = 1; i < argc; i++) {
		offset += snprintf((char *)(msg_data + offset),
				  sizeof(buf) - NLMSG_SPACE(0) - offset,
				  "%s ", argv[i]);
	}

	if (send_nl_msg(sd, msg_data, offset + 1) < 0) {
		close(sd);
		printf("send_nl_msg failed: %s\n", strerror(errno));
		return -errno;
	}

	memset(&iov, 0, sizeof(iov));
	iov.iov_base = rcv_buf;
	iov.iov_len = sizeof(rcv_buf);

	memset(&rcv_msg, 0, sizeof(rcv_msg));
	rcv_msg.msg_name = &kernel_addr;
	rcv_msg.msg_iov = &iov;
	rcv_msg.msg_iovlen = 1;

	/* Our switch driver uses different nlmsg_type values to indicate whether netlink
	 * message transmission is complete.If NLMSG_DONE is received, it means the
	 * transmission is completed and this program can exit. Plese refer to driver code
	 * for details.
	 */
	while (1) {
		memset(rcv_buf, 0, sizeof(rcv_buf));
		memset(&kernel_addr, 0, sizeof(kernel_addr));

		rcv_msg.msg_namelen = sizeof(kernel_addr);

		recv_len = recvmsg(sd, &rcv_msg, 0);
		if (recv_len < 0) {
			close(sd);
			printf("recvmsg failed: %s\n", strerror(errno));
			return -errno;
		}

		for (nlh = (struct nlmsghdr *)rcv_buf; NLMSG_OK(nlh, (size_t)recv_len);
		     nlh = NLMSG_NEXT(nlh, recv_len)) {
			if (nlh->nlmsg_type == NLMSG_ERROR) {
				err = NLMSG_DATA(nlh);
				printf("Kernel error: %d\n", -err->error);
				break;
			}

			payload = NLMSG_DATA(nlh);
			payload_len = nlh->nlmsg_len - NLMSG_HDRLEN;

			if (payload_len > 0) {
				payload[payload_len - 1] = '\0';
				printf("%s", payload);
			}
			if (nlh->nlmsg_type == NLMSG_NOOP)
				break;
			else if (nlh->nlmsg_type == NLMSG_DONE)
				goto done;
		}
	}

done:
	close(sd);
	return 0;
}
