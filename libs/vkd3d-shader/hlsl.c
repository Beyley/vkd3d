/*
 * HLSL utility functions
 *
 * Copyright 2012 Matteo Bruni for CodeWeavers
 * Copyright 2019-2020 Zebediah Figura for CodeWeavers
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

#include "hlsl.h"
#include <stdio.h>

void hlsl_note(struct hlsl_ctx *ctx, const struct vkd3d_shader_location *loc,
        enum vkd3d_shader_log_level level, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vkd3d_shader_vnote(ctx->message_context, loc, level, fmt, args);
    va_end(args);
}

void hlsl_error(struct hlsl_ctx *ctx, const struct vkd3d_shader_location *loc,
        enum vkd3d_shader_error error, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vkd3d_shader_verror(ctx->message_context, loc, error, fmt, args);
    va_end(args);

    if (!ctx->result)
        ctx->result = VKD3D_ERROR_INVALID_SHADER;
}

void hlsl_warning(struct hlsl_ctx *ctx, const struct vkd3d_shader_location *loc,
        enum vkd3d_shader_error error, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vkd3d_shader_vwarning(ctx->message_context, loc, error, fmt, args);
    va_end(args);
}

void hlsl_fixme(struct hlsl_ctx *ctx, const struct vkd3d_shader_location *loc, const char *fmt, ...)
{
    struct vkd3d_string_buffer *string;
    va_list args;

    va_start(args, fmt);
    string = hlsl_get_string_buffer(ctx);
    vkd3d_string_buffer_printf(string, "Aborting due to not yet implemented feature: ");
    vkd3d_string_buffer_vprintf(string, fmt, args);
    vkd3d_shader_error(ctx->message_context, loc, VKD3D_SHADER_ERROR_HLSL_NOT_IMPLEMENTED, "%s", string->buffer);
    hlsl_release_string_buffer(ctx, string);
    va_end(args);

    if (!ctx->result)
        ctx->result = VKD3D_ERROR_NOT_IMPLEMENTED;
}

bool hlsl_add_var(struct hlsl_ctx *ctx, struct hlsl_ir_var *decl, bool local_var)
{
    struct hlsl_scope *scope = ctx->cur_scope;
    struct hlsl_ir_var *var;

    LIST_FOR_EACH_ENTRY(var, &scope->vars, struct hlsl_ir_var, scope_entry)
    {
        if (!strcmp(decl->name, var->name))
            return false;
    }
    if (local_var && scope->upper->upper == ctx->globals)
    {
        /* Check whether the variable redefines a function parameter. */
        LIST_FOR_EACH_ENTRY(var, &scope->upper->vars, struct hlsl_ir_var, scope_entry)
        {
            if (!strcmp(decl->name, var->name))
                return false;
        }
    }

    list_add_tail(&scope->vars, &decl->scope_entry);
    return true;
}

struct hlsl_ir_var *hlsl_get_var(struct hlsl_scope *scope, const char *name)
{
    struct hlsl_ir_var *var;

    LIST_FOR_EACH_ENTRY(var, &scope->vars, struct hlsl_ir_var, scope_entry)
    {
        if (!strcmp(name, var->name))
            return var;
    }
    if (!scope->upper)
        return NULL;
    return hlsl_get_var(scope->upper, name);
}

void hlsl_free_var(struct hlsl_ir_var *decl)
{
    vkd3d_free((void *)decl->name);
    hlsl_cleanup_semantic(&decl->semantic);
    vkd3d_free(decl);
}

bool hlsl_type_is_row_major(const struct hlsl_type *type)
{
    /* Default to column-major if the majority isn't explicitly set, which can
     * happen for anonymous nodes. */
    return !!(type->modifiers & HLSL_MODIFIER_ROW_MAJOR);
}

unsigned int hlsl_type_minor_size(const struct hlsl_type *type)
{
    if (type->type != HLSL_CLASS_MATRIX || hlsl_type_is_row_major(type))
        return type->dimx;
    else
        return type->dimy;
}

unsigned int hlsl_type_major_size(const struct hlsl_type *type)
{
    if (type->type != HLSL_CLASS_MATRIX || hlsl_type_is_row_major(type))
        return type->dimy;
    else
        return type->dimx;
}

unsigned int hlsl_type_element_count(const struct hlsl_type *type)
{
    switch (type->type)
    {
        case HLSL_CLASS_VECTOR:
            return type->dimx;
        case HLSL_CLASS_MATRIX:
            return hlsl_type_major_size(type);
        case HLSL_CLASS_ARRAY:
            return type->e.array.elements_count;
        case HLSL_CLASS_STRUCT:
            return type->e.record.field_count;
        default:
            return 0;
    }
}

static unsigned int get_array_size(const struct hlsl_type *type)
{
    if (type->type == HLSL_CLASS_ARRAY)
        return get_array_size(type->e.array.type) * type->e.array.elements_count;
    return 1;
}

bool hlsl_type_is_resource(const struct hlsl_type *type)
{
    if (type->type == HLSL_CLASS_OBJECT)
    {
        switch (type->base_type)
        {
            case HLSL_TYPE_TEXTURE:
            case HLSL_TYPE_SAMPLER:
            case HLSL_TYPE_UAV:
                return true;
            default:
                return false;
        }
    }
    return false;
}

enum hlsl_regset hlsl_type_get_regset(const struct hlsl_type *type)
{
    if (type->type <= HLSL_CLASS_LAST_NUMERIC)
        return HLSL_REGSET_NUMERIC;

    if (type->type == HLSL_CLASS_OBJECT)
    {
        switch (type->base_type)
        {
            case HLSL_TYPE_TEXTURE:
                return HLSL_REGSET_TEXTURES;

            case HLSL_TYPE_SAMPLER:
                return HLSL_REGSET_SAMPLERS;

            case HLSL_TYPE_UAV:
                return HLSL_REGSET_UAVS;

            default:
                vkd3d_unreachable();
        }
    }

    vkd3d_unreachable();
}

unsigned int hlsl_type_get_sm4_offset(const struct hlsl_type *type, unsigned int offset)
{
    /* Align to the next vec4 boundary if:
     *  (a) the type is a struct or array type, or
     *  (b) the type would cross a vec4 boundary; i.e. a vec3 and a
     *      vec1 can be packed together, but not a vec3 and a vec2.
     */
    if (type->type > HLSL_CLASS_LAST_NUMERIC || (offset & 3) + type->reg_size[HLSL_REGSET_NUMERIC] > 4)
        return align(offset, 4);
    return offset;
}

static void hlsl_type_calculate_reg_size(struct hlsl_ctx *ctx, struct hlsl_type *type)
{
    bool is_sm4 = (ctx->profile->major_version >= 4);
    unsigned int k;

    for (k = 0; k <= HLSL_REGSET_LAST; ++k)
        type->reg_size[k] = 0;

    switch (type->type)
    {
        case HLSL_CLASS_SCALAR:
        case HLSL_CLASS_VECTOR:
            type->reg_size[HLSL_REGSET_NUMERIC] = is_sm4 ? type->dimx : 4;
            break;

        case HLSL_CLASS_MATRIX:
            if (hlsl_type_is_row_major(type))
                type->reg_size[HLSL_REGSET_NUMERIC] = is_sm4 ? (4 * (type->dimy - 1) + type->dimx) : (4 * type->dimy);
            else
                type->reg_size[HLSL_REGSET_NUMERIC] = is_sm4 ? (4 * (type->dimx - 1) + type->dimy) : (4 * type->dimx);
            break;

        case HLSL_CLASS_ARRAY:
        {
            if (type->e.array.elements_count == HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT)
                break;

            for (k = 0; k <= HLSL_REGSET_LAST; ++k)
            {
                unsigned int element_size = type->e.array.type->reg_size[k];

                if (is_sm4 && k == HLSL_REGSET_NUMERIC)
                    type->reg_size[k] = (type->e.array.elements_count - 1) * align(element_size, 4) + element_size;
                else
                    type->reg_size[k] = type->e.array.elements_count * element_size;
            }

            break;
        }

        case HLSL_CLASS_STRUCT:
        {
            unsigned int i;

            type->dimx = 0;
            for (i = 0; i < type->e.record.field_count; ++i)
            {
                struct hlsl_struct_field *field = &type->e.record.fields[i];

                for (k = 0; k <= HLSL_REGSET_LAST; ++k)
                {
                    if (k == HLSL_REGSET_NUMERIC)
                        type->reg_size[k] = hlsl_type_get_sm4_offset(field->type, type->reg_size[k]);
                    field->reg_offset[k] = type->reg_size[k];
                    type->reg_size[k] += field->type->reg_size[k];
                }

                type->dimx += field->type->dimx * field->type->dimy * get_array_size(field->type);
            }
            break;
        }

        case HLSL_CLASS_OBJECT:
        {
            if (hlsl_type_is_resource(type))
            {
                enum hlsl_regset regset = hlsl_type_get_regset(type);

                type->reg_size[regset] = 1;
            }
            break;
        }
    }
}

/* Returns the size of a type, considered as part of an array of that type, within a specific
 * register set. As such it includes padding after the type, when applicable. */
unsigned int hlsl_type_get_array_element_reg_size(const struct hlsl_type *type, enum hlsl_regset regset)
{
    if (regset == HLSL_REGSET_NUMERIC)
        return align(type->reg_size[regset], 4);
    return type->reg_size[regset];
}

static struct hlsl_type *hlsl_new_type(struct hlsl_ctx *ctx, const char *name, enum hlsl_type_class type_class,
        enum hlsl_base_type base_type, unsigned dimx, unsigned dimy)
{
    struct hlsl_type *type;

    if (!(type = hlsl_alloc(ctx, sizeof(*type))))
        return NULL;
    if (!(type->name = hlsl_strdup(ctx, name)))
    {
        vkd3d_free(type);
        return NULL;
    }
    type->type = type_class;
    type->base_type = base_type;
    type->dimx = dimx;
    type->dimy = dimy;
    hlsl_type_calculate_reg_size(ctx, type);

    list_add_tail(&ctx->types, &type->entry);

    return type;
}

static bool type_is_single_component(const struct hlsl_type *type)
{
    return type->type == HLSL_CLASS_SCALAR || type->type == HLSL_CLASS_OBJECT;
}

/* Given a type and a component index, this function moves one step through the path required to
 * reach that component within the type.
 * It returns the first index of this path.
 * It sets *type_ptr to the (outermost) type within the original type that contains the component.
 * It sets *index_ptr to the index of the component within *type_ptr.
 * So, this function can be called several times in sequence to obtain all the path's indexes until
 * the component is finally reached. */
static unsigned int traverse_path_from_component_index(struct hlsl_ctx *ctx,
        struct hlsl_type **type_ptr, unsigned int *index_ptr)
{
    struct hlsl_type *type = *type_ptr;
    unsigned int index = *index_ptr;

    assert(!type_is_single_component(type));
    assert(index < hlsl_type_component_count(type));

    switch (type->type)
    {
        case HLSL_CLASS_VECTOR:
            assert(index < type->dimx);
            *type_ptr = hlsl_get_scalar_type(ctx, type->base_type);
            *index_ptr = 0;
            return index;

        case HLSL_CLASS_MATRIX:
        {
            unsigned int y = index / type->dimx, x = index % type->dimx;
            bool row_major = hlsl_type_is_row_major(type);

            assert(index < type->dimx * type->dimy);
            *type_ptr = hlsl_get_vector_type(ctx, type->base_type, row_major ? type->dimx : type->dimy);
            *index_ptr = row_major ? x : y;
            return row_major ? y : x;
        }

        case HLSL_CLASS_ARRAY:
        {
            unsigned int elem_comp_count = hlsl_type_component_count(type->e.array.type);
            unsigned int array_index;

            *type_ptr = type->e.array.type;
            *index_ptr = index % elem_comp_count;
            array_index = index / elem_comp_count;
            assert(array_index < type->e.array.elements_count);
            return array_index;
        }

        case HLSL_CLASS_STRUCT:
        {
            struct hlsl_struct_field *field;
            unsigned int field_comp_count, i;

            for (i = 0; i < type->e.record.field_count; ++i)
            {
                field = &type->e.record.fields[i];
                field_comp_count = hlsl_type_component_count(field->type);
                if (index < field_comp_count)
                {
                    *type_ptr = field->type;
                    *index_ptr = index;
                    return i;
                }
                index -= field_comp_count;
            }
            vkd3d_unreachable();
        }

        default:
            vkd3d_unreachable();
    }
}

struct hlsl_type *hlsl_type_get_component_type(struct hlsl_ctx *ctx, struct hlsl_type *type,
        unsigned int index)
{
    while (!type_is_single_component(type))
        traverse_path_from_component_index(ctx, &type, &index);

    return type;
}

static bool init_deref(struct hlsl_ctx *ctx, struct hlsl_deref *deref, struct hlsl_ir_var *var,
        unsigned int path_len)
{
    deref->var = var;
    deref->path_len = path_len;
    deref->offset.node = NULL;

    if (path_len == 0)
    {
        deref->path = NULL;
        return true;
    }

    if (!(deref->path = hlsl_alloc(ctx, sizeof(*deref->path) * deref->path_len)))
    {
        deref->var = NULL;
        deref->path_len = 0;
        return false;
    }

    return true;
}

struct hlsl_type *hlsl_deref_get_type(struct hlsl_ctx *ctx, const struct hlsl_deref *deref)
{
    struct hlsl_type *type;
    unsigned int i;

    assert(deref);
    assert(!deref->offset.node);

    type = deref->var->data_type;
    for (i = 0; i < deref->path_len; ++i)
        type = hlsl_get_element_type_from_path_index(ctx, type, deref->path[i].node);
    return type;
}

/* Initializes a deref from another deref (prefix) and a component index.
 * *block is initialized to contain the new constant node instructions used by the deref's path. */
