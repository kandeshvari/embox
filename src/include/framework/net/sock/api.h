/**
 * @file
 * @brief The external API for accessing available net sockets.
 *
 * @date 05.07.11
 * @author Dmitry Zubarevich
 */

#ifndef FRAMEWORK_NET_SOCK_API_H_
#define FRAMEWORK_NET_SOCK_API_H_

#include __impl_x(framework/net/sock/api_impl.h)

#define net_sock_foreach(net_sock_ptr) \
	__net_sock_foreach(net_sock_ptr)

extern int net_sock_support(int family, int type);
extern const struct net_sock * net_sock_lookup(int family,
		int type, int protocol);

#endif /* FRAMEWORK_NET_SOCK_API_H_ */
