#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <math.h>

#include <wayland-client.h>
#include "shared/os-compatibility.h"

struct display {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shell *shell;
	struct wl_shm *shm;
	uint32_t formats;
};

struct buffer {
	struct wl_buffer *buffer;
	void *shm_data;
	int busy;
};

struct window {
	struct display *display;
	int width, height;
	struct wl_surface *surface;
	struct wl_shell_surface *shell_surface;
	struct buffer buffers[2];
	struct buffer *prev_buffer;
	struct wl_callback *callback;
};

static void
buffer_release(void *data, struct wl_buffer *buffer)
{
	struct buffer *mybuf = data;

	mybuf->busy = 0;
}

static const struct wl_buffer_listener buffer_listener = {
	buffer_release
};

static int
create_shm_buffer(struct display *display, struct buffer *buffer,
		  int width, int height, uint32_t format)
{
	struct wl_shm_pool *pool;
	int fd, size, stride;
	void *data;

	stride = width * 4;
	size = stride * height;

	fd = os_create_anonymous_file(size);
	if (fd < 0) {
		fprintf(stderr, "creating a buffer file for %d B failed: %m\n",
			size);
		return -1;
	}

	data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		fprintf(stderr, "mmap failed: %m\n");
		close(fd);
		return -1;
	}

	pool = wl_shm_create_pool(display->shm, fd, size);
	buffer->buffer = wl_shm_pool_create_buffer(pool, 0,
						   width, height,
						   stride, format);
	wl_buffer_add_listener(buffer->buffer, &buffer_listener, buffer);
	wl_shm_pool_destroy(pool);
	close(fd);

	buffer->shm_data = data;

	return 0;
}

static void
handle_ping(void *data, struct wl_shell_surface *shell_surface,
							uint32_t serial)
{
	wl_shell_surface_pong(shell_surface, serial);
}

static void
handle_configure(void *data, struct wl_shell_surface *shell_surface,
		 uint32_t edges, int32_t width, int32_t height)
{
}

static void
handle_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
}

static const struct wl_shell_surface_listener shell_surface_listener = {
	handle_ping,
	handle_configure,
	handle_popup_done
};

static struct window *
create_window(struct display *display, int width, int height)
{
	struct window *window;

	window = calloc(1, sizeof *window);
	if (!window)
		return NULL;

	window->callback = NULL;
	window->display = display;
	window->width = width;
	window->height = height;
	window->surface = wl_compositor_create_surface(display->compositor);
	window->shell_surface = wl_shell_get_shell_surface(display->shell,
							   window->surface);

	if (window->shell_surface)
		wl_shell_surface_add_listener(window->shell_surface,
					      &shell_surface_listener, window);

	wl_shell_surface_set_title(window->shell_surface, "simple-shm");

	wl_shell_surface_set_toplevel(window->shell_surface);

	return window;
}

static void
destroy_window(struct window *window)
{
	if (window->callback)
		wl_callback_destroy(window->callback);

	if (window->buffers[0].buffer)
		wl_buffer_destroy(window->buffers[0].buffer);
	if (window->buffers[1].buffer)
		wl_buffer_destroy(window->buffers[1].buffer);

	wl_shell_surface_destroy(window->shell_surface);
	wl_surface_destroy(window->surface);
	free(window);
}

static struct buffer *
window_next_buffer(struct window *window)
{
	struct buffer *buffer;
	int ret = 0;

	if (!window->buffers[0].busy)
		buffer = &window->buffers[0];
	else if (!window->buffers[1].busy)
		buffer = &window->buffers[1];
	else
		return NULL;

	if (!buffer->buffer) {
		ret = create_shm_buffer(window->display, buffer,
					window->width, window->height,
					WL_SHM_FORMAT_XRGB8888);

		if (ret < 0)
			return NULL;

		/* paint the padding */
		memset(buffer->shm_data, 0xff,
		       window->width * window->height * 4);
	}

	return buffer;
}
union x {
    struct color {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
    } c; 
    uint32_t val;
}colors[256];

uint8_t heatmap[250*250];// = 0;

