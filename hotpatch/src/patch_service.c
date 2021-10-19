#include "include/patch_service.h"
#include "include/utils.h"
#include <stddef.h>
#include "include/iotpatch.h"
#include "libebpf/include/ebpf_allocator.h"
#include "app/ihp_config.h"

enum update_protocol {
	EXIT = 0,
	LOAD, // load patch
	HEARTBEAT,
};

enum context_status {
	WAIT_FOR_INIT = 0,
	RUNNING = 1,
	//DISCONNECT = 2
};

static service_context svr_ctx = {0};
static patch_service svr = {0};
static uint8_t buf[4] = {0};

static void dispatch_patch_command(int cmd);

static void wait_for_patch() {
	while (true) {
		if (svr_ctx.status != RUNNING) {
			return;
		}

		size_t sz = svr.receive_buf(&svr_ctx, buf, 1);
		if (sz != 1) {
			DEBUG_LOG("Update client exist: %u\n", sz);
			svr.dis_connect(&svr_ctx);
			return;
		}
		int cmd = byte2int(buf, 1);
		DEBUG_LOG("recv status: %u cmd:%d\n", sz, cmd);
		dispatch_patch_command(cmd);
	}
	
}

// linux
static void debug_dump_patch(uint8_t *buf, int len) {

}

void dispatch_patch_command(int cmd) {
	if (cmd == EXIT) {
		DEBUG_LOG("Update client exist!");
		svr.dis_connect(&svr_ctx);
		return;
	} else if (cmd == LOAD) {
		size_t sz = svr.receive_buf(&svr_ctx, buf, 2);
		int len = byte2int(buf, 2);
		DEBUG_LOG("recv patch: %u pkt:%d\n", sz, len);
		patch_payload paylod;
		int pkt_len = len - 16;
		paylod.pkt = ebpf_malloc(pkt_len);
		svr.receive_buf(&svr_ctx, paylod.sign, 16);
		svr.receive_buf(&svr_ctx, paylod.pkt, pkt_len);
		patch_desc *patch = (patch_desc*) paylod.pkt;
		DEBUG_LOG("packet size: %d patch type:%d code_len:%d addr:0x%08x\n", pkt_len, patch->type, patch->code_len, patch->inst_addr);

		notify_new_patch(patch);
	} else if (cmd == HEARTBEAT) {
		svr_ctx.status = RUNNING;
		uint8_t buf[3] = {0};
		buf[0] = HEARTBEAT;
		int sz = svr.send_buf(&svr_ctx, buf, 1);
		DEBUG_LOG("heartbeat packet: %u\n", sz);
		if (sz != 1) {
			svr.dis_connect(&svr_ctx);
		}
	}
}

/*
 socket patch service impl
*/
#ifdef LINUX_TEST
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

static bool linux_setup_connect(service_context *ctx) {
	struct sockaddr_in serv_addr;
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		DEBUG_LOG("\n Error : Could not create client socket \n");
		return false;
	} 
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8000);
	const char *ip = "127.0.0.1";
	inet_aton(ip, &serv_addr.sin_addr);

	// DEBUG_LOG("Client connect to: %s\n", SERVER_ADDR);

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		DEBUG_LOG("\n Error : Client Connect Failed \n");
		return false;
	}

	ctx->sockfd = sockfd;
	ctx->status = RUNNING;
	return true;
}
	
static size_t linux_receive_buf(service_context *ctx, uint8_t *buf, size_t len) {
	return recv(ctx->sockfd, buf, len, 0);
}

static size_t linux_send_buf(service_context *ctx, uint8_t *buf, size_t len) {
	return send(ctx->sockfd, buf, len, 0);
}

static void linux_dis_connect(service_context *ctx) {
	if (ctx->status != WAIT_FOR_INIT) {
		DEBUG_LOG("Close client socket!\n");
		//shutdown(ctx->sockfd, 2);
		close(ctx->sockfd);
	}
	ctx->status = WAIT_FOR_INIT;
}

static bool setup_linux_service() {
	svr.setup_connect = linux_setup_connect;
	svr.receive_buf = linux_receive_buf;
	svr.send_buf = linux_send_buf;
	svr.dis_connect = linux_dis_connect;
	return svr.setup_connect(&svr_ctx);
}

static void* run_patch_task(void * args) {
	DEBUG_LOG("linux_patch_service_init\n");
	
	if (!setup_linux_service()) {
		return NULL;
	}

	wait_for_patch();

	DEBUG_LOG("Patch client exit!\n");
	return NULL;
}

static bool linux_patch_service_init() {
	if (svr_ctx.status != WAIT_FOR_INIT) {
		DEBUG_LOG("Client is already running.");
		return false;
	}

	pthread_t task;
	int res = pthread_create(&task, NULL, run_patch_task, NULL);
	// run_patch_task();
	//pthread_join(task, NULL);
	return true;
}
#endif