static bool init_deref_from_component_index(struct hlsl_ctx *ctx, struct hlsl_block *block,
        struct hlsl_deref *deref, const struct hlsl_deref *prefix, unsigned int index,
        const struct vkd3d_shader_location *loc)
{
    unsigned int path_len, path_index, deref_path_len, i;
    struct hlsl_type *path_type;
    struct hlsl_ir_constant *c;

    list_init(&block->instrs);

    path_len = 0;
    path_type = hlsl_deref_get_type(ctx, prefix);
    path_index = index;
    while (!type_is_single_component(path_type))
    {
        traverse_path_from_component_index(ctx, &path_type, &path_index);
        ++path_len;
    }

    if (!init_deref(ctx, deref, prefix->var, prefix->path_len + path_len))
        return false;

    deref_path_len = 0;
    for (i = 0; i < prefix->path_len; ++i)
        hlsl_src_from_node(&deref->path[deref_path_len++], prefix->path[i].node);

    path_type = hlsl_deref_get_type(ctx, prefix);
    path_index = index;
    while (!type_is_single_component(path_type))
    {
        unsigned int next_index = traverse_path_from_component_index(ctx, &path_type, &path_index);

        if (!(c = hlsl_new_uint_constant(ctx, next_index, loc)))
        {
            hlsl_free_instr_list(&block->instrs);
            return false;
        }
        list_add_tail(&block->instrs, &c->node.entry);

        hlsl_src_from_node(&deref->path[deref_path_len++], &c->node);
    }

    assert(deref_path_len == deref->path_len);

    return true;
}

struct hlsl_type *hlsl_get_element_type_from_path_index(struct hlsl_ctx *ctx, const struct hlsl_type *type,
        struct hlsl_ir_node *idx)
{
    assert(idx);

    switch (type->type)
    {
        case HLSL_CLASS_VECTOR:
            return hlsl_get_scalar_type(ctx, type->base_type);

        case HLSL_CLASS_MATRIX:
            if (hlsl_type_is_row_major(type))
                return hlsl_get_vector_type(ctx, type->base_type, type->dimx);
            else
                return hlsl_get_vector_type(ctx, type->base_type, type->dimy);

        case HLSL_CLASS_ARRAY:
            return type->e.array.type;

        case HLSL_CLASS_STRUCT:
        {
            struct hlsl_ir_constant *c = hlsl_ir_constant(idx);

            assert(c->value[0].u < type->e.record.field_count);
            return type->e.record.fields[c->value[0].u].type;
        }

        default:
            vkd3d_unreachable();
    }
}

struct hlsl_type *hlsl_new_array_type(struct hlsl_ctx *ctx, struct hlsl_type *basic_type, unsigned int array_size)
{
    struct hlsl_type *type;

    if (!(type = hlsl_alloc(ctx, sizeof(*type))))
        return NULL;

    type->type = HLSL_CLASS_ARRAY;
    type->modifiers = basic_type->modifiers;
    type->e.array.elements_count = array_size;
    type->e.array.type = basic_type;
    type->dimx = basic_type->dimx;
    type->dimy = basic_type->dimy;
    hlsl_type_calculate_reg_size(ctx, type);

    list_add_tail(&ctx->types, &type->entry);

    return type;
}

struct hlsl_type *hlsl_new_struct_type(struct hlsl_ctx *ctx, const char *name,
        struct hlsl_struct_field *fields, size_t field_count)
{
    struct hlsl_type *type;

    if (!(type = hlsl_alloc(ctx, sizeof(*type))))
        return NULL;
    type->type = HLSL_CLASS_STRUCT;
    type->base_type = HLSL_TYPE_VOID;
    type->name = name;
    type->dimy = 1;
    type->e.record.fields = fields;
    type->e.record.field_count = field_count;
    hlsl_type_calculate_reg_size(ctx, type);

    list_add_tail(&ctx->types, &type->entry);

    return type;
}

struct hlsl_type *hlsl_new_texture_type(struct hlsl_ctx *ctx, enum hlsl_sampler_dim dim, struct hlsl_type *format)
{
    struct hlsl_type *type;

    if (!(type = hlsl_alloc(ctx, sizeof(*type))))
        return NULL;
    type->type = HLSL_CLASS_OBJECT;
    type->base_type = HLSL_TYPE_TEXTURE;
    type->dimx = 4;
    type->dimy = 1;
    type->sampler_dim = dim;
    type->e.resource_format = format;
    hlsl_type_calculate_reg_size(ctx, type);
    list_add_tail(&ctx->types, &type->entry);
    return type;
}

struct hlsl_type *hlsl_new_uav_type(struct hlsl_ctx *ctx, enum hlsl_sampler_dim dim, struct hlsl_type *format)
{
    struct hlsl_type *type;

    if (!(type = vkd3d_calloc(1, sizeof(*type))))
        return NULL;
    type->type = HLSL_CLASS_OBJECT;
    type->base_type = HLSL_TYPE_UAV;
    type->dimx = format->dimx;
    type->dimy = 1;
    type->sampler_dim = dim;
    type->e.resource_format = format;
    hlsl_type_calculate_reg_size(ctx, type);
    list_add_tail(&ctx->types, &type->entry);
    return type;
}

static const char * get_case_insensitive_typename(const char *name)
{
    static const char *const names[] =
    {
        "dword",
        "float",
    };
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(names); ++i)
    {
        if (!ascii_strcasecmp(names[i], name))
            return names[i];
    }

    return NULL;
}

struct hlsl_type *hlsl_get_type(struct hlsl_scope *scope, const char *name, bool recursive, bool case_insensitive)
{
    struct rb_entry *entry = rb_get(&scope->types, name);

    if (entry)
        return RB_ENTRY_VALUE(entry, struct hlsl_type, scope_entry);

    if (scope->upper)
    {
        if (recursive)
            return hlsl_get_type(scope->upper, name, recursive, case_insensitive);
    }
    else
    {
        if (case_insensitive && (name = get_case_insensitive_typename(name)))
        {
            if ((entry = rb_get(&scope->types, name)))
                return RB_ENTRY_VALUE(entry, struct hlsl_type, scope_entry);
        }
    }

    return NULL;
}

struct hlsl_ir_function *hlsl_get_function(struct hlsl_ctx *ctx, const char *name)
{
    struct rb_entry *entry;

    if ((entry = rb_get(&ctx->functions, name)))
        return RB_ENTRY_VALUE(entry, struct hlsl_ir_function, entry);
    return NULL;
}

struct hlsl_ir_function_decl *hlsl_get_func_decl(struct hlsl_ctx *ctx, const char *name)
{
    struct hlsl_ir_function_decl *decl;
    struct hlsl_ir_function *func;
    struct rb_entry *entry;

    if ((entry = rb_get(&ctx->functions, name)))
    {
        func = RB_ENTRY_VALUE(entry, struct hlsl_ir_function, entry);
        RB_FOR_EACH_ENTRY(decl, &func->overloads, struct hlsl_ir_function_decl, entry)
            return decl;
    }

    return NULL;
}

unsigned int hlsl_type_component_count(const struct hlsl_type *type)
{
    switch (type->type)
    {
        case HLSL_CLASS_SCALAR:
        case HLSL_CLASS_VECTOR:
        case HLSL_CLASS_MATRIX:
            return type->dimx * type->dimy;

        case HLSL_CLASS_STRUCT:
        {
            unsigned int count = 0, i;

            for (i = 0; i < type->e.record.field_count; ++i)
                count += hlsl_type_component_count(type->e.record.fields[i].type);
            return count;
        }

        case HLSL_CLASS_ARRAY:
            return hlsl_type_component_count(type->e.array.type) * type->e.array.elements_count;

        case HLSL_CLASS_OBJECT:
            return 1;

        default:
            vkd3d_unreachable();
    }
}

bool hlsl_types_are_equal(const struct hlsl_type *t1, const struct hlsl_type *t2)
{
    if (t1 == t2)
        return true;

    if (t1->type != t2->type)
        return false;
    if (t1->base_type != t2->base_type)
        return false;
    if (t1->base_type == HLSL_TYPE_SAMPLER || t1->base_type == HLSL_TYPE_TEXTURE
            || t1->base_type == HLSL_TYPE_UAV)
    {
        if (t1->sampler_dim != t2->sampler_dim)
            return false;
        if (t1->base_type == HLSL_TYPE_TEXTURE && t1->sampler_dim != HLSL_SAMPLER_DIM_GENERIC
                && !hlsl_types_are_equal(t1->e.resource_format, t2->e.resource_format))
            return false;
    }
    if ((t1->modifiers & HLSL_MODIFIER_ROW_MAJOR)
            != (t2->modifiers & HLSL_MODIFIER_ROW_MAJOR))
        return false;
    if (t1->dimx != t2->dimx)
        return false;
    if (t1->dimy != t2->dimy)
        return false;
    if (t1->type == HLSL_CLASS_STRUCT)
    {
        size_t i;

        if (t1->e.record.field_count != t2->e.record.field_count)
            return false;

        for (i = 0; i < t1->e.record.field_count; ++i)
        {
            const struct hlsl_struct_field *field1 = &t1->e.record.fields[i];
            const struct hlsl_struct_field *field2 = &t2->e.record.fields[i];

            if (!hlsl_types_are_equal(field1->type, field2->type))
                return false;

            if (strcmp(field1->name, field2->name))
                return false;
        }
    }
    if (t1->type == HLSL_CLASS_ARRAY)
        return t1->e.array.elements_count == t2->e.array.elements_count
                && hlsl_types_are_equal(t1->e.array.type, t2->e.array.type);

    return true;
}

struct hlsl_type *hlsl_type_clone(struct hlsl_ctx *ctx, struct hlsl_type *old,
        unsigned int default_majority, unsigned int modifiers)
{
    struct hlsl_type *type;

    if (!(type = hlsl_alloc(ctx, sizeof(*type))))
        return NULL;

    if (old->name)
    {
        type->name = hlsl_strdup(ctx, old->name);
        if (!type->name)
        {
            vkd3d_free(type);
            return NULL;
        }
    }
    type->type = old->type;
    type->base_type = old->base_type;
    type->dimx = old->dimx;
    type->dimy = old->dimy;
    type->modifiers = old->modifiers | modifiers;
    if (!(type->modifiers & HLSL_MODIFIERS_MAJORITY_MASK))
        type->modifiers |= default_majority;
    type->sampler_dim = old->sampler_dim;
    type->is_minimum_precision = old->is_minimum_precision;
    switch (old->type)
    {
        case HLSL_CLASS_ARRAY:
            if (!(type->e.array.type = hlsl_type_clone(ctx, old->e.array.type, default_majority, modifiers)))
            {
                vkd3d_free((void *)type->name);
                vkd3d_free(type);
                return NULL;
            }
            type->e.array.elements_count = old->e.array.elements_count;
            break;

        case HLSL_CLASS_STRUCT:
        {
            size_t field_count = old->e.record.field_count, i;

            type->e.record.field_count = field_count;

            if (!(type->e.record.fields = hlsl_alloc(ctx, field_count * sizeof(*type->e.record.fields))))
            {
                vkd3d_free((void *)type->name);
                vkd3d_free(type);
                return NULL;
            }

            for (i = 0; i < field_count; ++i)
            {
                const struct hlsl_struct_field *src_field = &old->e.record.fields[i];
                struct hlsl_struct_field *dst_field = &type->e.record.fields[i];

                dst_field->loc = src_field->loc;
                if (!(dst_field->type = hlsl_type_clone(ctx, src_field->type, default_majority, modifiers)))
                {
                    vkd3d_free(type->e.record.fields);
                    vkd3d_free((void *)type->name);
                    vkd3d_free(type);
                    return NULL;
                }
                dst_field->name = hlsl_strdup(ctx, src_field->name);
                if (src_field->semantic.name)
                {
                    dst_field->semantic.name = hlsl_strdup(ctx, src_field->semantic.name);
                    dst_field->semantic.index = src_field->semantic.index;
                }
            }
            break;
        }

        default:
            break;
    }

    hlsl_type_calculate_reg_size(ctx, type);

    list_add_tail(&ctx->types, &type->entry);
    return type;
}

bool hlsl_scope_add_type(struct hlsl_scope *scope, struct hlsl_type *type)
{
    if (hlsl_get_type(scope, type->name, false, false))
        return false;

    rb_put(&scope->types, type->name, &type->scope_entry);
    return true;
}

struct hlsl_ir_expr *hlsl_new_cast(struct hlsl_ctx *ctx, struct hlsl_ir_node *node, struct hlsl_type *type,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_node *cast;

    cast = hlsl_new_unary_expr(ctx, HLSL_OP1_CAST, node, *loc);
    if (cast)
        cast->data_type = type;
    return hlsl_ir_expr(cast);
}

struct hlsl_ir_expr *hlsl_new_copy(struct hlsl_ctx *ctx, struct hlsl_ir_node *node)
{
    /* Use a cast to the same type as a makeshift identity expression. */
    return hlsl_new_cast(ctx, node, node->data_type, &node->loc);
}

struct hlsl_ir_var *hlsl_new_var(struct hlsl_ctx *ctx, const char *name, struct hlsl_type *type,
        const struct vkd3d_shader_location loc, const struct hlsl_semantic *semantic, unsigned int modifiers,
        const struct hlsl_reg_reservation *reg_reservation)
{
    struct hlsl_ir_var *var;

    if (!(var = hlsl_alloc(ctx, sizeof(*var))))
        return NULL;

    var->name = name;
    var->data_type = type;
    var->loc = loc;
    if (semantic)
        var->semantic = *semantic;
    var->storage_modifiers = modifiers;
    if (reg_reservation)
        var->reg_reservation = *reg_reservation;
    return var;
}

struct hlsl_ir_var *hlsl_new_synthetic_var(struct hlsl_ctx *ctx, const char *template,
        struct hlsl_type *type, const struct vkd3d_shader_location *loc)
{
    struct vkd3d_string_buffer *string;
    struct hlsl_ir_var *var;
    static LONG counter;
    const char *name;

    if (!(string = hlsl_get_string_buffer(ctx)))
        return NULL;
    vkd3d_string_buffer_printf(string, "<%s-%u>", template, InterlockedIncrement(&counter));
    if (!(name = hlsl_strdup(ctx, string->buffer)))
    {
        hlsl_release_string_buffer(ctx, string);
        return NULL;
    }
    var = hlsl_new_var(ctx, name, type, *loc, NULL, 0, NULL);
    hlsl_release_string_buffer(ctx, string);
    if (var)
        list_add_tail(&ctx->dummy_scope->vars, &var->scope_entry);
    return var;
}

static bool type_is_single_reg(const struct hlsl_type *type)
{
    return type->type == HLSL_CLASS_SCALAR || type->type == HLSL_CLASS_VECTOR;
}

