#include "include/particle_simulation.h"
#include "include/mpu_driver.h"

#ifdef CONFIG_PARTICLE_SIMULATION_ENABLE
volatile bool        running = false;
int                  nFlakes;

Color* sandbuffer;

esp_err_t particle_init() {
  return init_mpu();
}

void particle_initSim(int flakes) {
	nFlakes = flakes;
	Adafruit_PixelDust(CONFIG_PARTICLE_SIMULATION_WIDTH, CONFIG_PARTICLE_SIMULATION_HEIGHT, nFlakes, 1, 64, true);
	running = true;
}

void particle_disp() {
   	int ax, ay, az;
	dimension_t                x, y;
	getAccel(&ax, &ay, &az);
	PixelDust_iterate(ax, ay, az);

	// Erase canvas and draw new snowflake positions
	for(int i=0; i<CONFIG_PARTICLE_SIMULATION_WIDTH*CONFIG_PARTICLE_SIMULATION_HEIGHT; i++) {
      sandbuffer[i].value = 0;
    }
    
	for(int i=0; i<nFlakes; i++) {
		PixelDust_getPosition(i, &x, &y);
      	sandbuffer[x+y*CONFIG_PARTICLE_SIMULATION_WIDTH].RGB[3] = x*4;
		sandbuffer[x+y*CONFIG_PARTICLE_SIMULATION_WIDTH].RGB[2] = y*4;
		sandbuffer[x+y*CONFIG_PARTICLE_SIMULATION_WIDTH].RGB[1] = 255-x*4;
	}
}

void particle_setBuffer(Color* framebuffer) {
  sandbuffer = framebuffer;
}

bool particle_status() {
	return running;
}

void particle_disable() {
	running = false;
}
#endif
