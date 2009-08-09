/**
 * MojoShader; generate shader programs from bytecode of compiled
 *  Direct3D shaders.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

// !!! FIXME: most of these _MSC_VER should probably be _WINDOWS?
#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>  // GL headers need this for WINGDIAPI definition.
#endif

#if (defined(__APPLE__) && defined(__MACH__))
#define PLATFORM_MACOSX 1
#endif

#if PLATFORM_MACOSX
#include <Carbon/Carbon.h>
#endif

#define __MOJOSHADER_INTERNAL__ 1
#include "mojoshader_internal.h"

#define GL_GLEXT_LEGACY 1
#include "GL/gl.h"
#include "GL/glext.h"

#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT 0x140B
#endif

#ifndef GL_HALF_FLOAT_OES
#define GL_HALF_FLOAT_OES 0x8D61
#endif


struct MOJOSHADER_glShader
{
    const MOJOSHADER_parseData *parseData;
    GLuint handle;
    uint32 refcount;
};

typedef struct
{
    MOJOSHADER_shaderType shader_type;
    const MOJOSHADER_uniform *uniform;
    GLuint location;
} UniformMap;

typedef struct
{
    const MOJOSHADER_attribute *attribute;
    GLuint location;
} AttributeMap;

struct MOJOSHADER_glProgram
{
    MOJOSHADER_glShader *vertex;
    MOJOSHADER_glShader *fragment;
    GLuint handle;
    uint32 uniform_count;
    UniformMap *uniforms;
    uint32 attribute_count;
    AttributeMap *attributes;
    size_t vs_uniforms_float4_count;
    GLfloat *vs_uniforms_float4;
    size_t vs_uniforms_int4_count;
    GLint *vs_uniforms_int4;
    size_t vs_uniforms_bool_count;
    GLint *vs_uniforms_bool;
    size_t ps_uniforms_float4_count;
    GLfloat *ps_uniforms_float4;
    size_t ps_uniforms_int4_count;
    GLint *ps_uniforms_int4;
    size_t ps_uniforms_bool_count;
    GLint *ps_uniforms_bool;
    uint32 refcount;
    // GLSL uses these...location of uniform arrays.
    GLuint vs_float4_loc;
    GLuint vs_int4_loc;
    GLuint vs_bool_loc;
    GLuint ps_float4_loc;
    GLuint ps_int4_loc;
    GLuint ps_bool_loc;
};

#ifndef WINGDIAPI
#define WINGDIAPI
#endif

// Entry points in base OpenGL that lack function pointer prototypes...
typedef WINGDIAPI void (APIENTRYP PFNGLGETINTEGERVPROC) (GLenum pname, GLint *params);
typedef WINGDIAPI const GLubyte * (APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
typedef WINGDIAPI GLenum (APIENTRYP PFNGLGETERRORPROC) (void);
typedef WINGDIAPI void (APIENTRYP PFNGLENABLEPROC) (GLenum cap);
typedef WINGDIAPI void (APIENTRYP PFNGLDISABLEPROC) (GLenum cap);

struct MOJOSHADER_glContext
{
    // Allocators...
    MOJOSHADER_malloc malloc_fn;
    MOJOSHADER_free free_fn;
    void *malloc_data;

    // The constant register files...
    // Man, it kills me how much memory this takes...
    GLfloat vs_reg_file_f[8192 * 4];
    GLint vs_reg_file_i[2047 * 4];
    GLint vs_reg_file_b[2047];
    GLfloat ps_reg_file_f[8192 * 4];
    GLint ps_reg_file_i[2047 * 4];
    GLint ps_reg_file_b[2047];
    GLuint sampler_reg_file[16];

    // GL stuff...
    int opengl_major;
    int opengl_minor;
    MOJOSHADER_glProgram *bound_program;
    char profile[16];

    // Extensions...
    int have_base_opengl;
    int have_GL_ARB_vertex_program;
    int have_GL_ARB_fragment_program;
    int have_GL_NV_vertex_program2_option;
    int have_GL_NV_fragment_program2;
    int have_GL_NV_vertex_program3;
    int have_GL_NV_gpu_program4;
    int have_GL_ARB_shader_objects;
    int have_GL_ARB_vertex_shader;
    int have_GL_ARB_fragment_shader;
    int have_GL_ARB_shading_language_100;
    int have_GL_NV_half_float;
    int have_GL_ARB_half_float_vertex;
    int have_GL_OES_vertex_half_float;

    // Entry points...
    PFNGLGETSTRINGPROC glGetString;
    PFNGLGETERRORPROC glGetError;
    PFNGLGETINTEGERVPROC glGetIntegerv;
    PFNGLENABLEPROC glEnable;
    PFNGLDISABLEPROC glDisable;
    PFNGLDELETEOBJECTARBPROC glDeleteObject;
    PFNGLATTACHOBJECTARBPROC glAttachObject;
    PFNGLCOMPILESHADERARBPROC glCompileShader;
    PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObject;
    PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObject;
    PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArray;
    PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArray;
    PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocation;
    PFNGLGETINFOLOGARBPROC glGetInfoLog;
    PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameteriv;
    PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocation;
    PFNGLLINKPROGRAMARBPROC glLinkProgram;
    PFNGLSHADERSOURCEARBPROC glShaderSource;
    PFNGLUNIFORM1IARBPROC glUniform1i;
    PFNGLUNIFORM1IVARBPROC glUniform1iv;
    PFNGLUNIFORM4FVARBPROC glUniform4fv;
    PFNGLUNIFORM4IVARBPROC glUniform4iv;
    PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObject;
    PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointer;
    PFNGLGETPROGRAMIVARBPROC glGetProgramivARB;
    PFNGLGETPROGRAMSTRINGARBPROC glGetProgramStringARB;
    PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB;
    PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC glProgramLocalParameterI4ivNV;
    PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;
    PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
    PFNGLBINDPROGRAMARBPROC glBindProgramARB;
    PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;

    // interface for profile-specific things.
    int (*profileMaxUniforms)(MOJOSHADER_shaderType shader_type);
    int (*profileCompileShader)(const MOJOSHADER_parseData *pd, GLuint *s);
    void (*profileDeleteShader)(const GLuint shader);
    void (*profileDeleteProgram)(const GLuint program);
    GLint (*profileGetAttribLocation)(MOJOSHADER_glProgram *program, int idx);
    GLint (*profileGetUniformLocation)(MOJOSHADER_glProgram *program, MOJOSHADER_glShader *shader, int idx);
    GLint (*profileGetSamplerLocation)(MOJOSHADER_glProgram *, MOJOSHADER_glShader *, int);
    GLuint (*profileLinkProgram)(MOJOSHADER_glShader *, MOJOSHADER_glShader *);
    void (*profileFinalInitProgram)(MOJOSHADER_glProgram *program);
    void (*profileUseProgramObject)(MOJOSHADER_glProgram *program);
    void (*profilePushConstantArray)(MOJOSHADER_glProgram *, const MOJOSHADER_uniform *, const GLfloat *);
    void (*profilePushUniforms)(void);
    void (*profilePushSampler)(GLint loc, GLuint sampler);
    int (*profileMustPushConstantArrays)(void);
    int (*profileMustPushSamplers)(void);
};


static MOJOSHADER_glContext *ctx = NULL;

// Error state...
static char error_buffer[1024] = { '\0' };

static void set_error(const char *str)
{
    snprintf(error_buffer, sizeof (error_buffer), "%s", str);
} // set_error

#if PLATFORM_MACOSX
static inline int macosx_version_atleast(int x, int y, int z)
{
    static int checked = 0;
    static int combined = 0;

    if (!checked)
    {
        long ver, major, minor, patch;
        int convert = 0;

        if (Gestalt(gestaltSystemVersion, &ver) != noErr)
            ver = 0x1000;  // oh well.
        else if (ver < 0x1030)
            convert = 1;  // split (ver) into (major),(minor),(patch).
        else
        {
            // presumably this won't fail. But if it does, we'll just use the
            //  original version value. This might cut the value--10.12.11 will
            //  come out to 10.9.9, for example--but it's better than nothing.
            if (Gestalt(gestaltSystemVersionMajor, &major) != noErr)
                convert = 1;
            else if (Gestalt(gestaltSystemVersionMinor, &minor) != noErr)
                convert = 1;
            else if (Gestalt(gestaltSystemVersionBugFix, &patch) != noErr)
                convert = 1;
        } // else

        if (convert)
        {
            major = ((ver & 0xFF00) >> 8);
            major = (((major / 16) * 10) + (major % 16));
            minor = ((ver & 0xF0) >> 4);
            patch = (ver & 0xF);
        } // if

        combined = (major << 16) | (minor << 8) | patch;
        checked = 1;
    } // if

    return (combined >= ((x << 16) | (y << 8) | z));
} // macosx_version_atleast
#endif


static inline void *Malloc(const size_t len)
{
    void *retval = ctx->malloc_fn((int) len, ctx->malloc_data);
    if (retval == NULL)
        set_error("out of memory");
    return retval;
} // Malloc

static inline void Free(void *ptr)
{
    if (ptr != NULL)
        ctx->free_fn(ptr, ctx->malloc_data);
} // Free


static inline void toggle_gl_state(GLenum state, int val)
{
    if (val)
        ctx->glEnable(state);
    else
        ctx->glDisable(state);
} // toggle_gl_state


// profile-specific implementations...

#if SUPPORT_PROFILE_GLSL
static inline GLenum glsl_shader_type(const MOJOSHADER_shaderType t)
{
    if (t == MOJOSHADER_TYPE_VERTEX)
        return GL_VERTEX_SHADER;
    else if (t == MOJOSHADER_TYPE_PIXEL)
        return GL_FRAGMENT_SHADER;

    // !!! FIXME: geometry shaders?
    return GL_NONE;
} // glsl_shader_type


static int impl_GLSL_MustPushConstantArrays(void) { return 1; }
static int impl_GLSL_MustPushSamplers(void) { return 1; }

static int impl_GLSL_MaxUniforms(MOJOSHADER_shaderType shader_type)
{
    GLenum pname = GL_NONE;
    GLint val = 0;
    if (shader_type == MOJOSHADER_TYPE_VERTEX)
        pname = GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB;
    else if (shader_type == MOJOSHADER_TYPE_PIXEL)
        pname = GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB;
    else
        return -1;

    ctx->glGetIntegerv(pname, &val);
    return (int) val;
} // impl_GLSL_MaxUniforms


static int impl_GLSL_CompileShader(const MOJOSHADER_parseData *pd, GLuint *s)
{
    GLint ok = 0;
    GLint shaderlen = (GLint) pd->output_len;
    const GLenum shader_type = glsl_shader_type(pd->shader_type);
    GLuint shader = ctx->glCreateShaderObject(shader_type);

    ctx->glShaderSource(shader, 1, (const GLchar **) &pd->output, &shaderlen);
    ctx->glCompileShader(shader);
    ctx->glGetObjectParameteriv(shader, GL_OBJECT_COMPILE_STATUS_ARB, &ok);

    if (!ok)
    {
        GLsizei len = 0;
        ctx->glGetInfoLog(shader, sizeof (error_buffer), &len,
                          (GLchar *) error_buffer);
        *s = 0;
        return 0;
    } // if

    *s = shader;
    return 1;
} // impl_GLSL_CompileShader


static void impl_GLSL_DeleteShader(const GLuint shader)
{
    ctx->glDeleteObject(shader);
} // impl_GLSL_DeleteShader


static void impl_GLSL_DeleteProgram(const GLuint program)
{
    ctx->glDeleteObject(program);
} // impl_GLSL_DeleteProgram


static GLint impl_GLSL_GetUniformLocation(MOJOSHADER_glProgram *program,
                                          MOJOSHADER_glShader *shader, int idx)
{
    return 0;  // no-op, we push this as one big-ass array now.
} // impl_GLSL_GetUniformLocation


static GLint impl_GLSL_GetSamplerLocation(MOJOSHADER_glProgram *program,
                                          MOJOSHADER_glShader *shader, int idx)
{
    return ctx->glGetUniformLocation(program->handle,
                                     shader->parseData->samplers[idx].name);
} // impl_GLSL_GetSamplerLocation


static GLint impl_GLSL_GetAttribLocation(MOJOSHADER_glProgram *program, int idx)
{
    const MOJOSHADER_parseData *pd = program->vertex->parseData;
    const MOJOSHADER_attribute *a = pd->attributes;
    return ctx->glGetAttribLocation(program->handle, a[idx].name);
} // impl_GLSL_GetAttribLocation


static GLuint impl_GLSL_LinkProgram(MOJOSHADER_glShader *vshader,
                                    MOJOSHADER_glShader *pshader)
{
    const GLuint program = ctx->glCreateProgramObject();

    if (vshader != NULL) ctx->glAttachObject(program, vshader->handle);
    if (pshader != NULL) ctx->glAttachObject(program, pshader->handle);

    ctx->glLinkProgram(program);

    GLint ok = 0;
    ctx->glGetObjectParameteriv(program, GL_OBJECT_LINK_STATUS_ARB, &ok);
    if (!ok)
    {
        GLsizei len = 0;
        ctx->glGetInfoLog(program, sizeof (error_buffer), &len, (GLchar *) error_buffer);
        ctx->glDeleteObject(program);
        return 0;
    } // if

    return program;
} // impl_GLSL_LinkProgram


static void impl_GLSL_FinalInitProgram(MOJOSHADER_glProgram *program)
{
    program->vs_float4_loc =
        ctx->glGetUniformLocation(program->handle, "vs_uniforms_vec4");
    program->vs_int4_loc =
        ctx->glGetUniformLocation(program->handle, "vs_uniforms_ivec4");
    program->vs_bool_loc =
        ctx->glGetUniformLocation(program->handle, "vs_uniforms_bool");
    program->ps_float4_loc =
        ctx->glGetUniformLocation(program->handle, "ps_uniforms_vec4");
    program->ps_int4_loc =
        ctx->glGetUniformLocation(program->handle, "ps_uniforms_ivec4");
    program->ps_bool_loc =
        ctx->glGetUniformLocation(program->handle, "ps_uniforms_bool");
} // impl_GLSL_FinalInitProgram


static void impl_GLSL_UseProgramObject(MOJOSHADER_glProgram *program)
{
    ctx->glUseProgramObject((program != NULL) ? program->handle : 0);
} // impl_GLSL_UseProgramObject


static void impl_GLSL_PushConstantArray(MOJOSHADER_glProgram *program,
                                        const MOJOSHADER_uniform *u,
                                        const GLfloat *f)
{
    const GLint loc = ctx->glGetUniformLocation(program->handle, u->name);
    if (loc != -1)   // not optimized out?
        ctx->glUniform4fv(loc, u->array_count, f);
} // impl_GLSL_PushConstantArray


static void impl_GLSL_PushUniforms(void)
{
    const MOJOSHADER_glProgram *program = ctx->bound_program;

    assert(program->uniform_count > 0);  // don't call with nothing to do!

    if (program->vs_float4_loc != -1)
    {
        ctx->glUniform4fv(program->vs_float4_loc,
                          program->vs_uniforms_float4_count,
                          program->vs_uniforms_float4);
    } // if

    if (program->vs_int4_loc != -1)
    {
        ctx->glUniform4iv(program->vs_int4_loc,
                          program->vs_uniforms_int4_count,
                          program->vs_uniforms_int4);
    } // if

    if (program->vs_bool_loc != -1)
    {
        ctx->glUniform1iv(program->vs_bool_loc,
                          program->vs_uniforms_bool_count,
                          program->vs_uniforms_bool);
    } // if

    if (program->ps_float4_loc != -1)
    {
        ctx->glUniform4fv(program->ps_float4_loc,
                          program->ps_uniforms_float4_count,
                          program->ps_uniforms_float4);
    } // if

    if (program->ps_int4_loc != -1)
    {
        ctx->glUniform4iv(program->ps_int4_loc,
                          program->ps_uniforms_int4_count,
                          program->ps_uniforms_int4);
    } // if

    if (program->ps_bool_loc != -1)
    {
        ctx->glUniform1iv(program->ps_bool_loc,
                          program->ps_uniforms_bool_count,
                          program->ps_uniforms_bool);
    } // if
} // impl_GLSL_PushUniforms


static void impl_GLSL_PushSampler(GLint loc, GLuint sampler)
{
    ctx->glUniform1i(loc, sampler);
} // impl_GLSL_PushSampler

#endif  // SUPPORT_PROFILE_GLSL


#if SUPPORT_PROFILE_ARB1
static inline GLenum arb1_shader_type(const MOJOSHADER_shaderType t)
{
    if (t == MOJOSHADER_TYPE_VERTEX)
        return GL_VERTEX_PROGRAM_ARB;
    else if (t == MOJOSHADER_TYPE_PIXEL)
        return GL_FRAGMENT_PROGRAM_ARB;

    // !!! FIXME: geometry shaders?
    return GL_NONE;
} // arb1_shader_type

static int impl_ARB1_MustPushConstantArrays(void) { return 0; }
static int impl_ARB1_MustPushSamplers(void) { return 0; }

static int impl_ARB1_MaxUniforms(MOJOSHADER_shaderType shader_type)
{
    GLint retval = 0;
    const GLenum program_type = arb1_shader_type(shader_type);
    if (program_type == GL_NONE)
        return -1;

    ctx->glGetProgramivARB(program_type, GL_MAX_PROGRAM_PARAMETERS_ARB, &retval);
    return (int) retval;  // !!! FIXME: times four?
} // impl_ARB1_MaxUniforms


static int impl_ARB1_CompileShader(const MOJOSHADER_parseData *pd, GLuint *s)
{
    GLint shaderlen = (GLint) pd->output_len;
    const GLenum shader_type = arb1_shader_type(pd->shader_type);
    GLuint shader = 0;
    ctx->glGenProgramsARB(1, &shader);

    ctx->glGetError();  // flush any existing error state.
    ctx->glBindProgramARB(shader_type, shader);
    ctx->glProgramStringARB(shader_type, GL_PROGRAM_FORMAT_ASCII_ARB,
                                shaderlen, pd->output);

    if (ctx->glGetError() == GL_INVALID_OPERATION)
    { 
        GLint pos = 0;
        ctx->glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &pos);
        const GLubyte *errstr = ctx->glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        snprintf(error_buffer, sizeof (error_buffer),
                  "ARB1 compile error at position %d: %s",
                  (int) pos, (const char *) errstr);
        ctx->glBindProgramARB(shader_type, 0);
        ctx->glDeleteProgramsARB(1, &shader);
        *s = 0;
        return 0;
    } // if

    *s = shader;
    return 1;
} // impl_ARB1_CompileShader


static void impl_ARB1_DeleteShader(const GLuint _shader)
{
    GLuint shader = _shader;  // const removal.
    ctx->glDeleteProgramsARB(1, &shader);
} // impl_ARB1_DeleteShader


static void impl_ARB1_DeleteProgram(const GLuint program)
{
    // no-op. ARB1 doesn't have real linked programs.
} // impl_GLSL_DeleteProgram

static GLint impl_ARB1_GetUniformLocation(MOJOSHADER_glProgram *program,
                                          MOJOSHADER_glShader *shader, int idx)
{
    return 0;  // no-op, we push this as one big-ass array now.
} // impl_ARB1_GetUniformLocation

static GLint impl_ARB1_GetSamplerLocation(MOJOSHADER_glProgram *program,
                                          MOJOSHADER_glShader *shader, int idx)
{
    return shader->parseData->samplers[idx].index;
} // impl_ARB1_GetSamplerLocation


static GLint impl_ARB1_GetAttribLocation(MOJOSHADER_glProgram *program, int idx)
{
    return idx;  // map to vertex arrays in the same order as the parseData.
} // impl_ARB1_GetAttribLocation


static GLuint impl_ARB1_LinkProgram(MOJOSHADER_glShader *vshader,
                                    MOJOSHADER_glShader *pshader)
{
    // there is no formal linking in ARB1...just return a unique value.
    static GLuint retval = 1;
    return retval++;
} // impl_ARB1_LinkProgram


static void impl_ARB1_FinalInitProgram(MOJOSHADER_glProgram *program)
{
    // no-op.
} // impl_ARB1_FinalInitProgram


static void impl_ARB1_UseProgramObject(MOJOSHADER_glProgram *program)
{
    GLuint vhandle = 0;
    GLuint phandle = 0;
    if (program != NULL)
    {
        if (program->vertex != NULL)
            vhandle = program->vertex->handle;
        if (program->fragment != NULL)
            phandle = program->fragment->handle;
    } // if

    toggle_gl_state(GL_VERTEX_PROGRAM_ARB, vhandle != 0);
    toggle_gl_state(GL_FRAGMENT_PROGRAM_ARB, phandle != 0);

    ctx->glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vhandle);
    ctx->glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, phandle);
} // impl_ARB1_UseProgramObject


static void impl_ARB1_PushConstantArray(MOJOSHADER_glProgram *program,
                                        const MOJOSHADER_uniform *u,
                                        const GLfloat *f)
{
    // no-op. Constant arrays are defined in source code for arb1.
} // impl_ARB1_PushConstantArray


static void impl_ARB1_PushUniforms(void)
{
    // vertex shader uniforms come first in program->uniforms array.
    MOJOSHADER_shaderType shader_type = MOJOSHADER_TYPE_VERTEX;
    GLenum arb_shader_type = arb1_shader_type(shader_type);
    const MOJOSHADER_glProgram *program = ctx->bound_program;
    const uint32 count = program->uniform_count;
    const GLfloat *srcf = program->vs_uniforms_float4;
    const GLint *srci = program->vs_uniforms_int4;
    const GLint *srcb = program->vs_uniforms_bool;
    GLint loc = 0;
    uint32 i;

    assert(count > 0);  // shouldn't call this with nothing to do!

    for (i = 0; i < count; i++)
    {
        UniformMap *map = &program->uniforms[i];
        const MOJOSHADER_shaderType uniform_shader_type = map->shader_type;
        const MOJOSHADER_uniform *u = map->uniform;
        const MOJOSHADER_uniformType type = u->type;
        const int size = u->array_count ? u->array_count : 1;

        assert(!u->constant);

        // Did we switch from vertex to pixel (to geometry, etc)?
        if (shader_type != uniform_shader_type)
        {
            // we start with vertex, move to pixel, then to geometry, etc.
            //  The array should always be sorted as such.
            if (uniform_shader_type == MOJOSHADER_TYPE_PIXEL)
            {
                assert(shader_type == MOJOSHADER_TYPE_VERTEX);
                srcf = program->ps_uniforms_float4;
                srci = program->ps_uniforms_int4;
                srcb = program->ps_uniforms_bool;
                loc = 0;
            } // if
            else
            {
                // These should be ordered vertex, then pixel, then geometry.
                assert(0 && "Unexpected shader type");
            } // else

            shader_type = uniform_shader_type;
            arb_shader_type = arb1_shader_type(uniform_shader_type);
        } // if

        if (type == MOJOSHADER_UNIFORM_FLOAT)
        {
            int i;
            for (i = 0; i < size; i++, srcf += 4, loc++)
                ctx->glProgramLocalParameter4fvARB(arb_shader_type, loc, srcf);
        } // if
        else if (type == MOJOSHADER_UNIFORM_INT)
        {
            int i;
            if (ctx->glProgramLocalParameterI4ivNV != NULL)
            {
                // GL_NV_gpu_program4 has integer uniform loading support.
                for (i = 0; i < size; i++, srci += 4, loc++)
                    ctx->glProgramLocalParameterI4ivNV(arb_shader_type, loc, srci);
            } // if
            else
            {
                for (i = 0; i < size; i++, srci += 4, loc++)
                {
                    const GLfloat fv[4] = {
                        (GLfloat) srci[0], (GLfloat) srci[1],
                        (GLfloat) srci[2], (GLfloat) srci[3]
                    };
                    ctx->glProgramLocalParameter4fvARB(arb_shader_type, loc, fv);
                } // for
            } // else
        } // else if
        else if (type == MOJOSHADER_UNIFORM_BOOL)
        {
            int i;
            if (ctx->glProgramLocalParameterI4ivNV != NULL)
            {
                // GL_NV_gpu_program4 has integer uniform loading support.
                for (i = 0; i < size; i++, srcb++, loc++)
                {
                    const GLint ib = (GLint) ((*srcb) ? 1 : 0);
                    const GLint iv[4] = { ib, ib, ib, ib };
                    ctx->glProgramLocalParameterI4ivNV(arb_shader_type, loc, iv);
                } // for
            } // if
            else
            {
                for (i = 0; i < size; i++, srcb++, loc++)
                {
                    const GLfloat fb = (GLfloat) ((*srcb) ? 1.0f : 0.0f);
                    const GLfloat fv[4] = { fb, fb, fb, fb };
                    ctx->glProgramLocalParameter4fvARB(arb_shader_type, loc, fv);
                } // for
            } // else
        } // else if
    } // for
} // impl_ARB1_PushUniforms

static void impl_ARB1_PushSampler(GLint loc, GLuint sampler)
{
    // no-op in this profile...arb1 uses the texture units as-is.
    assert(loc == (GLint) sampler);
} // impl_ARB1_PushSampler

#endif  // SUPPORT_PROFILE_ARB1


const char *MOJOSHADER_glGetError(void)
{
    return error_buffer;
} // MOJOSHADER_glGetError


static void *loadsym(void *(*lookup)(const char *fn), const char *fn, int *ext)
{
    void *retval = NULL;
    if (lookup != NULL)
    {
        retval = lookup(fn);
        if (retval == NULL)
        {
            char arbfn[64];
            snprintf(arbfn, sizeof (arbfn), "%sARB", fn);
            retval = lookup(arbfn);
        } // if
    } // if

    if (retval == NULL)
        *ext = 0;

    return retval;
} // loadsym

static void lookup_entry_points(void *(*lookup)(const char *fnname))
{
    #define DO_LOOKUP(ext, typ, fn) { \
        int exist = ctx->have_##ext; \
        ctx->fn = (typ) loadsym(lookup, #fn, &exist); \
        ctx->have_##ext = exist; \
    }

    DO_LOOKUP(base_opengl, PFNGLGETSTRINGPROC, glGetString);
    DO_LOOKUP(base_opengl, PFNGLGETERRORPROC, glGetError);
    DO_LOOKUP(base_opengl, PFNGLGETINTEGERVPROC, glGetIntegerv);
    DO_LOOKUP(base_opengl, PFNGLENABLEPROC, glEnable);
    DO_LOOKUP(base_opengl, PFNGLDISABLEPROC, glDisable);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLDELETEOBJECTARBPROC, glDeleteObject);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLATTACHOBJECTARBPROC, glAttachObject);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLCOMPILESHADERARBPROC, glCompileShader);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLCREATEPROGRAMOBJECTARBPROC, glCreateProgramObject);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLCREATESHADEROBJECTARBPROC, glCreateShaderObject);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLGETINFOLOGARBPROC, glGetInfoLog);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLGETOBJECTPARAMETERIVARBPROC, glGetObjectParameteriv);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLGETUNIFORMLOCATIONARBPROC, glGetUniformLocation);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLLINKPROGRAMARBPROC, glLinkProgram);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLSHADERSOURCEARBPROC, glShaderSource);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLUNIFORM1IARBPROC, glUniform1i);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLUNIFORM1IVARBPROC, glUniform1iv);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLUNIFORM4FVARBPROC, glUniform4fv);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLUNIFORM4IVARBPROC, glUniform4iv);
    DO_LOOKUP(GL_ARB_shader_objects, PFNGLUSEPROGRAMOBJECTARBPROC, glUseProgramObject);
    DO_LOOKUP(GL_ARB_vertex_shader, PFNGLDISABLEVERTEXATTRIBARRAYARBPROC, glDisableVertexAttribArray);
    DO_LOOKUP(GL_ARB_vertex_shader, PFNGLENABLEVERTEXATTRIBARRAYARBPROC, glEnableVertexAttribArray);
    DO_LOOKUP(GL_ARB_vertex_shader, PFNGLGETATTRIBLOCATIONARBPROC, glGetAttribLocation);
    DO_LOOKUP(GL_ARB_vertex_shader, PFNGLVERTEXATTRIBPOINTERARBPROC, glVertexAttribPointer);
    DO_LOOKUP(GL_ARB_vertex_program, PFNGLVERTEXATTRIBPOINTERARBPROC, glVertexAttribPointer);
    DO_LOOKUP(GL_ARB_vertex_program, PFNGLGETPROGRAMIVARBPROC, glGetProgramivARB);
    DO_LOOKUP(GL_ARB_vertex_program, PFNGLGETPROGRAMSTRINGARBPROC, glGetProgramStringARB);
    DO_LOOKUP(GL_ARB_vertex_program, PFNGLPROGRAMLOCALPARAMETER4FVARBPROC, glProgramLocalParameter4fvARB);
    DO_LOOKUP(GL_ARB_vertex_program, PFNGLDELETEPROGRAMSARBPROC, glDeleteProgramsARB);
    DO_LOOKUP(GL_ARB_vertex_program, PFNGLGENPROGRAMSARBPROC, glGenProgramsARB);
    DO_LOOKUP(GL_ARB_vertex_program, PFNGLBINDPROGRAMARBPROC, glBindProgramARB);
    DO_LOOKUP(GL_ARB_vertex_program, PFNGLPROGRAMSTRINGARBPROC, glProgramStringARB);
    DO_LOOKUP(GL_NV_gpu_program4, PFNGLPROGRAMLOCALPARAMETERI4IVNVPROC, glProgramLocalParameterI4ivNV);

    #undef DO_LOOKUP
} // lookup_entry_points


static int verify_extension(const char *ext, int have, const char *extlist,
                            int major, int minor)
{
    if (have == 0)
        return 0;  // don't bother checking, we're missing an entry point.

    else if (!ctx->have_base_opengl)
        return 0;  // don't bother checking, we're missing basic functionality.

    // See if it's in the spec for this GL implementation's version.
    if (major >= 0)
    {
        if ( ((ctx->opengl_major << 16) | (ctx->opengl_minor & 0xFFFF)) >=
             ((major << 16) | (minor & 0xFFFF)) )
            return 1;
    } // if

    // Not available in the GL version, check the extension list.
    const char *ptr = strstr(extlist, ext);
    if (ptr == NULL)
        return 0;

    const char endchar = ptr[strlen(ext)];
    if ((endchar == '\0') || (endchar == ' '))
        return 1;  // extension is in the list.

    return 0;  // just not supported, fail.
} // verify_extension


static void parse_opengl_version_str(const char *verstr, int *maj, int *min)
{
    if (verstr == NULL)
        *maj = *min = 0;
    else
        sscanf(verstr, "%d.%d", maj, min);
} // parse_opengl_version_str


static inline void parse_opengl_version(const char *verstr)
{
    parse_opengl_version_str(verstr, &ctx->opengl_major, &ctx->opengl_minor);
} // parse_opengl_version


static int glsl_version_atleast(int maj, int min)
{
    int glslmin = 0;
    int glslmaj = 0;
    ctx->glGetError();  // flush any existing error state.
    const GLenum enumval = GL_SHADING_LANGUAGE_VERSION_ARB;
    const char *str = (const char *) ctx->glGetString(enumval);
    if (ctx->glGetError() == GL_INVALID_ENUM)
        return 0;  // this is a basic, 1.0-compliant implementation.
    parse_opengl_version_str(str, &glslmaj, &glslmin);
    return ( (glslmaj > maj) || ((glslmaj == maj) && (glslmin >= min)) );
} // glsl_version_atleast


static void load_extensions(void *(*lookup)(const char *fnname))
{
    const char *extlist = NULL;

    ctx->have_base_opengl = 1;
    ctx->have_GL_ARB_vertex_program = 1;
    ctx->have_GL_ARB_fragment_program = 1;
    ctx->have_GL_NV_vertex_program2_option = 1;
    ctx->have_GL_NV_fragment_program2 = 1;
    ctx->have_GL_NV_vertex_program3 = 1;
    ctx->have_GL_NV_gpu_program4 = 1;
    ctx->have_GL_ARB_shader_objects = 1;
    ctx->have_GL_ARB_vertex_shader = 1;
    ctx->have_GL_ARB_fragment_shader = 1;
    ctx->have_GL_ARB_shading_language_100 = 1;
    ctx->have_GL_NV_half_float = 1;
    ctx->have_GL_ARB_half_float_vertex = 1;
    ctx->have_GL_OES_vertex_half_float = 1;

    lookup_entry_points(lookup);

    if (!ctx->have_base_opengl)
        set_error("missing basic OpenGL entry points");
    else
    {
        parse_opengl_version((const char *) ctx->glGetString(GL_VERSION));
        extlist = (const char *) ctx->glGetString(GL_EXTENSIONS);
    } // else

    if (extlist == NULL)
        extlist = "";  // just in case.

    #define VERIFY_EXT(ext, major, minor) \
        ctx->have_##ext = verify_extension(#ext, ctx->have_##ext, extlist, major, minor)

    VERIFY_EXT(GL_ARB_vertex_program, -1, -1);
    VERIFY_EXT(GL_ARB_fragment_program, -1, -1);
    VERIFY_EXT(GL_ARB_shader_objects, 2, 0);
    VERIFY_EXT(GL_ARB_vertex_shader, 2, 0);
    VERIFY_EXT(GL_ARB_fragment_shader, 2, 0);
    VERIFY_EXT(GL_ARB_shading_language_100, 2, 0);
    VERIFY_EXT(GL_NV_vertex_program2_option, -1, -1);
    VERIFY_EXT(GL_NV_fragment_program2, -1, -1);
    VERIFY_EXT(GL_NV_vertex_program3, -1, -1);
    VERIFY_EXT(GL_NV_half_float, -1, -1);
    VERIFY_EXT(GL_ARB_half_float_vertex, 3, 0);
    VERIFY_EXT(GL_OES_vertex_half_float, -1, -1);

    #undef VERIFY_EXT
} // load_extensions


static int valid_profile(const char *profile)
{
    // If running on Mac OS X <= 10.4, don't ever pick GLSL, even if
    //  the system claims it is available.
    #if PLATFORM_MACOSX
    const int allow_glsl = macosx_version_atleast(10, 5, 0);
    #else
    const int allow_glsl = 1;
    #endif

    if (!ctx->have_base_opengl)
        return 0;

    #define MUST_HAVE(p, x) \
        if (!ctx->have_##x) { set_error(#p " profile needs " #x); return 0; }

    if (profile == NULL)
    {
        set_error("NULL profile");
        return 0;
    } // if

    #if SUPPORT_PROFILE_ARB1
    else if (strcmp(profile, MOJOSHADER_PROFILE_ARB1) == 0)
    {
        MUST_HAVE(MOJOSHADER_PROFILE_ARB1, GL_ARB_vertex_program);
        MUST_HAVE(MOJOSHADER_PROFILE_ARB1, GL_ARB_fragment_program);
    } // else if

    else if (strcmp(profile, MOJOSHADER_PROFILE_NV2) == 0)
    {
        MUST_HAVE(MOJOSHADER_PROFILE_NV2, GL_ARB_vertex_program);
        MUST_HAVE(MOJOSHADER_PROFILE_NV2, GL_ARB_fragment_program);
        MUST_HAVE(MOJOSHADER_PROFILE_NV2, GL_NV_vertex_program2_option);
        MUST_HAVE(MOJOSHADER_PROFILE_NV2, GL_NV_fragment_program2);
    } // else if

    else if (strcmp(profile, MOJOSHADER_PROFILE_NV3) == 0)
    {
        MUST_HAVE(MOJOSHADER_PROFILE_NV3, GL_ARB_vertex_program);
        MUST_HAVE(MOJOSHADER_PROFILE_NV3, GL_ARB_fragment_program);
        MUST_HAVE(MOJOSHADER_PROFILE_NV3, GL_NV_vertex_program3);
        MUST_HAVE(MOJOSHADER_PROFILE_NV3, GL_NV_fragment_program2);
    } // else if

    else if (strcmp(profile, MOJOSHADER_PROFILE_NV4) == 0)
    {
        MUST_HAVE(MOJOSHADER_PROFILE_NV4, GL_NV_gpu_program4);
    } // else if
    #endif

    #if SUPPORT_PROFILE_GLSL
    else if ((allow_glsl) && (strcmp(profile, MOJOSHADER_PROFILE_GLSL120) == 0))
    {
        MUST_HAVE(MOJOSHADER_PROFILE_GLSL, GL_ARB_shader_objects);
        MUST_HAVE(MOJOSHADER_PROFILE_GLSL, GL_ARB_vertex_shader);
        MUST_HAVE(MOJOSHADER_PROFILE_GLSL, GL_ARB_fragment_shader);
        MUST_HAVE(MOJOSHADER_PROFILE_GLSL, GL_ARB_shading_language_100);
        // if you got here, you have all the extensions.
        if (!glsl_version_atleast(1, 20))
            return 0;
    } // else if

    else if ((allow_glsl) && (strcmp(profile, MOJOSHADER_PROFILE_GLSL) == 0))
    {
        MUST_HAVE(MOJOSHADER_PROFILE_GLSL, GL_ARB_shader_objects);
        MUST_HAVE(MOJOSHADER_PROFILE_GLSL, GL_ARB_vertex_shader);
        MUST_HAVE(MOJOSHADER_PROFILE_GLSL, GL_ARB_fragment_shader);
        MUST_HAVE(MOJOSHADER_PROFILE_GLSL, GL_ARB_shading_language_100);
    } // else if
    #endif

    else
    {
        set_error("unknown or unsupported profile");
        return 0;
    } // else

    #undef MUST_HAVE

    return 1;
} // valid_profile


static const char *profile_priorities[] = {
#if SUPPORT_PROFILE_GLSL
    MOJOSHADER_PROFILE_GLSL120,
    MOJOSHADER_PROFILE_GLSL,
#endif
#if SUPPORT_PROFILE_ARB1
    MOJOSHADER_PROFILE_NV4,
    MOJOSHADER_PROFILE_NV3,
    MOJOSHADER_PROFILE_NV2,
    MOJOSHADER_PROFILE_ARB1,
#endif
};

int MOJOSHADER_glAvailableProfiles(void *(*lookup)(const char *fnname),
                                   const char **profs, const int size)
{
    int retval = 0;
    MOJOSHADER_glContext _ctx;
    MOJOSHADER_glContext *current_ctx = ctx;

    ctx = &_ctx;
    memset(ctx, '\0', sizeof (MOJOSHADER_glContext));
    load_extensions(lookup);

    if (ctx->have_base_opengl)
    {
        int i;
        for (i = 0; i < STATICARRAYLEN(profile_priorities); i++)
        {
            const char *profile = profile_priorities[i];
            if (valid_profile(profile))
            {
                if (retval < size)
                    profs[retval] = profile;
                retval++;
            } // if
        } // for
    } // if

    ctx = current_ctx;
    return retval;
} // MOJOSHADER_glAvailableProfiles


const char *MOJOSHADER_glBestProfile(void *(*lookup)(const char *fnname))
{
    const char *prof[STATICARRAYLEN(profile_priorities)];
    if (MOJOSHADER_glAvailableProfiles(lookup, prof, STATICARRAYLEN(prof)) <= 0)
    {
        set_error("no profiles available");
        return NULL;
    } // if

    return prof[0];  // profiles are sorted "best" to "worst."
} // MOJOSHADER_glBestProfile


MOJOSHADER_glContext *MOJOSHADER_glCreateContext(const char *profile,
                                        void *(*lookup)(const char *fnname),
                                        MOJOSHADER_malloc m, MOJOSHADER_free f,
                                        void *d)
{
    MOJOSHADER_glContext *retval = NULL;
    MOJOSHADER_glContext *current_ctx = ctx;
    ctx = NULL;

    if (m == NULL) m = MOJOSHADER_internal_malloc;
    if (f == NULL) f = MOJOSHADER_internal_free;

    ctx = (MOJOSHADER_glContext *) m(sizeof (MOJOSHADER_glContext), d);
    if (ctx == NULL)
    {
        set_error("out of memory");
        goto init_fail;
    } // if

    memset(ctx, '\0', sizeof (MOJOSHADER_glContext));
    ctx->malloc_fn = m;
    ctx->free_fn = f;
    ctx->malloc_data = d;
    snprintf(ctx->profile, sizeof (ctx->profile), "%s", profile);

    load_extensions(lookup);
    if (!valid_profile(profile))
        goto init_fail;

    MOJOSHADER_glBindProgram(NULL);

    // !!! FIXME: generalize this part.
    if (profile == NULL) {}

#if SUPPORT_PROFILE_GLSL
    else if ( (strcmp(profile, MOJOSHADER_PROFILE_GLSL) == 0) ||
              (strcmp(profile, MOJOSHADER_PROFILE_GLSL120) == 0) )
    {
        ctx->profileMaxUniforms = impl_GLSL_MaxUniforms;
        ctx->profileCompileShader = impl_GLSL_CompileShader;
        ctx->profileDeleteShader = impl_GLSL_DeleteShader;
        ctx->profileDeleteProgram = impl_GLSL_DeleteProgram;
        ctx->profileGetAttribLocation = impl_GLSL_GetAttribLocation;
        ctx->profileGetUniformLocation = impl_GLSL_GetUniformLocation;
        ctx->profileGetSamplerLocation = impl_GLSL_GetSamplerLocation;
        ctx->profileLinkProgram = impl_GLSL_LinkProgram;
        ctx->profileFinalInitProgram = impl_GLSL_FinalInitProgram;
        ctx->profileUseProgramObject = impl_GLSL_UseProgramObject;
        ctx->profilePushConstantArray = impl_GLSL_PushConstantArray;
        ctx->profilePushUniforms = impl_GLSL_PushUniforms;
        ctx->profilePushSampler = impl_GLSL_PushSampler;
        ctx->profileMustPushConstantArrays = impl_GLSL_MustPushConstantArrays;
        ctx->profileMustPushSamplers = impl_GLSL_MustPushSamplers;
    } // if
#endif

#if SUPPORT_PROFILE_ARB1
    else if ( (strcmp(profile, MOJOSHADER_PROFILE_ARB1) == 0) ||
              (strcmp(profile, MOJOSHADER_PROFILE_NV2) == 0) ||
              (strcmp(profile, MOJOSHADER_PROFILE_NV3) == 0) ||
              (strcmp(profile, MOJOSHADER_PROFILE_NV4) == 0) )
    {
        ctx->profileMaxUniforms = impl_ARB1_MaxUniforms;
        ctx->profileCompileShader = impl_ARB1_CompileShader;
        ctx->profileDeleteShader = impl_ARB1_DeleteShader;
        ctx->profileDeleteProgram = impl_ARB1_DeleteProgram;
        ctx->profileGetAttribLocation = impl_ARB1_GetAttribLocation;
        ctx->profileGetUniformLocation = impl_ARB1_GetUniformLocation;
        ctx->profileGetSamplerLocation = impl_ARB1_GetSamplerLocation;
        ctx->profileLinkProgram = impl_ARB1_LinkProgram;
        ctx->profileFinalInitProgram = impl_ARB1_FinalInitProgram;
        ctx->profileUseProgramObject = impl_ARB1_UseProgramObject;
        ctx->profilePushConstantArray = impl_ARB1_PushConstantArray;
        ctx->profilePushUniforms = impl_ARB1_PushUniforms;
        ctx->profilePushSampler = impl_ARB1_PushSampler;
        ctx->profileMustPushConstantArrays = impl_ARB1_MustPushConstantArrays;
        ctx->profileMustPushSamplers = impl_ARB1_MustPushSamplers;
    } // if
#endif

    assert(ctx->profileMaxUniforms != NULL);
    assert(ctx->profileCompileShader != NULL);
    assert(ctx->profileDeleteShader != NULL);
    assert(ctx->profileDeleteProgram != NULL);
    assert(ctx->profileMaxUniforms != NULL);
    assert(ctx->profileGetAttribLocation != NULL);
    assert(ctx->profileGetUniformLocation != NULL);
    assert(ctx->profileGetSamplerLocation != NULL);
    assert(ctx->profileLinkProgram != NULL);
    assert(ctx->profileFinalInitProgram != NULL);
    assert(ctx->profileUseProgramObject != NULL);
    assert(ctx->profilePushConstantArray != NULL);
    assert(ctx->profilePushUniforms != NULL);
    assert(ctx->profilePushSampler != NULL);
    assert(ctx->profileMustPushConstantArrays != NULL);
    assert(ctx->profileMustPushSamplers != NULL);

    retval = ctx;
    ctx = current_ctx;
    return retval;

init_fail:
    if (ctx != NULL)
        f(ctx, d);
    ctx = current_ctx;
    return NULL;
} // MOJOSHADER_glCreateContext


void MOJOSHADER_glMakeContextCurrent(MOJOSHADER_glContext *_ctx)
{
    ctx = _ctx;
} // MOJOSHADER_glMakeContextCurrent


int MOJOSHADER_glMaxUniforms(MOJOSHADER_shaderType shader_type)
{
    return ctx->profileMaxUniforms(shader_type);
} // MOJOSHADER_glMaxUniforms


MOJOSHADER_glShader *MOJOSHADER_glCompileShader(const unsigned char *tokenbuf,
                                                const unsigned int bufsize,
                                                const MOJOSHADER_swizzle *swiz,
                                                const unsigned int swizcount)
{
    MOJOSHADER_glShader *retval = NULL;
    GLuint shader = 0;
    const MOJOSHADER_parseData *pd = MOJOSHADER_parse(ctx->profile, tokenbuf,
                                                      bufsize, swiz, swizcount,
                                                      ctx->malloc_fn,
                                                      ctx->free_fn,
                                                      ctx->malloc_data);
    if (pd->error_count > 0)
    {
        // !!! FIXME: put multiple errors in the buffer? Don't use
        // !!! FIXME:  MOJOSHADER_glGetError() for this?
        set_error(pd->errors[0].error);
        goto compile_shader_fail;
    } // if

    retval = (MOJOSHADER_glShader *) Malloc(sizeof (MOJOSHADER_glShader));
    if (retval == NULL)
        goto compile_shader_fail;

    if (!ctx->profileCompileShader(pd, &shader))
        goto compile_shader_fail;

    retval->parseData = pd;
    retval->handle = shader;
    retval->refcount = 1;
    return retval;

compile_shader_fail:
    MOJOSHADER_freeParseData(pd);
    Free(retval);
    if (shader != 0)
        ctx->profileDeleteShader(shader);
    return NULL;
} // MOJOSHADER_glCompileShader


const MOJOSHADER_parseData *MOJOSHADER_glGetShaderParseData(
                                                MOJOSHADER_glShader *shader)
{
    return (shader != NULL) ? shader->parseData : NULL;
} // MOJOSHADER_glGetShaderParseData


static void shader_unref(MOJOSHADER_glShader *shader)
{
    if (shader != NULL)
    {
        const uint32 refcount = shader->refcount;
        if (refcount > 1)
            shader->refcount--;
        else
        {
            ctx->profileDeleteShader(shader->handle);
            MOJOSHADER_freeParseData(shader->parseData);
            Free(shader);
        } // else
    } // if
} // shader_unref


static void program_unref(MOJOSHADER_glProgram *program)
{
    if (program != NULL)
    {
        const uint32 refcount = program->refcount;
        if (refcount > 1)
            program->refcount--;
        else
        {
            ctx->profileDeleteProgram(program->handle);
            shader_unref(program->vertex);
            shader_unref(program->fragment);
            Free(program->vs_uniforms_float4);
            Free(program->vs_uniforms_int4);
            Free(program->vs_uniforms_bool);
            Free(program->ps_uniforms_float4);
            Free(program->ps_uniforms_int4);
            Free(program->ps_uniforms_bool);
            Free(program->uniforms);
            Free(program->attributes);
            Free(program);
        } // else
    } // if
} // program_unref


static void fill_constant_array(GLfloat *f, const int base, const int size,
                                const MOJOSHADER_parseData *pd)
{
    int i;
    int filled = 0;
    for (i = 0; i < pd->constant_count; i++)
    {
        const MOJOSHADER_constant *c = &pd->constants[i];
        if (c->type != MOJOSHADER_UNIFORM_FLOAT)
            continue;
        else if (c->index < base)
            continue;
        else if (c->index >= (base+size))
            continue;
        memcpy(&f[(c->index-base) * 4], &c->value.f, sizeof (c->value.f));
        filled++;
    } // for

    assert(filled == size);
} // fill_constant_array


static int lookup_uniforms(MOJOSHADER_glProgram *program,
                           MOJOSHADER_glShader *shader, int *bound)
{
    const MOJOSHADER_parseData *pd = shader->parseData;
    const MOJOSHADER_shaderType shader_type = pd->shader_type;
    uint32 float4_count = 0;
    uint32 int4_count = 0;
    uint32 bool_count = 0;
    int i;

    for (i = 0; i < pd->uniform_count; i++)
    {
        const MOJOSHADER_uniform *u = &pd->uniforms[i];

        if (u->constant)
        {
            // only do constants once, at link time. These aren't changed ever.
            if (ctx->profileMustPushConstantArrays())
            {
                const int base = u->index;
                const int size = u->array_count;
                GLfloat *f = (GLfloat *) alloca(sizeof (GLfloat) * (size * 4));
                fill_constant_array(f, base, size, pd);
                if (!(*bound))
                {
                    ctx->profileUseProgramObject(program);
                    *bound = 1;
                } // if
                ctx->profilePushConstantArray(program, u, f);
            } // if
        } // if

        else
        {
            const GLint loc = ctx->profileGetUniformLocation(program, shader, i);
            if (loc != -1)  // -1 means it was optimized out, or failure.
            {
                const int regcount = u->array_count;
                UniformMap *map = &program->uniforms[program->uniform_count];
                map->shader_type = shader_type;
                map->uniform = u;
                map->location = (GLuint) loc;
                program->uniform_count++;

                if (u->type == MOJOSHADER_UNIFORM_FLOAT)
                    float4_count += regcount ? regcount : 1;
                else if (u->type == MOJOSHADER_UNIFORM_INT)
                    int4_count += regcount ? regcount : 1;
                else if (u->type == MOJOSHADER_UNIFORM_BOOL)
                    bool_count += regcount ? regcount : 1;
                else
                    assert(0 && "Unexpected register type");
            } // if
        } // else
    } // for

    #define MAKE_ARRAY(typ, gltyp, siz, count) \
        if (count) { \
            const size_t buflen = sizeof (gltyp) * siz * count; \
            gltyp *ptr = (gltyp *) Malloc(buflen); \
            if (ptr == NULL) { \
                return 0; \
            } else if (shader_type == MOJOSHADER_TYPE_VERTEX) { \
                program->vs_uniforms_##typ = ptr; \
                program->vs_uniforms_##typ##_count = count; \
            } else if (shader_type == MOJOSHADER_TYPE_PIXEL) { \
                program->ps_uniforms_##typ = ptr; \
                program->ps_uniforms_##typ##_count = count; \
            } else { \
                assert(0 && "unsupported shader type"); \
            } \
            memset(ptr, '\0', buflen); \
        }

    MAKE_ARRAY(float4, GLfloat, 4, float4_count);
    MAKE_ARRAY(int4, GLint, 4, int4_count);
    MAKE_ARRAY(bool, GLint, 1, bool_count);

    #undef MAKE_ARRAY

    return 1;
} // lookup_uniforms


static void lookup_samplers(MOJOSHADER_glProgram *program,
                            MOJOSHADER_glShader *shader, int *bound)
{
    const MOJOSHADER_parseData *pd = shader->parseData;
    const MOJOSHADER_sampler *s = pd->samplers;
    int i;

    if ((pd->sampler_count == 0) || (!ctx->profileMustPushSamplers()))
        return;   // nothing to do here, so don't bother binding, etc.

    // Link up the Samplers. These never change after link time, since they
    //  are meant to be constant texture unit ids and not textures.

    if (!(*bound))
    {
        ctx->profileUseProgramObject(program);
        *bound = 1;
    } // if

    for (i = 0; i < pd->sampler_count; i++)
    {
        const GLint loc = ctx->profileGetSamplerLocation(program, shader, i);
        if (loc != -1)  // maybe the Sampler was optimized out?
            ctx->profilePushSampler(loc, s[i].index);
    } // for
} // lookup_samplers


static void lookup_attributes(MOJOSHADER_glProgram *program)
{
    int i;
    const MOJOSHADER_parseData *pd = program->vertex->parseData;
    const MOJOSHADER_attribute *a = pd->attributes;

    for (i = 0; i < pd->attribute_count; i++)
    {
        const GLint loc = ctx->profileGetAttribLocation(program, i);
        if (loc != -1)  // maybe the Attribute was optimized out?
        {
            AttributeMap *map = &program->attributes[program->attribute_count];
            map->attribute = &a[i];
            map->location = (GLuint) loc;
            program->attribute_count++;
        } // if
    } // for
} // lookup_attributes


// !!! FIXME: misnamed
// build a list of indexes that need to be overwritten with constant values
//  when pushing a uniform array to the GL.
static int build_constants_lists(MOJOSHADER_glProgram *program)
{
    int i;
    const int count = program->uniform_count;
    for (i = 0; i < count; i++)
    {
        UniformMap *map = &program->uniforms[i];
        const MOJOSHADER_uniform *u = map->uniform;
        const int size = u->array_count;

        assert(!u->constant);

        if (size == 0)
            continue;  // nothing to see here.

        // only use arrays for 'c' registers.
        const MOJOSHADER_uniformType type = u->type;
        assert(type == MOJOSHADER_UNIFORM_FLOAT);

        // !!! FIXME: deal with this.
    } // for

    return 1;
} // build_constants_lists


MOJOSHADER_glProgram *MOJOSHADER_glLinkProgram(MOJOSHADER_glShader *vshader,
                                               MOJOSHADER_glShader *pshader)
{
    int bound = 0;

    if ((vshader == NULL) && (pshader == NULL))
        return NULL;

    int numregs = 0;
    MOJOSHADER_glProgram *retval = NULL;
    const GLuint program = ctx->profileLinkProgram(vshader, pshader);
    if (program == 0)
        goto link_program_fail;

    retval = (MOJOSHADER_glProgram *) Malloc(sizeof (MOJOSHADER_glProgram));
    if (retval == NULL)
        goto link_program_fail;
    memset(retval, '\0', sizeof (MOJOSHADER_glProgram));

    numregs = 0;
    if (vshader != NULL) numregs += vshader->parseData->uniform_count;
    if (pshader != NULL) numregs += pshader->parseData->uniform_count;
    if (numregs > 0)
    {
        const size_t len = sizeof (UniformMap) * numregs;
        retval->uniforms = (UniformMap *) Malloc(len);
        if (retval->uniforms == NULL)
            goto link_program_fail;
        memset(retval->uniforms, '\0', len);
    } // if

    retval->handle = program;
    retval->vertex = vshader;
    retval->fragment = pshader;
    retval->refcount = 1;

    if (vshader != NULL)
    {
        if (vshader->parseData->attribute_count > 0)
        {
            const int count = vshader->parseData->attribute_count;
            const size_t len = sizeof (AttributeMap) * count;
            retval->attributes = (AttributeMap *) Malloc(len);
            if (retval->attributes == NULL)
                goto link_program_fail;

            memset(retval->attributes, '\0', len);
            lookup_attributes(retval);
        } // if

        if (!lookup_uniforms(retval, vshader, &bound))
            goto link_program_fail;
        lookup_samplers(retval, vshader, &bound);
        vshader->refcount++;
    } // if

    if (pshader != NULL)
    {
        if (!lookup_uniforms(retval, pshader, &bound))
            goto link_program_fail;
        lookup_samplers(retval, pshader, &bound);
        pshader->refcount++;
    } // if

    if (!build_constants_lists(retval))
        goto link_program_fail;

    if (bound)  // reset the old binding.
        ctx->profileUseProgramObject(ctx->bound_program);

    ctx->profileFinalInitProgram(retval);

    return retval;

link_program_fail:
    if (retval != NULL)
    {
        Free(retval->vs_uniforms_float4);
        Free(retval->vs_uniforms_int4);
        Free(retval->vs_uniforms_bool);
        Free(retval->ps_uniforms_float4);
        Free(retval->ps_uniforms_int4);
        Free(retval->ps_uniforms_bool);
        Free(retval->uniforms);
        Free(retval->attributes);
        Free(retval);
    } // if

    if (program != 0)
        ctx->profileDeleteProgram(program);

    if (bound)
        ctx->profileUseProgramObject(ctx->bound_program);

    return NULL;
} // MOJOSHADER_glLinkProgram


void MOJOSHADER_glBindProgram(MOJOSHADER_glProgram *program)
{
    GLuint handle = 0;
    int i;

    if (program == ctx->bound_program)
        return;  // nothing to do.

    // Disable any client-side arrays the current program could have used.
    // !!! FIXME: don't disable yet...see which ones get reused, and disable
    // !!! FIXME:  only what we don't need in MOJOSHADER_glProgramReady().
    if (ctx->bound_program != NULL)
    {
        const int count = ctx->bound_program->attribute_count;
        for (i = 0; i < count; i++)
        {
            const AttributeMap *map = &ctx->bound_program->attributes[i];
            ctx->glDisableVertexAttribArray(map->location);
        } // if
    } // for

    if (program != NULL)
    {
        handle = program->handle;
        program->refcount++;
    } // if

    ctx->profileUseProgramObject(program);
    program_unref(ctx->bound_program);
    ctx->bound_program = program;
} // MOJOSHADER_glBindProgram


static inline uint minuint(const uint a, const uint b)
{
    return ((a < b) ? a : b);
} // minuint


void MOJOSHADER_glSetVertexShaderUniformF(unsigned int idx, const float *data,
                                          unsigned int vec4n)
{
    const uint maxregs = STATICARRAYLEN(ctx->vs_reg_file_f) / 4;
    if (idx < maxregs)
    {
        assert(sizeof (GLfloat) == sizeof (float));
        const uint cpy = (minuint(maxregs - idx, vec4n) * sizeof (*data)) * 4;
        memcpy(ctx->vs_reg_file_f + (idx * 4), data, cpy);
    } // if
} // MOJOSHADER_glSetVertexShaderUniformF


void MOJOSHADER_glSetVertexShaderUniformI(unsigned int idx, const int *data,
                                          unsigned int ivec4n)
{
    const uint maxregs = STATICARRAYLEN(ctx->vs_reg_file_i) / 4;
    if (idx < maxregs)
    {
        assert(sizeof (GLint) == sizeof (int));
        const uint cpy = (minuint(maxregs - idx, ivec4n) * sizeof (*data)) * 4;
        memcpy(ctx->vs_reg_file_i + (idx * 4), data, cpy);
    } // if
} // MOJOSHADER_glSetVertexShaderUniformI


void MOJOSHADER_glSetVertexShaderUniformB(unsigned int idx, const int *data,
                                          unsigned int bcount)
{
    const uint maxregs = STATICARRAYLEN(ctx->vs_reg_file_f) / 4;
    if (idx < maxregs)
    {
        GLint *wptr = ctx->vs_reg_file_b + idx;
        GLint *endptr = wptr + minuint(maxregs - idx, bcount);
        while (wptr != endptr)
            *(wptr++) = *(data++) ? 1 : 0;
    } // if
} // MOJOSHADER_glSetVertexShaderUniformB


void MOJOSHADER_glSetPixelShaderUniformF(unsigned int idx, const float *data,
                                         unsigned int vec4n)
{
    const uint maxregs = STATICARRAYLEN(ctx->ps_reg_file_f) / 4;
    if (idx < maxregs)
    {
        assert(sizeof (GLfloat) == sizeof (float));
        const uint cpy = (minuint(maxregs - idx, vec4n) * sizeof (*data)) * 4;
        memcpy(ctx->ps_reg_file_f + (idx * 4), data, cpy);
    } // if
} // MOJOSHADER_glSetPixelShaderUniformF


void MOJOSHADER_glSetPixelShaderUniformI(unsigned int idx, const int *data,
                                         unsigned int ivec4n)
{
    const uint maxregs = STATICARRAYLEN(ctx->ps_reg_file_i) / 4;
    if (idx < maxregs)
    {
        assert(sizeof (GLint) == sizeof (int));
        const uint cpy = (minuint(maxregs - idx, ivec4n) * sizeof (*data)) * 4;
        memcpy(ctx->ps_reg_file_i + (idx * 4), data, cpy);
    } // if
} // MOJOSHADER_glSetPixelShaderUniformI


void MOJOSHADER_glSetPixelShaderUniformB(unsigned int idx, const int *data,
                                         unsigned int bcount)
{
    const uint maxregs = STATICARRAYLEN(ctx->ps_reg_file_f) / 4;
    if (idx < maxregs)
    {
        GLint *wptr = ctx->ps_reg_file_b + idx;
        GLint *endptr = wptr + minuint(maxregs - idx, bcount);
        while (wptr != endptr)
            *(wptr++) = *(data++) ? 1 : 0;
    } // if
} // MOJOSHADER_glSetPixelShaderUniformB


static inline GLenum opengl_attr_type(const MOJOSHADER_attributeType type)
{
    switch (type)
    {
        case MOJOSHADER_ATTRIBUTE_UNKNOWN: return GL_NONE; // oh well.
        case MOJOSHADER_ATTRIBUTE_BYTE: return GL_BYTE;
        case MOJOSHADER_ATTRIBUTE_UBYTE: return GL_UNSIGNED_BYTE;
        case MOJOSHADER_ATTRIBUTE_SHORT: return GL_SHORT;
        case MOJOSHADER_ATTRIBUTE_USHORT: return GL_UNSIGNED_SHORT;
        case MOJOSHADER_ATTRIBUTE_INT: return GL_INT;
        case MOJOSHADER_ATTRIBUTE_UINT: return GL_UNSIGNED_INT;
        case MOJOSHADER_ATTRIBUTE_FLOAT: return GL_FLOAT;
        case MOJOSHADER_ATTRIBUTE_DOUBLE: return GL_DOUBLE;

        case MOJOSHADER_ATTRIBUTE_HALF_FLOAT:
            if (ctx->have_GL_NV_half_float)
                return GL_HALF_FLOAT_NV;
            else if (ctx->have_GL_ARB_half_float_vertex)
                return GL_HALF_FLOAT;
            else if (ctx->have_GL_OES_vertex_half_float)
                return GL_HALF_FLOAT;
            break;
    } // switch

    return GL_NONE;  // oh well. Raises a GL error later.
} // opengl_attr_type


// !!! FIXME: shouldn't (index) be unsigned?
void MOJOSHADER_glSetVertexAttribute(MOJOSHADER_usage usage,
                                     int index, unsigned int size,
                                     MOJOSHADER_attributeType type,
                                     int normalized, unsigned int stride,
                                     const void *ptr)
{
    if ((ctx->bound_program == NULL) || (ctx->bound_program->vertex == NULL))
        return;

    const GLenum gl_type = opengl_attr_type(type);
    const GLboolean norm = (normalized) ? GL_TRUE : GL_FALSE;
    const int count = ctx->bound_program->attribute_count;
    GLuint gl_index = 0;
    int i;

    for (i = 0; i < count; i++)
    {
        const AttributeMap *map = &ctx->bound_program->attributes[i];
        const MOJOSHADER_attribute *a = map->attribute;

        // !!! FIXME: is this array guaranteed to be sorted by usage?
        // !!! FIXME:  if so, we can break if a->usage > usage.

        if ((a->usage == usage) && (a->index == index))
        {
            gl_index = map->location;
            break;
        } // if
    } // for

    if (i == count)
        return;  // nothing to do, this shader doesn't use this stream.

    // these happen to work in both ARB1 and GLSL, but if something alien
    //  shows up, we'll have to split these into profile*() functions.
    ctx->glVertexAttribPointer(gl_index, size, gl_type, norm, stride, ptr);
    ctx->glEnableVertexAttribArray(gl_index);
} // MOJOSHADER_glSetVertexAttribute


void MOJOSHADER_glProgramReady(void)
{
    const MOJOSHADER_glProgram *program = ctx->bound_program;

    if (program == NULL)
        return;  // nothing to do.

    // push Uniforms to the program from our register files...
    if (program->uniform_count > 0)
    {
        // vertex shader uniforms come first in program->uniforms array.
        const uint32 count = program->uniform_count;
        const GLfloat *srcf = ctx->vs_reg_file_f;
        const GLint *srci = ctx->vs_reg_file_i;
        const GLint *srcb = ctx->vs_reg_file_b;
        MOJOSHADER_shaderType shader_type = MOJOSHADER_TYPE_VERTEX;
        GLfloat *dstf = program->vs_uniforms_float4;
        GLint *dsti = program->vs_uniforms_int4;
        GLint *dstb = program->vs_uniforms_bool;
        uint32 i;

        for (i = 0; i < count; i++)
        {
            UniformMap *map = &program->uniforms[i];
            const MOJOSHADER_shaderType uniform_shader_type = map->shader_type;
            const MOJOSHADER_uniform *u = map->uniform;
            const MOJOSHADER_uniformType type = u->type;
            const int index = u->index;
            const int size = u->array_count ? u->array_count : 1;

            assert(!u->constant);

            // Did we switch from vertex to pixel (to geometry, etc)?
            if (shader_type != uniform_shader_type)
            {
                // we start with vertex, move to pixel, then to geometry, etc.
                //  The array should always be sorted as such.
                if (uniform_shader_type == MOJOSHADER_TYPE_PIXEL)
                {
                    assert(shader_type == MOJOSHADER_TYPE_VERTEX);
                    srcf = ctx->ps_reg_file_f;
                    srci = ctx->ps_reg_file_i;
                    srcb = ctx->ps_reg_file_b;
                    dstf = program->ps_uniforms_float4;
                    dsti = program->ps_uniforms_int4;
                    dstb = program->ps_uniforms_bool;
                } // if
                else
                {
                    // Should be ordered vertex, then pixel, then geometry.
                    assert(0 && "Unexpected shader type");
                } // else

                shader_type = uniform_shader_type;
            } // if

            if (type == MOJOSHADER_UNIFORM_FLOAT)
            {
                const size_t count = 4 * size;
                const GLfloat *f = &srcf[index * 4];
                memcpy(dstf, f, sizeof (GLfloat) * count);
                dstf += count;
            } // if
            else if (type == MOJOSHADER_UNIFORM_INT)
            {
                const size_t count = 4 * size;
                const GLint *i = &srci[index * 4];
                memcpy(dsti, i, sizeof (GLint) * count);
                dsti += count;
            } // else if
            else if (type == MOJOSHADER_UNIFORM_BOOL)
            {
                const size_t count = size;
                const GLint *b = &srcb[index];
                memcpy(dstb, &b, sizeof (GLint) * count);
                dstb += count;
            } // else if

            // !!! FIXME: set constants that overlap the array.
        } // for

        ctx->profilePushUniforms();
    } // if
} // MOJOSHADER_glProgramReady


void MOJOSHADER_glDeleteProgram(MOJOSHADER_glProgram *program)
{
    program_unref(program);
} // MOJOSHADER_glDeleteProgram


void MOJOSHADER_glDeleteShader(MOJOSHADER_glShader *shader)
{
    shader_unref(shader);
} // MOJOSHADER_glDeleteShader


void MOJOSHADER_glDestroyContext(MOJOSHADER_glContext *_ctx)
{
    MOJOSHADER_glContext *current_ctx = ctx;
    ctx = _ctx;
    MOJOSHADER_glBindProgram(NULL);
    lookup_entry_points(NULL);
    Free(ctx);
    ctx = ((current_ctx == _ctx) ? NULL : current_ctx);
} // MOJOSHADER_glDestroyContext

// end of mojoshader_opengl.c ...