static void
setup_palette() 
{
    //heatmap = malloc(250*250);
    int i;
    for (i = 0; i < 32; ++i) {
        /* black to blue, 32 values*/
        colors[i].c.b = i << 1;
         
        /* blue to red, 32 values*/
        colors[i + 32].c.r = i << 3;
        colors[i + 32].c.b =  64 - (i << 1);
        
        /*red to yellow, 32 values*/
        colors[i + 64].c.r = 255;
        colors[i + 64].c.g = i << 3;
        
        /* yellow to white, 162 */
        colors[i + 96].c.r = 255;
        colors[i + 96].c.g = 255;
        colors[i + 96].c.b = i << 2;
        colors[i + 128].c.r = 255;
        colors[i + 128].c.g = 255;
        colors[i + 128].c.b = 64 + (i << 2);
        colors[i + 160].c.r = 255;
        colors[i + 160].c.g = 255;
        colors[i + 160].c.b = 128 + (i << 2);
        colors[i + 192].c.r = 255;
        colors[i + 192].c.g = 255;
        colors[i + 192].c.b = 192 + i;
        colors[i + 224].c.r = 255;
        colors[i + 224].c.g = 255;
        colors[i + 224].c.b = 224 + i;
    } 
}

static uint32_t xrgb(uint8_t heat) {
    //return (colors[heat].val & 0x00ffffff) | (heat << 24);
    return colors[heat].val;
}

static int heat(int pos, uint32_t *pixel) {
    //return pixel[pos] >> 24;
    //return pixel[pos];
    return heatmap[pos];
}
static uint32_t * pixel;

static void setHeat(uint8_t heat, int pos ) {
    heatmap[pos] = heat;
    pixel[pos] = xrgb(heat);
}

static int
aSin(int i) 
{
    float rad = ((float)i * 0.703125) * 0.0174532;
    return sin(rad) * 1024;
}

static void
paint_pixels_plasma(void *image, int padding, int width, int height, uint32_t time)
{
    static int pos1=0, pos2=0, pos3=0, pos4 =0, tpos1=0, tpos2=0, tpos3=0, tpos4=0;
    int i,j;

    pixel = image;
    tpos4 = pos4;
    tpos3 = pos3;

    for(i=padding; i < height-padding; i++) {
        tpos1 = pos1 +5;
        tpos2 = pos2 +3;
        tpos3 &= 0x1ff; //lower 9 bits
        tpos4 &= 0x1ff;

        for(j=padding; j < width-padding; j++) {
            
            tpos1 &= 0x1ff;
            tpos2 &= 0x1ff;
                
            int x = aSin(tpos1) + aSin(tpos2) + aSin(tpos3) + aSin(tpos4);
            uint8_t index = 0x80 + (x>>4);
            pixel[i*width+j] = x & 0xff | (x/5) << 8 | (x/3) << 16; //index;

            tpos1 += 5;
            tpos2 += 3;
        }

        tpos3 += 1;
        tpos4 += 3;
    }

    // move the plasma
    pos1 += 9;
    pos3 += 8; 
}

static void
paint_pixels_fire(void *image, int padding, int width, int height, uint32_t time)
{
    pixel = image;
    int i, j,x, y, index, temp;
    j = width  * (height-padding - 1);  // bottom row
    for (i = padding; i < width-padding; i++) {
        int random = 1 + (int)(16.0 * (rand()/(RAND_MAX+1.0)));
        //pixel[i+j] = xrgb((random > 9) ? 0xff : 0x00);
        setHeat((random>9) ? 0xff: 0, i+j);
    }
    
    /* move fire upward, start at bottom */
    for (index = padding; index < height-padding; index++) {
        for (i = padding; i < width-padding; i++) {
            
            temp = heat(j+i, pixel); // left border pixel
            temp += heat(j+i-width, pixel); //left pixel above
           
            if (i == padding) { // left border
                temp += heat(j+i+1, pixel); // next right pixel
                temp /= 3; //average
            }
            else if (i == width-1-padding) {      // right border
                temp += heat(j+i-1, pixel);     // next left pixel
                temp /= 3; //average
            }
            else {
                temp += heat(j+i+1, pixel);   // right pixel
                temp += heat(j+i-1, pixel);   // left pixel
                temp /= 4; // average
            }

            if (temp > 1)
                temp -=1; //decay

            //pixel[j+i-width] = xrgb(temp);    // pixel above
            setHeat(temp, j+i-width);
        }
        j -= width;
    }
}