bool hlsl_copy_deref(struct hlsl_ctx *ctx, struct hlsl_deref *deref, const struct hlsl_deref *other)
{
    unsigned int i;

    memset(deref, 0, sizeof(*deref));

    if (!other)
        return true;

    assert(!other->offset.node);

    if (!init_deref(ctx, deref, other->var, other->path_len))
        return false;

    for (i = 0; i < deref->path_len; ++i)
        hlsl_src_from_node(&deref->path[i], other->path[i].node);

    return true;
}

void hlsl_cleanup_deref(struct hlsl_deref *deref)
{
    unsigned int i;

    for (i = 0; i < deref->path_len; ++i)
        hlsl_src_remove(&deref->path[i]);
    vkd3d_free(deref->path);

    deref->path = NULL;
    deref->path_len = 0;

    hlsl_src_remove(&deref->offset);
}

/* Initializes a simple variable derefence, so that it can be passed to load/store functions. */
void hlsl_init_simple_deref_from_var(struct hlsl_deref *deref, struct hlsl_ir_var *var)
{
    memset(deref, 0, sizeof(*deref));
    deref->var = var;
}

static void init_node(struct hlsl_ir_node *node, enum hlsl_ir_node_type type,
        struct hlsl_type *data_type, const struct vkd3d_shader_location *loc)
{
    memset(node, 0, sizeof(*node));
    node->type = type;
    node->data_type = data_type;
    node->loc = *loc;
    list_init(&node->uses);
}

struct hlsl_ir_store *hlsl_new_simple_store(struct hlsl_ctx *ctx, struct hlsl_ir_var *lhs, struct hlsl_ir_node *rhs)
{
    struct hlsl_deref lhs_deref;

    hlsl_init_simple_deref_from_var(&lhs_deref, lhs);
    return hlsl_new_store_index(ctx, &lhs_deref, NULL, rhs, 0, &rhs->loc);
}

struct hlsl_ir_store *hlsl_new_store_index(struct hlsl_ctx *ctx, const struct hlsl_deref *lhs,
        struct hlsl_ir_node *idx, struct hlsl_ir_node *rhs, unsigned int writemask, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_store *store;
    unsigned int i;

    assert(lhs);
    assert(!lhs->offset.node);

    if (!(store = hlsl_alloc(ctx, sizeof(*store))))
        return NULL;
    init_node(&store->node, HLSL_IR_STORE, NULL, loc);

    if (!init_deref(ctx, &store->lhs, lhs->var, lhs->path_len + !!idx))
    {
        vkd3d_free(store);
        return NULL;
    }
    for (i = 0; i < lhs->path_len; ++i)
        hlsl_src_from_node(&store->lhs.path[i], lhs->path[i].node);
    if (idx)
        hlsl_src_from_node(&store->lhs.path[lhs->path_len], idx);

    hlsl_src_from_node(&store->rhs, rhs);

    if (!writemask && type_is_single_reg(rhs->data_type))
        writemask = (1 << rhs->data_type->dimx) - 1;
    store->writemask = writemask;

    return store;
}

struct hlsl_ir_store *hlsl_new_store_component(struct hlsl_ctx *ctx, struct hlsl_block *block,
        const struct hlsl_deref *lhs, unsigned int comp, struct hlsl_ir_node *rhs)
{
    struct hlsl_block comp_path_block;
    struct hlsl_ir_store *store;

    list_init(&block->instrs);

    if (!(store = hlsl_alloc(ctx, sizeof(*store))))
        return NULL;
    init_node(&store->node, HLSL_IR_STORE, NULL, &rhs->loc);

    if (!init_deref_from_component_index(ctx, &comp_path_block, &store->lhs, lhs, comp, &rhs->loc))
    {
        vkd3d_free(store);
        return NULL;
    }
    list_move_tail(&block->instrs, &comp_path_block.instrs);
    hlsl_src_from_node(&store->rhs, rhs);

    if (type_is_single_reg(rhs->data_type))
        store->writemask = (1 << rhs->data_type->dimx) - 1;

    list_add_tail(&block->instrs, &store->node.entry);

    return store;
}

struct hlsl_ir_node *hlsl_new_call(struct hlsl_ctx *ctx, struct hlsl_ir_function_decl *decl,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_call *call;

    if (!(call = hlsl_alloc(ctx, sizeof(*call))))
        return NULL;

    init_node(&call->node, HLSL_IR_CALL, NULL, loc);
    call->decl = decl;
    return &call->node;
}

struct hlsl_ir_constant *hlsl_new_constant(struct hlsl_ctx *ctx, struct hlsl_type *type,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_constant *c;

    assert(type->type <= HLSL_CLASS_VECTOR);

    if (!(c = hlsl_alloc(ctx, sizeof(*c))))
        return NULL;

    init_node(&c->node, HLSL_IR_CONSTANT, type, loc);

    return c;
}

struct hlsl_ir_constant *hlsl_new_bool_constant(struct hlsl_ctx *ctx, bool b, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_constant *c;

    if ((c = hlsl_new_constant(ctx, hlsl_get_scalar_type(ctx, HLSL_TYPE_BOOL), loc)))
        c->value[0].u = b ? ~0u : 0;

    return c;
}

struct hlsl_ir_constant *hlsl_new_float_constant(struct hlsl_ctx *ctx, float f,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_constant *c;

    if ((c = hlsl_new_constant(ctx, hlsl_get_scalar_type(ctx, HLSL_TYPE_FLOAT), loc)))
        c->value[0].f = f;

    return c;
}

struct hlsl_ir_constant *hlsl_new_int_constant(struct hlsl_ctx *ctx, int n,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_constant *c;

    c = hlsl_new_constant(ctx, hlsl_get_scalar_type(ctx, HLSL_TYPE_INT), loc);

    if (c)
        c->value[0].i = n;

    return c;
}

struct hlsl_ir_constant *hlsl_new_uint_constant(struct hlsl_ctx *ctx, unsigned int n,
        const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_constant *c;

    c = hlsl_new_constant(ctx, hlsl_get_scalar_type(ctx, HLSL_TYPE_UINT), loc);

    if (c)
        c->value[0].u = n;

    return c;
}

struct hlsl_ir_node *hlsl_new_expr(struct hlsl_ctx *ctx, enum hlsl_ir_expr_op op,
        struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS],
        struct hlsl_type *data_type, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_expr *expr;
    unsigned int i;

    if (!(expr = hlsl_alloc(ctx, sizeof(*expr))))
        return NULL;
    init_node(&expr->node, HLSL_IR_EXPR, data_type, loc);
    expr->op = op;
    for (i = 0; i < HLSL_MAX_OPERANDS; ++i)
        hlsl_src_from_node(&expr->operands[i], operands[i]);
    return &expr->node;
}

struct hlsl_ir_node *hlsl_new_unary_expr(struct hlsl_ctx *ctx, enum hlsl_ir_expr_op op,
        struct hlsl_ir_node *arg, struct vkd3d_shader_location loc)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {arg};

    return hlsl_new_expr(ctx, op, operands, arg->data_type, &loc);
}

struct hlsl_ir_node *hlsl_new_binary_expr(struct hlsl_ctx *ctx, enum hlsl_ir_expr_op op,
        struct hlsl_ir_node *arg1, struct hlsl_ir_node *arg2)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS] = {arg1, arg2};

    assert(hlsl_types_are_equal(arg1->data_type, arg2->data_type));
    return hlsl_new_expr(ctx, op, operands, arg1->data_type, &arg1->loc);
}

struct hlsl_ir_if *hlsl_new_if(struct hlsl_ctx *ctx, struct hlsl_ir_node *condition, struct vkd3d_shader_location loc)
{
    struct hlsl_ir_if *iff;

    if (!(iff = hlsl_alloc(ctx, sizeof(*iff))))
        return NULL;
    init_node(&iff->node, HLSL_IR_IF, NULL, &loc);
    hlsl_src_from_node(&iff->condition, condition);
    list_init(&iff->then_instrs.instrs);
    list_init(&iff->else_instrs.instrs);
    return iff;
}

struct hlsl_ir_load *hlsl_new_load_index(struct hlsl_ctx *ctx, const struct hlsl_deref *deref,
        struct hlsl_ir_node *idx, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_load *load;
    struct hlsl_type *type;
    unsigned int i;

    assert(!deref->offset.node);

    type = hlsl_deref_get_type(ctx, deref);
    if (idx)
        type = hlsl_get_element_type_from_path_index(ctx, type, idx);

    if (!(load = hlsl_alloc(ctx, sizeof(*load))))
        return NULL;
    init_node(&load->node, HLSL_IR_LOAD, type, loc);

    if (!init_deref(ctx, &load->src, deref->var, deref->path_len + !!idx))
    {
        vkd3d_free(load);
        return NULL;
    }
    for (i = 0; i < deref->path_len; ++i)
        hlsl_src_from_node(&load->src.path[i], deref->path[i].node);
    if (idx)
        hlsl_src_from_node(&load->src.path[deref->path_len], idx);

    return load;
}

struct hlsl_ir_load *hlsl_new_var_load(struct hlsl_ctx *ctx, struct hlsl_ir_var *var,
        struct vkd3d_shader_location loc)
{
    struct hlsl_deref var_deref;

    hlsl_init_simple_deref_from_var(&var_deref, var);
    return hlsl_new_load_index(ctx, &var_deref, NULL, &loc);
}

struct hlsl_ir_load *hlsl_new_load_component(struct hlsl_ctx *ctx, struct hlsl_block *block,
        const struct hlsl_deref *deref, unsigned int comp, const struct vkd3d_shader_location *loc)
{
    struct hlsl_type *type, *comp_type;
    struct hlsl_block comp_path_block;
    struct hlsl_ir_load *load;

    list_init(&block->instrs);

    if (!(load = hlsl_alloc(ctx, sizeof(*load))))
        return NULL;

    type = hlsl_deref_get_type(ctx, deref);
    comp_type = hlsl_type_get_component_type(ctx, type, comp);
    init_node(&load->node, HLSL_IR_LOAD, comp_type, loc);

    if (!init_deref_from_component_index(ctx, &comp_path_block, &load->src, deref, comp, loc))
    {
        vkd3d_free(load);
        return NULL;
    }
    list_move_tail(&block->instrs, &comp_path_block.instrs);

    list_add_tail(&block->instrs, &load->node.entry);

    return load;
}

struct hlsl_ir_resource_load *hlsl_new_resource_load(struct hlsl_ctx *ctx,
        const struct hlsl_resource_load_params *params, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_resource_load *load;

    if (!(load = hlsl_alloc(ctx, sizeof(*load))))
        return NULL;
    init_node(&load->node, HLSL_IR_RESOURCE_LOAD, params->format, loc);
    load->load_type = params->type;
    if (!hlsl_copy_deref(ctx, &load->resource, &params->resource))
    {
        vkd3d_free(load);
        return NULL;
    }
    if (!hlsl_copy_deref(ctx, &load->sampler, &params->sampler))
    {
        hlsl_cleanup_deref(&load->resource);
        vkd3d_free(load);
        return NULL;
    }
    hlsl_src_from_node(&load->coords, params->coords);
    hlsl_src_from_node(&load->texel_offset, params->texel_offset);
    hlsl_src_from_node(&load->lod, params->lod);
    return load;
}

struct hlsl_ir_resource_store *hlsl_new_resource_store(struct hlsl_ctx *ctx, const struct hlsl_deref *resource,
        struct hlsl_ir_node *coords, struct hlsl_ir_node *value, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_resource_store *store;

    if (!(store = hlsl_alloc(ctx, sizeof(*store))))
        return NULL;
    init_node(&store->node, HLSL_IR_RESOURCE_STORE, NULL, loc);
    hlsl_copy_deref(ctx, &store->resource, resource);
    hlsl_src_from_node(&store->coords, coords);
    hlsl_src_from_node(&store->value, value);
    return store;
}

struct hlsl_ir_swizzle *hlsl_new_swizzle(struct hlsl_ctx *ctx, DWORD s, unsigned int components,
        struct hlsl_ir_node *val, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_swizzle *swizzle;
    struct hlsl_type *type;

    if (!(swizzle = hlsl_alloc(ctx, sizeof(*swizzle))))
        return NULL;
    if (components == 1)
        type = hlsl_get_scalar_type(ctx, val->data_type->base_type);
    else
        type = hlsl_get_vector_type(ctx, val->data_type->base_type, components);
    init_node(&swizzle->node, HLSL_IR_SWIZZLE, type, loc);
    hlsl_src_from_node(&swizzle->val, val);
    swizzle->swizzle = s;
    return swizzle;
}

struct hlsl_ir_jump *hlsl_new_jump(struct hlsl_ctx *ctx, enum hlsl_ir_jump_type type, struct vkd3d_shader_location loc)
{
    struct hlsl_ir_jump *jump;

    if (!(jump = hlsl_alloc(ctx, sizeof(*jump))))
        return NULL;
    init_node(&jump->node, HLSL_IR_JUMP, NULL, &loc);
    jump->type = type;
    return jump;
}

struct hlsl_ir_loop *hlsl_new_loop(struct hlsl_ctx *ctx, struct vkd3d_shader_location loc)
{
    struct hlsl_ir_loop *loop;

    if (!(loop = hlsl_alloc(ctx, sizeof(*loop))))
        return NULL;
    init_node(&loop->node, HLSL_IR_LOOP, NULL, &loc);
    list_init(&loop->body.instrs);
    return loop;
}

struct clone_instr_map
{
    struct
    {
        const struct hlsl_ir_node *src;
        struct hlsl_ir_node *dst;
    } *instrs;
    size_t count, capacity;
};

static struct hlsl_ir_node *clone_instr(struct hlsl_ctx *ctx,
        struct clone_instr_map *map, const struct hlsl_ir_node *instr);

