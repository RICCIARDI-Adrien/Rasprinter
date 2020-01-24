/** @file Main.c
 * Wait for a button press, render a label picture and print it.
 * @author Adrien RICCIARDI
 */
#include <errno.h>
#include <fcntl.h>
#include <linux/gpio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Rendered image width in pixels. */
#define MAIN_OUTPUT_IMAGE_WIDTH 1200
/** How many pixels to add as offset from the image left corner when rendering text. */
#define MAIN_IMAGE_TEXT_LEFT_OFFSET 190

/** The output file to generate. */
#define MAIN_OUTPUT_IMAGE_FILE_NAME "/tmp/Label.bmp"

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The file descriptor used to access GPIO character device. */
static int Main_GPIO_Chip_File_Descriptor = -1;
/** The button GPIO file descriptor. */
static int Main_Button_GPIO_File_Descriptor = -1;
/** The led GPIO file descriptor. */
static int Main_Led_GPIO_File_Descriptor = -1;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Automatically and gracefully release SDL resources. */
static void MainExit(void)
{
	close(Main_Led_GPIO_File_Descriptor);
	close(Main_Button_GPIO_File_Descriptor);
	close(Main_GPIO_Chip_File_Descriptor);
	TTF_Quit();
	SDL_Quit();
}

/** Retrieve a file descriptor on the GPIO to read state.
 * @return -1 if an error occurred,
 * @return 0 on success.
 */
