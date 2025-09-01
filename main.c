/* Baseer 1.0.0a */
#include "baseer.h"

baseer_target_t *baseer_open(char *file_path)
{
    baseer_target_t *target = NULL;

    struct stat info;
    FILE *handler = NULL;

    RETURN_NULL_IF(file_path == NULL)
    RETURN_NULL_IF(stat(file_path, &info) != 0)
    RETURN_NULL_IF((info.st_size == 0) || (info.st_size > BASEER_MAX_FILE_SIZE))

    handler = fopen(file_path, "rb");
    RETURN_NULL_IF(handler == NULL)

    target = (baseer_target_t *)malloc(sizeof(baseer_target_t));
    if (target == NULL)
    {
        fclose(handler);
        return NULL;
    }

    target->size = info.st_size;
    target->block = malloc(target->size);
    if (target->block == NULL)
    {
        free(target);
        fclose(handler);
        return NULL;
    }

    fread(target->block,
                 target->size, 1, handler);
    fclose(handler);

    // printf("\033[5;31mBlinking Red Text\033[0m\n");
       // for (int i = 0; i < 10; i++) {
       //  if (i % 2 == 0)
       //      printf("\033[31mRed!\033[0m\r"); // red
       //  else
       //      printf("\033[41mRed!\033[0m\r"); // green
       //  fflush(stdout);
       //  usleep(300000); // 0.3 sec
    // }
    
    target->version = BASEER_VERSION;
    target->loaded_extensions = 0;

    target->extensions = malloc(sizeof(baseer_extension_t*) * BASEER_MAX_EXTENSIONS);
    if (target->extensions == NULL) {
        free(target->block);
        free(target);
        return NULL;
    }
    return target;
}

void baseer_close(baseer_target_t *target)
{
    if (target)
    {
        target->size = 0;
        target->version = 0;

        if (target->block)
            free(target->block);

        if (target->extensions) 
            free(target->extensions);

        free(target);
    }
}

bool baseer_load_extension(baseer_target_t *target, baseer_extension_t *extension)
{
    if (extension == NULL || target == NULL)
        return false; // No input

    if (target->loaded_extensions >= BASEER_MAX_EXTENSIONS)
        return false; // No space left
    target->extensions[target->loaded_extensions++] = extension;
    return true;
}

bool baseer_unload_extension(baseer_target_t *target, char *extension_name)
{

    if (extension_name == NULL || target == NULL)
        return false;

    if (target->loaded_extensions == 0)
        return false; // No extensions exists

    for (int i=0; i<target -> loaded_extensions; i++) {
        // if(target -> extensions[i]->info.name == extension_name) {
        if(strcmp(target -> extensions[i]->info.name, extension_name) == 0) {
            baseer_extension_t *tmp = target->extensions[i];
            target->extensions[i] = target->extensions[target->loaded_extensions];
            target->extensions[target->loaded_extensions] = tmp;
            target->loaded_extensions--;
            break;
        }
    }
    return true;
}

// Execute all loaded extensions
void baseer_execute_extensions(baseer_target_t *target) {
    printf("Loaded extensions: %d\n", target->loaded_extensions);
    for (unsigned int i = 0; i < target->loaded_extensions; i++) {
        printf("\033[31mExtension name:\033[0m %s\n",target->extensions[i]->info.name);
        printf("\033[32mAuthor name:\033[0m \033[34m%s\033[0m\n", target->extensions[i]->info.author);
        printf("\033[33mDescription:\033[0m %s\n", target->extensions[i]->info.description);
        target->extensions[i]->execute(target);
        printf("\n");
    }
}

bool print_loaded_ext(baseer_target_t* target) {
    if (target == NULL)
        return false;
    printf("extensions: %d\n", target ->loaded_extensions);
    return true;
}

// int piplines(baseer_pipline_t *piplines) {
    // return NULL;
// }

bool baseer_load_extension_from_so(baseer_target_t *target, const char *so_path) {
    void *handle = dlopen(so_path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return false;
    }

    baseer_extension_t *ext = (baseer_extension_t *)dlsym(handle, "baseer_extension");
    if (!ext) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return false;
    }

    if (target->extensions == NULL) {
        target->extensions = malloc(sizeof(baseer_extension_t*) * BASEER_MAX_EXTENSIONS);
        if (!target->extensions) { dlclose(handle); return false; }
        target->loaded_extensions = 0;
    }

    target->extensions[target->loaded_extensions++] = ext;

    printf("Loaded extension: %s\n", ext->info.name);

    return true;  // Keep handle open if you plan to unload later
}

int main(int argc, char**args)
{
    if(argc != 2) return 1;
    baseer_target_t *target = baseer_open(args[1]);
    if (target == NULL)
        return 1;

    // baseer_extension_info_t ext_infos[] = { 
    //     {"Print loaded extensions", "Good Guy", "print loaded extensions"},
    // };
    // baseer_extension_t extens[] = {
    //     {ext_infos[0], print_loaded_ext},
    // };

    // size_t ext_count = sizeof(ext_infos) / sizeof(ext_infos[0]);

    // // baseer_print(target);
    
    // for(int i=0; i< ext_count; i++) {
    //     baseer_load_extension(target, &extens[i]);
    // }
    baseer_load_extension_from_so(target, "extensions/print_version.so");
    baseer_load_extension_from_so(target, "extensions/print_size.so");
    // baseer_load_extension_from_so(target, "extensions/print_hex.so");
    // baseer_load_extension_from_so(target, "extensions/print_string.so");
    
    baseer_execute_extensions(target);

    printf("\n\n\n\n\n");
    // baseer_unload_extension(target, "Print app version");
    baseer_unload_extension(target, "PrintSize");
    baseer_execute_extensions(target);

    baseer_close(target);

    return 0;
}