static bool clone_block(struct hlsl_ctx *ctx, struct hlsl_block *dst_block,
        const struct hlsl_block *src_block, struct clone_instr_map *map)
{
    const struct hlsl_ir_node *src;
    struct hlsl_ir_node *dst;

    LIST_FOR_EACH_ENTRY(src, &src_block->instrs, struct hlsl_ir_node, entry)
    {
        if (!(dst = clone_instr(ctx, map, src)))
        {
            hlsl_free_instr_list(&dst_block->instrs);
            return false;
        }
        list_add_tail(&dst_block->instrs, &dst->entry);

        if (!list_empty(&src->uses))
        {
            if (!vkd3d_array_reserve((void **)&map->instrs, &map->capacity, map->count + 1, sizeof(*map->instrs)))
            {
                hlsl_free_instr_list(&dst_block->instrs);
                return false;
            }

            map->instrs[map->count].dst = dst;
            map->instrs[map->count].src = src;
            ++map->count;
        }
    }
    return true;
}

static struct hlsl_ir_node *map_instr(const struct clone_instr_map *map, struct hlsl_ir_node *src)
{
    size_t i;

    if (!src)
        return NULL;

    for (i = 0; i < map->count; ++i)
    {
        if (map->instrs[i].src == src)
            return map->instrs[i].dst;
    }

    /* The block passed to hlsl_clone_block() should have been free of external
     * references. */
    vkd3d_unreachable();
}

static bool clone_deref(struct hlsl_ctx *ctx, struct clone_instr_map *map,
        struct hlsl_deref *dst, const struct hlsl_deref *src)
{
    unsigned int i;

    assert(!src->offset.node);

    if (!init_deref(ctx, dst, src->var, src->path_len))
        return false;

    for (i = 0; i < src->path_len; ++i)
        hlsl_src_from_node(&dst->path[i], map_instr(map, src->path[i].node));

    return true;
}

static void clone_src(struct clone_instr_map *map, struct hlsl_src *dst, const struct hlsl_src *src)
{
    hlsl_src_from_node(dst, map_instr(map, src->node));
}

static struct hlsl_ir_node *clone_call(struct hlsl_ctx *ctx, struct hlsl_ir_call *src)
{
    return hlsl_new_call(ctx, src->decl, &src->node.loc);
}

static struct hlsl_ir_node *clone_constant(struct hlsl_ctx *ctx, struct hlsl_ir_constant *src)
{
    struct hlsl_ir_constant *dst;

    if (!(dst = hlsl_new_constant(ctx, src->node.data_type, &src->node.loc)))
        return NULL;
    memcpy(dst->value, src->value, sizeof(src->value));
    return &dst->node;
}

static struct hlsl_ir_node *clone_expr(struct hlsl_ctx *ctx, struct clone_instr_map *map, struct hlsl_ir_expr *src)
{
    struct hlsl_ir_node *operands[HLSL_MAX_OPERANDS];
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(operands); ++i)
        operands[i] = map_instr(map, src->operands[i].node);

    return hlsl_new_expr(ctx, src->op, operands, src->node.data_type, &src->node.loc);
}

static struct hlsl_ir_node *clone_if(struct hlsl_ctx *ctx, struct clone_instr_map *map, struct hlsl_ir_if *src)
{
    struct hlsl_ir_if *dst;

    if (!(dst = hlsl_new_if(ctx, map_instr(map, src->condition.node), src->node.loc)))
        return NULL;

    if (!clone_block(ctx, &dst->then_instrs, &src->then_instrs, map)
            || !clone_block(ctx, &dst->else_instrs, &src->else_instrs, map))
    {
        hlsl_free_instr(&dst->node);
        return NULL;
    }
    return &dst->node;
}

static struct hlsl_ir_node *clone_jump(struct hlsl_ctx *ctx, struct hlsl_ir_jump *src)
{
    struct hlsl_ir_jump *dst;

    if (!(dst = hlsl_new_jump(ctx, src->type, src->node.loc)))
        return NULL;
    return &dst->node;
}

static struct hlsl_ir_node *clone_load(struct hlsl_ctx *ctx, struct clone_instr_map *map, struct hlsl_ir_load *src)
{
    struct hlsl_ir_load *dst;

    if (!(dst = hlsl_alloc(ctx, sizeof(*dst))))
        return NULL;
    init_node(&dst->node, HLSL_IR_LOAD, src->node.data_type, &src->node.loc);

    if (!clone_deref(ctx, map, &dst->src, &src->src))
    {
        vkd3d_free(dst);
        return NULL;
    }
    return &dst->node;
}

static struct hlsl_ir_node *clone_loop(struct hlsl_ctx *ctx, struct clone_instr_map *map, struct hlsl_ir_loop *src)
{
    struct hlsl_ir_loop *dst;

    if (!(dst = hlsl_new_loop(ctx, src->node.loc)))
        return NULL;
    if (!clone_block(ctx, &dst->body, &src->body, map))
    {
        hlsl_free_instr(&dst->node);
        return NULL;
    }
    return &dst->node;
}

static struct hlsl_ir_node *clone_resource_load(struct hlsl_ctx *ctx,
        struct clone_instr_map *map, struct hlsl_ir_resource_load *src)
{
    struct hlsl_ir_resource_load *dst;

    if (!(dst = hlsl_alloc(ctx, sizeof(*dst))))
        return NULL;
    init_node(&dst->node, HLSL_IR_RESOURCE_LOAD, src->node.data_type, &src->node.loc);
    dst->load_type = src->load_type;
    if (!clone_deref(ctx, map, &dst->resource, &src->resource))
    {
        vkd3d_free(dst);
        return NULL;
    }
    if (!clone_deref(ctx, map, &dst->sampler, &src->sampler))
    {
        hlsl_cleanup_deref(&dst->resource);
        vkd3d_free(dst);
        return NULL;
    }
    clone_src(map, &dst->coords, &src->coords);
    clone_src(map, &dst->lod, &src->lod);
    clone_src(map, &dst->texel_offset, &src->texel_offset);
    return &dst->node;
}

static struct hlsl_ir_node *clone_resource_store(struct hlsl_ctx *ctx,
        struct clone_instr_map *map, struct hlsl_ir_resource_store *src)
{
    struct hlsl_ir_resource_store *dst;

    if (!(dst = hlsl_alloc(ctx, sizeof(*dst))))
        return NULL;
    init_node(&dst->node, HLSL_IR_RESOURCE_STORE, NULL, &src->node.loc);
    if (!clone_deref(ctx, map, &dst->resource, &src->resource))
    {
        vkd3d_free(dst);
        return NULL;
    }
    clone_src(map, &dst->coords, &src->coords);
    clone_src(map, &dst->value, &src->value);
    return &dst->node;
}

static struct hlsl_ir_node *clone_store(struct hlsl_ctx *ctx, struct clone_instr_map *map, struct hlsl_ir_store *src)
{
    struct hlsl_ir_store *dst;

    if (!(dst = hlsl_alloc(ctx, sizeof(*dst))))
        return NULL;
    init_node(&dst->node, HLSL_IR_STORE, NULL, &src->node.loc);

    if (!clone_deref(ctx, map, &dst->lhs, &src->lhs))
    {
        vkd3d_free(dst);
        return NULL;
    }
    clone_src(map, &dst->rhs, &src->rhs);
    dst->writemask = src->writemask;
    return &dst->node;
}

static struct hlsl_ir_node *clone_swizzle(struct hlsl_ctx *ctx,
        struct clone_instr_map *map, struct hlsl_ir_swizzle *src)
{
    struct hlsl_ir_swizzle *dst;

    if (!(dst = hlsl_new_swizzle(ctx, src->swizzle, src->node.data_type->dimx,
            map_instr(map, src->val.node), &src->node.loc)))
        return NULL;
    return &dst->node;
}

static struct hlsl_ir_node *clone_instr(struct hlsl_ctx *ctx,
        struct clone_instr_map *map, const struct hlsl_ir_node *instr)
{
    switch (instr->type)
    {
        case HLSL_IR_CALL:
            return clone_call(ctx, hlsl_ir_call(instr));

        case HLSL_IR_CONSTANT:
            return clone_constant(ctx, hlsl_ir_constant(instr));

        case HLSL_IR_EXPR:
            return clone_expr(ctx, map, hlsl_ir_expr(instr));

        case HLSL_IR_IF:
            return clone_if(ctx, map, hlsl_ir_if(instr));

        case HLSL_IR_JUMP:
            return clone_jump(ctx, hlsl_ir_jump(instr));

        case HLSL_IR_LOAD:
            return clone_load(ctx, map, hlsl_ir_load(instr));

        case HLSL_IR_LOOP:
            return clone_loop(ctx, map, hlsl_ir_loop(instr));

        case HLSL_IR_RESOURCE_LOAD:
            return clone_resource_load(ctx, map, hlsl_ir_resource_load(instr));

        case HLSL_IR_RESOURCE_STORE:
            return clone_resource_store(ctx, map, hlsl_ir_resource_store(instr));

        case HLSL_IR_STORE:
            return clone_store(ctx, map, hlsl_ir_store(instr));

        case HLSL_IR_SWIZZLE:
            return clone_swizzle(ctx, map, hlsl_ir_swizzle(instr));
    }

    vkd3d_unreachable();
}

bool hlsl_clone_block(struct hlsl_ctx *ctx, struct hlsl_block *dst_block, const struct hlsl_block *src_block)
{
    struct clone_instr_map map = {0};
    bool ret;

    ret = clone_block(ctx, dst_block, src_block, &map);
    vkd3d_free(map.instrs);
    return ret;
}

struct hlsl_ir_function_decl *hlsl_new_func_decl(struct hlsl_ctx *ctx,
        struct hlsl_type *return_type, const struct hlsl_func_parameters *parameters,
        const struct hlsl_semantic *semantic, const struct vkd3d_shader_location *loc)
{
    struct hlsl_ir_function_decl *decl;
    struct hlsl_ir_constant *constant;
    struct hlsl_ir_store *store;

    if (!(decl = hlsl_alloc(ctx, sizeof(*decl))))
        return NULL;
    list_init(&decl->body.instrs);
    decl->return_type = return_type;
    decl->parameters = *parameters;
    decl->loc = *loc;

    if (!hlsl_types_are_equal(return_type, ctx->builtin_types.Void))
    {
        if (!(decl->return_var = hlsl_new_synthetic_var(ctx, "retval", return_type, loc)))
        {
            vkd3d_free(decl);
            return NULL;
        }
        decl->return_var->semantic = *semantic;
    }

    if (!(decl->early_return_var = hlsl_new_synthetic_var(ctx, "early_return",
            hlsl_get_scalar_type(ctx, HLSL_TYPE_BOOL), loc)))
        return decl;

    if (!(constant = hlsl_new_bool_constant(ctx, false, loc)))
        return decl;
    list_add_tail(&decl->body.instrs, &constant->node.entry);

    if (!(store = hlsl_new_simple_store(ctx, decl->early_return_var, &constant->node)))
        return decl;
    list_add_tail(&decl->body.instrs, &store->node.entry);

    return decl;
}

struct hlsl_buffer *hlsl_new_buffer(struct hlsl_ctx *ctx, enum hlsl_buffer_type type, const char *name,
        const struct hlsl_reg_reservation *reservation, struct vkd3d_shader_location loc)
{
    struct hlsl_buffer *buffer;

    if (!(buffer = hlsl_alloc(ctx, sizeof(*buffer))))
        return NULL;
    buffer->type = type;
    buffer->name = name;
    if (reservation)
        buffer->reservation = *reservation;
    buffer->loc = loc;
    list_add_tail(&ctx->buffers, &buffer->entry);
    return buffer;
}

static int compare_hlsl_types_rb(const void *key, const struct rb_entry *entry)
{
    const struct hlsl_type *type = RB_ENTRY_VALUE(entry, const struct hlsl_type, scope_entry);
    const char *name = key;

    if (name == type->name)
        return 0;

    if (!name || !type->name)
    {
        ERR("hlsl_type without a name in a scope?\n");
        return -1;
    }
    return strcmp(name, type->name);
}

static struct hlsl_scope *hlsl_new_scope(struct hlsl_ctx *ctx, struct hlsl_scope *upper)
{
    struct hlsl_scope *scope;

    if (!(scope = hlsl_alloc(ctx, sizeof(*scope))))
        return NULL;
    list_init(&scope->vars);
    rb_init(&scope->types, compare_hlsl_types_rb);
    scope->upper = upper;
    list_add_tail(&ctx->scopes, &scope->entry);
    return scope;
}

void hlsl_push_scope(struct hlsl_ctx *ctx)
{
    struct hlsl_scope *new_scope;

    if (!(new_scope = hlsl_new_scope(ctx, ctx->cur_scope)))
        return;

    TRACE("Pushing a new scope.\n");
    ctx->cur_scope = new_scope;
}

void hlsl_pop_scope(struct hlsl_ctx *ctx)
{
    struct hlsl_scope *prev_scope = ctx->cur_scope->upper;

    assert(prev_scope);
    TRACE("Popping current scope.\n");
    ctx->cur_scope = prev_scope;
}

static int compare_param_hlsl_types(const struct hlsl_type *t1, const struct hlsl_type *t2)
{
    int r;

    if ((r = vkd3d_u32_compare(t1->type, t2->type)))
    {
        if (!((t1->type == HLSL_CLASS_SCALAR && t2->type == HLSL_CLASS_VECTOR)
                || (t1->type == HLSL_CLASS_VECTOR && t2->type == HLSL_CLASS_SCALAR)))
            return r;
    }
    if ((r = vkd3d_u32_compare(t1->base_type, t2->base_type)))
        return r;
    if (t1->base_type == HLSL_TYPE_SAMPLER || t1->base_type == HLSL_TYPE_TEXTURE)
    {
        if ((r = vkd3d_u32_compare(t1->sampler_dim, t2->sampler_dim)))
            return r;
        if (t1->base_type == HLSL_TYPE_TEXTURE && t1->sampler_dim != HLSL_SAMPLER_DIM_GENERIC
                && (r = compare_param_hlsl_types(t1->e.resource_format, t2->e.resource_format)))
            return r;
    }
    if ((r = vkd3d_u32_compare(t1->dimx, t2->dimx)))
        return r;
    if ((r = vkd3d_u32_compare(t1->dimy, t2->dimy)))
        return r;
    if (t1->type == HLSL_CLASS_STRUCT)
    {
        size_t i;

        if (t1->e.record.field_count != t2->e.record.field_count)
            return t1->e.record.field_count - t2->e.record.field_count;

        for (i = 0; i < t1->e.record.field_count; ++i)
        {
            const struct hlsl_struct_field *field1 = &t1->e.record.fields[i];
            const struct hlsl_struct_field *field2 = &t2->e.record.fields[i];

            if ((r = compare_param_hlsl_types(field1->type, field2->type)))
                return r;

            if ((r = strcmp(field1->name, field2->name)))
                return r;
        }
        return 0;
    }
    if (t1->type == HLSL_CLASS_ARRAY)
    {
        if ((r = vkd3d_u32_compare(t1->e.array.elements_count, t2->e.array.elements_count)))
            return r;
        return compare_param_hlsl_types(t1->e.array.type, t2->e.array.type);
    }

    return 0;
}

