//#include "resource.h"
//#include "hash.h"
//
//#include <stdlib.h>
//#include <string.h>
//#include <assert.h>
//
//enum
//{
//	BUFFER_SIZE = 4096,
//	BUFFER_COUNT = 2
//};
//
//struct LoadRequest
//{
//	int wait_index;
//
//	union
//	{
//		const char* texture_name;
//	};
//};
//typedef struct LoadRequest LoadRequest;
//
//struct ResourceLoader
//{
//	volatile bool run;
//	char* base_path;
//	char* user_path;
//
//	SDL_Thread* thread;
//	SDL_mutex* queue_mutex;
//	SDL_mutex* token_mutex;
//	SDL_cond* queue_cond;
//	SDL_cond* token_cond;
//	SDL_atomic_t token_completed;
//	SDL_atomic_t token_counter;
//
//	int next_set;
//	int current_tokens[3];
//
//	LoadRequest requests[BUFFER_SIZE];
//	uint32_t request_count;
//};
//typedef struct ResourceLoader ResourceLoader;
//
//static ResourceLoader* resource_loader;
//
//static int loader_function(void* data)
//{
//	TracyCSetThreadName("Resource Loader");
//
//	int max_token = 0;
//
//	while (resource_loader->run)
//	{
//		TracyCZoneN(ctx, "Resource Loader Update", true);
//
//		SDL_LockMutex(resource_loader->queue_mutex);
//
//		// Check for pending tokens
//		bool all_tokens_sigaled = (resource_loader->token_completed.value == SDL_AtomicGet(&resource_loader->token_counter));
//
//		while ((resource_loader->request_count == 0) && all_tokens_sigaled && resource_loader->run)
//		{
//			// Wait until there's a request
//			SDL_CondWait(resource_loader->queue_cond, resource_loader->queue_mutex);
//		}
//
//		SDL_UnlockMutex(resource_loader->queue_mutex);
//
//		resource_loader->next_set = (resource_loader->next_set + 1) % BUFFER_COUNT;
//
//		// Signal pending tokens from previous frames
//		SDL_LockMutex(resource_loader->token_mutex);
//		SDL_AtomicSet(&resource_loader->token_completed, resource_loader->current_tokens[resource_loader->next_set]);
//		SDL_UnlockMutex(resource_loader->token_mutex);
//		SDL_CondBroadcast(resource_loader->token_cond);
//
//		uint64_t completion_mask = 0;
//
//		SDL_LockMutex(resource_loader->queue_mutex);
//
//		uint32_t request_count = resource_loader->request_count;
//		resource_loader->request_count = 0;
//
//		// Use a copy of requests for thread safety
//		size_t size = sizeof(LoadRequest) * request_count;
//		LoadRequest* active_requests = malloc(size);
//		assert(active_requests);
//		memcpy(active_requests, resource_loader->requests, size);
//
//		SDL_UnlockMutex(resource_loader->queue_mutex);
//
//		for (uint32_t i = 0; i < request_count; ++i)
//		{
//			LoadRequest* request = &active_requests[i];
//
//			// Actually load texture
//			SDL_RWops* rwops = SDL_RWFromFile(request->texture_name, "rb");
//			if (!rwops)
//			{
//				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
//				return UINT16_MAX;
//			}
//
//			int64_t size = SDL_RWsize(rwops);
//			uint8_t* data = malloc(size);
//			SDL_RWread(rwops, data, 1, size);
//
//			// Create texture
//			SDL_Log("Loaded texture of size %I64d", size);
//
//			SDL_RWclose(rwops);
//			free(data);
//
//			bool completed = true;
//			if (request->wait_index && completed)
//			{
//				assert(max_token < request->wait_index);
//				max_token = request->wait_index;
//			}
//		}
//
//		int last_token = SDL_AtomicGet(&resource_loader->token_completed);
//		int next_token = max(max_token, last_token);
//		resource_loader->current_tokens[resource_loader->next_set] = next_token;
//
//		TracyCZoneEnd(ctx);
//	}
//
//	return 0;
//}
//
//void resource_init(void)
//{
//	resource_loader = calloc(1, sizeof(*resource_loader));
//	assert(resource_loader);
//
//	resource_loader->run = true;
//
//	resource_loader->base_path = SDL_GetBasePath();
//	resource_loader->user_path = SDL_GetPrefPath("Jeremie St-Amand", "Battlecry");
//
//	resource_loader->queue_mutex = SDL_CreateMutex();
//	resource_loader->token_mutex = SDL_CreateMutex();
//	resource_loader->queue_cond = SDL_CreateCond();
//	resource_loader->token_cond = SDL_CreateCond();
//
//	resource_loader->thread = SDL_CreateThread(loader_function, "Loader", NULL);
//}
//
//void resource_quit(void)
//{
//	assert(resource_loader);
//
//	resource_loader->run = false;
//
//	SDL_CondSignal(resource_loader->queue_cond);
//	int result;
//	SDL_WaitThread(resource_loader->thread, &result);
//	assert(result == 0);
//
//	SDL_DestroyCond(resource_loader->queue_cond);
//	SDL_DestroyCond(resource_loader->token_cond);
//	SDL_DestroyMutex(resource_loader->queue_mutex);
//	SDL_DestroyMutex(resource_loader->token_mutex);
//
//	SDL_free(resource_loader->base_path);
//	SDL_free(resource_loader->user_path);
//
//	free(resource_loader);
//}
//
//void resource_load_texture(const char* name, Texture* texture)
//{
//	SDL_LockMutex(resource_loader->queue_mutex);
//
//	int token = SDL_AtomicAdd(&resource_loader->token_counter, 1) + 1;
//
//	resource_loader->requests[resource_loader->request_count++] = (LoadRequest){
//		.wait_index = token,
//		.texture_name = name
//	};
//
//	SDL_Log("Added texture %s with token %d", name, token);
//
//	SDL_UnlockMutex(resource_loader->queue_mutex);
//	SDL_CondSignal(resource_loader->queue_cond);
//}
//
//void resource_unload_texture(uint16_t id)
//{
//
//}
