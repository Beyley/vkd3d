/*
 * Copyright 2017 Józef Kucia for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "vkd3d_common.h"
#include "vkd3d_shader.h"

#define MAX_COMPILE_OPTIONS 1

static bool read_shader(struct vkd3d_shader_code *shader, const char *filename)
{
    struct stat st;
    void *code;
    FILE *fd;

    memset(shader, 0, sizeof(*shader));

    if (stat(filename, &st) == -1)
    {
        fprintf(stderr, "Could not stat file: '%s'.\n", filename);
        return false;
    }
    shader->size = st.st_size;

    if (!(fd = fopen(filename, "rb")))
    {
        fprintf(stderr, "Cannot open file for reading: '%s'.\n", filename);
        return false;
    }

    if (!(code = malloc(shader->size)))
    {
        fprintf(stderr, "Out of memory.\n");
        fclose(fd);
        return false;
    }
    shader->code = code;

    if (fread(code, 1, shader->size, fd) != shader->size)
    {
        fprintf(stderr, "Could not read shader bytecode from file: '%s'.\n", filename);
        free(code);
        fclose(fd);
        return false;
    }

    fclose(fd);
    return true;
}

static bool write_shader(const struct vkd3d_shader_code *shader, const char *filename)
{
    FILE *fd;

    if (!(fd = fopen(filename, "wb")))
    {
        fprintf(stderr, "Cannot open file for writing: '%s'.\n", filename);
        return false;
    }

    if (fwrite(shader->code, 1, shader->size, fd) != shader->size)
        fprintf(stderr, "Could not write shader bytecode to file: '%s'.\n", filename);

    fclose(fd);
    return true;
}

static const struct
{
    const char *name;
    enum vkd3d_shader_compile_option_name compile_option;
}
compiler_options[] =
{
    {"--strip-debug", VKD3D_SHADER_COMPILE_OPTION_STRIP_DEBUG},
};

static void print_usage(const char *program_name)
{
    unsigned int i;

    fprintf(stderr, "usage: %s", program_name);
    for (i = 0; i < ARRAY_SIZE(compiler_options); ++i)
        fprintf(stderr, " [%s]", compiler_options[i].name);
    fprintf(stderr, " [-o <out_spirv_filename>] <dxbc_filename>\n");
}

struct options
{
    const char *filename;
    const char *output_filename;
    struct vkd3d_shader_compile_option compile_options[MAX_COMPILE_OPTIONS];
    unsigned int compile_option_count;
};

static void add_compile_option(struct options *options,
        enum vkd3d_shader_compile_option_name name, unsigned int value)
{
    struct vkd3d_shader_compile_option *o;
    unsigned int i;

    for (i = 0; i < options->compile_option_count; ++i)
    {
        o = &options->compile_options[i];

        if (o->name == name)
        {
            o->value = value;
            return;
        }
    }

    if (options->compile_option_count >= ARRAY_SIZE(options->compile_options))
    {
        fprintf(stderr, "Ignoring option.\n");
        return;
    }

    o = &options->compile_options[options->compile_option_count++];
    o->name = name;
    o->value = value;
}

static bool parse_command_line(int argc, char **argv, struct options *options)
{
    unsigned int i, j;

    if (argc < 2)
        return false;

    memset(options, 0, sizeof(*options));

    for (i = 1; i < argc - 1; ++i)
    {
        if (!strcmp(argv[i], "-o"))
        {
            if (i + 1 >= argc - 1)
                return false;
            options->output_filename = argv[++i];
            continue;
        }

        for (j = 0; j < ARRAY_SIZE(compiler_options); ++j)
        {
            if (!strcmp(argv[i], compiler_options[j].name))
            {
                add_compile_option(options, compiler_options[j].compile_option, 1);
                break;
            }
        }
        if (j == ARRAY_SIZE(compiler_options))
            return false;
    }

    options->filename = argv[argc - 1];
    return true;
}

int main(int argc, char **argv)
{
    struct vkd3d_shader_compile_info info;
    struct vkd3d_shader_code spirv;
    struct options options;
    int ret;

    if (!parse_command_line(argc, argv, &options))
    {
        print_usage(argv[0]);
        return 1;
    }

    info.type = VKD3D_SHADER_STRUCTURE_TYPE_COMPILE_INFO;
    info.next = NULL;
    info.source_type = VKD3D_SHADER_SOURCE_DXBC_TPF;
    info.target_type = VKD3D_SHADER_TARGET_SPIRV_BINARY;
    info.options = options.compile_options;
    info.option_count = options.compile_option_count;

    if (!read_shader(&info.source, options.filename))
    {
        fprintf(stderr, "Failed to read DXBC shader.\n");
        return 1;
    }

    ret = vkd3d_shader_compile(&info, &spirv);
    vkd3d_shader_free_shader_code(&info.source);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to compile DXBC shader, ret %d.\n", ret);
        return 1;
    }

    if (options.output_filename)
        write_shader(&spirv, options.output_filename);

    vkd3d_shader_free_shader_code(&spirv);
    return 0;
}