static int compare_function_decl_rb(const void *key, const struct rb_entry *entry)
{
    const struct hlsl_ir_function_decl *decl = RB_ENTRY_VALUE(entry, const struct hlsl_ir_function_decl, entry);
    const struct hlsl_func_parameters *parameters = key;
    size_t i;
    int r;

    if ((r = vkd3d_u32_compare(parameters->count, decl->parameters.count)))
        return r;

    for (i = 0; i < parameters->count; ++i)
    {
        if ((r = compare_param_hlsl_types(parameters->vars[i]->data_type, decl->parameters.vars[i]->data_type)))
            return r;
    }
    return 0;
}

struct vkd3d_string_buffer *hlsl_type_to_string(struct hlsl_ctx *ctx, const struct hlsl_type *type)
{
    struct vkd3d_string_buffer *string;

    static const char *const base_types[] =
    {
        [HLSL_TYPE_FLOAT] = "float",
        [HLSL_TYPE_HALF] = "half",
        [HLSL_TYPE_DOUBLE] = "double",
        [HLSL_TYPE_INT] = "int",
        [HLSL_TYPE_UINT] = "uint",
        [HLSL_TYPE_BOOL] = "bool",
    };

    if (!(string = hlsl_get_string_buffer(ctx)))
        return NULL;

    if (type->name)
    {
        vkd3d_string_buffer_printf(string, "%s", type->name);
        return string;
    }

    switch (type->type)
    {
        case HLSL_CLASS_SCALAR:
            assert(type->base_type < ARRAY_SIZE(base_types));
            vkd3d_string_buffer_printf(string, "%s", base_types[type->base_type]);
            return string;

        case HLSL_CLASS_VECTOR:
            assert(type->base_type < ARRAY_SIZE(base_types));
            vkd3d_string_buffer_printf(string, "%s%u", base_types[type->base_type], type->dimx);
            return string;

        case HLSL_CLASS_MATRIX:
            assert(type->base_type < ARRAY_SIZE(base_types));
            vkd3d_string_buffer_printf(string, "%s%ux%u", base_types[type->base_type], type->dimy, type->dimx);
            return string;

        case HLSL_CLASS_ARRAY:
        {
            struct vkd3d_string_buffer *inner_string;
            const struct hlsl_type *t;

            for (t = type; t->type == HLSL_CLASS_ARRAY; t = t->e.array.type)
                ;

            if ((inner_string = hlsl_type_to_string(ctx, t)))
            {
                vkd3d_string_buffer_printf(string, "%s", inner_string->buffer);
                hlsl_release_string_buffer(ctx, inner_string);
            }

            for (t = type; t->type == HLSL_CLASS_ARRAY; t = t->e.array.type)
            {
                if (t->e.array.elements_count == HLSL_ARRAY_ELEMENTS_COUNT_IMPLICIT)
                    vkd3d_string_buffer_printf(string, "[]");
                else
                    vkd3d_string_buffer_printf(string, "[%u]", t->e.array.elements_count);
            }
            return string;
        }

        case HLSL_CLASS_STRUCT:
            vkd3d_string_buffer_printf(string, "<anonymous struct>");
            return string;

        case HLSL_CLASS_OBJECT:
        {
            static const char *const dimensions[] =
            {
                [HLSL_SAMPLER_DIM_1D]        = "1D",
                [HLSL_SAMPLER_DIM_2D]        = "2D",
                [HLSL_SAMPLER_DIM_3D]        = "3D",
                [HLSL_SAMPLER_DIM_CUBE]      = "Cube",
                [HLSL_SAMPLER_DIM_1DARRAY]   = "1DArray",
                [HLSL_SAMPLER_DIM_2DARRAY]   = "2DArray",
                [HLSL_SAMPLER_DIM_2DMS]      = "2DMS",
                [HLSL_SAMPLER_DIM_2DMSARRAY] = "2DMSArray",
                [HLSL_SAMPLER_DIM_CUBEARRAY] = "CubeArray",
            };

            switch (type->base_type)
            {
                case HLSL_TYPE_TEXTURE:
                    if (type->sampler_dim == HLSL_SAMPLER_DIM_GENERIC)
                    {
                        vkd3d_string_buffer_printf(string, "Texture");
                        return string;
                    }

                    assert(type->sampler_dim < ARRAY_SIZE(dimensions));
                    assert(type->e.resource_format->base_type < ARRAY_SIZE(base_types));
                    vkd3d_string_buffer_printf(string, "Texture%s<%s%u>", dimensions[type->sampler_dim],
                            base_types[type->e.resource_format->base_type], type->e.resource_format->dimx);
                    return string;

                case HLSL_TYPE_UAV:
                    vkd3d_string_buffer_printf(string, "RWTexture%s<%s%u>", dimensions[type->sampler_dim],
                            base_types[type->e.resource_format->base_type], type->e.resource_format->dimx);
                    return string;

                default:
                    vkd3d_string_buffer_printf(string, "<unexpected type>");
                    return string;
            }
        }

        default:
            vkd3d_string_buffer_printf(string, "<unexpected type>");
            return string;
    }
}

const char *debug_hlsl_type(struct hlsl_ctx *ctx, const struct hlsl_type *type)
{
    struct vkd3d_string_buffer *string;
    const char *ret;

    if (!(string = hlsl_type_to_string(ctx, type)))
        return NULL;
    ret = vkd3d_dbg_sprintf("%s", string->buffer);
    hlsl_release_string_buffer(ctx, string);
    return ret;
}

struct vkd3d_string_buffer *hlsl_modifiers_to_string(struct hlsl_ctx *ctx, unsigned int modifiers)
{
    struct vkd3d_string_buffer *string;

    if (!(string = hlsl_get_string_buffer(ctx)))
        return NULL;

    if (modifiers & HLSL_STORAGE_EXTERN)
        vkd3d_string_buffer_printf(string, "extern ");
    if (modifiers & HLSL_STORAGE_NOINTERPOLATION)
        vkd3d_string_buffer_printf(string, "nointerpolation ");
    if (modifiers & HLSL_MODIFIER_PRECISE)
        vkd3d_string_buffer_printf(string, "precise ");
    if (modifiers & HLSL_STORAGE_SHARED)
        vkd3d_string_buffer_printf(string, "shared ");
    if (modifiers & HLSL_STORAGE_GROUPSHARED)
        vkd3d_string_buffer_printf(string, "groupshared ");
    if (modifiers & HLSL_STORAGE_STATIC)
        vkd3d_string_buffer_printf(string, "static ");
    if (modifiers & HLSL_STORAGE_UNIFORM)
        vkd3d_string_buffer_printf(string, "uniform ");
    if (modifiers & HLSL_MODIFIER_VOLATILE)
        vkd3d_string_buffer_printf(string, "volatile ");
    if (modifiers & HLSL_MODIFIER_CONST)
        vkd3d_string_buffer_printf(string, "const ");
    if (modifiers & HLSL_MODIFIER_ROW_MAJOR)
        vkd3d_string_buffer_printf(string, "row_major ");
    if (modifiers & HLSL_MODIFIER_COLUMN_MAJOR)
        vkd3d_string_buffer_printf(string, "column_major ");
    if ((modifiers & (HLSL_STORAGE_IN | HLSL_STORAGE_OUT)) == (HLSL_STORAGE_IN | HLSL_STORAGE_OUT))
        vkd3d_string_buffer_printf(string, "inout ");
    else if (modifiers & HLSL_STORAGE_IN)
        vkd3d_string_buffer_printf(string, "in ");
    else if (modifiers & HLSL_STORAGE_OUT)
        vkd3d_string_buffer_printf(string, "out ");

    if (string->content_size)
        string->buffer[--string->content_size] = 0;

    return string;
}

const char *hlsl_node_type_to_string(enum hlsl_ir_node_type type)
{
    static const char * const names[] =
    {
        "HLSL_IR_CALL",
        "HLSL_IR_CONSTANT",
        "HLSL_IR_EXPR",
        "HLSL_IR_IF",
        "HLSL_IR_LOAD",
        "HLSL_IR_LOOP",
        "HLSL_IR_JUMP",
        "HLSL_IR_RESOURCE_LOAD",
        "HLSL_IR_RESOURCE_STORE",
        "HLSL_IR_STORE",
        "HLSL_IR_SWIZZLE",
    };

    if (type >= ARRAY_SIZE(names))
        return "Unexpected node type";
    return names[type];
}

const char *hlsl_jump_type_to_string(enum hlsl_ir_jump_type type)
{
    static const char * const names[] =
    {
        "HLSL_IR_JUMP_BREAK",
        "HLSL_IR_JUMP_CONTINUE",
        "HLSL_IR_JUMP_DISCARD",
        "HLSL_IR_JUMP_RETURN",
    };

    assert(type < ARRAY_SIZE(names));
    return names[type];
}

static void dump_instr(struct hlsl_ctx *ctx, struct vkd3d_string_buffer *buffer, const struct hlsl_ir_node *instr);

static void dump_instr_list(struct hlsl_ctx *ctx, struct vkd3d_string_buffer *buffer, const struct list *list)
{
    struct hlsl_ir_node *instr;

    LIST_FOR_EACH_ENTRY(instr, list, struct hlsl_ir_node, entry)
    {
        dump_instr(ctx, buffer, instr);
        vkd3d_string_buffer_printf(buffer, "\n");
    }
}

static void dump_src(struct vkd3d_string_buffer *buffer, const struct hlsl_src *src)
{
    if (src->node->index)
        vkd3d_string_buffer_printf(buffer, "@%u", src->node->index);
    else
        vkd3d_string_buffer_printf(buffer, "%p", src->node);
}

static void dump_ir_var(struct hlsl_ctx *ctx, struct vkd3d_string_buffer *buffer, const struct hlsl_ir_var *var)
{
    if (var->storage_modifiers)
    {
        struct vkd3d_string_buffer *string;

        if ((string = hlsl_modifiers_to_string(ctx, var->storage_modifiers)))
            vkd3d_string_buffer_printf(buffer, "%s ", string->buffer);
        hlsl_release_string_buffer(ctx, string);
    }
    vkd3d_string_buffer_printf(buffer, "%s %s", debug_hlsl_type(ctx, var->data_type), var->name);
    if (var->semantic.name)
        vkd3d_string_buffer_printf(buffer, " : %s%u", var->semantic.name, var->semantic.index);
}

static void dump_deref(struct vkd3d_string_buffer *buffer, const struct hlsl_deref *deref)
{
    unsigned int i;

    if (deref->var)
    {
        vkd3d_string_buffer_printf(buffer, "%s", deref->var->name);
        if (deref->path_len)
        {
            vkd3d_string_buffer_printf(buffer, "[");
            for (i = 0; i < deref->path_len; ++i)
            {
                vkd3d_string_buffer_printf(buffer, "[");
                dump_src(buffer, &deref->path[i]);
                vkd3d_string_buffer_printf(buffer, "]");
            }
            vkd3d_string_buffer_printf(buffer, "]");
        }
        else if (deref->offset.node)
        {
            vkd3d_string_buffer_printf(buffer, "[");
            dump_src(buffer, &deref->offset);
            vkd3d_string_buffer_printf(buffer, "]");
        }
    }
    else
    {
        vkd3d_string_buffer_printf(buffer, "(nil)");
    }
}

const char *debug_hlsl_writemask(unsigned int writemask)
{
    static const char components[] = {'x', 'y', 'z', 'w'};
    char string[5];
    unsigned int i = 0, pos = 0;

    assert(!(writemask & ~VKD3DSP_WRITEMASK_ALL));

    while (writemask)
    {
        if (writemask & 1)
            string[pos++] = components[i];
        writemask >>= 1;
        i++;
    }
    string[pos] = '\0';
    return vkd3d_dbg_sprintf(".%s", string);
}

const char *debug_hlsl_swizzle(unsigned int swizzle, unsigned int size)
{
    static const char components[] = {'x', 'y', 'z', 'w'};
    char string[5];
    unsigned int i;

    assert(size <= ARRAY_SIZE(components));
    for (i = 0; i < size; ++i)
        string[i] = components[hlsl_swizzle_get_component(swizzle, i)];
    string[size] = 0;
    return vkd3d_dbg_sprintf(".%s", string);
}

static void dump_ir_call(struct hlsl_ctx *ctx, struct vkd3d_string_buffer *buffer, const struct hlsl_ir_call *call)
{
    const struct hlsl_ir_function_decl *decl = call->decl;
    struct vkd3d_string_buffer *string;
    size_t i;

    if (!(string = hlsl_type_to_string(ctx, decl->return_type)))
        return;

    vkd3d_string_buffer_printf(buffer, "call %s %s(", string->buffer, decl->func->name);
    hlsl_release_string_buffer(ctx, string);

    for (i = 0; i < decl->parameters.count; ++i)
    {
        const struct hlsl_ir_var *param = decl->parameters.vars[i];

        if (!(string = hlsl_type_to_string(ctx, param->data_type)))
            return;

        if (i)
            vkd3d_string_buffer_printf(buffer, ", ");
        vkd3d_string_buffer_printf(buffer, "%s", string->buffer);

        hlsl_release_string_buffer(ctx, string);
    }
    vkd3d_string_buffer_printf(buffer, ")");
}

