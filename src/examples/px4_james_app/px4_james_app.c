/**
 * @file px4_james_app.c
 * Minimal application example for PX4 autopilot
 *
 * @author Example User <mail@example.com>
 */

#include <px4_log.h>
#include <poll.h>
#include <uORB/topics/sensor_combined.h>
#include <px4_config.h>
#include <px4_tasks.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <math.h>

#include <uORB/uORB.h>
#include <uORB/topics/sensor_combined.h>
#include <uORB/topics/vehicle_attitude.h>
#include <uORB/topics/vehicle_gps_position.h>
#include <uORB/topics/sensor_accel.h>

__EXPORT int px4_james_app_main(int argc, char *argv[]);

int px4_james_app_main(int argc, char *argv[])
{
    PX4_INFO("Hello Sky, this is J, trying to read sensor data!");
    /* subscribe to sensor_combined topic */
    int snsor_sub_fd = orb_subscribe(ORB_ID(sensor_combined));
    /* subscribe to vehicle_global_position topic */
    int position_sub_fd = orb_subscribe(ORB_ID(vehicle_gps_position));
    /* subscribe to sensor_accel topic */
    int accel_sub_fd = orb_subscribe(ORB_ID(sensor_accel));



    /* limit the update rate to 20 Hz */
    orb_set_interval(snsor_sub_fd, 50);
    orb_set_interval(position_sub_fd, 50);
    orb_set_interval(accel_sub_fd, 50);


    
    /* advertise attitude topic */
     struct vehicle_attitude_s att;
    memset(&att, 0, sizeof(att));
    orb_advert_t att_pubb = orb_advertise(ORB_ID(vehicle_attitude), &att);

    /* one could wait for multiple topics with this technique, just using one here */
    px4_pollfd_struct_t fds[] = {
      { .fd = snsor_sub_fd,   .events = POLLIN },
      { .fd = accel_sub_fd, .events = POLLIN},
      { .fd = position_sub_fd, .events = POLLIN },
        /* there could be more file descriptors here, in the form like:
         * { .fd = other_sub_fd,   .events = POLLIN },
         */
    };

    int error_counter = 0;

    for (int i = 0; i < 3; i++) {
        /* wait for sensor update of 3 file descriptor for 1000 ms (1 second) */
        int poll_ret = px4_poll(fds, 3, 1000);

        /* handle the poll result */
        if (poll_ret == 0) {
            /* this means none of our providers is giving us data */
            PX4_ERR("Got no data within a second");

        } else if (poll_ret < 0) {
            /* this is seriously bad - should be an emergency */
            if (error_counter < 10 || error_counter % 50 == 0) {
                /* use a counter to prevent flooding (and slowing us down) */
                PX4_ERR("ERROR return value from poll(): %d", poll_ret);
            }

            error_counter++;

        } else {

	    if (fds[0].revents & POLLIN) {
	      /* obtained data for the first file descriptor */
	     struct sensor_combined_s raw;
	     /* copy sensors raw data into local buffer */
	    orb_copy(ORB_ID(sensor_combined), snsor_sub_fd, &raw);
	    PX4_INFO ("Accelerometer:\t%8.4f \t%8.4f\t%8.4f",
	           (double)raw.accelerometer_m_s2[0],
	         (double)raw.accelerometer_m_s2[1],
	         (double)raw.accelerometer_m_s2[2]);
	    PX4_INFO("Gyro:\t%8.4f \t%8.4f\t%8.4f",
	         (double)raw.gyro_rad[0],
	         (double)raw.gyro_rad[1],
	         (double)raw.gyro_rad[2]);

                /* set att and publish this information for other apps
                 the following does not have any meaning, it's just an example
                */
	    att.q[0] = raw.accelerometer_m_s2[0];
	    att.q[1] = raw.accelerometer_m_s2[1];
	    att.q[2] = raw.accelerometer_m_s2[2];

	  	      orb_publish(ORB_ID(vehicle_attitude), att_pubb, &att);
	    }

	    if (fds[1].revents & POLLIN) {
	      struct sensor_accel_s rawacc;
	      orb_copy(ORB_ID(sensor_accel), accel_sub_fd, &rawacc);
	      PX4_INFO ("Accel temperature:\t%8.4f",
			(double)rawacc.temperature);}

	    if (fds[2].revents & POLLIN) {
	      struct vehicle_gps_position_s rawgps;
	      orb_copy(ORB_ID(vehicle_gps_position), position_sub_fd, &rawgps);
	      PX4_INFO("Latitude: \t%8.4f\t Longitude\t%8.4f\t Altitude\t%8.4f",
		       (double)rawgps.lat,
		       (double)rawgps.lon,
		       (double)rawgps.alt);}

            /* there could be more file descriptors here, in the form like:
             * if (fds[1..n].revents & POLLIN) {}
             */
        }
    }

    PX4_INFO("exiting");

    return 0;
}