static int MainOpenGPIO(void)
{
	struct gpiohandle_request GPIO_Request;
	
	// Get access to GPIO controller
	Main_GPIO_Chip_File_Descriptor = open("/dev/gpiochip0", O_RDONLY);
	if (Main_GPIO_Chip_File_Descriptor == -1)
	{
		printf("Error : failed to open GPIO chip device (%s).\n", strerror(errno));
		return -1;
	}
	
	// Get a file descriptor on the button GPIO configured in input mode
	memset(&GPIO_Request, 0, sizeof(GPIO_Request));
	GPIO_Request.lineoffsets[0] = 22; // GPIO 22
	GPIO_Request.flags = GPIOHANDLE_REQUEST_INPUT;
	GPIO_Request.lines = 1;
	if (ioctl(Main_GPIO_Chip_File_Descriptor, GPIO_GET_LINEHANDLE_IOCTL, &GPIO_Request) < 0)
	{
		printf("Error : could not retrieve button GPIO handle (%s).\n", strerror(errno));
		return -1;
	}
	Main_Button_GPIO_File_Descriptor = GPIO_Request.fd;
	
	// Get a file descriptor on the led GPIO configured in output mode
	memset(&GPIO_Request, 0, sizeof(GPIO_Request));
	GPIO_Request.lineoffsets[0] = 27; // GPIO 27
	GPIO_Request.flags = GPIOHANDLE_REQUEST_OUTPUT;
	GPIO_Request.lines = 1;
	if (ioctl(Main_GPIO_Chip_File_Descriptor, GPIO_GET_LINEHANDLE_IOCTL, &GPIO_Request) < 0)
	{
		printf("Error : could not retrieve led GPIO handle (%s).\n", strerror(errno));
		return -1;
	}
	Main_Led_GPIO_File_Descriptor = GPIO_Request.fd;
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(void)
{
	SDL_Surface *Pointer_Surface, *Pointer_Temporary_Surface, *Pointer_Picture_Surface;
	SDL_Renderer *Pointer_Renderer;
	TTF_Font *Pointer_Font;
	SDL_Color Color_Black = { 0, 0, 0, 255 };
	struct tm *Pointer_Date_And_Time;
	time_t Current_Time;
	char String[64];
	SDL_Rect Destination_Rectangle;
	struct gpiohandle_data GPIO_Data;
	
	// Get access to button GPIO
	if (MainOpenGPIO() != 0) return EXIT_FAILURE;
	
	// Configure printer for operation
	system("lpoptions -d DYMO_LabelWriter_450 -o Resolution=300x600dpi -o DymoPrintQuality=Graphics -o DymoPrintDensity=Normal -o PageSize=w162h90");
	
	// Create a software renderer (to avoid creating a window on a X11-less system)
	// Create the surface the label will be rendered to
	Pointer_Surface = SDL_CreateRGBSurfaceWithFormat(0, MAIN_OUTPUT_IMAGE_WIDTH, 640, 32, SDL_PIXELFORMAT_ARGB8888);
	if (Pointer_Surface == NULL)
	{
		printf("Error : failed to create surface (%s).\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	// Create the renderer
	Pointer_Renderer = SDL_CreateSoftwareRenderer(Pointer_Surface);
	if (Pointer_Renderer == NULL)
	{
		printf("Error : failed to create renderer (%s).\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	
	// Cache the picture
	Pointer_Picture_Surface = SDL_LoadBMP("Picture.bmp");
	if (Pointer_Picture_Surface == NULL)
	{
		printf("Error : failed to load picture bitmap image (%s).\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	
	// Initialize font renderer
	if (TTF_Init() != 0)
	{
		printf("Error : failed to initialize SDL TTF (%s).\n", TTF_GetError());
		return EXIT_FAILURE;
	}
	
	// Try to load font
	Pointer_Font = TTF_OpenFont("Sansation_Regular.ttf", 140);
	if (Pointer_Font == NULL)
	{
		printf("Error : failed to load font (%s).\n", TTF_GetError());
		return EXIT_FAILURE;
	}

	// Automatically free SDL resources on application exit
	atexit(MainExit);
	
	while (1)
	{
		// Read button state
		if (ioctl(Main_Button_GPIO_File_Descriptor, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &GPIO_Data) < 0)
		{
			printf("Error : could not read GPIO state (%s).\n", strerror(errno));
			return -1;
		}
		
		// Button is active low
		if (GPIO_Data.values[0] == 0)
		{
			// Clear rendering area with a white background
			SDL_SetRenderDrawColor(Pointer_Renderer, 255, 255, 255, 255);
			SDL_RenderClear(Pointer_Renderer);
			
			// Render the picture
			Destination_Rectangle.x = 40;
			Destination_Rectangle.y = 10;
			SDL_BlitSurface(Pointer_Picture_Surface, NULL, Pointer_Surface, &Destination_Rectangle);
			
			// Retrieve current date and time
			Current_Time = time(NULL);
			Pointer_Date_And_Time = localtime(&Current_Time);
			
			// Render the date at the specified location
			sprintf(String, "%02d/%02d/%d", Pointer_Date_And_Time->tm_mday, Pointer_Date_And_Time->tm_mon + 1, Pointer_Date_And_Time->tm_year + 1900);
			Pointer_Temporary_Surface = TTF_RenderText_Blended(Pointer_Font, String, Color_Black);
			if (Pointer_Temporary_Surface == NULL)
			{
				printf("Error : failed to render date string surface (%s).\n", TTF_GetError());
				return EXIT_FAILURE;
			}
			Destination_Rectangle.x = ((MAIN_OUTPUT_IMAGE_WIDTH - Pointer_Temporary_Surface->w) / 2) + MAIN_IMAGE_TEXT_LEFT_OFFSET;
			Destination_Rectangle.y = 150;
			SDL_BlitSurface(Pointer_Temporary_Surface, NULL, Pointer_Surface, &Destination_Rectangle);
			SDL_FreeSurface(Pointer_Temporary_Surface);
			
			// Render the time at the specified location
			sprintf(String, "%02d:%02d:%02d", Pointer_Date_And_Time->tm_hour, Pointer_Date_And_Time->tm_min, Pointer_Date_And_Time->tm_sec);
			Pointer_Temporary_Surface = TTF_RenderText_Blended(Pointer_Font, String, Color_Black);
			if (Pointer_Temporary_Surface == NULL)
			{
				printf("Error : failed to render time string surface (%s).\n", TTF_GetError());
				return EXIT_FAILURE;
			}
			Destination_Rectangle.x = ((MAIN_OUTPUT_IMAGE_WIDTH - Pointer_Temporary_Surface->w) / 2) + MAIN_IMAGE_TEXT_LEFT_OFFSET;
			Destination_Rectangle.y = 350;
			SDL_BlitSurface(Pointer_Temporary_Surface, NULL, Pointer_Surface, &Destination_Rectangle);
			SDL_FreeSurface(Pointer_Temporary_Surface);
			
			// Convert the surface to 24-bit RGB to generate a bitmap understandable by "lp" command
			Pointer_Temporary_Surface = SDL_ConvertSurfaceFormat(Pointer_Surface, SDL_PIXELFORMAT_RGB24, 0);
			if (Pointer_Temporary_Surface == NULL)
			{
				printf("Error : failed to convert image surface to 24-bit RGB (%s).\n", SDL_GetError());
				return EXIT_FAILURE;
			}
			
			// Store the generated bitmap to a temporary file
			if (SDL_SaveBMP(Pointer_Temporary_Surface, MAIN_OUTPUT_IMAGE_FILE_NAME) != 0)
			{
				printf("Error : failed to save image to a bitmap file (%s).\n", SDL_GetError());
				return EXIT_FAILURE;
			}
			SDL_FreeSurface(Pointer_Temporary_Surface);
			
			// Print the label
			system("lp " MAIN_OUTPUT_IMAGE_FILE_NAME);
			
			// Wait for the button to be released
			do
			{
				usleep(50000);
				if (ioctl(Main_Button_GPIO_File_Descriptor, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &GPIO_Data) < 0)
				{
					printf("Error : could not read button release state (%s).\n", strerror(errno));
					return EXIT_FAILURE;
				}
			} while (GPIO_Data.values[0] == 0);
			
			usleep(200000);
		}
		
		// Wait some time to avoid consuming 100% CPU time
		usleep(50000);
	}
}