static void dump_ir_constant(struct vkd3d_string_buffer *buffer, const struct hlsl_ir_constant *constant)
{
    struct hlsl_type *type = constant->node.data_type;
    unsigned int x;

    if (type->dimx != 1)
        vkd3d_string_buffer_printf(buffer, "{");
    for (x = 0; x < type->dimx; ++x)
    {
        const union hlsl_constant_value *value = &constant->value[x];

        switch (type->base_type)
        {
            case HLSL_TYPE_BOOL:
                vkd3d_string_buffer_printf(buffer, "%s ", value->u ? "true" : "false");
                break;

            case HLSL_TYPE_DOUBLE:
                vkd3d_string_buffer_printf(buffer, "%.16e ", value->d);
                break;

            case HLSL_TYPE_FLOAT:
            case HLSL_TYPE_HALF:
                vkd3d_string_buffer_printf(buffer, "%.8e ", value->f);
                break;

            case HLSL_TYPE_INT:
                vkd3d_string_buffer_printf(buffer, "%d ", value->i);
                break;

            case HLSL_TYPE_UINT:
                vkd3d_string_buffer_printf(buffer, "%u ", value->u);
                break;

            default:
                vkd3d_unreachable();
        }
    }
    if (type->dimx != 1)
        vkd3d_string_buffer_printf(buffer, "}");
}

const char *debug_hlsl_expr_op(enum hlsl_ir_expr_op op)
{
    static const char *const op_names[] =
    {
        [HLSL_OP0_VOID]         = "void",

        [HLSL_OP1_ABS]          = "abs",
        [HLSL_OP1_BIT_NOT]      = "~",
        [HLSL_OP1_CAST]         = "cast",
        [HLSL_OP1_COS]          = "cos",
        [HLSL_OP1_COS_REDUCED]  = "cos_reduced",
        [HLSL_OP1_DSX]          = "dsx",
        [HLSL_OP1_DSY]          = "dsy",
        [HLSL_OP1_EXP2]         = "exp2",
        [HLSL_OP1_FRACT]        = "fract",
        [HLSL_OP1_LOG2]         = "log2",
        [HLSL_OP1_LOGIC_NOT]    = "!",
        [HLSL_OP1_NEG]          = "-",
        [HLSL_OP1_NRM]          = "nrm",
        [HLSL_OP1_RCP]          = "rcp",
        [HLSL_OP1_REINTERPRET]  = "reinterpret",
        [HLSL_OP1_ROUND]        = "round",
        [HLSL_OP1_RSQ]          = "rsq",
        [HLSL_OP1_SAT]          = "sat",
        [HLSL_OP1_SIGN]         = "sign",
        [HLSL_OP1_SIN]          = "sin",
        [HLSL_OP1_SIN_REDUCED]  = "sin_reduced",
        [HLSL_OP1_SQRT]         = "sqrt",

        [HLSL_OP2_ADD]         = "+",
        [HLSL_OP2_BIT_AND]     = "&",
        [HLSL_OP2_BIT_OR]      = "|",
        [HLSL_OP2_BIT_XOR]     = "^",
        [HLSL_OP2_CRS]         = "crs",
        [HLSL_OP2_DIV]         = "/",
        [HLSL_OP2_DOT]         = "dot",
        [HLSL_OP2_EQUAL]       = "==",
        [HLSL_OP2_GEQUAL]      = ">=",
        [HLSL_OP2_LESS]        = "<",
        [HLSL_OP2_LOGIC_AND]   = "&&",
        [HLSL_OP2_LOGIC_OR]    = "||",
        [HLSL_OP2_LSHIFT]      = "<<",
        [HLSL_OP2_MAX]         = "max",
        [HLSL_OP2_MIN]         = "min",
        [HLSL_OP2_MOD]         = "%",
        [HLSL_OP2_MUL]         = "*",
        [HLSL_OP2_NEQUAL]      = "!=",
        [HLSL_OP2_RSHIFT]      = ">>",

        [HLSL_OP3_DP2ADD]      = "dp2add",
        [HLSL_OP3_LERP]        = "lerp",
    };

    return op_names[op];
}

static void dump_ir_expr(struct vkd3d_string_buffer *buffer, const struct hlsl_ir_expr *expr)
{
    unsigned int i;

    vkd3d_string_buffer_printf(buffer, "%s (", debug_hlsl_expr_op(expr->op));
    for (i = 0; i < HLSL_MAX_OPERANDS && expr->operands[i].node; ++i)
    {
        dump_src(buffer, &expr->operands[i]);
        vkd3d_string_buffer_printf(buffer, " ");
    }
    vkd3d_string_buffer_printf(buffer, ")");
}

static void dump_ir_if(struct hlsl_ctx *ctx, struct vkd3d_string_buffer *buffer, const struct hlsl_ir_if *if_node)
{
    vkd3d_string_buffer_printf(buffer, "if (");
    dump_src(buffer, &if_node->condition);
    vkd3d_string_buffer_printf(buffer, ") {\n");
    dump_instr_list(ctx, buffer, &if_node->then_instrs.instrs);
    vkd3d_string_buffer_printf(buffer, "      %10s   } else {\n", "");
    dump_instr_list(ctx, buffer, &if_node->else_instrs.instrs);
    vkd3d_string_buffer_printf(buffer, "      %10s   }", "");
}

static void dump_ir_jump(struct vkd3d_string_buffer *buffer, const struct hlsl_ir_jump *jump)
{
    switch (jump->type)
    {
        case HLSL_IR_JUMP_BREAK:
            vkd3d_string_buffer_printf(buffer, "break");
            break;

        case HLSL_IR_JUMP_CONTINUE:
            vkd3d_string_buffer_printf(buffer, "continue");
            break;

        case HLSL_IR_JUMP_DISCARD:
            vkd3d_string_buffer_printf(buffer, "discard");
            break;

        case HLSL_IR_JUMP_RETURN:
            vkd3d_string_buffer_printf(buffer, "return");
            break;
    }
}

static void dump_ir_loop(struct hlsl_ctx *ctx, struct vkd3d_string_buffer *buffer, const struct hlsl_ir_loop *loop)
{
    vkd3d_string_buffer_printf(buffer, "for (;;) {\n");
    dump_instr_list(ctx, buffer, &loop->body.instrs);
    vkd3d_string_buffer_printf(buffer, "      %10s   }", "");
}

static void dump_ir_resource_load(struct vkd3d_string_buffer *buffer, const struct hlsl_ir_resource_load *load)
{
    static const char *const type_names[] =
    {
        [HLSL_RESOURCE_LOAD] = "load_resource",
        [HLSL_RESOURCE_SAMPLE] = "sample",
        [HLSL_RESOURCE_SAMPLE_LOD] = "sample_lod",
        [HLSL_RESOURCE_GATHER_RED] = "gather_red",
        [HLSL_RESOURCE_GATHER_GREEN] = "gather_green",
        [HLSL_RESOURCE_GATHER_BLUE] = "gather_blue",
        [HLSL_RESOURCE_GATHER_ALPHA] = "gather_alpha",
    };

    assert(load->load_type < ARRAY_SIZE(type_names));
    vkd3d_string_buffer_printf(buffer, "%s(resource = ", type_names[load->load_type]);
    dump_deref(buffer, &load->resource);
    vkd3d_string_buffer_printf(buffer, ", sampler = ");
    dump_deref(buffer, &load->sampler);
    vkd3d_string_buffer_printf(buffer, ", coords = ");
    dump_src(buffer, &load->coords);
    if (load->texel_offset.node)
    {
        vkd3d_string_buffer_printf(buffer, ", offset = ");
        dump_src(buffer, &load->texel_offset);
    }
    if (load->lod.node)
    {
        vkd3d_string_buffer_printf(buffer, ", lod = ");
        dump_src(buffer, &load->lod);
    }
    vkd3d_string_buffer_printf(buffer, ")");
}

static void dump_ir_resource_store(struct vkd3d_string_buffer *buffer, const struct hlsl_ir_resource_store *store)
{
    vkd3d_string_buffer_printf(buffer, "store_resource(resource = ");
    dump_deref(buffer, &store->resource);
    vkd3d_string_buffer_printf(buffer, ", coords = ");
    dump_src(buffer, &store->coords);
    vkd3d_string_buffer_printf(buffer, ", value = ");
    dump_src(buffer, &store->value);
    vkd3d_string_buffer_printf(buffer, ")");
}

static void dump_ir_store(struct vkd3d_string_buffer *buffer, const struct hlsl_ir_store *store)
{
    vkd3d_string_buffer_printf(buffer, "= (");
    dump_deref(buffer, &store->lhs);
    if (store->writemask != VKD3DSP_WRITEMASK_ALL)
        vkd3d_string_buffer_printf(buffer, "%s", debug_hlsl_writemask(store->writemask));
    vkd3d_string_buffer_printf(buffer, " ");
    dump_src(buffer, &store->rhs);
    vkd3d_string_buffer_printf(buffer, ")");
}

static void dump_ir_swizzle(struct vkd3d_string_buffer *buffer, const struct hlsl_ir_swizzle *swizzle)
{
    unsigned int i;

    dump_src(buffer, &swizzle->val);
    if (swizzle->val.node->data_type->dimy > 1)
    {
        vkd3d_string_buffer_printf(buffer, ".");
        for (i = 0; i < swizzle->node.data_type->dimx; ++i)
            vkd3d_string_buffer_printf(buffer, "_m%u%u", (swizzle->swizzle >> i * 8) & 0xf, (swizzle->swizzle >> (i * 8 + 4)) & 0xf);
    }
    else
    {
        vkd3d_string_buffer_printf(buffer, "%s", debug_hlsl_swizzle(swizzle->swizzle, swizzle->node.data_type->dimx));
    }
}

static void dump_instr(struct hlsl_ctx *ctx, struct vkd3d_string_buffer *buffer, const struct hlsl_ir_node *instr)
{
    if (instr->index)
        vkd3d_string_buffer_printf(buffer, "%4u: ", instr->index);
    else
        vkd3d_string_buffer_printf(buffer, "%p: ", instr);

    vkd3d_string_buffer_printf(buffer, "%10s | ", instr->data_type ? debug_hlsl_type(ctx, instr->data_type) : "");

    switch (instr->type)
    {
        case HLSL_IR_CALL:
            dump_ir_call(ctx, buffer, hlsl_ir_call(instr));
            break;

        case HLSL_IR_CONSTANT:
            dump_ir_constant(buffer, hlsl_ir_constant(instr));
            break;

        case HLSL_IR_EXPR:
            dump_ir_expr(buffer, hlsl_ir_expr(instr));
            break;

        case HLSL_IR_IF:
            dump_ir_if(ctx, buffer, hlsl_ir_if(instr));
            break;

        case HLSL_IR_JUMP:
            dump_ir_jump(buffer, hlsl_ir_jump(instr));
            break;

        case HLSL_IR_LOAD:
            dump_deref(buffer, &hlsl_ir_load(instr)->src);
            break;

        case HLSL_IR_LOOP:
            dump_ir_loop(ctx, buffer, hlsl_ir_loop(instr));
            break;

        case HLSL_IR_RESOURCE_LOAD:
            dump_ir_resource_load(buffer, hlsl_ir_resource_load(instr));
            break;

        case HLSL_IR_RESOURCE_STORE:
            dump_ir_resource_store(buffer, hlsl_ir_resource_store(instr));
            break;

        case HLSL_IR_STORE:
            dump_ir_store(buffer, hlsl_ir_store(instr));
            break;

        case HLSL_IR_SWIZZLE:
            dump_ir_swizzle(buffer, hlsl_ir_swizzle(instr));
            break;
    }
}

void hlsl_dump_function(struct hlsl_ctx *ctx, const struct hlsl_ir_function_decl *func)
{
    struct vkd3d_string_buffer buffer;
    size_t i;

    vkd3d_string_buffer_init(&buffer);
    vkd3d_string_buffer_printf(&buffer, "Dumping function %s.\n", func->func->name);
    vkd3d_string_buffer_printf(&buffer, "Function parameters:\n");
    for (i = 0; i < func->parameters.count; ++i)
    {
        dump_ir_var(ctx, &buffer, func->parameters.vars[i]);
        vkd3d_string_buffer_printf(&buffer, "\n");
    }
    if (func->has_body)
        dump_instr_list(ctx, &buffer, &func->body.instrs);

    vkd3d_string_buffer_trace(&buffer);
    vkd3d_string_buffer_cleanup(&buffer);
}

void hlsl_replace_node(struct hlsl_ir_node *old, struct hlsl_ir_node *new)
{
    struct hlsl_src *src, *next;

    assert(old->data_type->dimx == new->data_type->dimx);
    assert(old->data_type->dimy == new->data_type->dimy);

    LIST_FOR_EACH_ENTRY_SAFE(src, next, &old->uses, struct hlsl_src, entry)
    {
        hlsl_src_remove(src);
        hlsl_src_from_node(src, new);
    }
    list_remove(&old->entry);
    hlsl_free_instr(old);
}


void hlsl_free_type(struct hlsl_type *type)
{
    struct hlsl_struct_field *field;
    size_t i;

    vkd3d_free((void *)type->name);
    if (type->type == HLSL_CLASS_STRUCT)
    {
        for (i = 0; i < type->e.record.field_count; ++i)
        {
            field = &type->e.record.fields[i];

            vkd3d_free((void *)field->name);
            hlsl_cleanup_semantic(&field->semantic);
        }
        vkd3d_free((void *)type->e.record.fields);
    }
    vkd3d_free(type);
}

void hlsl_free_instr_list(struct list *list)
{
    struct hlsl_ir_node *node, *next_node;

    if (!list)
        return;
    /* Iterate in reverse, to avoid use-after-free when unlinking sources from
     * the "uses" list. */
    LIST_FOR_EACH_ENTRY_SAFE_REV(node, next_node, list, struct hlsl_ir_node, entry)
        hlsl_free_instr(node);
}

static void free_ir_call(struct hlsl_ir_call *call)
{
    vkd3d_free(call);
}

static void free_ir_constant(struct hlsl_ir_constant *constant)
{
    vkd3d_free(constant);
}

static void free_ir_expr(struct hlsl_ir_expr *expr)
{
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(expr->operands); ++i)
        hlsl_src_remove(&expr->operands[i]);
    vkd3d_free(expr);
}

static void free_ir_if(struct hlsl_ir_if *if_node)
{
    hlsl_free_instr_list(&if_node->then_instrs.instrs);
    hlsl_free_instr_list(&if_node->else_instrs.instrs);
    hlsl_src_remove(&if_node->condition);
    vkd3d_free(if_node);
}

static void free_ir_jump(struct hlsl_ir_jump *jump)
{
    vkd3d_free(jump);
}