#ifdef ZEPHYR_OS
#include <string.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(iotpatch_test, LOG_LEVEL_DBG);

#include <zephyr.h>
#include <errno.h>
#include <stdio.h>

#include <net/socket.h>
#include <net/tls_credentials.h>

#ifdef DEV_COAP
#include "common.h"
// #endif

// #ifdef DEV_MQTT
// #include "config.h"
// #endif

static void* run_patch_task(void * args);

K_THREAD_DEFINE(patch_tcp4_thread_id, STACK_SIZE,
	run_patch_task, NULL, NULL, NULL,
	THREAD_PRIORITY, 0, K_FOREVER);

struct configs conf = {
	.ipv4 = {
		.proto = "IPv4",
		.udp.sock = INVALID_SOCK,
		.tcp.sock = INVALID_SOCK,
	},
	.ipv6 = {
		.proto = "IPv6",
		.udp.sock = INVALID_SOCK,
		.tcp.sock = INVALID_SOCK,
	},
};

static int start_tcp_proto(struct data *data, struct sockaddr *addr,
			   socklen_t addrlen)
{
	int ret;

	LOG_INF("Before sleep\n");

	// k_sleep(20000);

	LOG_INF("After sleep\n");

	for (int i = 0; i < 20; ++i) {
		LOG_INF("hehe\n");
	#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
		// data->tcp.sock = socket(addr->sa_family, SOCK_STREAM, IPPROTO_TLS_1_2);
	#else
		data->tcp.sock = socket(addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
	#endif
		if (data->tcp.sock < 0) {
			LOG_ERR("Failed to create client TCP socket (%s): %d", data->proto,
				errno);
			continue;
		}
		ret = connect(data->tcp.sock, addr, addrlen);
		if (ret < 0) {
			LOG_ERR("Cannot connect to client TCP remote (%s): %d", data->proto,
				errno);
			ret = -errno;
		}
		k_sleep(500);
		if (ret >= 0) break;
	}

	return ret;
}

static bool zephyr_setup_connect(service_context *ctx) {
	// struct sockaddr_in serv_addr;
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PEER_PORT);
	inet_pton(AF_INET, CONFIG_NET_CONFIG_PEER_IPV4_ADDR,
		&serv_addr.sin_addr);
	int ret = start_tcp_proto(&conf.ipv4, (struct sockaddr *)&serv_addr,
				      sizeof(serv_addr));

	if(ret < 0)
	{
		DEBUG_LOG("\n Error : Client Connect Failed \n");
		return false;
	}

	ctx->sockfd = conf.ipv4.tcp.sock;
	ctx->status = RUNNING;
	LOG_INF("Client Connect Success!\n");
	return true;
}

	
static size_t zephyr_receive_buf(service_context *ctx, uint8_t *buf, size_t len) {
	return recv(ctx->sockfd, buf, len, 0);
}

static size_t zephyr_send_buf(service_context *ctx, uint8_t *buf, size_t len) {
	return send(ctx->sockfd, buf, len, 0);
}

static void zephyr_dis_connect(service_context *ctx) {
	if (ctx->status != WAIT_FOR_INIT) {
		DEBUG_LOG("Close client socket!\n");
		//shutdown(ctx->sockfd, 2);
		close(ctx->sockfd);
	}
	ctx->status = WAIT_FOR_INIT;
}

static bool setup_zephyr_service() {
	svr.setup_connect = zephyr_setup_connect;
	svr.receive_buf = zephyr_receive_buf;
	svr.send_buf = zephyr_send_buf;
	svr.dis_connect = zephyr_dis_connect;
	return svr.setup_connect(&svr_ctx);
}

static void* run_patch_task(void * args) {
	LOG_INF("zephyr_patch_service_init\n");
	
	if (!setup_zephyr_service()) {
		return NULL;
	}

	wait_for_patch();

	LOG_INF("Patch client exit!\n");
	return NULL;
}

static bool zephyr_patch_service_init() {
	if (svr_ctx.status != WAIT_FOR_INIT) {
		LOG_INF("Client is already running.");
		return false;
	}

	if (IS_ENABLED(CONFIG_NET_IPV4)) {
		k_thread_start(patch_tcp4_thread_id);
	}
	// run_patch_task();
	//pthread_join(task, NULL);
	return true;
}
#endif // end DEV_COAP

#endif // ZEPHYR_OS

bool start_patch_service() {
	init_patch_sys();
	#if defined(LINUX_TEST)
	return linux_patch_service_init();
	#endif

	// #if defined(ZEPHYR_OS)
	// return zephyr_patch_service_init();
	// #endif

	// if (svr.ctx == NULL) {
	// 	DEBUG_LOG("ERROR: Patch Service Init Failed\n");
	// 	return false;
	// }

	// wait_for_patch();
	return false;
}
