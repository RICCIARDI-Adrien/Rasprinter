/** @file Main.c
 * Wait for a button press, render a label picture and print it.
 * @author Adrien RICCIARDI
 */
#include <errno.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Rendered image width in pixels. */
#define MAIN_OUTPUT_IMAGE_WIDTH 1140

/** The output file to generate. */
#define MAIN_OUTPUT_IMAGE_FILE_NAME "/tmp/Label.bmp"

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Automatically and gracefully release SDL resources. */
static void Exit(void)
{
	TTF_Quit();
	SDL_Quit();
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(void)
{
	SDL_Surface *Pointer_Surface, *Pointer_Temporary_Surface;
	SDL_Renderer *Pointer_Renderer;
	TTF_Font *Pointer_Font;
	SDL_Color Color_Black = { 0, 0, 0, 255 };
	struct tm *Pointer_Date_And_Time;
	time_t Current_Time;
	char String[64];
	SDL_Rect Destination_Rectangle;
	
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
	atexit(Exit);
	
	/*while (1)
	{
	}*/
	
	// Clear rendering area with a white background
	SDL_SetRenderDrawColor(Pointer_Renderer, 255, 255, 255, 255);
	SDL_RenderClear(Pointer_Renderer);
	
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
	Destination_Rectangle.x = ((MAIN_OUTPUT_IMAGE_WIDTH - Pointer_Temporary_Surface->w) / 2);
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
	Destination_Rectangle.x = ((MAIN_OUTPUT_IMAGE_WIDTH - Pointer_Temporary_Surface->w) / 2);
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
	
	return 0;
}
