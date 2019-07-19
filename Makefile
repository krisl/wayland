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

eglexample: eglexample.c
	cc -o eglexample eglexample.c -lwayland-client -lwayland-egl -lEGL

seat: seat.c
	cc -o seat seat.c -lwayland-client

pointer: pointer.c
	cc -o pointer pointer.c -lwayland-client -lwayland-egl -lEGL -lGLESv2

cursor: cursor.c
	cc -o cursor cursor.c -lwayland-client -lwayland-egl -lEGL -lGLESv2 -lwayland-cursor
