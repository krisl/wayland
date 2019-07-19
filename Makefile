connect: connect.c
	cc -o connect connect.c -lwayland-client

registry: registry.c
	cc -o registry registry.c -lwayland-client

surface: surface.c
	cc -o surface surface.c -lwayland-client

sharedmem: sharedmem.c
	cc -o sharedmem sharedmem.c -lwayland-client

surface2: surface2.c
	cc -o surface2 surface2.c -lwayland-client

damage: damage.c
	cc -o damage damage.c -lwayland-client