static void
paint_pixels(void *image, int padding, int width, int height, uint32_t time)
{
	const int halfh = padding + (height - padding * 2) / 2;
	const int halfw = padding + (width  - padding * 2) / 2;
	int ir, or;
	uint32_t *pixel = image;
	int y;

	/* squared radii thresholds */
	or = (halfw < halfh ? halfw : halfh) - 8;
	ir = or - 32;
	or *= or;
	ir *= ir;

	pixel += padding * width;
	for (y = padding; y < height - padding; y++) {
		int x;
		int y2 = (y - halfh) * (y - halfh);

		pixel += padding;
		for (x = padding; x < width - padding; x++) {
			uint32_t v;

			/* squared distance from center */
			int r2 = (x - halfw) * (x - halfw) + y2;

			if (r2 < ir)
				v = (r2 / 32 + time / 64) * 0x0080401;
			else if (r2 < or)
				v = (y + time / 32) * 0x0080401;
			else
				v = (x + time / 16) * 0x0080401;
			v &= 0x00ffffff;

			/* cross if compositor uses X from XRGB as alpha */
			if (abs(x - y) > 6 && abs(x + y - height) > 6)
				v |= 0xff000000;

			*pixel++ = v;
		}

		pixel += padding;
	}
}

static const struct wl_callback_listener frame_listener;

static void
redraw(void *data, struct wl_callback *callback, uint32_t time)
{
	struct window *window = data;
	struct buffer *buffer;

	buffer = window_next_buffer(window);
	if (!buffer) {
		fprintf(stderr,
			!callback ? "Failed to create the first buffer.\n" :
			"Both buffers busy at redraw(). Server bug?\n");
		abort();
	}

	paint_pixels_plasma(buffer->shm_data, 20, window->width, window->height, time);

	wl_surface_attach(window->surface, buffer->buffer, 0, 0);
	wl_surface_damage(window->surface,
			  20, 20, window->width - 40, window->height - 40);

	if (callback)
		wl_callback_destroy(callback);

	window->callback = wl_surface_frame(window->surface);
	wl_callback_add_listener(window->callback, &frame_listener, window);
	wl_surface_commit(window->surface);
	buffer->busy = 1;
}

static const struct wl_callback_listener frame_listener = {
	redraw
};

static void
shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
	struct display *d = data;

	d->formats |= (1 << format);
}

struct wl_shm_listener shm_listenter = {
	shm_format
};

static void
registry_handle_global(void *data, struct wl_registry *registry,
		       uint32_t id, const char *interface, uint32_t version)
{
	struct display *d = data;

	if (strcmp(interface, "wl_compositor") == 0) {
		d->compositor =
			wl_registry_bind(registry,
					 id, &wl_compositor_interface, 1);
	} else if (strcmp(interface, "wl_shell") == 0) {
		d->shell = wl_registry_bind(registry,
					    id, &wl_shell_interface, 1);
	} else if (strcmp(interface, "wl_shm") == 0) {
		d->shm = wl_registry_bind(registry,
					  id, &wl_shm_interface, 1);
		wl_shm_add_listener(d->shm, &shm_listenter, d);
	}
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
			      uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_handle_global_remove
};

static struct display *
create_display(void)
{
	struct display *display;

	display = malloc(sizeof *display);
	display->display = wl_display_connect(NULL);
	assert(display->display);

	display->formats = 0;
	display->registry = wl_display_get_registry(display->display);
	wl_registry_add_listener(display->registry,
				 &registry_listener, display);
	wl_display_roundtrip(display->display);
	if (display->shm == NULL) {
		fprintf(stderr, "No wl_shm global\n");
		exit(1);
	}

	wl_display_roundtrip(display->display);

	if (!(display->formats & (1 << WL_SHM_FORMAT_XRGB8888))) {
		fprintf(stderr, "WL_SHM_FORMAT_XRGB32 not available\n");
		exit(1);
	}

	wl_display_get_fd(display->display);
	
	return display;
}

static void
destroy_display(struct display *display)
{
	if (display->shm)
		wl_shm_destroy(display->shm);

	if (display->shell)
		wl_shell_destroy(display->shell);

	if (display->compositor)
		wl_compositor_destroy(display->compositor);

	wl_registry_destroy(display->registry);
	wl_display_flush(display->display);
	wl_display_disconnect(display->display);
	free(display);
}

static int running = 1;

static void
signal_int(int signum)
{
	running = 0;
}

int
main(int argc, char **argv)
{
	struct sigaction sigint;
	struct display *display;
	struct window *window;
	int ret = 0;

	display = create_display();
	window = create_window(display, 250, 250);
	if (!window)
		return 1;

	sigint.sa_handler = signal_int;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = SA_RESETHAND;
	sigaction(SIGINT, &sigint, NULL);

	/* Initialise damage to full surface, so the padding gets painted */
	wl_surface_damage(window->surface, 0, 0,
			  window->width, window->height);
    setup_palette();
	redraw(window, NULL, 0);

	while (running && ret != -1)
		ret = wl_display_dispatch(display->display);

	fprintf(stderr, "simple-shm exiting\n");
	destroy_window(window);
	destroy_display(display);

	return 0;
}