static void free_ir_load(struct hlsl_ir_load *load)
{
    hlsl_cleanup_deref(&load->src);
    vkd3d_free(load);
}

static void free_ir_loop(struct hlsl_ir_loop *loop)
{
    hlsl_free_instr_list(&loop->body.instrs);
    vkd3d_free(loop);
}

static void free_ir_resource_load(struct hlsl_ir_resource_load *load)
{
    hlsl_cleanup_deref(&load->sampler);
    hlsl_cleanup_deref(&load->resource);
    hlsl_src_remove(&load->coords);
    hlsl_src_remove(&load->lod);
    hlsl_src_remove(&load->texel_offset);
    vkd3d_free(load);
}

static void free_ir_resource_store(struct hlsl_ir_resource_store *store)
{
    hlsl_src_remove(&store->resource.offset);
    hlsl_src_remove(&store->coords);
    hlsl_src_remove(&store->value);
    vkd3d_free(store);
}

static void free_ir_store(struct hlsl_ir_store *store)
{
    hlsl_src_remove(&store->rhs);
    hlsl_cleanup_deref(&store->lhs);
    vkd3d_free(store);
}

static void free_ir_swizzle(struct hlsl_ir_swizzle *swizzle)
{
    hlsl_src_remove(&swizzle->val);
    vkd3d_free(swizzle);
}

void hlsl_free_instr(struct hlsl_ir_node *node)
{
    assert(list_empty(&node->uses));

    switch (node->type)
    {
        case HLSL_IR_CALL:
            free_ir_call(hlsl_ir_call(node));
            break;

        case HLSL_IR_CONSTANT:
            free_ir_constant(hlsl_ir_constant(node));
            break;

        case HLSL_IR_EXPR:
            free_ir_expr(hlsl_ir_expr(node));
            break;

        case HLSL_IR_IF:
            free_ir_if(hlsl_ir_if(node));
            break;

        case HLSL_IR_JUMP:
            free_ir_jump(hlsl_ir_jump(node));
            break;

        case HLSL_IR_LOAD:
            free_ir_load(hlsl_ir_load(node));
            break;

        case HLSL_IR_LOOP:
            free_ir_loop(hlsl_ir_loop(node));
            break;

        case HLSL_IR_RESOURCE_LOAD:
            free_ir_resource_load(hlsl_ir_resource_load(node));
            break;

        case HLSL_IR_RESOURCE_STORE:
            free_ir_resource_store(hlsl_ir_resource_store(node));
            break;

        case HLSL_IR_STORE:
            free_ir_store(hlsl_ir_store(node));
            break;

        case HLSL_IR_SWIZZLE:
            free_ir_swizzle(hlsl_ir_swizzle(node));
            break;
    }
}

void hlsl_free_attribute(struct hlsl_attribute *attr)
{
    unsigned int i;

    for (i = 0; i < attr->args_count; ++i)
        hlsl_src_remove(&attr->args[i]);
    hlsl_free_instr_list(&attr->instrs);
    vkd3d_free((void *)attr->name);
    vkd3d_free(attr);
}

void hlsl_cleanup_semantic(struct hlsl_semantic *semantic)
{
    vkd3d_free((void *)semantic->name);
    memset(semantic, 0, sizeof(*semantic));
}

static void free_function_decl(struct hlsl_ir_function_decl *decl)
{
    unsigned int i;

    for (i = 0; i < decl->attr_count; ++i)
        hlsl_free_attribute((void *)decl->attrs[i]);
    vkd3d_free((void *)decl->attrs);

    vkd3d_free(decl->parameters.vars);
    hlsl_free_instr_list(&decl->body.instrs);
    vkd3d_free(decl);
}

static void free_function_decl_rb(struct rb_entry *entry, void *context)
{
    free_function_decl(RB_ENTRY_VALUE(entry, struct hlsl_ir_function_decl, entry));
}

static void free_function(struct hlsl_ir_function *func)
{
    rb_destroy(&func->overloads, free_function_decl_rb, NULL);
    vkd3d_free((void *)func->name);
    vkd3d_free(func);
}

static void free_function_rb(struct rb_entry *entry, void *context)
{
    free_function(RB_ENTRY_VALUE(entry, struct hlsl_ir_function, entry));
}

void hlsl_add_function(struct hlsl_ctx *ctx, char *name, struct hlsl_ir_function_decl *decl)
{
    struct hlsl_ir_function *func;
    struct rb_entry *func_entry;

    func_entry = rb_get(&ctx->functions, name);
    if (func_entry)
    {
        func = RB_ENTRY_VALUE(func_entry, struct hlsl_ir_function, entry);
        decl->func = func;

        if (rb_put(&func->overloads, &decl->parameters, &decl->entry) == -1)
            ERR("Failed to insert function overload.\n");
        vkd3d_free(name);
        return;
    }
    func = hlsl_alloc(ctx, sizeof(*func));
    func->name = name;
    rb_init(&func->overloads, compare_function_decl_rb);
    decl->func = func;
    rb_put(&func->overloads, &decl->parameters, &decl->entry);
    rb_put(&ctx->functions, func->name, &func->entry);
}

unsigned int hlsl_map_swizzle(unsigned int swizzle, unsigned int writemask)
{
    unsigned int i, ret = 0;

    /* Leave replicate swizzles alone; some instructions need them. */
    if (swizzle == HLSL_SWIZZLE(X, X, X, X)
            || swizzle == HLSL_SWIZZLE(Y, Y, Y, Y)
            || swizzle == HLSL_SWIZZLE(Z, Z, Z, Z)
            || swizzle == HLSL_SWIZZLE(W, W, W, W))
        return swizzle;

    for (i = 0; i < 4; ++i)
    {
        if (writemask & (1 << i))
        {
            ret |= (swizzle & 3) << (i * 2);
            swizzle >>= 2;
        }
    }
    return ret;
}

unsigned int hlsl_swizzle_from_writemask(unsigned int writemask)
{
    static const unsigned int swizzles[16] =
    {
        0,
        HLSL_SWIZZLE(X, X, X, X),
        HLSL_SWIZZLE(Y, Y, Y, Y),
        HLSL_SWIZZLE(X, Y, X, X),
        HLSL_SWIZZLE(Z, Z, Z, Z),
        HLSL_SWIZZLE(X, Z, X, X),
        HLSL_SWIZZLE(Y, Z, X, X),
        HLSL_SWIZZLE(X, Y, Z, X),
        HLSL_SWIZZLE(W, W, W, W),
        HLSL_SWIZZLE(X, W, X, X),
        HLSL_SWIZZLE(Y, W, X, X),
        HLSL_SWIZZLE(X, Y, W, X),
        HLSL_SWIZZLE(Z, W, X, X),
        HLSL_SWIZZLE(X, Z, W, X),
        HLSL_SWIZZLE(Y, Z, W, X),
        HLSL_SWIZZLE(X, Y, Z, W),
    };

    return swizzles[writemask & 0xf];
}

unsigned int hlsl_combine_writemasks(unsigned int first, unsigned int second)
{
    unsigned int ret = 0, i, j = 0;

    for (i = 0; i < 4; ++i)
    {
        if (first & (1 << i))
        {
            if (second & (1 << j++))
                ret |= (1 << i);
        }
    }

    return ret;
}

unsigned int hlsl_combine_swizzles(unsigned int first, unsigned int second, unsigned int dim)
{
    unsigned int ret = 0, i;
    for (i = 0; i < dim; ++i)
    {
        unsigned int s = hlsl_swizzle_get_component(second, i);
        ret |= hlsl_swizzle_get_component(first, s) << HLSL_SWIZZLE_SHIFT(i);
    }
    return ret;
}

static const struct hlsl_profile_info *get_target_info(const char *target)
{
    unsigned int i;

    static const struct hlsl_profile_info profiles[] =
    {
        {"cs_4_0",              VKD3D_SHADER_TYPE_COMPUTE,  4, 0, 0, 0, false},
        {"cs_4_1",              VKD3D_SHADER_TYPE_COMPUTE,  4, 1, 0, 0, false},
        {"cs_5_0",              VKD3D_SHADER_TYPE_COMPUTE,  5, 0, 0, 0, false},
        {"ds_5_0",              VKD3D_SHADER_TYPE_DOMAIN,   5, 0, 0, 0, false},
        {"fx_2_0",              VKD3D_SHADER_TYPE_EFFECT,   2, 0, 0, 0, false},
        {"fx_4_0",              VKD3D_SHADER_TYPE_EFFECT,   4, 0, 0, 0, false},
        {"fx_4_1",              VKD3D_SHADER_TYPE_EFFECT,   4, 1, 0, 0, false},
        {"fx_5_0",              VKD3D_SHADER_TYPE_EFFECT,   5, 0, 0, 0, false},
        {"gs_4_0",              VKD3D_SHADER_TYPE_GEOMETRY, 4, 0, 0, 0, false},
        {"gs_4_1",              VKD3D_SHADER_TYPE_GEOMETRY, 4, 1, 0, 0, false},
        {"gs_5_0",              VKD3D_SHADER_TYPE_GEOMETRY, 5, 0, 0, 0, false},
        {"hs_5_0",              VKD3D_SHADER_TYPE_HULL,     5, 0, 0, 0, false},
        {"ps.1.0",              VKD3D_SHADER_TYPE_PIXEL,    1, 0, 0, 0, false},
        {"ps.1.1",              VKD3D_SHADER_TYPE_PIXEL,    1, 1, 0, 0, false},
        {"ps.1.2",              VKD3D_SHADER_TYPE_PIXEL,    1, 2, 0, 0, false},
        {"ps.1.3",              VKD3D_SHADER_TYPE_PIXEL,    1, 3, 0, 0, false},
        {"ps.1.4",              VKD3D_SHADER_TYPE_PIXEL,    1, 4, 0, 0, false},
        {"ps.2.0",              VKD3D_SHADER_TYPE_PIXEL,    2, 0, 0, 0, false},
        {"ps.2.a",              VKD3D_SHADER_TYPE_PIXEL,    2, 1, 0, 0, false},
        {"ps.2.b",              VKD3D_SHADER_TYPE_PIXEL,    2, 2, 0, 0, false},
        {"ps.2.sw",             VKD3D_SHADER_TYPE_PIXEL,    2, 0, 0, 0, true},
        {"ps.3.0",              VKD3D_SHADER_TYPE_PIXEL,    3, 0, 0, 0, false},
        {"ps_1_0",              VKD3D_SHADER_TYPE_PIXEL,    1, 0, 0, 0, false},
        {"ps_1_1",              VKD3D_SHADER_TYPE_PIXEL,    1, 1, 0, 0, false},
        {"ps_1_2",              VKD3D_SHADER_TYPE_PIXEL,    1, 2, 0, 0, false},
        {"ps_1_3",              VKD3D_SHADER_TYPE_PIXEL,    1, 3, 0, 0, false},
        {"ps_1_4",              VKD3D_SHADER_TYPE_PIXEL,    1, 4, 0, 0, false},
        {"ps_2_0",              VKD3D_SHADER_TYPE_PIXEL,    2, 0, 0, 0, false},
        {"ps_2_a",              VKD3D_SHADER_TYPE_PIXEL,    2, 1, 0, 0, false},
        {"ps_2_b",              VKD3D_SHADER_TYPE_PIXEL,    2, 2, 0, 0, false},
        {"ps_2_sw",             VKD3D_SHADER_TYPE_PIXEL,    2, 0, 0, 0, true},
        {"ps_3_0",              VKD3D_SHADER_TYPE_PIXEL,    3, 0, 0, 0, false},
        {"ps_3_sw",             VKD3D_SHADER_TYPE_PIXEL,    3, 0, 0, 0, true},
        {"ps_4_0",              VKD3D_SHADER_TYPE_PIXEL,    4, 0, 0, 0, false},
        {"ps_4_0_level_9_0",    VKD3D_SHADER_TYPE_PIXEL,    4, 0, 9, 0, false},
        {"ps_4_0_level_9_1",    VKD3D_SHADER_TYPE_PIXEL,    4, 0, 9, 1, false},
        {"ps_4_0_level_9_3",    VKD3D_SHADER_TYPE_PIXEL,    4, 0, 9, 3, false},
        {"ps_4_1",              VKD3D_SHADER_TYPE_PIXEL,    4, 1, 0, 0, false},
        {"ps_5_0",              VKD3D_SHADER_TYPE_PIXEL,    5, 0, 0, 0, false},
        {"tx_1_0",              VKD3D_SHADER_TYPE_TEXTURE,  1, 0, 0, 0, false},
        {"vs.1.0",              VKD3D_SHADER_TYPE_VERTEX,   1, 0, 0, 0, false},
        {"vs.1.1",              VKD3D_SHADER_TYPE_VERTEX,   1, 1, 0, 0, false},
        {"vs.2.0",              VKD3D_SHADER_TYPE_VERTEX,   2, 0, 0, 0, false},
        {"vs.2.a",              VKD3D_SHADER_TYPE_VERTEX,   2, 1, 0, 0, false},
        {"vs.2.sw",             VKD3D_SHADER_TYPE_VERTEX,   2, 0, 0, 0, true},
        {"vs.3.0",              VKD3D_SHADER_TYPE_VERTEX,   3, 0, 0, 0, false},
        {"vs.3.sw",             VKD3D_SHADER_TYPE_VERTEX,   3, 0, 0, 0, true},
        {"vs_1_0",              VKD3D_SHADER_TYPE_VERTEX,   1, 0, 0, 0, false},
        {"vs_1_1",              VKD3D_SHADER_TYPE_VERTEX,   1, 1, 0, 0, false},
        {"vs_2_0",              VKD3D_SHADER_TYPE_VERTEX,   2, 0, 0, 0, false},
        {"vs_2_a",              VKD3D_SHADER_TYPE_VERTEX,   2, 1, 0, 0, false},
        {"vs_2_sw",             VKD3D_SHADER_TYPE_VERTEX,   2, 0, 0, 0, true},
        {"vs_3_0",              VKD3D_SHADER_TYPE_VERTEX,   3, 0, 0, 0, false},
        {"vs_3_sw",             VKD3D_SHADER_TYPE_VERTEX,   3, 0, 0, 0, true},
        {"vs_4_0",              VKD3D_SHADER_TYPE_VERTEX,   4, 0, 0, 0, false},
        {"vs_4_0_level_9_0",    VKD3D_SHADER_TYPE_VERTEX,   4, 0, 9, 0, false},
        {"vs_4_0_level_9_1",    VKD3D_SHADER_TYPE_VERTEX,   4, 0, 9, 1, false},
        {"vs_4_0_level_9_3",    VKD3D_SHADER_TYPE_VERTEX,   4, 0, 9, 3, false},
        {"vs_4_1",              VKD3D_SHADER_TYPE_VERTEX,   4, 1, 0, 0, false},
        {"vs_5_0",              VKD3D_SHADER_TYPE_VERTEX,   5, 0, 0, 0, false},
    };

    for (i = 0; i < ARRAY_SIZE(profiles); ++i)
    {
        if (!strcmp(target, profiles[i].name))
            return &profiles[i];
    }

    return NULL;
}

