/*
 * -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- 
 */

/*
 *	This file is part of Seed, the GObject Introspection<->Javascript bindings.
 *
 *		Seed is free software: you can redistribute it and/or modify 
 *		it under the terms of the GNU Lesser General Public License as
 *		published by the Free Software Foundation, either version 3 of
 *		the License, or (at your option) any later version. 
 *		Seed is distributed in the hope that it will be useful, 
 *		but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *		GNU Lesser General Public License for more details. 
 *		You should have received a copy of the GNU Lesser General Public License 
 *		along with Seed.  If not, see <http://www.gnu.org/licenses/>. 
 *
 *		Copyright (C) Robert Carr 2008 <carrr@rpi.edu>
 */

#include <unistd.h>
#include <stdio.h>
#include "seed-private.h"
#include <string.h>
#include <sys/mman.h>

static JSValueRef
seed_include(JSContextRef ctx,
			 JSObjectRef function,
			 JSObjectRef this_object,
			 size_t argumentCount,
			 const JSValueRef arguments[], JSValueRef * exception)
{
	JSStringRef file_contents, file_name;
	gchar *import_file;
	gchar *buffer, *walk;

	if (argumentCount != 1)
	{
		gchar *mes =
			g_strdup_printf("Seed.include expected 1 argument, "
							"got %d", (unsigned int)argumentCount);
		seed_make_exception(ctx, exception, "ArgumentError", mes);
		g_free(mes);
		return JSValueMakeNull(ctx);
	}
	import_file = seed_value_to_string(ctx, arguments[0], exception);

	g_file_get_contents(import_file, &buffer, 0, 0);

	if (!buffer)
	{
		gchar *mes = g_strdup_printf("File not found: %s", import_file);
		seed_make_exception(ctx, exception, "FileNotFound", mes);

		g_free(import_file);
		g_free(buffer);
		g_free(mes);
		return JSValueMakeNull(ctx);
	}

	walk = buffer;

	if (*walk == '#')
	{
		while (*walk != '\n')
			walk++;
		walk++;
	}

	walk = g_strdup(walk);
	g_free(buffer);

	file_contents = JSStringCreateWithUTF8CString(walk);
	file_name = JSStringCreateWithUTF8CString(import_file);

	JSEvaluateScript(ctx, file_contents, NULL, file_name, 0, exception);

	JSStringRelease(file_contents);
	JSStringRelease(file_name);
	g_free(import_file);
	g_free(walk);

	return JSValueMakeNull(ctx);
}

static JSValueRef
seed_print(JSContextRef ctx,
		   JSObjectRef function,
		   JSObjectRef this_object,
		   size_t argumentCount,
		   const JSValueRef arguments[], JSValueRef * exception)
{
	if (argumentCount != 1)
	{
		gchar *mes =
			g_strdup_printf("Seed.print expected 1 argument," " got %d",
							(unsigned int)argumentCount);
		seed_make_exception(ctx, exception, "ArgumentError", mes);
		g_free(mes);
		return JSValueMakeNull(ctx);
	}

	gchar *buf = seed_value_to_string(ctx, arguments[0], exception);
	printf("%s\n", buf);
	g_free(buf);

	return JSValueMakeNull(ctx);
}

const gchar *seed_g_type_name_to_string(GITypeInfo * type)
{
	GITypeTag type_tag = g_type_info_get_tag(type);

	const gchar *type_name;

	if (type_tag == GI_TYPE_TAG_INTERFACE)
	{
		GIBaseInfo *interface = g_type_info_get_interface(type);
		type_name = g_base_info_get_name(interface);
		g_base_info_unref(interface);
	}
	else
	{
		type_name = g_type_tag_to_string(type_tag);
	}

	return type_name;
}

static JSValueRef
seed_introspect(JSContextRef ctx,
				JSObjectRef function,
				JSObjectRef this_object,
				size_t argumentCount,
				const JSValueRef arguments[], JSValueRef * exception)
{
	// TODO: LEAKY!

	GICallableInfo *info;
	JSObjectRef data_obj, args_obj;
	gint i;

	if (argumentCount != 1)
	{
		gchar *mes =
			g_strdup_printf("Seed.introspect expected 1 argument, "
							"got %d", (unsigned int)argumentCount);
		seed_make_exception(ctx, exception, "ArgumentError", mes);
		g_free(mes);
		return JSValueMakeNull(ctx);
	}

	if (!JSValueIsObject(ctx, arguments[0]))
		return JSValueMakeNull(ctx);
	if (!JSValueIsObjectOfClass(ctx, arguments[0], gobject_method_class))
		return JSValueMakeNull(ctx);

	info = (GICallableInfo *) JSObjectGetPrivate((JSObjectRef) arguments[0]);
	data_obj = JSObjectMake(ctx, NULL, NULL);

	seed_object_set_property(ctx, data_obj, "name", (JSValueRef)
							 seed_value_from_string(ctx, g_base_info_get_name
													((GIBaseInfo *) info),
													exception));

	seed_object_set_property(ctx, data_obj, "return_type",
							 seed_value_from_string
							 (ctx, seed_g_type_name_to_string
							  (g_callable_info_get_return_type(info)),
							  exception));

	args_obj = JSObjectMake(ctx, NULL, NULL);

	seed_object_set_property(ctx, data_obj, "args", args_obj);

	for (i = 0; i < g_callable_info_get_n_args(info); ++i)
	{
		JSObjectRef argument = JSObjectMake(ctx, NULL, NULL);

		const gchar *arg_name =
			seed_g_type_name_to_string(g_arg_info_get_type
									   (g_callable_info_get_arg(info, i)));

		seed_object_set_property(ctx, argument, "type",
								 seed_value_from_string(ctx,
														arg_name, exception));

		JSObjectSetPropertyAtIndex(ctx, args_obj, i, argument, NULL);
	}

	return data_obj;
}

