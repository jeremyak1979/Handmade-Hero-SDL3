#include <stdio.h>
#include <sys/mman.h>
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif


#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


struct offscreen_buffer {
	SDL_Texture *texture;
	void *Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct window_dimension {
	int Width;
	int Height;
};

global_variable struct offscreen_buffer GlobalBackBuffer;
global_variable bool running;

internal struct window_dimension
GetWindowDimension(SDL_Window *); 

internal bool 
HandleEvent(struct offscreen_buffer, SDL_Event *);

internal void 
SDLResizeTexture(struct offscreen_buffer *, SDL_Renderer *,int,int);

internal void 
SDLUpdateWindow(struct offscreen_buffer, SDL_Renderer *);

internal void 
RenderWeirdGradient(struct offscreen_buffer, int, int);


int main(int argc, char *argv[]){

	if(SDL_Init(SDL_INIT_VIDEO) == 0){
		fprintf(stderr, "Init failed: %s\n", SDL_GetError());
	}

	SDL_Window *Window = SDL_CreateWindow("Handmade Hero", 
										  640, 
										  480, 
										  SDL_WINDOW_RESIZABLE);
	

	if(Window) {
		SDL_Renderer *rend = SDL_CreateRenderer(Window, NULL);
		if(rend){
			running = true;
			int Blue = 0;
			int Green = 175;

			struct window_dimension Dimension;
			Dimension = GetWindowDimension(Window);
			
			SDLResizeTexture(&GlobalBackBuffer, rend, 
							 Dimension.Width, Dimension.Height);

			while(running) {
				SDL_Event event;
				while(SDL_PollEvent(&event)){
					if(HandleEvent(GlobalBackBuffer, &event))
						running = false;
				}

				RenderWeirdGradient(GlobalBackBuffer, Blue, Green);
				SDLUpdateWindow(GlobalBackBuffer, rend);

				Blue++;
				Green++;
			}

		} else {
			// TODO(Jeremy): Logging
		}

	} else {
		// TODO(Jeremy): Logging
	}

	SDL_Quit();

	return 0;
}
	
internal bool 
HandleEvent(struct offscreen_buffer Buffer, SDL_Event *event)
{
	bool ShouldQuit = false;
	switch(event->type){

		case SDL_EVENT_QUIT: {
			printf("SDL_QUIT\n");
			ShouldQuit = true;
		} break; 

		case SDL_EVENT_WINDOW_FOCUS_GAINED: {
			printf("Gained focus\n");
		} break;

		case SDL_EVENT_WINDOW_EXPOSED: {
			SDL_Window *win = SDL_GetWindowFromID(event->window.windowID);
			SDL_Renderer *rend = SDL_GetRenderer(win);
			SDLUpdateWindow(GlobalBackBuffer, rend);
		} break;
	}

	return(ShouldQuit);
}

internal void 
SDLResizeTexture(struct offscreen_buffer *Buffer, SDL_Renderer *rend, 
				int Width, int Height)
{
	if(Buffer->Memory) {
		munmap(Buffer->Memory, 
			   Width * Height * Buffer->BytesPerPixel);
	}

	Buffer->BytesPerPixel = 4;
	Buffer->Width = Width;
	Buffer->Height = Height;

	if(Buffer->texture) {
		SDL_DestroyTexture(Buffer->texture);
	}

	Buffer->texture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_XRGB8888, 
										SDL_TEXTUREACCESS_STREAMING, 
										Buffer->Width, Buffer->Height);

	int MemSize = Width * Height * Buffer->BytesPerPixel;

	Buffer->Memory = mmap(0, MemSize, PROT_READ | PROT_WRITE, 
						MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;

}

internal void 
SDLUpdateWindow(struct offscreen_buffer Buffer, SDL_Renderer *rend)
{
	SDL_UpdateTexture(Buffer.texture, 0, Buffer.Memory, 
					  Buffer.Width * Buffer.BytesPerPixel);

	// TODO(Jeremy): Aspect Ratio Correction
	SDL_RenderClear(rend);
	SDL_RenderTexture(rend, Buffer.texture, 0, 0);

	//SDL_SetRenderTarget(rend, NULL);
	SDL_RenderPresent(rend);
}

internal void 
RenderWeirdGradient(struct offscreen_buffer Buffer,int Blue,int Green){

	uint8 *Row = (uint8 *)Buffer.Memory;
	for(int Y = 0; Y < Buffer.Height; ++Y){
		uint32 *Pixel = (uint32 *)Row;
		for(int X = 0; X < Buffer.Width; ++X){
			uint8 xBlue = (X + Blue);
			uint8 xGreen = (Y + Green);
			*Pixel++ = ((255 << 24)|(5 << 16)|(xGreen << 8)|xBlue);
		}
		Row = (uint8 *)Pixel;
	}
}

internal struct window_dimension
GetWindowDimension(SDL_Window *Window){
	struct window_dimension temp;
	SDL_GetWindowSize(Window, &temp.Width, &temp.Height);
	return temp;
}	