static int compare_function_rb(const void *key, const struct rb_entry *entry)
{
    const char *name = key;
    const struct hlsl_ir_function *func = RB_ENTRY_VALUE(entry, const struct hlsl_ir_function,entry);

    return strcmp(name, func->name);
}

static void declare_predefined_types(struct hlsl_ctx *ctx)
{
    unsigned int x, y, bt, i, v;
    struct hlsl_type *type;

    static const char * const names[] =
    {
        "float",
        "half",
        "double",
        "int",
        "uint",
        "bool",
    };
    char name[15];

    static const char *const variants_float[] = {"min10float", "min16float"};
    static const char *const variants_int[] = {"min12int", "min16int"};
    static const char *const variants_uint[] = {"min16uint"};

    static const char *const sampler_names[] =
    {
        [HLSL_SAMPLER_DIM_GENERIC] = "sampler",
        [HLSL_SAMPLER_DIM_1D]      = "sampler1D",
        [HLSL_SAMPLER_DIM_2D]      = "sampler2D",
        [HLSL_SAMPLER_DIM_3D]      = "sampler3D",
        [HLSL_SAMPLER_DIM_CUBE]    = "samplerCUBE",
    };

    static const struct
    {
        char name[13];
        enum hlsl_type_class class;
        enum hlsl_base_type base_type;
        unsigned int dimx, dimy;
    }
    effect_types[] =
    {
        {"dword",           HLSL_CLASS_SCALAR, HLSL_TYPE_UINT,          1, 1},
        {"float",           HLSL_CLASS_SCALAR, HLSL_TYPE_FLOAT,         1, 1},
        {"VECTOR",          HLSL_CLASS_VECTOR, HLSL_TYPE_FLOAT,         4, 1},
        {"MATRIX",          HLSL_CLASS_MATRIX, HLSL_TYPE_FLOAT,         4, 4},
        {"STRING",          HLSL_CLASS_OBJECT, HLSL_TYPE_STRING,        1, 1},
        {"TEXTURE",         HLSL_CLASS_OBJECT, HLSL_TYPE_TEXTURE,       1, 1},
        {"PIXELSHADER",     HLSL_CLASS_OBJECT, HLSL_TYPE_PIXELSHADER,   1, 1},
        {"VERTEXSHADER",    HLSL_CLASS_OBJECT, HLSL_TYPE_VERTEXSHADER,  1, 1},
    };

    for (bt = 0; bt <= HLSL_TYPE_LAST_SCALAR; ++bt)
    {
        for (y = 1; y <= 4; ++y)
        {
            for (x = 1; x <= 4; ++x)
            {
                sprintf(name, "%s%ux%u", names[bt], y, x);
                type = hlsl_new_type(ctx, name, HLSL_CLASS_MATRIX, bt, x, y);
                hlsl_scope_add_type(ctx->globals, type);
                ctx->builtin_types.matrix[bt][x - 1][y - 1] = type;

                if (y == 1)
                {
                    sprintf(name, "%s%u", names[bt], x);
                    type = hlsl_new_type(ctx, name, HLSL_CLASS_VECTOR, bt, x, y);
                    hlsl_scope_add_type(ctx->globals, type);
                    ctx->builtin_types.vector[bt][x - 1] = type;

                    if (x == 1)
                    {
                        sprintf(name, "%s", names[bt]);
                        type = hlsl_new_type(ctx, name, HLSL_CLASS_SCALAR, bt, x, y);
                        hlsl_scope_add_type(ctx->globals, type);
                        ctx->builtin_types.scalar[bt] = type;
                    }
                }
            }
        }
    }

    for (bt = 0; bt <= HLSL_TYPE_LAST_SCALAR; ++bt)
    {
        unsigned int n_variants = 0;
        const char *const *variants;

        switch (bt)
        {
            case HLSL_TYPE_FLOAT:
                variants = variants_float;
                n_variants = ARRAY_SIZE(variants_float);
                break;

            case HLSL_TYPE_INT:
                variants = variants_int;
                n_variants = ARRAY_SIZE(variants_int);
                break;

            case HLSL_TYPE_UINT:
                variants = variants_uint;
                n_variants = ARRAY_SIZE(variants_uint);
                break;

            default:
                break;
        }

        for (v = 0; v < n_variants; ++v)
        {
            for (y = 1; y <= 4; ++y)
            {
                for (x = 1; x <= 4; ++x)
                {
                    sprintf(name, "%s%ux%u", variants[v], y, x);
                    type = hlsl_new_type(ctx, name, HLSL_CLASS_MATRIX, bt, x, y);
                    type->is_minimum_precision = 1;
                    hlsl_scope_add_type(ctx->globals, type);

                    if (y == 1)
                    {
                        sprintf(name, "%s%u", variants[v], x);
                        type = hlsl_new_type(ctx, name, HLSL_CLASS_VECTOR, bt, x, y);
                        type->is_minimum_precision = 1;
                        hlsl_scope_add_type(ctx->globals, type);

                        if (x == 1)
                        {
                            sprintf(name, "%s", variants[v]);
                            type = hlsl_new_type(ctx, name, HLSL_CLASS_SCALAR, bt, x, y);
                            type->is_minimum_precision = 1;
                            hlsl_scope_add_type(ctx->globals, type);
                        }
                    }
                }
            }
        }
    }

    for (bt = 0; bt <= HLSL_SAMPLER_DIM_LAST_SAMPLER; ++bt)
    {
        type = hlsl_new_type(ctx, sampler_names[bt], HLSL_CLASS_OBJECT, HLSL_TYPE_SAMPLER, 1, 1);
        type->sampler_dim = bt;
        ctx->builtin_types.sampler[bt] = type;
    }

    ctx->builtin_types.Void = hlsl_new_type(ctx, "void", HLSL_CLASS_OBJECT, HLSL_TYPE_VOID, 1, 1);

    for (i = 0; i < ARRAY_SIZE(effect_types); ++i)
    {
        type = hlsl_new_type(ctx, effect_types[i].name, effect_types[i].class,
                effect_types[i].base_type, effect_types[i].dimx, effect_types[i].dimy);
        hlsl_scope_add_type(ctx->globals, type);
    }
}

static bool hlsl_ctx_init(struct hlsl_ctx *ctx, const char *source_name,
        const struct hlsl_profile_info *profile, struct vkd3d_shader_message_context *message_context)
{
    memset(ctx, 0, sizeof(*ctx));

    ctx->profile = profile;

    ctx->message_context = message_context;

    if (!(ctx->source_files = hlsl_alloc(ctx, sizeof(*ctx->source_files))))
        return false;
    if (!(ctx->source_files[0] = hlsl_strdup(ctx, source_name ? source_name : "<anonymous>")))
    {
        vkd3d_free(ctx->source_files);
        return false;
    }
    ctx->source_files_count = 1;
    ctx->location.source_name = ctx->source_files[0];
    ctx->location.line = ctx->location.column = 1;
    vkd3d_string_buffer_cache_init(&ctx->string_buffers);

    ctx->matrix_majority = HLSL_MODIFIER_COLUMN_MAJOR;

    list_init(&ctx->scopes);

    if (!(ctx->dummy_scope = hlsl_new_scope(ctx, NULL)))
    {
        vkd3d_free((void *)ctx->source_files[0]);
        vkd3d_free(ctx->source_files);
        return false;
    }
    hlsl_push_scope(ctx);
    ctx->globals = ctx->cur_scope;

    list_init(&ctx->types);
    declare_predefined_types(ctx);

    rb_init(&ctx->functions, compare_function_rb);

    list_init(&ctx->static_initializers);
    list_init(&ctx->extern_vars);

    list_init(&ctx->buffers);

    if (!(ctx->globals_buffer = hlsl_new_buffer(ctx, HLSL_BUFFER_CONSTANT,
            hlsl_strdup(ctx, "$Globals"), NULL, ctx->location)))
        return false;
    if (!(ctx->params_buffer = hlsl_new_buffer(ctx, HLSL_BUFFER_CONSTANT,
            hlsl_strdup(ctx, "$Params"), NULL, ctx->location)))
        return false;
    ctx->cur_buffer = ctx->globals_buffer;

    return true;
}

static void hlsl_ctx_cleanup(struct hlsl_ctx *ctx)
{
    struct hlsl_buffer *buffer, *next_buffer;
    struct hlsl_scope *scope, *next_scope;
    struct hlsl_ir_var *var, *next_var;
    struct hlsl_type *type, *next_type;
    unsigned int i;

    for (i = 0; i < ctx->source_files_count; ++i)
        vkd3d_free((void *)ctx->source_files[i]);
    vkd3d_free(ctx->source_files);
    vkd3d_string_buffer_cache_cleanup(&ctx->string_buffers);

    rb_destroy(&ctx->functions, free_function_rb, NULL);

    LIST_FOR_EACH_ENTRY_SAFE(scope, next_scope, &ctx->scopes, struct hlsl_scope, entry)
    {
        LIST_FOR_EACH_ENTRY_SAFE(var, next_var, &scope->vars, struct hlsl_ir_var, scope_entry)
            hlsl_free_var(var);
        rb_destroy(&scope->types, NULL, NULL);
        vkd3d_free(scope);
    }

    LIST_FOR_EACH_ENTRY_SAFE(type, next_type, &ctx->types, struct hlsl_type, entry)
        hlsl_free_type(type);

    LIST_FOR_EACH_ENTRY_SAFE(buffer, next_buffer, &ctx->buffers, struct hlsl_buffer, entry)
    {
        vkd3d_free((void *)buffer->name);
        vkd3d_free(buffer);
    }
}

int hlsl_compile_shader(const struct vkd3d_shader_code *hlsl, const struct vkd3d_shader_compile_info *compile_info,
        struct vkd3d_shader_code *out, struct vkd3d_shader_message_context *message_context)
{
    const struct vkd3d_shader_hlsl_source_info *hlsl_source_info;
    struct hlsl_ir_function_decl *decl, *entry_func = NULL;
    const struct hlsl_profile_info *profile;
    struct hlsl_ir_function *func;
    const char *entry_point;
    struct hlsl_ctx ctx;
    int ret;

    if (!(hlsl_source_info = vkd3d_find_struct(compile_info->next, HLSL_SOURCE_INFO)))
    {
        ERR("No HLSL source info given.\n");
        return VKD3D_ERROR_INVALID_ARGUMENT;
    }
    entry_point = hlsl_source_info->entry_point ? hlsl_source_info->entry_point : "main";

    if (!(profile = get_target_info(hlsl_source_info->profile)))
    {
        FIXME("Unknown compilation target %s.\n", debugstr_a(hlsl_source_info->profile));
        return VKD3D_ERROR_NOT_IMPLEMENTED;
    }

    vkd3d_shader_dump_shader(compile_info->source_type, profile->type, &compile_info->source);

    if (compile_info->target_type == VKD3D_SHADER_TARGET_D3D_BYTECODE && profile->major_version > 3)
    {
        vkd3d_shader_error(message_context, NULL, VKD3D_SHADER_ERROR_HLSL_INCOMPATIBLE_PROFILE,
                "The '%s' target profile is incompatible with the 'd3dbc' target type.", profile->name);
        return VKD3D_ERROR_INVALID_ARGUMENT;
    }
    else if (compile_info->target_type == VKD3D_SHADER_TARGET_DXBC_TPF && profile->major_version < 4)
    {
        vkd3d_shader_error(message_context, NULL, VKD3D_SHADER_ERROR_HLSL_INCOMPATIBLE_PROFILE,
                "The '%s' target profile is incompatible with the 'dxbc-tpf' target type.", profile->name);
        return VKD3D_ERROR_INVALID_ARGUMENT;
    }

    if (!hlsl_ctx_init(&ctx, compile_info->source_name, profile, message_context))
        return VKD3D_ERROR_OUT_OF_MEMORY;

    if ((ret = hlsl_lexer_compile(&ctx, hlsl)) == 2)
    {
        hlsl_ctx_cleanup(&ctx);
        return VKD3D_ERROR_OUT_OF_MEMORY;
    }

    if (ctx.result)
    {
        hlsl_ctx_cleanup(&ctx);
        return ctx.result;
    }

    /* If parsing failed without an error condition being recorded, we
     * plausibly hit some unimplemented feature. */
    if (ret)
    {
        hlsl_ctx_cleanup(&ctx);
        return VKD3D_ERROR_NOT_IMPLEMENTED;
    }

    if ((func = hlsl_get_function(&ctx, entry_point)))
    {
        RB_FOR_EACH_ENTRY(decl, &func->overloads, struct hlsl_ir_function_decl, entry)
        {
            if (!decl->has_body)
                continue;
            if (entry_func)
            {
                /* Depending on d3dcompiler version, either the first or last is
                 * selected. */
                hlsl_fixme(&ctx, &decl->loc, "Multiple valid entry point definitions.");
            }
            entry_func = decl;
        }
    }

    if (!entry_func)
    {
        const struct vkd3d_shader_location loc = {.source_name = compile_info->source_name};

        hlsl_error(&ctx, &loc, VKD3D_SHADER_ERROR_HLSL_NOT_DEFINED,
                "Entry point \"%s\" is not defined.", entry_point);
        hlsl_ctx_cleanup(&ctx);
        return VKD3D_ERROR_INVALID_SHADER;
    }

    ret = hlsl_emit_bytecode(&ctx, entry_func, compile_info->target_type, out);

    hlsl_ctx_cleanup(&ctx);
    return ret;
}