static JSValueRef
seed_check_syntax(JSContextRef ctx,
				  JSObjectRef function,
				  JSObjectRef this_object,
				  size_t argumentCount,
				  const JSValueRef arguments[], JSValueRef * exception)
{
	if (argumentCount == 1)
	{
		JSStringRef jsstr = JSValueToStringCopy(ctx,
												arguments[0],
												exception);
		JSCheckScriptSyntax(ctx, jsstr, 0, 0, exception);
		if (jsstr)
			JSStringRelease(jsstr);
	}
	else
	{
		gchar *mes = g_strdup_printf("Seed.check_syntax expected "
									 "1 argument, got %d",
									 (unsigned int)argumentCount);
		seed_make_exception(ctx, exception, "ArgumentError", mes);
		g_free(mes);
	}
	return JSValueMakeNull(ctx);
}

static JSValueRef
seed_fork(JSContextRef ctx,
		  JSObjectRef function,
		  JSObjectRef this_object,
		  size_t argumentCount,
		  const JSValueRef arguments[], JSValueRef * exception)
{
	pid_t child;

	child = fork();
	return seed_value_from_int(ctx, child, exception);
}

static JSValueRef
seed_spawn(JSContextRef ctx,
		   JSObjectRef function,
		   JSObjectRef this_object,
		   size_t argumentCount,
		   const JSValueRef arguments[], JSValueRef * exception)
{
	gchar *line, *stdout, *stderr;
	JSObjectRef ret;
	GError *error = NULL;

	if (argumentCount != 1)
	{
		// I am so lazy
		seed_make_exception(ctx, exception, "ArgumentError",
							"Seed.spawn expected 1 argument");
		return JSValueMakeNull(ctx);
	}

	line = seed_value_to_string(ctx, arguments[0], exception);
	g_spawn_command_line_sync(line, &stdout, &stderr, NULL, &error);
	if (error)
	{
		seed_make_exception_from_gerror(ctx, exception, error);

		g_free(line);
		g_error_free(error);
		return JSValueMakeNull(ctx);
	}

	ret = JSObjectMake(ctx, NULL, NULL);
	seed_object_set_property(ctx, ret, "stdout",
							 seed_value_from_string(ctx, stdout, exception));
	seed_object_set_property(ctx, ret, "stderr",
							 seed_value_from_string(ctx, stderr, exception));

	g_free(stdout);
	g_free(stderr);

	return ret;
}

static JSValueRef
seed_quit(JSContextRef ctx,
		  JSObjectRef function,
		  JSObjectRef this_object,
		  size_t argumentCount,
		  const JSValueRef arguments[], JSValueRef * exception)
{
	if (argumentCount == 1)
	{
		exit(seed_value_to_int(ctx, arguments[0], NULL));
	}
	else if (argumentCount > 1)
	{
		gchar *mes = g_strdup_printf("Seed.quit expected " "1 argument, got %d",
									 (unsigned int)argumentCount);
		seed_make_exception(ctx, exception, "ArgumentError", mes);
		g_free(mes);
	}

	exit(EXIT_SUCCESS);
}

void seed_init_builtins(SeedEngine * local_eng, gint * argc, gchar *** argv)
{
	guint i;
	JSObjectRef arrayObj;
	JSValueRef argcref;
	JSObjectRef obj = (JSObjectRef) seed_object_get_property(local_eng->context,
															 local_eng->global,
															 "Seed");

	seed_create_function(local_eng->context, "include", &seed_include, obj);
	seed_create_function(local_eng->context, "print", &seed_print, obj);
	seed_create_function(local_eng->context,
						 "check_syntax", &seed_check_syntax, obj);
	seed_create_function(local_eng->context,
						 "introspect", &seed_introspect, obj);
	seed_create_function(local_eng->context, "fork", &seed_fork, obj);
	seed_create_function(local_eng->context, "spawn", &seed_spawn, obj);
	seed_create_function(local_eng->context, "quit", &seed_quit, obj);

	arrayObj = JSObjectMake(local_eng->context, NULL, NULL);

	if (argc)
	{
		for (i = 0; i < *argc; ++i)
		{
			// TODO: exceptions!

			JSObjectSetPropertyAtIndex(local_eng->context, arrayObj, i,
									   seed_value_from_string
									   (local_eng->context, (*argv)[i], 0),
									   NULL);
		}

		argcref = seed_value_from_int(local_eng->context, *argc, 0);
	}
	else
	{
		argcref = seed_value_from_int(local_eng->context, 0, 0);
	}

	seed_object_set_property(local_eng->context, arrayObj, "length", argcref);
	seed_object_set_property(local_eng->context, obj, "argv", arrayObj);
}
