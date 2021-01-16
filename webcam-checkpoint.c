#include "libuvc/libuvc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


void outputValue(FILE *fp, char *parameterName, int value, uvc_error_t errorCode)
{
	if (errorCode < 0)
	{
		//uvc_perror(errorCode, parameterName);
		// silently fail as the webcam might not support this parameter
	}
	else
	{
		fprintf(fp, "%s=%d\n", parameterName, value);
	}
}


void displayWritingErrors(char *parameterName, uvc_error_t errorCode)
{
	if (errorCode < 0)
	{
		uvc_perror(errorCode, "UVC error");
		fprintf(stderr, "Failed to write parameter %s to camera\n", parameterName);
	}
}


int main(int argc, char **argv)
{
	if (argc != 3 || (strcmp(argv[1], "load") && strcmp(argv[1], "save")))
	{
		fprintf(stderr, "Usage: %s load|save <filename>\n", argv[0]);
		exit(1);
	}

	FILE *fp;
	bool saving = false;

	if (strcmp(argv[1], "load") == 0)
	{
		fp = fopen(argv[2], "r");
	}
	else
	{
		fp = fopen(argv[2], "w");
		saving = true;
	}

	if (!fp)
	{
		fprintf(stderr, "Failed to open %s\n", argv[2]);
		exit(1);
	}

	uvc_context_t *ctx;
	uvc_device_t *dev;
	uvc_device_handle_t *devh;
	uvc_stream_ctrl_t ctrl;
	uvc_error_t errorCode;

	/* Initialize a UVC service context. Libuvc will set up its own libusb
	* context. Replace NULL with a libusb_context pointer to run libuvc
	* from an existing libusb context. */
	errorCode = uvc_init(&ctx, NULL);

	if (errorCode < 0)
	{
		uvc_perror(errorCode, "uvc_init");
		return errorCode;
	}

	//puts("UVC initialized");

	/* Locates the first attached UVC device, stores in dev */
	errorCode = uvc_find_device(ctx, &dev, 0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
	if (errorCode < 0)
	{
		uvc_perror(errorCode, "uvc_find_device"); /* no devices found */
	}
	else
	{
		//puts("Device found");

		/* Try to open the device: requires exclusive access */
		errorCode = uvc_open(dev, &devh);
		if (errorCode < 0)
		{
			uvc_perror(errorCode, "uvc_open"); /* unable to open device */
		}
		else
		{
			//puts("Device opened");

			/* Print out a message containing all the information that libuvc
			* knows about the device */
			//uvc_print_diag(devh, stderr);

			if (saving)
			{
				// saving 

				uint8_t value8;
				uint16_t value16;
				uint32_t value32;

				// these are saved in a specific order, so when they're read back in
				// any manual settings are applied *after* setting the webcam to manual mode

				errorCode = uvc_get_ae_mode(devh, &value8, UVC_GET_CUR);
				outputValue(fp, "exposure_auto", value8, errorCode);
				if (value8 == 1)
				{
					// manual exposure, so output exposure value
					errorCode = uvc_get_exposure_abs(devh, &value32, UVC_GET_CUR);
					outputValue(fp, "exposure_absolute", value32, errorCode);
				}

				errorCode = uvc_get_ae_priority(devh, &value8, UVC_GET_CUR);
				outputValue(fp, "exposure_auto_priority", value8, errorCode);

				errorCode = uvc_get_gain(devh, &value16, UVC_GET_CUR);
				outputValue(fp, "gain", value16, errorCode);

				errorCode = uvc_get_backlight_compensation(devh, &value16, UVC_GET_CUR);
				outputValue(fp, "backlight_compensation", value16, errorCode);

				errorCode = uvc_get_power_line_frequency(devh, &value8, UVC_GET_CUR);
				outputValue(fp, "power_line_frequency", value8, errorCode);

				errorCode = uvc_get_white_balance_temperature_auto(devh, &value8, UVC_GET_CUR);
				outputValue(fp, "white_balance_temperature_auto", value8, errorCode);
				if (value8 == 0)
				{
					// manual white balance, so output temperature
					errorCode = uvc_get_white_balance_temperature(devh, &value16, UVC_GET_CUR);
					outputValue(fp, "white_balance_temperature", value16, errorCode);
				}

				int16_t brightness;
				errorCode = uvc_get_brightness(devh, &brightness, UVC_GET_CUR);
				outputValue(fp, "brightness", brightness, errorCode);

				errorCode = uvc_get_contrast(devh, &value16, UVC_GET_CUR);
				outputValue(fp, "contrast", value16, errorCode);

				errorCode = uvc_get_saturation(devh, &value16, UVC_GET_CUR);
				outputValue(fp, "saturation", value16, errorCode);

				errorCode = uvc_get_sharpness(devh, &value16, UVC_GET_CUR);
				outputValue(fp, "sharpness", value16, errorCode);

				errorCode = uvc_get_focus_auto(devh, &value8, UVC_GET_CUR);
				outputValue(fp, "focus_auto", value8, errorCode);
				if (value8 == 0)
				{
					// manual focus, so output value
					errorCode = uvc_get_focus_abs(devh, &value16, UVC_GET_CUR);
					outputValue(fp, "focus_absolute", value16, errorCode);
				}

				errorCode = uvc_get_zoom_abs(devh, &value16, UVC_GET_CUR);
				outputValue(fp, "zoom_absolute", value16, errorCode);

				int32_t pan, tilt;
				errorCode = uvc_get_pantilt_abs(devh, &pan, &tilt, UVC_GET_CUR);
				outputValue(fp, "pan_absolute", pan, errorCode);
				outputValue(fp, "tilt_absolute", tilt, errorCode);
			}
			else
			{
				// loading

				// we have to save pan and tilt and set them both at the same time
				int32_t pan = 0;
				int32_t tilt = 0;

				char *line = NULL;
				size_t len = 0;

				while (getline(&line, &len, fp) != -1)
				{
					char *pos = strchr(line, '=');
					if (pos != NULL)
					{
						// null terminatator instead of = to split strings
						*pos = 0;
						char *name = line;
						char *valueString = pos + 1;
						int value = atoi(valueString);

						// printf("name=%s\n", name);
						// printf("value=%d\n\n", value);

						if (!strcmp(name, "exposure_auto"))
						{
							errorCode = uvc_set_ae_mode(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "exposure_auto_priority"))
						{
							errorCode = uvc_set_ae_priority(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "exposure_absolute"))
						{
							errorCode = uvc_set_exposure_abs(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "gain"))
						{
							errorCode = uvc_set_gain(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "backlight_compensation"))
						{
							errorCode = uvc_set_backlight_compensation(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "power_line_frequency"))
						{
							errorCode = uvc_set_power_line_frequency(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "white_balance_temperature_auto"))
						{
							errorCode = uvc_set_white_balance_temperature_auto(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "white_balance_temperature"))
						{
							errorCode = uvc_set_white_balance_temperature(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "brightness"))
						{
							errorCode = uvc_set_brightness(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "contrast"))
						{
							errorCode = uvc_set_contrast(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "saturation"))
						{
							errorCode = uvc_set_saturation(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "sharpness"))
						{
							errorCode = uvc_set_sharpness(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "focus_auto"))
						{
							errorCode = uvc_set_focus_auto(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "focus_absolute"))
						{
							errorCode = uvc_set_focus_abs(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "zoom_absolute"))
						{
							errorCode = uvc_set_zoom_abs(devh, value);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "pan_absolute"))
						{
							pan = value;
							errorCode = uvc_set_pantilt_abs(devh, pan, tilt);
							displayWritingErrors(name, errorCode);
						}
						else if (!strcmp(name, "tilt_absolute"))
						{
							tilt = value;
							errorCode = uvc_set_pantilt_abs(devh, pan, tilt);
							displayWritingErrors(name, errorCode);
						}
					}
				}

				free(line);
			}

			/* Release our handle on the device */
			uvc_close(devh);
			//puts("Device closed");
		}
		/* Release the device descriptor */
		uvc_unref_device(dev);
	}

	/* Close the UVC context. This closes and cleans up any existing device handles,
	* and it closes the libusb context if one was not provided. */
	uvc_exit(ctx);
	//puts("UVC exited");

	fclose(fp);

	return 0;
}
