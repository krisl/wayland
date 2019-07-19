connect: connect.c
	cc -o connect connect.c -lwayland-client

registry: registry.c
	cc -o registry registry.c -lwayland-client

suface: surface.c
	cc -o surface surface.c -lwayland-client
